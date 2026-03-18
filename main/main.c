/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const int BTN_R = 9;
const int BTN_B = 10;
const int BTN_G = 11;
const int BTN_Y = 15;

const int LED_R = 16;
const int LED_Y = 17;
const int LED_G = 18;
const int LED_B = 19;

volatile int flag_r = 0;
volatile int flag_y = 0;
volatile int flag_g = 0;
volatile int flag_b = 0;

void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) { // Fall edge
        if (gpio == BTN_R) {
            flag_r = 1;
        } else if (gpio == BTN_Y) {
            flag_y = 1;
        } else if (gpio == BTN_G) {
            flag_g = 1;
        } else if (gpio == BTN_B) {
            flag_b = 1;
        }
    }
}

void inicializa_hardware() {
    gpio_init(BTN_R);
    gpio_set_dir(BTN_R, GPIO_IN);
    gpio_pull_up(BTN_R);

    gpio_init(BTN_Y);
    gpio_set_dir(BTN_Y, GPIO_IN);
    gpio_pull_up(BTN_Y);

    gpio_init(BTN_G);
    gpio_set_dir(BTN_G, GPIO_IN);
    gpio_pull_up(BTN_G);

    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);

    gpio_init(LED_Y);
    gpio_set_dir(LED_Y, GPIO_OUT);

    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);

    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(BTN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_B, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_G, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_Y, GPIO_IRQ_EDGE_FALL, true);
}

// int core_1_entry() {
//     inicializa_hardware();
//     while (true) {
//         printf("Hello from core 1!\n");
//         sleep_ms(1000);
//     }
// }

int main() {
    stdio_init_all();
    inicializa_hardware();

    while (true) {
        if (flag_r) {
            gpio_put(LED_R, 1);
            sleep_ms(200);
            gpio_put(LED_R, 0);
            flag_r = 0;
        }
        if (flag_y) {
            gpio_put(LED_Y, 1);
            sleep_ms(200);
            gpio_put(LED_Y, 0);
            flag_y = 0;
        }
        if (flag_g) {
            gpio_put(LED_G, 1);
            sleep_ms(200);
            gpio_put(LED_G, 0);
            flag_g = 0;
        }
        if (flag_b) {
            gpio_put(LED_B, 1);
            sleep_ms(200);
            gpio_put(LED_B, 0);
            flag_b = 0;
        }
    }
}
