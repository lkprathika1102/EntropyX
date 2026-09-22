#include "gy87.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_clock.h"
#include "fsl_debug_console.h"

#define SDA_PORT PORT4
#define SDA_GPIO GPIO4
#define SDA_PIN  0U

#define SCL_PORT PORT4
#define SCL_GPIO GPIO4
#define SCL_PIN  1U

static uint8_t g_mpu_address = MPU6050_ADDR;
static bool g_qmc_online = false;

static inline void i2c_delay(void) {
    for (volatile uint32_t i = 0; i < 45; i++) {
        __NOP();
    }
}

static inline void sda_high(void) {
    gpio_pin_config_t cfg = {kGPIO_DigitalInput, 0};
    GPIO_PinInit(SDA_GPIO, SDA_PIN, &cfg);
}

static inline void sda_low(void) {
    gpio_pin_config_t cfg = {kGPIO_DigitalOutput, 0};
    GPIO_PinInit(SDA_GPIO, SDA_PIN, &cfg);
    GPIO_PinWrite(SDA_GPIO, SDA_PIN, 0U);
}

static inline void scl_high(void) {
    gpio_pin_config_t cfg = {kGPIO_DigitalInput, 0};
    GPIO_PinInit(SCL_GPIO, SCL_PIN, &cfg);
}

static inline void scl_low(void) {
    gpio_pin_config_t cfg = {kGPIO_DigitalOutput, 0};
    GPIO_PinInit(SCL_GPIO, SCL_PIN, &cfg);
    GPIO_PinWrite(SCL_GPIO, SCL_PIN, 0U);
}

static inline uint8_t sda_read(void) {
    return (uint8_t)GPIO_PinRead(SDA_GPIO, SDA_PIN);
}

static void bb_i2c_start(void) {
    sda_high();
    scl_high();
    i2c_delay();
    sda_low();
    i2c_delay();
    scl_low();
    i2c_delay();
}

static void bb_i2c_stop(void) {
    sda_low();
    i2c_delay();
    scl_high();
    i2c_delay();
    sda_high();
    i2c_delay();
}

static bool bb_i2c_write_byte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        if (byte & 0x80) {
            sda_high();
        } else {
            sda_low();
        }
        byte <<= 1;
        i2c_delay();
        scl_high();
        i2c_delay();
        scl_low();
        i2c_delay();
    }

    sda_high();
    i2c_delay();
    scl_high();
    i2c_delay();
    uint8_t nack = sda_read();
    scl_low();
    i2c_delay();

    return (nack == 0);
}

static uint8_t bb_i2c_read_byte(bool send_ack) {
    uint8_t byte = 0;
    sda_high();

    for (uint8_t i = 0; i < 8; i++) {
        byte <<= 1;
        scl_high();
        i2c_delay();
        if (sda_read()) {
            byte |= 1;
        }
        scl_low();
        i2c_delay();
    }

    if (send_ack) {
        sda_low();
    } else {
        sda_high();
    }
    i2c_delay();
    scl_high();
    i2c_delay();
    scl_low();
    sda_high();
    i2c_delay();

    return byte;
}

static bool bb_i2c_write_reg(uint8_t dev, uint8_t reg, uint8_t val) {
    bb_i2c_start();
    if (!bb_i2c_write_byte(dev << 1)) {
        bb_i2c_stop();
        return false;
    }
    if (!bb_i2c_write_byte(reg)) {
        bb_i2c_stop();
        return false;
    }
    if (!bb_i2c_write_byte(val)) {
        bb_i2c_stop();
        return false;
    }
    bb_i2c_stop();
    return true;
}

static bool bb_i2c_read_buffer(uint8_t dev, uint8_t reg, uint8_t *buf, uint8_t len) {
    bb_i2c_start();
    if (!bb_i2c_write_byte(dev << 1)) {
        bb_i2c_stop();
        return false;
    }
    if (!bb_i2c_write_byte(reg)) {
        bb_i2c_stop();
        return false;
    }

    bb_i2c_start(); 
    if (!bb_i2c_write_byte((dev << 1) | 1)) {
        bb_i2c_stop();
        return false;
    }

    for (uint8_t i = 0; i < len; i++) {
        buf[i] = bb_i2c_read_byte(i < (len - 1));
    }
    bb_i2c_stop();
    return true;
}

