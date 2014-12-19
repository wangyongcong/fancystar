#include "fscorepch.h"
#include "wyc/game/input_win.h"
#include "wyc/util/util.h"

using namespace wyc;

xinput_buffer::xinput_buffer() : keyque(KEYMSG_QUEUE_SIZE), mouseque(MOUSEMSG_QUEUE_SIZE) 
{
	chars.reserve(CHAR_BUFF_SIZE);
	x=y=0;
	offx=offy=offz=0;
	move=0;
}

void xinput_buffer::clear() 
{
	keyque.clear();
	mouseque.clear();
	chars.clear();
	x=y=0;
	offx=offy=offz=0;
	move=0;
}

xinput::xinput()
{
	m_widx=0;
}

void xinput::key_event(int32_t msg, uint32_t key)
{
	m_inputLock.lock();
	KEYMSG t;
	t.msg=msg;
	t.key=key;
	m_msgbuff[m_widx].keyque.push(t);
	m_inputLock.unlock();
}

void xinput::mouse_button(int32_t msg, int16_t x, int16_t y, uint32_t button)
{
	m_inputLock.lock();
	MOUSEMSG t;
	t.msg=msg;
	t.x=x;
	t.y=y;
	t.button=button;
	m_msgbuff[m_widx].mouseque.push(t);
	m_inputLock.unlock();
}

void xinput::mouse_move(int16_t x, int16_t y)
{
	m_inputLock.lock(); 
	m_msgbuff[m_widx].x=x;
	m_msgbuff[m_widx].y=y;
	m_msgbuff[m_widx].move+=1;
	m_inputLock.unlock();
}

void xinput::mouse_wheel(int16_t delta)
{
	m_inputLock.lock();
	m_msgbuff[m_widx].offz+=delta;
	m_inputLock.unlock();
}

void xinput::add_character(wchar_t wc)
{
	m_inputLock.lock();
	m_msgbuff[m_widx].chars+=wc;
	m_inputLock.unlock();
}

void xinput::reset()
{
	m_inputLock.lock();
	m_msgbuff[0].clear();
	m_msgbuff[1].clear();
	m_widx=0;
	m_lastx=0;
	m_lasty=0;
	m_inputLock.unlock();
}

void xinput::update_input()
{
	m_inputLock.lock();
	xinput_buffer &rbuff=m_msgbuff[m_widx];
	if(rbuff.move) {
		rbuff.offx=rbuff.x-m_lastx;
		rbuff.offy=rbuff.y-m_lasty;
	}
	else {
		rbuff.offx=0;
		rbuff.offy=0;
	}
	m_lastx=rbuff.x;
	m_lasty=rbuff.y;
	m_widx=1-m_widx;
	xinput_buffer &wbuff=m_msgbuff[m_widx];
	wbuff.keyque.clear();
	wbuff.mouseque.clear();
	wbuff.chars.clear();
	wbuff.offz=0;
	wbuff.move=0;
	m_inputLock.unlock();
}

void xinput::trace_keyque(const xkeyque& msgque) const
{
	xkeyque::const_iterator iter=msgque.begin(), end=msgque.end();
	while(iter!=end) {
		if(iter->msg==EV_KEY_DOWN) {
			wyc_print("keydown '%c'(%X)",char(iter->key),iter->key);
		}
		else if(iter->msg==EV_KEY_UP) { 
			wyc_print("keyup '%c'(%X)",char(iter->key),iter->key);
		}
		else {
			wyc_print("unknow keymsg '%c'(%X)",char(iter->key),iter->key);
		}
		++iter;
	}
}

void xinput::trace_mouseque(const xmouseque& msgque) const
{
	xmouseque::const_iterator iter=msgque.begin(), end=msgque.end();
	uint32_t msg;
	int x, y;
	while(iter!=end) {
		msg=iter->msg;
		x=iter->x;
		y=iter->y;
		switch(msg) {
		case EV_LB_DOWN:
			wyc_print("lb down (%d,%d)",x,y);break;
		case EV_MB_DOWN:
			wyc_print("mb down (%d,%d)",x,y);break;
		case EV_RB_DOWN:
			wyc_print("rb down (%d,%d)",x,y);break;
		case EV_LB_UP:
			wyc_print("lb up (%d,%d)",x,y);break;
		case EV_MB_UP:
			wyc_print("mb up (%d,%d)",x,y);break;
		case EV_RB_UP:
			wyc_print("rb up (%d,%d)",x,y);break;
		case EV_LB_DBLC:
			wyc_print("lb double click (%d,%d)",x,y);break;
		case EV_MB_DBLC:
			wyc_print("mb double click (%d,%d)",x,y);break;
		case EV_RB_DBLC:
			wyc_print("rb double click (%d,%d)",x,y);break;
		default:
			wyc_print("unknown mouse msg (%d,%d)",x,y);
		};
		++iter;
	}
}


