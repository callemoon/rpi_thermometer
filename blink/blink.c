/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "ssd1306.h"

#include <string.h>


#include "stdio.h"


void core1_entry() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN; 

    while (1)
    {
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);    
    }
}

int main() {
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
    uint32_t scrollPos;
    char buffer[9];
    char string[] = "        RPI PICO             ";

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(16, GPIO_FUNC_I2C);
    gpio_set_function(17, GPIO_FUNC_I2C);
    gpio_pull_up(16);
    gpio_pull_up(17);
    // Make the I2C pins available to picotool
    //bi_decl(bi_2pins_with_func(21, 22, GPIO_FUNC_I2C));

    multicore_launch_core1(core1_entry);

    ssd1306_Init();

    scrollPos = 0;
  

    while (true) {  
        //memcpy(buffer, &string[scrollPos], 8);
        //buffer[8] = 0;

        //scrollPos++;
        //if(scrollPos >= 16)
        //{
        //    scrollPos = 0;
        //}

        uint8_t buf[] = {0xF3};    // read temp
        uint8_t data[2];

        i2c_write_blocking(i2c0, 0x40, buf, 1, false);
        sleep_ms(500);
        i2c_read_blocking(i2c0, 0x40, data, 2, false);

        float temp  = ((data[0] * 256.0) + data[1]);
        float celsTemp = ((175.72 * temp) / 65536.0) - 46.85;

        sprintf(buffer, "%2.2f", celsTemp);

        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString(buffer, Font_11x18, White);

        uint8_t buf2[] = {0xF5};    // read hum
        i2c_write_blocking(i2c0, 0x40, buf2, 1, false);
        sleep_ms(500);
        i2c_read_blocking(i2c0, 0x40, data, 2, false);

        float humidity  = ((data[0] * 256.0) + data[1]);
        humidity = ((125 * humidity) / 65536.0) - 6;

        sprintf(buffer, "%2.2f", humidity);

        ssd1306_SetCursor(0, 20);
        ssd1306_WriteString(buffer, Font_7x10, White);

        ssd1306_UpdateScreen();
    }
#endif
}