static void scan_bus(void) {
    PRINTF("[*] Scanning I2C bus (0x01 - 0x7F)...\r\n");
    uint8_t found = 0;
    for (uint8_t addr = 1; addr < 128; addr++) {
        bb_i2c_start();
        bool ack = bb_i2c_write_byte(addr << 1);
        bb_i2c_stop();

        if (ack) {
            PRINTF("   -> [FOUND] 0x%02X ", addr);
            if (addr == 0x68) {
                PRINTF("(MPU6050 Primary)\r\n");
                g_mpu_address = 0x68;
            } else if (addr == 0x69) {
                PRINTF("(MPU6050 Alternate)\r\n");
                g_mpu_address = 0x69;
            } else if (addr == 0x2C) {
                PRINTF("(QMC6308 Magnetometer)\r\n");
                g_qmc_online = true;
            } else if (addr == 0x1E) {
                PRINTF("(HMC5883L Magnetometer)\r\n");
            } else if (addr == 0x77) {
                PRINTF("(BMP180 Barometer)\r\n");
            } else if (addr == 0x30 || addr == 0x32) {
                PRINTF("(Onboard FXLS8974C Accelerometer)\r\n");
            } else {
                PRINTF("(Unknown Device)\r\n");
            }
            found++;
        }
    }
    PRINTF("[*] Scan complete: %d device(s) responded.\r\n\r\n", found);
}

bool gy87_init(void) {
    CLOCK_EnableClock(kCLOCK_Port4);
    CLOCK_EnableClock(kCLOCK_Gpio4);

    port_pin_config_t cfg = {
        .pullSelect = kPORT_PullUp,
        .slewRate = kPORT_FastSlewRate,
        .passiveFilterEnable = kPORT_PassiveFilterDisable,
        .openDrainEnable = kPORT_OpenDrainEnable,
        .driveStrength = kPORT_LowDriveStrength,
        .mux = kPORT_MuxAlt0,
        .inputBuffer = true, 
        .lockRegister = kPORT_UnlockRegister
    };
    PORT_SetPinConfig(SDA_PORT, SDA_PIN, &cfg);
    PORT_SetPinConfig(SCL_PORT, SCL_PIN, &cfg);

    sda_high();
    scl_high();
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    scan_bus();

    PRINTF("[*] Initializing MPU at 0x%02X...\r\n", g_mpu_address);
    bb_i2c_write_reg(g_mpu_address, 0x6B, 0x80);
    SDK_DelayAtLeastUs(50000, CLOCK_GetFreq(kCLOCK_CoreSysClk));
    bb_i2c_write_reg(g_mpu_address, 0x6B, 0x00);
    SDK_DelayAtLeastUs(15000, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    bb_i2c_write_reg(g_mpu_address, 0x6A, 0x00);
    SDK_DelayAtLeastUs(5000, CLOCK_GetFreq(kCLOCK_CoreSysClk));
    bb_i2c_write_reg(g_mpu_address, 0x37, 0x02);
    SDK_DelayAtLeastUs(15000, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    scan_bus();

    if (g_qmc_online) {
        PRINTF("[*] Initializing QMC6308 Magnetometer...\r\n");
        bb_i2c_write_reg(QMC6308_ADDR, 0x0A, 0x80);
        SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CoreSysClk));
        bb_i2c_write_reg(QMC6308_ADDR, 0x0B, 0x01);
        bb_i2c_write_reg(QMC6308_ADDR, 0x09, 0x1D);
    }

    return true;
}

bool gy87_read_all(sensor_data_t *data) {
    uint8_t mpu_buf[14] = {0};
    uint8_t qmc_buf[6]  = {0};
    bool ok = false;

    if (bb_i2c_read_buffer(g_mpu_address, 0x3B, mpu_buf, 14)) {
        data->accel_x = (int16_t)((mpu_buf[0] << 8) | mpu_buf[1]);
        data->accel_y = (int16_t)((mpu_buf[2] << 8) | mpu_buf[3]);
        data->accel_z = (int16_t)((mpu_buf[4] << 8) | mpu_buf[5]);
        data->gyro_x  = (int16_t)((mpu_buf[8] << 8) | mpu_buf[9]);
        data->gyro_y  = (int16_t)((mpu_buf[10] << 8) | mpu_buf[11]);
        data->gyro_z  = (int16_t)((mpu_buf[12] << 8) | mpu_buf[13]);
        ok = true;
    }

    if (g_qmc_online && bb_i2c_read_buffer(QMC6308_ADDR, 0x00, qmc_buf, 6)) {
        data->mag_x = (int16_t)((qmc_buf[1] << 8) | qmc_buf[0]);
        data->mag_y = (int16_t)((qmc_buf[3] << 8) | qmc_buf[2]);
        data->mag_z = (int16_t)((qmc_buf[5] << 8) | qmc_buf[4]);
    } else {
        data->mag_x = data->gyro_x ^ data->accel_z;
        data->mag_y = data->gyro_y ^ data->accel_x;
        data->mag_z = data->gyro_z ^ data->accel_y;
    }

    data->onboard_accel_z = data->accel_z ^ (int16_t)DWT->CYCCNT;
    data->aux_entropy     = (int16_t)(DWT->CYCCNT & 0xFFFF);

    return ok;
}
