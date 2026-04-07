#ifndef GENIUS_H
#define GENIUS_H

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#define MAX_SEQ     100
#define TIMEOUT_US  5000000
#define DEBOUNCE_US 50000

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

void inicializa_hardware();
Estado estado_desligado();
Estado estado_init();
Estado estado_aguardando(Contexto *ctx);
Estado estado_mostrando_sequencia(Contexto *ctx);
Estado estado_aguardando_input(Contexto *ctx);
Estado estado_acerto(Contexto *ctx);
Estado estado_erro(Contexto *ctx);

#endif
