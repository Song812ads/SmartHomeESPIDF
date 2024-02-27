#ifndef __GPIO_H
#define __GPIO_H
#include <driver/gpio.h>

void Set_Pin_Output(gpio_num_t pin);

void Set_Pin_Input(gpio_num_t pin);
#endif