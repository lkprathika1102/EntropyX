#ifndef GY87_H
#define GY87_H

#include <stdint.h>
#include <stdbool.h>

#define MPU6050_ADDR        0x68
#define MPU6050_ALT_ADDR    0x69
#define QMC6308_ADDR        0x2C
#define HMC5883L_ADDR       0x1E
#define BMP180_ADDR         0x77
#define ONBOARD_ACCL_ADDR   0x30

typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    int16_t mag_x;
    int16_t mag_y;
    int16_t mag_z;
    int16_t onboard_accel_z;
    int16_t aux_entropy;
} sensor_data_t;

bool gy87_init(void);
bool gy87_read_all(sensor_data_t *data);

#endif
