/*
 * keycodes.h
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

#ifndef __KEYKODES_H__
#define __KEYKODES_H__

#include <stdint.h>

void set_key_buf(uint8_t MOD, uint8_t KEY);
#define release_key()  set_key_buf(0,0)
void press_key_mod(char key, uint8_t mod);
#define press_key(k)   press_key_mod(k, 0)

#define MOD_CTRL	0x01
#define MOD_SHIFT	0x02
#define MOD_ALT		0x04
#define MOD_GUI		0x08

#define LEFT(mod)   (mod)
#define RIGHT(mod)  ((mod << 4))

#define KEY_A		4
#define KEY_B		5
#define KEY_C		6
#define KEY_D		7
#define KEY_E		8
#define KEY_F		9
#define KEY_G		10
#define KEY_H		11
#define KEY_I		12
#define KEY_J		13
#define KEY_K		14
#define KEY_L		15
#define KEY_M		16
#define KEY_N		17
#define KEY_O		18
#define KEY_P		19
#define KEY_Q		20
#define KEY_R		21
#define KEY_S		22
#define KEY_T		23
#define KEY_U		24
#define KEY_V		25
#define KEY_W		26
#define KEY_X		27
#define KEY_Y		28
#define KEY_Z		29
#define KEY_1		30
#define KEY_2		31
#define KEY_3		32
#define KEY_4		33
#define KEY_5		34
#define KEY_6		35
#define KEY_7		36
#define KEY_8		37
#define KEY_9		38
#define KEY_0		39
#define KEY_ENTER	40
#define KEY_ESC		41
#define KEY_BACKSPACE	42
#define KEY_TAB		43
#define KEY_SPACE	44
#define KEY_MINUS	45
#define KEY_EQUAL	46
#define KEY_LEFT_BRACE	47
#define KEY_RIGHT_BRACE	48
#define KEY_BACKSLASH	49
#define KEY_NUMBER	50
#define KEY_SEMICOLON	51
#define KEY_QUOTE	52
#define KEY_TILDE	53
#define KEY_COMMA	54
#define KEY_PERIOD	55
#define KEY_SLASH	56
#define KEY_CAPS_LOCK	57
#define KEY_F1		58
#define KEY_F2		59
#define KEY_F3		60
#define KEY_F4		61
#define KEY_F5		62
#define KEY_F6		63
#define KEY_F7		64
#define KEY_F8		65
#define KEY_F9		66
#define KEY_F10		67
#define KEY_F11		68
#define KEY_F12		69
#define KEY_PRINTSCREEN	70
#define KEY_SCROLL_LOCK	71
#define KEY_PAUSE	72
#define KEY_INSERT	73
#define KEY_HOME	74
#define KEY_PAGE_UP	75
#define KEY_DELETE	76
#define KEY_END		77
#define KEY_PAGE_DOWN	78
#define KEY_RIGHT	79
#define KEY_LEFT	80
#define KEY_DOWN	81
#define KEY_UP		82
#define KEY_NUM_LOCK	83
#define KEYPAD_SLASH	84
#define KEYPAD_ASTERIX	85
#define KEYPAD_MINUS	86
#define KEYPAD_PLUS	87
#define KEYPAD_ENTER	88
#define KEYPAD_1	89
#define KEYPAD_2	90
#define KEYPAD_3	91
#define KEYPAD_4	92
#define KEYPAD_5	93
#define KEYPAD_6	94
#define KEYPAD_7	95
#define KEYPAD_8	96
#define KEYPAD_9	97
#define KEYPAD_0	98
#define KEYPAD_PERIOD	99

#define _(x)  (x|0x80)
// array for keycodes according to ASCII table; MSB is MOD_SHIFT flag
static const uint8_t keycodes[] = {
	// space !"#$%&'
	KEY_SPACE, _(KEY_1), _(KEY_QUOTE), _(KEY_3), _(KEY_4), _(KEY_5), _(KEY_7), KEY_QUOTE,
	// ()*+,-./
	_(KEY_9), _(KEY_0), _(KEY_8), _(KEY_EQUAL), KEY_COMMA, KEY_MINUS, KEY_PERIOD, KEY_SLASH,
	// 0..9
	39, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	// :;<=>?@
	_(KEY_SEMICOLON), KEY_SEMICOLON, _(KEY_COMMA), KEY_EQUAL, _(KEY_PERIOD), _(KEY_SLASH), _(KEY_2),
	// A..Z:  for a in $(seq 0 25); do printf "$((a+132)),"; done
	132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,
	// [\]^_`
	KEY_LEFT_BRACE, KEY_BACKSLASH, KEY_RIGHT_BRACE, _(KEY_6), _(KEY_MINUS), KEY_TILDE,
	// a..z: for a in $(seq 0 25); do printf "$((a+4)),"; done
	4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,
	// {|}~
	_(KEY_LEFT_BRACE), _(KEY_BACKSLASH), _(KEY_RIGHT_BRACE), _(KEY_TILDE)
};

#endif // __KEYKODES_H__
