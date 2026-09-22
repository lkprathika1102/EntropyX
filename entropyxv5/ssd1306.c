#include "ssd1306.h"
#include "fsl_lpi2c.h"
#include "fsl_port.h"
#include "fsl_clock.h"
#include <string.h>
#include <stdio.h>

#define OLED_I2C_BASE LPI2C5
#define OLED_WIDTH    128
#define OLED_HEIGHT   64

static uint8_t s_oled_buffer[1024];

static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x00, 0x08, 0x14, 0x22, 0x41}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x41, 0x22, 0x14, 0x08, 0x00}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // F
    {0x3E, 0x41, 0x41, 0x51, 0x32}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}  // Z
};

static void oled_write_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    lpi2c_master_transfer_t transfer;
    memset(&transfer, 0, sizeof(transfer));
    transfer.slaveAddress   = OLED_I2C_ADDR;
    transfer.direction      = kLPI2C_Write;
    transfer.data           = buf;
    transfer.dataSize       = 2;
    transfer.flags          = kLPI2C_TransferDefaultFlag;
    LPI2C_MasterTransferBlocking(OLED_I2C_BASE, &transfer);
}

void ssd1306_init(void) {
    CLOCK_EnableClock(kCLOCK_Port1);

    port_pin_config_t cfg = {
        .pullSelect = kPORT_PullUp,
        .slewRate = kPORT_FastSlewRate,
        .passiveFilterEnable = kPORT_PassiveFilterDisable,
        .openDrainEnable = kPORT_OpenDrainEnable,
        .driveStrength = kPORT_LowDriveStrength,
        .mux = kPORT_MuxAlt2,
        .inputBuffer = true,
        .lockRegister = kPORT_UnlockRegister
    };
    PORT_SetPinConfig(PORT1, 16U, &cfg);
    PORT_SetPinConfig(PORT1, 17U, &cfg);

    CLOCK_SetClkDiv(kCLOCK_DivFlexcom5Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM5);

    lpi2c_master_config_t config;
    LPI2C_MasterGetDefaultConfig(&config);
    config.baudRate_Hz = 100000;
    LPI2C_MasterInit(OLED_I2C_BASE, &config, CLOCK_GetFreq(kCLOCK_Fro12M));

    SDK_DelayAtLeastUs(50000, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    oled_write_cmd(0xAE); // Display OFF
    oled_write_cmd(0xD5); oled_write_cmd(0x80);
    oled_write_cmd(0xA8); oled_write_cmd(0x3F);
    oled_write_cmd(0xD3); oled_write_cmd(0x00);
    oled_write_cmd(0x40);
    oled_write_cmd(0xAD); oled_write_cmd(0x8B); // SH1106 DC-DC on
    oled_write_cmd(0x8D); oled_write_cmd(0x14); // SSD1306 Charge Pump on
    oled_write_cmd(0xA1); // Segment remap
    oled_write_cmd(0xC8); // COM scan direction
    oled_write_cmd(0xDA); oled_write_cmd(0x12);
    oled_write_cmd(0x81); oled_write_cmd(0xCF);
    oled_write_cmd(0xD9); oled_write_cmd(0xF1);
    oled_write_cmd(0xDB); oled_write_cmd(0x40);
    oled_write_cmd(0xA4);
    oled_write_cmd(0xA6);
    oled_write_cmd(0xAF); // Display ON

    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    ssd1306_clear();
    ssd1306_update();
}

void ssd1306_clear(void) {
    memset(s_oled_buffer, 0, sizeof(s_oled_buffer));
}

void ssd1306_update(void) {
    for (uint8_t page = 0; page < 8; page++) {
        oled_write_cmd(0xB0 + page);
        oled_write_cmd(0x02);
        oled_write_cmd(0x10);

        for (uint8_t col = 0; col < OLED_WIDTH; col += 16) {
            uint8_t tx_chunk[17];
            tx_chunk[0] = 0x40;
            memcpy(&tx_chunk[1], &s_oled_buffer[(page * OLED_WIDTH) + col], 16);

            lpi2c_master_transfer_t transfer;
            memset(&transfer, 0, sizeof(transfer));
            transfer.slaveAddress   = OLED_I2C_ADDR;
            transfer.direction      = kLPI2C_Write;
            transfer.data           = tx_chunk;
            transfer.dataSize       = 17;
            transfer.flags          = kLPI2C_TransferDefaultFlag;
            LPI2C_MasterTransferBlocking(OLED_I2C_BASE, &transfer);
        }
    }
}

void ssd1306_draw_string(uint8_t x, uint8_t page, const char *str) {
    if (page >= 8) return;
    while (*str) {
        char ch = *str++;

        if (ch >= 'a' && ch <= 'z') ch -= 32;
        if (ch < 32 || ch > 90) ch = ' ';

        uint8_t char_idx = ch - 32;
        if (x > (OLED_WIDTH - 6)) break;

        for (uint8_t col = 0; col < 5; col++) {
            s_oled_buffer[(page * OLED_WIDTH) + x + col] = font5x7[char_idx][col];
        }
        s_oled_buffer[(page * OLED_WIDTH) + x + 5] = 0x00;
        x += 6;
    }
}

void ssd1306_display_dashboard(uint32_t key_id, float h_min, bool pass, const uint8_t *key, float avalanche) {
    char line[24];
    ssd1306_clear();


    ssd1306_draw_string(40, 0, "ENTROPYX");


    for (uint8_t i = 0; i < OLED_WIDTH; i++) {
        s_oled_buffer[OLED_WIDTH + i] = 0x01;
    }

    snprintf(line, sizeof(line), "KEY #%03lu  NIST:%s", (unsigned long)key_id, pass ? "PASS" : "FAIL");
    ssd1306_draw_string(0, 2, line);


    int ent_i = (int)(h_min * 100.0f);
    int av_i  = (int)(avalanche * 10.0f);
    snprintf(line, sizeof(line), "H:%d.%02d  AVL:%d.%1d%%", ent_i / 100, ent_i % 100, av_i / 10, av_i % 10);
    ssd1306_draw_string(0, 3, line);


    ssd1306_draw_string(0, 5, "256-BIT SHA256 KEY:");


    snprintf(line, sizeof(line), "%02X%02X%02X%02X%02X%02X%02X%02X",
             key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7]);
    ssd1306_draw_string(0, 6, line);

    snprintf(line, sizeof(line), "%02X%02X%02X%02X...%02X%02X",
             key[8], key[9], key[10], key[11], key[30], key[31]);
    ssd1306_draw_string(0, 7, line);

    ssd1306_update();
}
