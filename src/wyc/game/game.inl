#ifndef __INLINE_WYC_XWIN
#define __INLINE_WYC_XWIN

namespace wyc
{

inline xgame& xgame::singleton() {
	xgame *pGame=(xgame*)ms_tlsGameContext.get_data();
	assert(pGame);
	return *pGame;
}

inline void xgame::exit() {
	if(!is_child()) 
		PostMessage(m_hMainWnd,WM_CLOSE,0,0);
	else exit_game_thread();
}

inline bool xgame::is_quit() const {
	return have_state(m_gameState,XWIN_QUIT);
}

inline bool xgame::is_active() const {
	return m_hMainWnd==::GetForegroundWindow();
}

inline bool xgame::is_maximized() const {
	return ::IsZoomed(m_hMainWnd)?true:false;
}

inline bool xgame::is_minimized() const {
	return ::IsIconic(m_hMainWnd)?true:false;
}

inline bool xgame::is_child() const {
	return have_state(m_gameState,XWIN_AS_CHILDREN);
}

inline HINSTANCE xgame::get_instance() const {
	return m_hInstance;
}

inline HWND xgame::main_window() const {
	return m_hMainWnd;
}

inline HWND xgame::target_window() const {
	return m_hTargetWnd;
}

inline void xgame::set_app_name(const wchar_t *app_name) {
	m_app_name=app_name;
}

inline const std::wstring& xgame::app_name() const {
	return m_app_name;
}

inline void xgame::set_code_name(const char *code_name) {
	m_code_name=code_name;
}

inline const std::string& xgame::code_name() const {
	return m_code_name;
}

inline int xgame::client_width() const {
	return m_nWidth;
}

inline int xgame::client_height() const {
	return m_nHeight;
}

inline xinput& xgame::input()
{
	return m_input;
}

inline bool xgame::fullscreen() const
{
	return wyc::have_state(m_gameState,XWIN_FULL_SCREEN);
}

inline void xgame::set_fullscreen(bool b)
{
	b?wyc::add_state(m_gameState,XWIN_FULL_SCREEN):wyc::remove_state(m_gameState,XWIN_FULL_SCREEN);
}

inline bool xgame::debug_mode() const
{
	return wyc::have_state(m_gameState,XWIN_DEBUG_MODE);
}

inline void xgame::set_debug(bool b)
{
	b?wyc::add_state(m_gameState,XWIN_DEBUG_MODE):wyc::remove_state(m_gameState,XWIN_DEBUG_MODE);
}

inline xrenderer* xgame::get_renderer()
{
	return m_renderer;
}

inline const std::wstring& xgame::app_file() const {
	return m_app_file;
}

inline const std::wstring& xgame::work_path() const {
	return m_work_path;
}

inline const std::wstring& xgame::home_path() const {
	return m_home_path;
}

inline bool xgame::send_async_task(worker_function_t func, xrefobj *task, worker_callback_t cb, int hint)
{
	if(!m_worker_awaked.exchange(true,std::memory_order_acq_rel)) 
	{
		ResumeThread(m_hWorkerThread);
	}
	return m_scheduler->add_task(func,task,cb,hint);
}

inline xressvr* xgame::get_resource_server()
{
	return &m_ressvr;
}

inline const xgame_metric& xgame::get_metrics() const
{
	return m_metrics;
}

}; // namespace wyc

#endif // __INLINE_WYC_XWIN
