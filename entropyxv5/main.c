
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "fsl_common.h"

#include "gy87.h"
#include "entropy_math.h"
#include "crypto_hw.h"
#include "ssd1306.h"


#define KEY_DISPLAY_HOLD_MS 3000U

static uint8_t g_raw_entropy_pool[ENTROPY_RAW_POOL_BITS];
static uint16_t g_pool_bit_index = 0;
static uint8_t g_previous_key[32] = {0};
static bool g_first_key_generated = false;
static float g_last_avalanche_pct = 50.0f;
static uint32_t g_key_counter = 0;

static uint8_t calculate_hamming_distance(const uint8_t *a, const uint8_t *b, uint8_t len) {
    uint8_t diff = 0;
    for (uint8_t i = 0; i < len; i++) {
        uint8_t xor_val = a[i] ^ b[i];
        while (xor_val) {
            diff += (xor_val & 1);
            xor_val >>= 1;
        }
    }
    return diff;
}

int main(void) {
    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("\r\n=======================================================\r\n");
    PRINTF("   EntropyX: PHYSICAL MEMS SENSOR TRNG SUBSYSTEM       \r\n");
    PRINTF("   NXP FRDM-MCXN236 (150MHz Cortex-M33 + EdgeLock Crypto)\r\n");
    PRINTF("=======================================================\r\n\r\n");

    PRINTF("[STEP 1] Initializing Crypto Hardware Accelerator...\r\n");
    crypto_hw_init();

    PRINTF("[STEP 2] Initializing Sensor Bus (LPI2C2)...\r\n");
    gy87_init();

    PRINTF("[STEP 3] Initializing 1.3-inch OLED Display (LPI2C5)...\r\n");
    ssd1306_init();

    // Initial Splash Screen
    ssd1306_clear();
    ssd1306_draw_string(40, 1, "ENTROPYX");
    ssd1306_draw_string(10, 4, "HARVESTING NOISE");
    ssd1306_draw_string(14, 6, "PLEASE WAIT...");
    ssd1306_update();

    PRINTF("[+] Display ready. Entering entropy harvesting loop...\r\n\r\n");

    uint32_t sample_cycle = 0;

    while (1) {
        sensor_data_t sample;

        if (gy87_read_all(&sample)) {
            sample_cycle++;

            if ((sample_cycle % 100) == 0) {
                PRINTF("[LIVE] Accel(%6d, %6d, %6d) | Gyro(%6d, %6d, %6d) | Mag(%6d, %6d, %6d)\r\n",
                       sample.accel_x, sample.accel_y, sample.accel_z,
                       sample.gyro_x,  sample.gyro_y,  sample.gyro_z,
                       sample.mag_x,   sample.mag_y,   sample.mag_z);
            }

            int16_t sample_array[11] = {
                sample.accel_x, sample.accel_y, sample.accel_z,
                sample.gyro_x,  sample.gyro_y,  sample.gyro_z,
                sample.mag_x,   sample.mag_y,   sample.mag_z,
                sample.onboard_accel_z,
                sample.aux_entropy
            };

            entropy_accumulate(sample_array, 11, g_raw_entropy_pool, &g_pool_bit_index);


            if (g_pool_bit_index >= ENTROPY_RAW_POOL_BITS) {
                PRINTF("\r\n--- [ENTROPY POOL FILLED (1024 BITS)] ---\r\n");

                nist_stats_t nist = entropy_run_nist_eval(g_raw_entropy_pool, ENTROPY_RAW_POOL_BITS);

                int ent_int = (int)(nist.min_entropy_per_bit * 1000.0f);
                PRINTF("Min-Entropy (H_min) : %d.%03d bits/bit\r\n", ent_int / 1000, ent_int % 1000);
                PRINTF("NIST Monobit Test   : %s\r\n", nist.monobit_pass ? "PASS" : "FAIL");
                PRINTF("NIST Runs Test      : %s\r\n", nist.runs_pass    ? "PASS" : "FAIL");
                PRINTF("NIST Serial Test    : %s\r\n", nist.serial_pass  ? "PASS" : "FAIL");

                if (nist.passes_all_criteria) {
                    g_key_counter++;
                    PRINTF("[+] Pool verified. Generating Key #%lu via Toeplitz + SHA-256...\r\n", (unsigned long)g_key_counter);

                    uint8_t extracted_bytes[EXTRACTED_KEY_BYTES];
                    entropy_toeplitz_extract(g_raw_entropy_pool, ENTROPY_RAW_POOL_BITS, extracted_bytes, EXTRACTED_KEY_BYTES);

                    uint8_t final_crypto_key[32];
                    if (crypto_hw_sha256(extracted_bytes, EXTRACTED_KEY_BYTES, final_crypto_key)) {
                        PRINTF("\r\n===============================================================\r\n");
                        PRINTF("  KEY #%03lu (256-BIT SHA-256 CONDITIONED):\r\n  0x", (unsigned long)g_key_counter);
                        for (uint8_t k = 0; k < 32; k++) {
                            PRINTF("%02X", final_crypto_key[k]);
                        }
                        PRINTF("\r\n===============================================================\r\n");

                        if (g_first_key_generated) {
                            uint8_t bit_diff = calculate_hamming_distance(g_previous_key, final_crypto_key, 32);
                            g_last_avalanche_pct = ((float)bit_diff / 256.0f) * 100.0f;
                            int diff_int = (int)(g_last_avalanche_pct * 100.0f);
                            PRINTF("[AVALANCHE CHECK] Hamming Distance: %d/256 bits changed (%d.%02d%%)\r\n",
                                   bit_diff, diff_int / 100, diff_int % 100);
                        }


                        ssd1306_display_dashboard(g_key_counter, nist.min_entropy_per_bit, true, final_crypto_key, g_last_avalanche_pct);

                        memcpy(g_previous_key, final_crypto_key, 32);
                        g_first_key_generated = true;


                        PRINTF("[OLED] Displaying Key #%lu for %u ms...\r\n", (unsigned long)g_key_counter, KEY_DISPLAY_HOLD_MS);
                        SDK_DelayAtLeastUs(KEY_DISPLAY_HOLD_MS * 1000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));
                    }
                } else {
                    PRINTF("[!] Statistical test threshold not met. Whitening next block...\r\n");
                }


                g_pool_bit_index = 0;
                PRINTF("---------------------------------------------------------------\r\n\r\n");
            }
        }

        SDK_DelayAtLeastUs(5000, CLOCK_GetFreq(kCLOCK_CoreSysClk));
    }
}
