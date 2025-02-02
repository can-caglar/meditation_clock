#ifndef MUSIC_H
#define MUSIC_H

#include <stdint.h>
#include <stddef.h>
#include "bsp.h"

// Helper functions
struct Notes
{
	uint32_t freq;
	uint32_t duration_ms;
};

// Songs
extern const struct Notes ASpacemanCameTravelling[];

void play_church_bell(enum Quarter q, uint8_t hour);
void play_tone(uint32_t freq, uint32_t duration_ms);
void play_song(const struct Notes* song);

#endif
