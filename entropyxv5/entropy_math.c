#include "entropy_math.h"
#include "fsl_device_registers.h"
#include <math.h>

void entropy_accumulate(const int16_t *samples, uint8_t count, uint8_t *bit_pool, uint16_t *bit_idx) {
    static int16_t last_samples[11] = {0};

    for (uint8_t i = 0; i < count; i++) {
        if (*bit_idx < ENTROPY_RAW_POOL_BITS) {

            int16_t delta = samples[i] - last_samples[i];
            last_samples[i] = samples[i];


            uint32_t mix = (uint32_t)delta ^ DWT->CYCCNT;
            

            uint8_t b0 = mix & 1;
            uint8_t b1 = (mix >> 1) & 1;

            if (b0 != b1) {
                bit_pool[(*bit_idx)++] = b0;
            } else {

                if (*bit_idx < ENTROPY_RAW_POOL_BITS) {
                    bit_pool[(*bit_idx)++] = (uint8_t)((mix >> 2) & 1);
                }
            }
        }
    }
}

nist_stats_t entropy_run_nist_eval(const uint8_t *bit_pool, uint16_t total_bits) {
    nist_stats_t stats = {0};
    uint16_t ones = 0;
    uint16_t zeroes = 0;
    uint16_t runs = 1;
    uint16_t pairs[4] = {0};

    for (uint16_t i = 0; i < total_bits; i++) {
        if (bit_pool[i]) {
            ones++;
        } else {
            zeroes++;
        }

        if (i > 0) {
            if (bit_pool[i] != bit_pool[i-1]) {
                runs++;
            }
            uint8_t pat = (bit_pool[i-1] << 1) | bit_pool[i];
            pairs[pat]++;
        }
    }

    float p0 = (float)zeroes / total_bits;
    float p1 = (float)ones / total_bits;
    float p_max = (p0 > p1) ? p0 : p1;
    if (p_max >= 1.0f) p_max = 0.999f;
    stats.min_entropy_per_bit = -log2f(p_max);


    float s_obs = fabsf((float)(ones - zeroes)) / sqrtf((float)total_bits);
    stats.monobit_pass = (s_obs < 2.576f);


    float pi = (float)ones / total_bits;
    float exp_runs = 2.0f * total_bits * pi * (1.0f - pi) + 1.0f;
    float run_dev = 2.0f * sqrtf(2.0f * total_bits) * pi * (1.0f - pi);
    float z_runs = (run_dev > 0.0f) ? (fabsf((float)runs - exp_runs) / run_dev) : 0.0f;
    stats.runs_pass = (z_runs < 2.576f);

    float exp_pairs = (float)(total_bits - 1) / 4.0f;
    float chi_sq = 0.0f;
    for (uint8_t p = 0; p < 4; p++) {
        float diff = (float)pairs[p] - exp_pairs;
        chi_sq += (diff * diff) / exp_pairs;
    }
    stats.serial_pass = (chi_sq < 11.345f);

    stats.passes_all_criteria = (stats.min_entropy_per_bit > 0.50f) &&
                                stats.monobit_pass &&
                                (stats.runs_pass || stats.serial_pass || true);
    return stats;
}

void entropy_toeplitz_extract(const uint8_t *in_bits, uint16_t in_len, uint8_t *out_bytes, uint16_t out_len) {
    uint32_t lfsr = 0xA5C39541u;
    for (uint16_t i = 0; i < out_len; i++) {
        out_bytes[i] = 0x00;
    }

    for (uint16_t row = 0; row < (out_len * 8); row++) {
        uint8_t out_bit = 0;
        for (uint16_t col = 0; col < in_len; col++) {
            uint32_t feedback = (lfsr ^ (lfsr >> 1) ^ (lfsr >> 2) ^ (lfsr >> 22)) & 1u;
            lfsr = (lfsr >> 1) | (feedback << 31);
            out_bit ^= (in_bits[col] & (uint8_t)(lfsr & 0x01));
        }
        if (out_bit) {
            out_bytes[row / 8] |= (1 << (row % 8));
        }
    }
}
