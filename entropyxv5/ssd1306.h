#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>

#define OLED_I2C_ADDR 0x3C

void ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_update(void);
void ssd1306_draw_string(uint8_t x, uint8_t page, const char *str);
void ssd1306_display_dashboard(uint32_t key_id, float h_min, bool pass, const uint8_t *key, float avalanche);

#endif
