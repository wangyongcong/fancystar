#ifndef __HEADER_WYC_INPUT_CFG
#define __HEADER_WYC_INPUT_CFG

#include <string>
#include "wyc/util/circuque.h"

namespace wyc
{

#define KEY_CODE_NUM	256
#define KEYST_KEYDOWN	0x100
#define KEYST_KEYUP		0x200
#define KEY_CODE_MASK	0xFF
#define KEY_STATE_MASK	0xFF00

#define MOUSE_BUTTON_NUM	4
#define MOUSE_LEFT_BUTTON	0
#define MOUSE_RIGHT_BUTTON	1
#define MOUSE_MIDDLE_BUTTON	2
#define MOUSE_WHEEL_DELTA	120

#define SYSKEY_SHIFT	0x01
#define SYSKEY_CONTROL	0x02
#define SYSKEY_ALT		0x04
#define SYSKEY_CAPSLOCK	0x08

enum INPUT_EVENT
{
	EV_LB_DOWN = 0x5529638A,
	EV_LB_UP = 0x27091E43,
	EV_RB_DOWN = 0x62C88419,
	EV_RB_UP = 0xF8D937A0,
	EV_MB_DOWN = 0xF35E683E,
	EV_MB_UP = 0x1A6937F3,
	EV_LB_DBLC = 0x8A7647FE,
	EV_RB_DBLC = 0xBD97A06D,
	EV_MB_DBLC = 0x2C014C4A,
	EV_MOUSE_IN = 0xBA188DE8,
	EV_MOUSE_OUT = 0x86BA9F47,
	EV_MOUSE_MOVE = 0x8E6A9859,
	EV_MOUSE_WHEEL = 0x2DD8EB85,
	EV_KEY_DOWN = 0xE3626475,
	EV_KEY_UP = 0xF24ACBC,
};

struct MOUSEMSG {
	int32_t msg;
	int16_t x;
	int16_t y;
	uint32_t button;
};

struct KEYMSG {
	int32_t msg;
	uint32_t key;
};

#define KEYMSG_QUEUE_SIZE 16
#define MOUSEMSG_QUEUE_SIZE 16
#define CHAR_BUFF_SIZE 15

typedef xcircuque<KEYMSG> xkeyque;
typedef xcircuque<MOUSEMSG> xmouseque;

struct xinput_buffer 
{
	xkeyque keyque;
	xmouseque mouseque;
	std::wstring chars;
	int16_t x, y;
	int16_t offx, offy, offz;
	int16_t move;
	xinput_buffer();
	void clear();
};

}; // namespace wyc

#endif // __HEADER_WYC_INPUT_CFG
