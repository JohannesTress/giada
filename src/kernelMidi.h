/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * KernelMidi
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ------------------------------------------------------------------ */


#ifndef KERNELMIDI_H
#define KERNELMIDI_H


#include <stdint.h>
#include "channel.h"


namespace kernelMidi {

	inline int getNoteOnOff(uint32_t iValue) { return (iValue >> 24) & 0xFF; }
	inline int getNoteValue(uint32_t iValue) { return (iValue >> 16) & 0xFF; }
	inline int getVelocity (uint32_t iValue) { return (iValue >> 8)  & 0xFF; }

	inline uint32_t getIValue (int type, int note, int vel) { return (type << 24) | (note << 16) | (vel << 8) | (0x00); }

	void send(uint32_t s, struct channel *ch);

	void send(int b1, int b2, int b3, channel *ch);
}

#endif
