#ifndef __I2C_LCD_H_
#define __I2C_LCD_H_
#include "i2c_lcd.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "unistd.h"
void lcd_init (void);
void lcd_send_cmd (char cmd);
void lcd_send_data (char data);
void lcd_send_string (char *str);
void lcd_put_cur(int row, int col);
void lcd_clear (void);
#endif