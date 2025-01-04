#include "music.h"
#include "bsp.h"
#include <stdbool.h>
#include "qpc.h"

Q_DEFINE_THIS_FILE

#define sixteenth ((whole) / 16)
#define eigth ((whole) / 8)
#define quarter ((whole) / 4)
#define half ((whole) / 2)
#define three_quarters (half + quarter)
#define whole (1000)
#define whole_plus_half (whole + half)
#define double (whole * 2)
#define double_plus_half (double + half)
#define quadro (whole * 4)

#define PAUSE 0
#define C3 131
#define C4 262
#define Db4 277
#define D4 294
#define Eb4 311
#define E4 330
#define F4 349
#define Gb4 370
#define G4 392
#define Ab4 415
#define A4 440
#define Bb4 466
#define B4 494
#define C5 523
#define Db5 554
#define D5 587
#define Eb5 622
#define E5 659
#define F5 698
#define Gb5 740
#define G5 784
#define Ab5 831
#define A5 880
#define Bb5 932
#define B5 988
#define C6 1046
#define Db6 1109
#define D6 1175
#define Eb6 1245
#define E6 1319
#define F6 1397

#define END_OF_SONG {0, 0}

static struct Notes ChurchQuarter[] = {
    {C4, whole}, {PAUSE, eigth}, {D4, whole_plus_half}, {PAUSE, quarter},
    END_OF_SONG
};

static struct Notes ChurchGong[] = {
    {C3, whole_plus_half}, {PAUSE, whole},
    END_OF_SONG
};

const struct Notes ASpacemanCameTravelling[] = {
 	{F5, whole}, {G5, half}, {F5, half}, {E5, whole},
 	{D5, half}, {C5, half}, {D5, whole}, {D5, half},
 	{C5, half}, {D5, whole}, {E5, whole},

	{F5, whole}, {G5, half}, {F5, half}, {E5, whole},
 	{D5, half}, {C5, half}, {D5, whole},
    END_OF_SONG
};

void play_tone(uint32_t freq, uint32_t duration_ms) {

	bool pause = false;
	if (duration_ms > 30)
	{
		duration_ms -= 30;
		pause = true;
	}

	if (freq == PAUSE)
	{
        BSP_pwmBuzz(0);
	}
	else
	{
        BSP_pwmBuzz(freq);
	}
    BSP_sleep(duration_ms);

	// Turn off the tone
    BSP_pwmBuzz(0);

	if (pause)
		BSP_sleep(10);
}

void play_song(const struct Notes* song) {
    const struct Notes* v = song;
    while ((v->duration_ms != 0) || (v->freq != 0)) {
		play_tone(v->freq, v->duration_ms);
        v++;
    }
}

void play_church_bell(enum Quarter q, uint8_t hour) {
    Q_ASSERT(q > 0);
    for (int i = 0; i < q; i++) {
        play_song(ChurchQuarter);
    }
    if (q == Fourth) {
        k_sleep(K_MSEC(2000));
        for (int i = 0; i < hour; i++) {
            play_song(ChurchGong);
        }
    }
}
