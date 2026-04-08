#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "genius.h"
#include "audio.h"

int main() {
    set_sys_clock_khz(176000, true);
    stdio_init_all();
    inicializa_hardware();
    srand((unsigned int)time_us_64());

    multicore_launch_core1(core1_entry);

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
            estado_atual = estado_erro(&ctx);
        } else if (estado_atual == ESTADO_2P_MOSTRANDO_SEQUENCIA) {
            estado_atual = estado_2p_mostrando_sequencia(&ctx);
        } else if (estado_atual == ESTADO_2P_AGUARDANDO_REPETICAO) {
            estado_atual = estado_2p_aguardando_repeticao(&ctx);
        } else if (estado_atual == ESTADO_2P_ACERTO) {
            estado_atual = estado_2p_acerto(&ctx);
        } else if (estado_atual == ESTADO_2P_ERRO) {
            estado_atual = estado_2p_erro(&ctx);
        }
    }
}
