#include "fscorepch.h"
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/game/frametime.h"
#include "wyc/game/game.h"

namespace wyc 
{

#define GETX_LPARAM(lparam) short(lparam&0xFFFF)
#define GETY_LPARAM(lparam) short((lparam>>16)&0xFFFF)

LRESULT xgame::window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	xgame *pGame=xgame::get_game_instance(hwnd);
	if(!pGame) 
		return DefWindowProc(hwnd, msg, wparam, lparam);
	switch(msg)
	{
	case WM_KEYDOWN:
		if((lparam&0x40000000)==0)
		{
			if(wparam==VK_ESCAPE) {
				PostMessage(hwnd,WM_CLOSE,0,0);
				return 0;
			}
			pGame->input().key_event(EV_KEY_DOWN,wparam);
		}
		break;
	case WM_KEYUP:
		pGame->input().key_event(EV_KEY_UP,wparam);
		break;
	case WM_LBUTTONDOWN:
		pGame->capture_mouse();
		pGame->input().mouse_button(EV_LB_DOWN,GETX_LPARAM(lparam),GETY_LPARAM(lparam),wparam);
		break;
	case WM_LBUTTONUP:
		pGame->release_mouse();
		pGame->input().mouse_button(EV_LB_UP,GETX_LPARAM(lparam),GETY_LPARAM(lparam),wparam);
		break;
	case WM_LBUTTONDBLCLK:
		pGame->input().mouse_button(EV_LB_DBLC,GETX_LPARAM(lparam),GETY_LPARAM(lparam),wparam);
		break;
	case WM_MBUTTONDOWN:
		pGame->capture_mouse();
		pGame->input().mouse_button(EV_MB_DOWN,GETX_LPARAM(lparam),GETY_LPARAM(lparam),wparam);
		break;
	case WM_MBUTTONUP:
		pGame->release_mouse();
		pGame->input().mouse_button(EV_MB_UP,GETX_LPARAM(lparam),GETY_LPARAM(lparam),wparam);
		break;
	case WM_MBUTTONDBLCLK:
		pGame->input().mouse_button(EV_MB_DBLC,GETX_LPARAM(lparam),GETY_LPARAM(lparam),wparam);
		break;
	case WM_RBUTTONDOWN:
		pGame->capture_mouse();
		pGame->input().mouse_button(EV_RB_DOWN,GETX_LPARAM(lparam),GETY_LPARAM(lparam),wparam);
		break;
	case WM_RBUTTONUP:
		pGame->release_mouse();
		pGame->input().mouse_button(EV_RB_UP,GETX_LPARAM(lparam),GETY_LPARAM(lparam),wparam);
		break;
	case WM_RBUTTONDBLCLK:
		pGame->input().mouse_button(EV_RB_DBLC,GETX_LPARAM(lparam),GETY_LPARAM(lparam),wparam);
		break;
	case WM_MOUSEMOVE:
		pGame->input().mouse_move(GETX_LPARAM(lparam),GETY_LPARAM(lparam));
		break;
	case WM_MOUSEWHEEL:
		pGame->input().mouse_wheel(GET_WHEEL_DELTA_WPARAM(wparam));
		break;
	case WM_CHAR: 
		pGame->input().add_character(wchar_t(wparam));
		break;
	case WM_ACTIVATE:
		if(LOWORD(wparam)==WA_INACTIVE)
			pGame->deactivate_window();
		else
			pGame->activate_window();
		break;
	case WM_CLOSE:
		if(!pGame->query_exit())
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	LRESULT ret;
	if(pGame->on_window_message(msg,wparam,lparam,ret))
		return ret;
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

unsigned int xgame::game_proc(void *pctx)
{
	xgame *pGame=(xgame*)pctx;
	if(!pGame->init_game()) 
	{
		wyc_error("game init failed");
		return 0;
	}

	while(!pGame->is_quit()) 
	{
		pGame->m_clock.tick();
		if(pGame->m_logic_timer.elapsed_time()>=pGame->m_logic_period)
		{
			pGame->process_logic();
		}
		Sleep(1);
	}

	pGame->quit_game();
	return 0;
}

unsigned int xgame::game_worker_proc(void* pctx)
{
	xgame *pGame=(xgame*)pctx;
	while(!pGame->m_shutdown_worker.load(std::memory_order_acquire)) {
		while(!pGame->m_end_task.load(std::memory_order_acquire))
		{
			pGame->m_worker.update();
			SwitchToThread();
		}
		pGame->m_worker.update();
		pGame->m_end_task.store(false,std::memory_order_release);
		SuspendThread(pGame->m_hWorkerThread);
	}
	return 0;
}

void xgame::process_logic() 
{
	wyc::xcode_timer ct_all, ct_sub;
	ct_all.start();

	// fixed frame time
	double frame_time = m_logic_period;

	// 渲染(上一帧)
	ct_sub.start();
	on_render(m_clock.time(), frame_time);
	m_renderer->flush();
	ct_sub.stop();
	m_metrics.m_render_time_last+=ct_sub.get_time();

	// 处理输入
	ct_sub.start();
	m_input.update_input();
	on_input(m_input.get_buffer());
	ct_sub.stop();
	ct_sub.get_time();

	// 游戏逻辑
	ct_sub.start();
	int tick_count= int(m_logic_timer.elapsed_time()/frame_time);
	if(tick_count>8) {
		m_metrics.m_tick_skipped_last += tick_count-8;
		tick_count = 8;
	}
	for(int i=0; i<tick_count; ++i) {
		on_logic(m_clock.time(),frame_time);
		m_metrics.m_tick_count+=1;
	}
	ct_sub.stop();
	m_metrics.m_logic_time_last+=ct_sub.get_time();

	// 处理异步任务
	if(m_worker_awaked.load(std::memory_order_acquire)) {
		m_end_task.store(true,std::memory_order_release);
		// 等待异步任务完成
		while(m_end_task.load(std::memory_order_acquire))
			SwitchToThread();
		m_worker_awaked.store(false,std::memory_order_relaxed);
		// 处理异步消息
		ct_sub.start();
		m_scheduler->listen();
		ct_sub.stop();
		ct_sub.get_time();
	}

	// 资源管理
	ct_sub.start();
	m_ressvr.update(frame_time);
	m_objctx.update();
	ct_sub.stop();
	ct_sub.get_time();

	// 等待渲染完成
	ct_sub.start();
	m_renderer->swap_buffer();
	// sync CPU wiht GPU (and the vertical blanks if vsync is on)
	if(have_state(m_gameState,XWIN_SYNC_RENDER_FRAME))
		m_renderer->wait_for_finish();
	ct_sub.stop();
	
	frame_time*=tick_count;
	m_logic_timer.consume(frame_time);
	ct_all.stop();
	m_metrics.m_cosumed_time_last+=ct_all.get_time();

	// 更新数据统计
	m_metrics.m_frame_count+=1;
	m_metrics.m_frame_count_last+=1;
	m_metrics.m_frame_time_last+=frame_time;
	if(m_metric_timer.elapsed_time()>=2.0) {
		if(m_metrics.m_frame_time_last>0)
			m_metrics.m_fps=m_metrics.m_frame_count_last/m_metrics.m_frame_time_last;
		update_metrics(m_metrics);
		m_metric_timer.reset();
		m_metrics.m_frame_time_last=0;
		m_metrics.m_frame_count_last=0;
		m_metrics.m_cosumed_time_last=0;
		m_metrics.m_render_time_last=0;
		m_metrics.m_logic_time_last=0;
		m_metrics.m_tick_skipped_last=0;
	}
	
}

} // namespace wyc


