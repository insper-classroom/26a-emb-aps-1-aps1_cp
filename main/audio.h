#ifndef GENIUS_AUDIO_H
#define GENIUS_AUDIO_H

#define SOUND_START       0
#define SOUND_PONTO       1
#define SOUND_FAIL        2
#define SOUND_UNDAIA      3
#define SOUND_MUSIC_START 4
#define SOUND_MUSIC_STOP  5

void core1_entry();
void play_sound(int sound_id);
void play_music();
void stop_music();
void wait_sound_done();

#endif
