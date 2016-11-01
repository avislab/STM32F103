/*
 * keycodes.c
 *
 * Copyright 2015 Edward V. Emelianov <eddy@sao.ru, edward.emelianoff@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "keycodes.h"

//#define _(x)  (x|0x80)
// array for keycodes according to ASCII table; MSB is MOD_SHIFT flag

/*
 * Keyboard buffer:
 * buf[1]: MOD
 * buf[2]: reserved
 * buf[3]..buf[8] - keycodes 1..6
 */

uint8_t key_buf[9] = {2,0,0,0,0,0,0,0,0};

void set_key_buf(uint8_t MOD, uint8_t KEY){
	key_buf[1] = MOD;
	key_buf[3] = KEY;
	//return key_buf;
}

/**
 * return buffer for sending symbol "ltr" with addition modificator mod
 */
void press_key_mod(char ltr, uint8_t mod){
	uint8_t MOD = 0;
	uint8_t KEY = 0;
	if(ltr > 31){
		KEY = keycodes[ltr - 32];
		if(KEY & 0x80){
			MOD = MOD_SHIFT;
			KEY &= 0x7f;
		}
	}else if (ltr == '\n') KEY = KEY_ENTER;
	key_buf[1] = MOD | mod;
	key_buf[3] = KEY;
	//return key_buf;
}
