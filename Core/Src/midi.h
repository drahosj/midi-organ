/*
 * midi.h
 *
 *  Created on: Oct 18, 2020
 *      Author: jake
 */

#ifndef SRC_MIDI_H_
#define SRC_MIDI_H_
#include <stdint.h>

extern uint8_t  notemap1[];
extern uint8_t  notemap2[];

enum midi_event_type {
	MIDI_NOTE_ON,
	MIDI_NOTE_OFF,
	N_MIDI_EVENTS
};

struct midi_event {
	int type;
	int number;
};

int midi_getevents(uint8_t * old, uint8_t * new, int chainlen, struct midi_event * eventbuf, int maxevents);

#endif /* SRC_MIDI_H_ */
