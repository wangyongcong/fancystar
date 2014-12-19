#ifndef __HEADER_WYC_XWIN
#define __HEADER_WYC_XWIN

#include <atomic>
#include "wyc/util/util.h"
#include "wyc/util/circuque.h"
#include "wyc/obj/ressvr.h"
#include "wyc/thread/tls.h"
#include "wyc/render/renderer.h"
#include "wyc/game/input_win.h"
#include "wyc/game/frametime.h"
#include "wyc/game/game_metric.h"

namespace wyc
{

class xgame
{
protected:
	static xthread_local ms_tlsGameContext;
	static unsigned ms_processState;
	typedef std::vector<std::pair<HWND,xgame*> > xgame_window_map_t;
	static xgame_window_map_t ms_gameWinMap;
	static wyc::xcritical_section ms_globalLock;

	// windows system
	HINSTANCE m_hInstance;
	HWND m_hMainWnd;
	HACCEL m_hAccel;
	HWND m_hTargetWnd;
	HANDLE m_hGameThread;
	unsigned m_tidGame;
	HANDLE m_hWorkerThread;
	unsigned m_tidWorker;

	// game state
	enum GAMESTATE
	{
		XWIN_NOTINIT=0,
		XWIN_QUIT=1,
		XWIN_FULL_SCREEN=2,
		XWIN_DEBUG_MODE=4,
		XWIN_LOCK_FRAME=8,
		XWIN_CAPTURED_MOUSE=0x10,
		XWIN_AS_CHILDREN=0x20,
		XWIN_SYNC_RENDER_FRAME=0x40,
		XWIN_SEPARATED_GAME_THREAD=0x80,
	};
	uint32_t m_gameState;
	// input setting
	xinput m_input;
	// display setting
	xpointer<xrenderer> m_renderer;
	int	m_nWidth, m_nHeight;
	// game setting
	std::wstring m_app_name;
	std::wstring m_app_file;
	std::wstring m_work_path;
	std::wstring m_home_path;
	std::string m_code_name;
	std::string m_res_path;
	// objct context
	xobject_context m_objctx;
	// async task manager
	xworker_server m_worker;
	xscheduler *m_scheduler;
	std::atomic_bool m_worker_awaked;
	std::atomic_bool m_end_task;
	std::atomic_bool m_shutdown_worker;
	// resource manager
	xressvr m_ressvr;
	// game time
	xframe_clock m_clock;
	xframe_timer m_logic_timer, m_metric_timer;
	double m_logic_period; 
	// metrics
	xgame_metric m_metrics;
public:
	/// 游戏进程初始化
	static bool init_game_process();
	/// 结束游戏进程
	static void exit_game_process();
	/// 游戏进程是否已经初始化
	static bool game_process_ready();
	/// 游戏窗口管理
	static bool add_game_window(HWND hwnd, xgame *pgame);
	static void remove_game_window(HWND hwnd);
	static xgame* get_game_instance(HWND hwnd);
	/// 创建游戏对象
	static xgame* create_game();
	/// 获取全局游戏对象
	static xgame& singleton();
	xgame();
	virtual ~xgame();
	/// 创建游戏窗口
	bool create(HINSTANCE hInstance, HWND hMainWnd, const wchar_t* strAppTitle, \
		int x, int y, int width, int height, bool bFullScreen);
	/// 运行游戏
	int run();
	/// 退出游戏
	void exit();
	HINSTANCE get_instance() const;
	HWND main_window() const;
	HWND target_window() const;
	xinput& input();
	void set_app_name(const wchar_t *app_name);
	const std::wstring& app_name() const;
	const std::wstring& app_file() const;
	const std::wstring& work_path() const;
	const std::wstring& home_path() const;
	void set_code_name(const char *code_name);
	const std::string& code_name() const;
	void set_resource_path(const char *path);
	void get_resource_path(std::string &filename) const;
	int client_width() const;
	int client_height() const;
	bool fullscreen() const;
	void set_fullscreen(bool b);
	bool debug_mode() const;
	void set_debug(bool b);
	bool is_quit() const;
	bool is_active() const;
	bool is_maximized() const;
	bool is_minimized() const;
	bool is_child() const;
	void capture_mouse();
	void release_mouse();
	void activate_window();
	void deactivate_window();
	xrenderer* get_renderer();
	xressvr* get_resource_server();
	bool send_async_task(worker_function_t func, xrefobj *task=0, worker_callback_t cb=0, int hint=0);
	const xgame_metric& get_metrics() const;
protected:
	static HWND create_game_window(HINSTANCE hInstance, const wchar_t* strAppTitle, int width, int height, bool bFullScreen);
	static HWND create_target_window(HINSTANCE hInstance, HWND hParent, const wchar_t *name, int x, int y, unsigned width, unsigned height);
	static LRESULT WINAPI window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static unsigned int WINAPI game_proc(void* pvParam);
	static unsigned int WINAPI game_worker_proc(void* pvParam);
	static xobject_context* get_object_context();
	// No copy operation
	xgame(const xgame&);
	xgame& operator= (const xgame&);
	void init_work_path();
	bool init_game_thread();
	void exit_game_thread();
	bool init_game();
	void quit_game();
	void process_logic();
	/// 自定义消息处理(窗口线程)
	virtual bool on_window_message(UINT, WPARAM, LPARAM, LRESULT&) {return false;}
	/// 游戏初始化
	virtual bool on_game_init() {return true;}
	/// 离开游戏
	virtual void on_game_exit() {}
	/// 输入处理回调
	virtual void on_input(const xinput_buffer &input) {input;}
	/// 逻辑回调
	virtual void on_logic(double accum_time, double frame_time) {accum_time,frame_time;}
	/// 渲染回调
	virtual void on_render(double accum_time, double frame_time) {accum_time,frame_time;}
	/// 询问是否离开游戏
	virtual bool query_exit() {return true;}
	/// 更新游戏统计数据
	virtual void update_metrics(const xgame_metric&) {}
};

}; // namespace wyc

#include "wyc/game/game.inl"

#endif // end of __HEADER_WYC_XWIN


