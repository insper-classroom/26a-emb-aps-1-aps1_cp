/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#define MAX_SEQ 100
#define TIMEOUT_US 20000000

const int BTN_R = 9;
const int BTN_B = 10;
const int BTN_G = 11;
const int BTN_Y = 15;
const int BTN_W = 14;

const int LED_R = 16;
const int LED_Y = 17;
const int LED_G = 18;
const int LED_B = 19;

volatile int flag_r = 0;
volatile int flag_y = 0;
volatile int flag_g = 0;
volatile int flag_b = 0;
volatile int flag_w = 0;
volatile bool flag_alarme = false;
alarm_id_t alarme_id = -1;

typedef enum {
    ESTADO_DESLIGADO,
    ESTADO_INIT,
    ESTADO_AGUARDANDO,
    ESTADO_MOSTRANDO_SEQUENCIA,
    ESTADO_AGUARDANDO_INPUT,
    ESTADO_ACERTO,
    ESTADO_ERRO,
} Estado;

typedef struct {
    int sequencia[MAX_SEQ];
    int seq_len;
    int input_idx;
} Contexto;

int64_t alarme_callback(alarm_id_t id, void *user_data) {
    flag_alarme = true;
    return 0;
}

void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (gpio == BTN_R) {
            flag_r = 1;
        } else if (gpio == BTN_Y) {
            flag_y = 1;
        } else if (gpio == BTN_G) {
            flag_g = 1;
        } else if (gpio == BTN_B) {
            flag_b = 1;
        } else if (gpio == BTN_W) {
            flag_w = 1;
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

    gpio_init(BTN_W);
    gpio_set_dir(BTN_W, GPIO_IN);
    gpio_pull_up(BTN_W);

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
    gpio_set_irq_enabled(BTN_W, GPIO_IRQ_EDGE_FALL, true);
}

void limpa_flags() {
    flag_r = 0;
    flag_y = 0;
    flag_g = 0;
    flag_b = 0;
    flag_w = 0;
}

void contexto_reset(Contexto *ctx) {
    (*ctx).seq_len      = 1;
    (*ctx).input_idx    = 0;
    (*ctx).sequencia[0] = rand() % 4;
}

int ler_botao() {
    if (flag_r) {
        limpa_flags();
        return 0;
    } else if (flag_y) {
        limpa_flags();
        return 1;
    } else if (flag_g) {
        limpa_flags();
        return 2;
    } else if (flag_b) {
        limpa_flags();
        return 3;
    } else if (flag_w) {
        limpa_flags();
        return 4;
    }
    return -1;
}

void liga_led(int ret) {
    if (ret == 0) {
        gpio_put(LED_R, 1);
        sleep_ms(500);
        gpio_put(LED_R, 0);
    } else if (ret == 1) {
        gpio_put(LED_Y, 1);
        sleep_ms(500);
        gpio_put(LED_Y, 0);
    } else if (ret == 2) {
        gpio_put(LED_G, 1);
        sleep_ms(500);
        gpio_put(LED_G, 0);
    } else if (ret == 3) {
        gpio_put(LED_B, 1);
        sleep_ms(500);
        gpio_put(LED_B, 0);
    } else if (ret == 4) {
        gpio_put(LED_R, 1);
        gpio_put(LED_Y, 1);
        gpio_put(LED_G, 1);
        gpio_put(LED_B, 1);
        sleep_ms(500);
        gpio_put(LED_R, 0);
        gpio_put(LED_Y, 0);
        gpio_put(LED_G, 0);
        gpio_put(LED_B, 0);
    }
}

Estado estado_desligado() {
    printf("Estado: DESLIGADO, aperte o botao branco para ligar\n");
    limpa_flags();
    while (!flag_w) {
        sleep_ms(5);
    }
    limpa_flags();
    return ESTADO_INIT;
}

Estado estado_init() {
    liga_led(4);
    printf("Genius iniciado\n");
    return ESTADO_AGUARDANDO;
}

Estado estado_aguardando(Contexto *ctx) {
    printf("Aperte o botao verde para iniciar o jogo\n");
    while (!flag_g) {
        if (flag_w) {
            limpa_flags();
            return ESTADO_DESLIGADO;
        }
        sleep_ms(5);
    }
    limpa_flags();
    contexto_reset(ctx);
    return ESTADO_MOSTRANDO_SEQUENCIA;
}

Estado estado_mostrando_sequencia(Contexto *ctx) {
    flag_alarme = false;
    printf("Mostrando sequencia:\n");
    sleep_ms(500);
    for (int i = 0; i < (*ctx).seq_len; i++) {
        liga_led((*ctx).sequencia[i]);
        sleep_ms(500);
    }
    (*ctx).input_idx = 0;
    flag_alarme = false;
    if (alarme_id >= 0) cancel_alarm(alarme_id);
    alarme_id = add_alarm_in_us(TIMEOUT_US, alarme_callback, NULL, true);
    return ESTADO_AGUARDANDO_INPUT;
}

Estado estado_aguardando_input(Contexto *ctx) {
    if (flag_alarme) {
        printf("Timeout! Sem resposta em 5 segundos.\n");
        flag_alarme = false;
        return ESTADO_ERRO;
    }

    int btn = ler_botao();

    if (btn == -1) {
        return ESTADO_AGUARDANDO_INPUT;
    }

    liga_led(btn);

    if (btn != (*ctx).sequencia[(*ctx).input_idx]) {
        return ESTADO_ERRO;
    }

    (*ctx).input_idx++;

    if ((*ctx).input_idx < (*ctx).seq_len) {
        flag_alarme = false;
        if (alarme_id >= 0) cancel_alarm(alarme_id);
        alarme_id = add_alarm_in_us(TIMEOUT_US, alarme_callback, NULL, true);
        return ESTADO_AGUARDANDO_INPUT;
    }

    return ESTADO_ACERTO;
}

Estado estado_acerto(Contexto *ctx) {
    flag_alarme = false;
    if ((*ctx).seq_len == MAX_SEQ) {
        printf("Parabens! Voce completou a sequencia!\n");
        return ESTADO_AGUARDANDO;
    }
    (*ctx).sequencia[(*ctx).seq_len] = rand() % 4;
    (*ctx).seq_len++;
    printf("Acertou! Proximo nivel: %d.\n", (*ctx).seq_len);
    return ESTADO_MOSTRANDO_SEQUENCIA;
}

Estado estado_erro() {
    printf("Errou! Reiniciando jogo...\n");
    sleep_ms(2000);
    return ESTADO_AGUARDANDO;
}

int main() {
    stdio_init_all();
    inicializa_hardware();
    srand(to_ms_since_boot(get_absolute_time()));

    Estado estado_atual = ESTADO_DESLIGADO;
    Contexto ctx = { .seq_len = 0, .input_idx = 0 };

    while (true) {
        if (estado_atual == ESTADO_DESLIGADO) {
            estado_atual = estado_desligado();
        } else if (estado_atual == ESTADO_INIT) {
            estado_atual = estado_init();
        } else if (estado_atual == ESTADO_AGUARDANDO) {
            estado_atual = estado_aguardando(&ctx);
        } else if (estado_atual == ESTADO_MOSTRANDO_SEQUENCIA) {
            estado_atual = estado_mostrando_sequencia(&ctx);
        } else if (estado_atual == ESTADO_AGUARDANDO_INPUT) {
            estado_atual = estado_aguardando_input(&ctx);
        } else if (estado_atual == ESTADO_ACERTO) {
            estado_atual = estado_acerto(&ctx);
        } else if (estado_atual == ESTADO_ERRO) {
            estado_atual = estado_erro();
        }
    }
}