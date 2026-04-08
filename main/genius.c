#include "genius.h"
#include "audio.h"

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
volatile bool flag_alarme = false;
volatile alarm_id_t alarme_id = -1;

static volatile uint64_t ultimo_disparo[32] = {0};
static uint32_t semente_p2 = 0;

int64_t alarme_callback(alarm_id_t id, void *user_data) {
    flag_alarme = true;
    return 0;
}

void btn_callback(uint gpio, uint32_t events) {
    if(events & GPIO_IRQ_EDGE_FALL){
        uint64_t agora = time_us_64();
        if(agora - ultimo_disparo[gpio] < DEBOUNCE_US){
            return;
        }
        ultimo_disparo[gpio] = agora;

        if(gpio == BTN_R){
            flag_r = 1;
        } else if(gpio == BTN_Y){
            flag_y = 1;
        } else if(gpio == BTN_G){
            flag_g = 1;
        } else if(gpio == BTN_B){
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

void limpa_flags() {
    flag_r = 0;
    flag_y = 0;
    flag_g = 0;
    flag_b = 0;
}

void contexto_reset(Contexto *ctx) {
    ctx->seq_len = 1;
    ctx->input_idx = 0;
    ctx->sequencia[0] = rand() % 4;
}

int ler_botao() {
    int r = flag_r, y = flag_y, g = flag_g, b = flag_b;
    if(!r && !y && !g && !b){
        return -1;
    }
    limpa_flags();
    if(r){
        return 0;
    }
    if(y){
        return 1;
    }
    if(g){
        return 2;
    }
    if(b){
        return 3;
    }
    return -1;
}

void liga_led(int ret) {
    if(ret == 0){
        gpio_put(LED_R, 1);
        sleep_ms(500);
        gpio_put(LED_R, 0);
    } else if(ret == 1){
        gpio_put(LED_Y, 1);
        sleep_ms(500);
        gpio_put(LED_Y, 0);
    } else if(ret == 2){
        gpio_put(LED_G, 1);
        sleep_ms(500);
        gpio_put(LED_G, 0);
    } else if(ret == 3){
        gpio_put(LED_B, 1);
        sleep_ms(500);
        gpio_put(LED_B, 0);
    } else if(ret == 4){
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
    printf("Estado: DESLIGADO, aperte o botao azul para ligar\n");
    limpa_flags();
    while(!flag_b){
        sleep_ms(5);
    }
    limpa_flags();
    return ESTADO_INIT;
}

Estado estado_init() {
    semente_p2 = (uint32_t)time_us_64();
    play_sound(SOUND_UNDAIA);
    liga_led(4);
    wait_sound_done();
    printf("Genius iniciado\n");
    return ESTADO_AGUARDANDO;
}

Estado estado_aguardando(Contexto *ctx) {
    stop_music();
    printf("Verde: 1 jogador | Vermelho: 2 jogadores | Amarelo: desligar\n");
    while(1){
        if(flag_g){
            srand((unsigned int)time_us_64());
            limpa_flags();
            play_music();
            play_sound(SOUND_START);
            ctx->modo = 1;
            contexto_reset(ctx);
            return ESTADO_MOSTRANDO_SEQUENCIA;
        }
        if(flag_r){
            limpa_flags();
            play_music();
            play_sound(SOUND_START);
            ctx->modo = 2;
            ctx->jogador_atual = 0;
            ctx->seq_len_2p[0] = 1;
            ctx->seq_len_2p[1] = 1;
            srand((unsigned int)time_us_64());
            ctx->sequencia_2p[0][0] = rand() % 4;
            srand(semente_p2);
            ctx->sequencia_2p[1][0] = rand() % 4;
            ctx->input_idx_2p[0] = 0;
            ctx->input_idx_2p[1] = 0;
            return ESTADO_2P_MOSTRANDO_SEQUENCIA;
        }
        if(flag_y){
            limpa_flags();
            return ESTADO_DESLIGADO;
        }
        sleep_ms(5);
    }
}

Estado estado_mostrando_sequencia(Contexto *ctx) {
    printf("Mostrando sequencia:\n");
    sleep_ms(500);
    for(int i = 0; i < ctx->seq_len; i++){
        play_sound(SOUND_PONTO);
        liga_led(ctx->sequencia[i]);
        sleep_ms(500);
    }
    ctx->input_idx = 0;
    limpa_flags();
    flag_alarme = false;
    if(alarme_id >= 0){
        cancel_alarm(alarme_id);
    }
    alarme_id = add_alarm_in_us(TIMEOUT_US, alarme_callback, NULL, true);
    return ESTADO_AGUARDANDO_INPUT;
}

Estado estado_aguardando_input(Contexto *ctx) {
    if(flag_alarme){
        printf("Timeout!\n");
        flag_alarme = false;
        return ESTADO_ERRO;
    }

    int btn = ler_botao();
    if(btn == -1){
        return ESTADO_AGUARDANDO_INPUT;
    }

    liga_led(btn);
    limpa_flags();

    if(btn != ctx->sequencia[ctx->input_idx]){
        return ESTADO_ERRO;
    }

    play_sound(SOUND_PONTO);
    ctx->input_idx++;

    if(ctx->input_idx < ctx->seq_len){
        flag_alarme = false;
        if(alarme_id >= 0){
            cancel_alarm(alarme_id);
            alarme_id = -1;
        }
        alarme_id = add_alarm_in_us(TIMEOUT_US, alarme_callback, NULL, true);
        return ESTADO_AGUARDANDO_INPUT;
    }

    return ESTADO_ACERTO;
}

Estado estado_acerto(Contexto *ctx) {
    flag_alarme = false;
    if(alarme_id >= 0){
        cancel_alarm(alarme_id);
        alarme_id = -1;
    }
    if(ctx->seq_len == MAX_SEQ){
        printf("Parabens! Voce completou a sequencia!\n");
        return ESTADO_AGUARDANDO;
    }
    ctx->sequencia[ctx->seq_len] = rand() % 4;
    ctx->seq_len++;
    printf("Acertou! Proximo nivel: %d.\n", ctx->seq_len);
    return ESTADO_MOSTRANDO_SEQUENCIA;
}

Estado estado_erro(Contexto *ctx) {
    if(alarme_id >= 0){
        cancel_alarm(alarme_id);
        alarme_id = -1;
    }
    flag_alarme = false;
    play_sound(SOUND_FAIL);

    int pontos = ctx->seq_len - 1;
    printf("Errou! Pontuacao: %d. Reiniciando jogo...\n", pontos);

    for(int i = 0; i < pontos; i++){
        liga_led(4);
        sleep_ms(250);
    }

    sleep_ms(2000);
    return ESTADO_AGUARDANDO;
}

Estado estado_2p_mostrando_sequencia(Contexto *ctx) {
    int p = ctx->jogador_atual;
    printf("Jogador %d: repita a sequencia\n", p + 1);
    sleep_ms(1000);

    for(int i = 0; i < ctx->seq_len_2p[p]; i++){
        play_sound(SOUND_PONTO);
        liga_led(ctx->sequencia_2p[p][i]);
        sleep_ms(500);
    }

    ctx->input_idx_2p[p] = 0;
    limpa_flags();
    flag_alarme = false;
    if(alarme_id >= 0){
        cancel_alarm(alarme_id);
    }
    alarme_id = add_alarm_in_us(TIMEOUT_US, alarme_callback, NULL, true);
    return ESTADO_2P_AGUARDANDO_REPETICAO;
}

Estado estado_2p_aguardando_repeticao(Contexto *ctx) {
    int p = ctx->jogador_atual;

    if(flag_alarme){
        flag_alarme = false;
        ctx->vencedor = 1 - p;
        return ESTADO_2P_ERRO;
    }

    int btn = ler_botao();
    if(btn == -1){
        return ESTADO_2P_AGUARDANDO_REPETICAO;
    }

    liga_led(btn);
    limpa_flags();

    if(btn != ctx->sequencia_2p[p][ctx->input_idx_2p[p]]){
        if(alarme_id >= 0){
            cancel_alarm(alarme_id);
            alarme_id = -1;
        }
        ctx->vencedor = 1 - p;
        return ESTADO_2P_ERRO;
    }

    play_sound(SOUND_PONTO);
    ctx->input_idx_2p[p]++;

    if(ctx->input_idx_2p[p] < ctx->seq_len_2p[p]){
        flag_alarme = false;
        if(alarme_id >= 0){
            cancel_alarm(alarme_id);
            alarme_id = -1;
        }
        alarme_id = add_alarm_in_us(TIMEOUT_US, alarme_callback, NULL, true);
        return ESTADO_2P_AGUARDANDO_REPETICAO;
    }

    return ESTADO_2P_ACERTO;
}

Estado estado_2p_acerto(Contexto *ctx) {
    int p = ctx->jogador_atual;
    flag_alarme = false;
    if(alarme_id >= 0){
        cancel_alarm(alarme_id);
        alarme_id = -1;
    }

    if(ctx->seq_len_2p[p] >= MAX_SEQ){
        printf("Empate! Sequencia maxima atingida!\n");
        play_sound(SOUND_FAIL);
        sleep_ms(2000);
        return ESTADO_AGUARDANDO;
    }

    ctx->sequencia_2p[p][ctx->seq_len_2p[p]] = rand() % 4;
    ctx->seq_len_2p[p]++;
    printf("Correto! Nivel do Jogador %d: %d\n", p + 1, ctx->seq_len_2p[p]);

    ctx->jogador_atual = 1 - p;
    sleep_ms(1000);
    return ESTADO_2P_MOSTRANDO_SEQUENCIA;
}

Estado estado_2p_erro(Contexto *ctx) {
    if(alarme_id >= 0){
        cancel_alarm(alarme_id);
        alarme_id = -1;
    }
    flag_alarme = false;
    play_sound(SOUND_FAIL);

    int pontos = ctx->seq_len_2p[1 - ctx->vencedor] - 1;
    printf("Jogador %d venceu! Jogador %d errou na sequencia %d.\n",
           ctx->vencedor + 1, (1 - ctx->vencedor) + 1, pontos + 1);

    for(int i = 0; i < pontos; i++){
        liga_led(4);
        sleep_ms(250);
    }

    sleep_ms(2000);
    return ESTADO_AGUARDANDO;
}
