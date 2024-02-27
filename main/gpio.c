#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "gpio.h"
#define TEMP_PIN 18

typedef struct {
    uint64_t event_count;
} example_queue_element_t;
example_queue_element_t ele;

gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };

void delay(uint64_t delay)
{   uint64_t count=0;
    gptimer_set_raw_count(gptimer,0);
    while (count<delay)
    gptimer_get_raw_count(gptimer, &count);
	// printf("timer = %lld\n",count);
}
void Set_Pin_Output(gpio_num_t pin)
{
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

}

void Set_Pin_Input(gpio_num_t pin)
{
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; 
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

}
uint8_t DS18B20_Start (void) // send the start signal to the sensor
{
	uint8_t Response = 0;
	Set_Pin_Output(TEMP_PIN);   // set the pin as output
	gpio_set_level(TEMP_PIN, 0);  // pull the pin low

	delay (480);   // delay according to datasheet
	Set_Pin_Input(TEMP_PIN);    // release the pin by setting the pin as input
	
    delay (80);    // delay according to datasheet

    if (!(gpio_get_level(TEMP_PIN))) Response = 1;    // if the pin is low i.e the presence pulse is detected
	else Response = -1;
	delay (400); // 480 us delay totally.
	return Response;
} // complete the initialization
void DS18B20_Write (uint8_t data) // write data to the sensor
{
	Set_Pin_Output(TEMP_PIN);  // set as output
	for (int i=0; i<8; i++)
	{
		if ((data & (1<<i))!=0)  // if the bit is high
		{	// write 1
			Set_Pin_Output(TEMP_PIN);  // set as output
			gpio_set_level (TEMP_PIN, 0);  // pull the pin LOW
			delay (1);  // wait for 1 us

			Set_Pin_Input(TEMP_PIN);  // set as input
			delay (60);  // wait for 60 us
		}
		else  // if the bit is low
		{	// write 0
			Set_Pin_Output(TEMP_PIN);
			gpio_set_level (TEMP_PIN, 0);  // pull the pin LOW
			delay (60);  // wait for 60 us
			Set_Pin_Input(TEMP_PIN);
		}
	}
}
uint8_t DS18B20_Read (void)
{
	uint8_t value=0;
	Set_Pin_Input(TEMP_PIN);
	for (int i=0;i<8;i++)
	{
		Set_Pin_Output(TEMP_PIN);   // set as output
		gpio_set_level (TEMP_PIN, 0);  // pull the data pin LOW
		delay (2);  // wait for 2 us
		Set_Pin_Input(TEMP_PIN);  // set as input
		if (gpio_get_level (TEMP_PIN))  // if the pin is HIGH
		{
			value |= 1<<i;  // read = 1
		}
		delay (60);  // wait for 60 us
	}
	return value;
}

    

    



