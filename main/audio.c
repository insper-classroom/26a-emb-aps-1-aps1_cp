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

#define AUDIO_PIN 22

static volatile int playing_sound = 0;
static volatile int sample_index = 0;
static volatile const uint8_t *current_wav_data = NULL;
static volatile uint32_t current_wav_length = 0;

static void pwm_interrupt_handler() {
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN));
    if (playing_sound) {
        if (sample_index < ((int)current_wav_length << 3) - 1) {
            pwm_set_gpio_level(AUDIO_PIN, current_wav_data[sample_index >> 3]);
            sample_index++;
        } else {
            playing_sound = 0;
            sample_index = 0;
            pwm_set_gpio_level(AUDIO_PIN, 0);
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

    while (true) {
        if (multicore_fifo_rvalid()) {
            uint32_t cmd = multicore_fifo_pop_blocking();
            playing_sound = 0;
            sample_index = 0;

            if (cmd == SOUND_START) {
                current_wav_data = AUDIO_WAV_DATA;
                current_wav_length = AUDIO_WAV_DATA_LENGTH;
                playing_sound = 1;
            } else if (cmd == SOUND_PONTO) {
                current_wav_data = ponto_wav_data;
                current_wav_length = sizeof(ponto_wav_data);
                playing_sound = 1;
            } else if (cmd == SOUND_FAIL) {
                current_wav_data = fail_wav_data;
                current_wav_length = sizeof(fail_wav_data);
                playing_sound = 1;
            } else if (cmd == SOUND_UNDAIA) {
                current_wav_data = undaia_wav_data;
                current_wav_length = sizeof(undaia_wav_data);
                playing_sound = 1;
            }
        }
        tight_loop_contents();
    }
}

void play_sound(int sound_id) {
    multicore_fifo_push_blocking((uint32_t)sound_id);
}
