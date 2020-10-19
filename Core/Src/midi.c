/*
 * midi.c
 *
 *  Created on: Oct 18, 2020
 *      Author: jake
 */

#include "midi.h"


int midi_getevents(uint8_t * old, uint8_t * new, int chainlen, struct midi_event * eventbuf, int maxevents)
{
	int event_index = 0;
	for (int i = 0; i < chainlen; i++) {
		if (old[i] != new[i]) {
			for (int j = 0; j < 8; j++) {
				int o = (old[i] >> j) & 0x01;
				int n = (new[i] >> j) & 0x01;

				if (n && !o) {
					eventbuf[event_index].type = MIDI_NOTE_ON;
					eventbuf[event_index].number = (i * 8) + (7 - j);
					event_index++;
				} else if (!n && o) {
					eventbuf[event_index].type = MIDI_NOTE_OFF;
					eventbuf[event_index].number = (i * 8) + (7 - j);
					event_index++;
				}

				if (event_index == maxevents) {
					return event_index;
				}
			}
		}
	}
	return event_index;
}
