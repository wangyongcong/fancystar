#ifndef __HEADER_WYC_INPUT_WIN
#define __HEADER_WYC_INPUT_WIN

#include "wyc/basedef.h"
#include "wyc/thread/thread.h"
#include "wyc/game/input.h"

namespace wyc {

/**************************************************************
  线程安全性：写入操作是线程安全的，允许多个线程同时将消息写入
  队列；但读取操作则不是，当读取消息队列的时候，队列中的内容可
  能正被改写。通常情况下，只有一个线程允许调用update_input()和
  get_buffer()来获取队列中的消息
**************************************************************/

class xinput
{
protected:
	xcritical_section m_inputLock;
	uint8_t m_widx;
	xinput_buffer m_msgbuff[2];
	int16_t m_lastx, m_lasty;
public:
	xinput();
	// 重置消息队列
	void reset();
	// 发送键盘输入消息(输入线程)
	void key_event(int32_t msg, uint32_t key);
	// 发送鼠标输入消息(输入线程)
	void mouse_button(int32_t msg, int16_t x, int16_t y, uint32_t button);
	void mouse_move(int16_t x, int16_t y);
	void mouse_wheel(int16_t z);
	// 发送字符输入消息(输入线程)
	void add_character(wchar_t wc);
	// 更新/读取消息队列(读取线程)
	void update_input();
	const xinput_buffer& get_buffer() const;
	// 输出消息队列中的内容(读取线程)
	void trace_keyque(const xkeyque& msgque) const;
	void trace_mouseque(const xmouseque& msgque) const;
};

inline const xinput_buffer& xinput::get_buffer() const
{
	return m_msgbuff[1-m_widx];
}

}; // namespace wyc

#endif // end of __HEADER_WYC_INPUT_WIN

