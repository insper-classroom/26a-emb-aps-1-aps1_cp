#include "audio.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"

#include "Start.h"

#define WAV_DATA ponto_wav_data
#include "Ponto.h"
#undef WAV_DATA
#undef WAV_DATA_LENGTH

#define WAV_DATA fail_wav_data
#include "Fail.h"
#undef WAV_DATA
#undef WAV_DATA_LENGTH

#define WAV_DATA undaia_wav_data
#include "Undaia.h"
#undef WAV_DATA
#undef WAV_DATA_LENGTH

#define WAV_DATA jogo_wav_data
#include "Jogo.h"
#undef WAV_DATA
#undef WAV_DATA_LENGTH

#define AUDIO_PIN 22

typedef enum {
    AUDIO_IDLE,
    AUDIO_MUSIC,
    AUDIO_SFX,
} AudioState;

static volatile AudioState audio_state = AUDIO_IDLE;

static volatile int sfx_index = 0;
static volatile const uint8_t *sfx_wav_data = NULL;
static volatile uint32_t sfx_wav_length = 0;

static volatile int music_position = 0;
static volatile const uint8_t *music_wav_data = NULL;
static volatile uint32_t music_wav_length = 0;

static void pwm_interrupt_handler() {
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN));

    if(audio_state == AUDIO_SFX){
        if(sfx_index < ((int)sfx_wav_length << 3) - 1){
            pwm_set_gpio_level(AUDIO_PIN, sfx_wav_data[sfx_index >> 3]);
            sfx_index++;
        } else {
            sfx_index = 0;
            if(music_wav_data != NULL){
                audio_state = AUDIO_MUSIC;
            } else {
                audio_state = AUDIO_IDLE;
                pwm_set_gpio_level(AUDIO_PIN, 0);
            }
        }
    } else if(audio_state == AUDIO_MUSIC){
        if(music_position < ((int)music_wav_length << 3) - 1){
            pwm_set_gpio_level(AUDIO_PIN, music_wav_data[music_position >> 3]);
            music_position++;
        } else {
            music_position = 0;
        }
    } else {
        pwm_set_gpio_level(AUDIO_PIN, 0);
    }
}

void core1_entry() {
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    int slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    pwm_clear_irq(slice);
    pwm_set_irq_enabled(slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 8.0f);
    pwm_config_set_wrap(&config, 250);
    pwm_init(slice, &config, true);
    pwm_set_gpio_level(AUDIO_PIN, 0);

    while(true){
        if(multicore_fifo_rvalid()){
            uint32_t cmd = multicore_fifo_pop_blocking();

            if(cmd == SOUND_START){
                sfx_wav_data = AUDIO_WAV_DATA;
                sfx_wav_length = AUDIO_WAV_DATA_LENGTH;
                sfx_index = 0;
                audio_state = AUDIO_SFX;
            } else if(cmd == SOUND_PONTO){
                sfx_wav_data = ponto_wav_data;
                sfx_wav_length = sizeof(ponto_wav_data);
                sfx_index = 0;
                audio_state = AUDIO_SFX;
            } else if(cmd == SOUND_FAIL){
                music_wav_data = NULL;
                sfx_wav_data = fail_wav_data;
                sfx_wav_length = sizeof(fail_wav_data);
                sfx_index = 0;
                audio_state = AUDIO_SFX;
            } else if(cmd == SOUND_UNDAIA){
                music_wav_data = NULL;
                sfx_wav_data = undaia_wav_data;
                sfx_wav_length = sizeof(undaia_wav_data);
                sfx_index = 0;
                audio_state = AUDIO_SFX;
            } else if(cmd == SOUND_MUSIC_START){
                music_wav_data = jogo_wav_data;
                music_wav_length = sizeof(jogo_wav_data);
                music_position = 0;
                audio_state = AUDIO_MUSIC;
            } else if(cmd == SOUND_MUSIC_STOP){
                music_wav_data = NULL;
                audio_state = AUDIO_IDLE;
                pwm_set_gpio_level(AUDIO_PIN, 0);
            }
        }
        tight_loop_contents();
    }
}

void play_sound(int sound_id) {
    multicore_fifo_push_blocking((uint32_t)sound_id);
}

void play_music() {
    multicore_fifo_push_blocking((uint32_t)SOUND_MUSIC_START);
}

void stop_music() {
    multicore_fifo_push_blocking((uint32_t)SOUND_MUSIC_STOP);
}

void wait_sound_done() {
    while(audio_state == AUDIO_SFX){
        tight_loop_contents();
    }
}
