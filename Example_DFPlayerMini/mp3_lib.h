#ifndef _MP3_LIB_H_
#define _MP3_LIB_H_

////////////////////////////////////////////////////////////////////////////////
//DFPlayer Commands
////////////////////////////////////////////////////////////////////////////////
#define MP3_NEXT					0x01
#define MP3_PREVIOUS				0x02
#define MP3_TRAKING_NUM				0x03 // 0..2999
#define MP3_INC_VOLUME				0x04
#define MP3_DEC_VOLUME				0x05
#define MP3_VOLUME					0x06 // 0..30
#define MP3_EQ						0x07 // 0-Normal / 1-Pop / 2-Rock / 3-Jazz / 4-Classic / 5-Base
#define MP3_PLAYBACK_MODE			0x08 // 0-Repeat / 1-folder repeat / 2-single repeat / 3-random
#define MP3_PLAYBACK_SOURCE			0x09 // 0-U / 1-TF / 2-AUX / 3-SLEEP / 4-FLASH
#define MP3_STANDBY					0x0A
#define MP3_NORMAL_WORK				0x0B
#define MP3_RESET					0x0C
#define MP3_PLAYBACK				0x0D
#define MP3_PAUSE					0x0E
#define MP3_PLAY_FOLDER_FILE		0x0F // 0..10
#define MP3_VOLUME_ADJUST			0x10
#define MP3_REPEAT					0x11 // 0-stop play / 1-start repeat play
// Query the System Parameters
#define MP3_Q_STAY1					0x3C
#define MP3_Q_STAY2					0x3D
#define MP3_Q_STAY3					0x3E
#define MP3_Q_SEND_PRM				0x3F
#define MP3_Q_ERROR					0x40
#define MP3_Q_REPLY					0x41
#define MP3_Q_STATUS				0x42
#define MP3_Q_VALUE					0x43
#define MP3_Q_EQ					0x44
#define MP3_Q_PLAYBACK_MODE			0x45
#define MP3_Q_SOFT_VERSION			0x46
#define MP3_Q_TF_CARD_FILES			0x47
#define MP3_Q_U_DISK_CARD_FILES		0x48
#define MP3_Q_FLASH_CARD_FILES		0x49
#define MP3_Q_KEEPON				0x4A
#define MP3_Q_CURRENT_TRACK_TF		0x4B
#define MP3_Q_CURRENT_TRACK_U_DISK	0x4C
#define MP3_Q_CURRENT_TRACK_FLASH	0x4D
////////////////////////////////////////////////////////////////////////////////
//Commands parameters
////////////////////////////////////////////////////////////////////////////////
#define MP3_EQ_Normal					0
#define MP3_EQ_Pop						1
#define MP3_EQ_Rock						2
#define MP3_EQ_Jazz						3
#define MP3_EQ_Classic					4
#define MP3_EQ_Base						5

#define MP3_PLAYBACK_MODE_Repeat		0
#define MP3_PLAYBACK_MODE_folder_repeat	1
#define MP3_PLAYBACK_MODE_single_repeat	2
#define MP3_PLAYBACK_MODE_random		3

#define MP3_PLAYBACK_SOURCE_U			0
#define MP3_PLAYBACK_SOURCE_TF			1
#define MP3_PLAYBACK_SOURCE_AUX			2
#define MP3_PLAYBACK_SOURCE_SLEEP		3
#define MP3_PLAYBACK_SOURCE_FLASH		4
////////////////////////////////////////////////////////////////////////////////
#define mp3_RX_Buf_SIZE 20
#define MP3_QUEUE_LEN	10
#define MP3_NO_VALUE	0xFF // !!! Please not use file with this name (255.mp3)
////////////////////////////////////////////////////////////////////////////////

void MP3_init(void);
void MP3_send_cmd (uint8_t cmd, uint16_t high_arg, uint16_t low_arg);
void MP3_say(uint8_t prefix, int value, uint8_t suffix);
void MP3_queue_processing(void);
void MP3_set_folder (uint8_t folder);

#endif
