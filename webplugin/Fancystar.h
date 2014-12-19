/**********************************************************\

  Auto-generated Fancystar.h

  This file contains the auto-generated main plugin object
  implementation for the Fancystar project

\**********************************************************/
#ifndef H_FancystarPLUGIN
#define H_FancystarPLUGIN

#include "PluginWindow.h"
#include "PluginEvents/MouseEvents.h"
#include "PluginEvents/KeyboardEvents.h"
#include "PluginEvents/AttachedEvent.h"
#include "PluginEvents/DrawingEvents.h"
#include "PluginCore.h"

FB_FORWARD_PTR(Fancystar)
class Fancystar : public FB::PluginCore
{
	void *m_hGameContext;
	HMODULE m_hGameLoader;
	enum MOUSE_STATE 
	{
		LEFT_BUTTON=0,
		RIGHT_BUTTON,
		MIDDLE_BUTTON,
		BUTTON_DOWN=0x10,
	};
	typedef void (*pfnOnMouseButton) (void*,int,int,int);
	pfnOnMouseButton on_mouse_button;
	typedef void (*pfnOnMouseMove) (void*,int,int);
	pfnOnMouseMove on_mouse_move;
	typedef void (*pfnOnKeyEvent) (void *,unsigned); 
	pfnOnKeyEvent on_key_down;
	pfnOnKeyEvent on_key_up;
	static std::wstring m_gamePath;
public:
    static void StaticInitialize();
    static void StaticDeinitialize();
public:
    Fancystar();
    virtual ~Fancystar();
public:
    void onPluginReady();
    void shutdown();
    virtual FB::JSAPIPtr createJSAPI();
    // If you want your plugin to always be windowless, set this to true
    // If you want your plugin to be optionally windowless based on the
    // value of the "windowless" param tag, remove this method or return
    // FB::PluginCore::isWindowless()
    virtual bool isWindowless() { return false; }
	
	bool load_game(const std::string &name);


    BEGIN_PLUGIN_EVENT_MAP()
        EVENTTYPE_CASE(FB::KeyDownEvent, onKeyDown, FB::PluginWindow)
        EVENTTYPE_CASE(FB::KeyUpEvent, onKeyUp, FB::PluginWindow)
        EVENTTYPE_CASE(FB::MouseDownEvent, onMouseDown, FB::PluginWindow)
        EVENTTYPE_CASE(FB::MouseUpEvent, onMouseUp, FB::PluginWindow)
        EVENTTYPE_CASE(FB::MouseMoveEvent, onMouseMove, FB::PluginWindow)
        EVENTTYPE_CASE(FB::MouseMoveEvent, onMouseMove, FB::PluginWindow)
        EVENTTYPE_CASE(FB::AttachedEvent, onWindowAttached, FB::PluginWindow)
        EVENTTYPE_CASE(FB::DetachedEvent, onWindowDetached, FB::PluginWindow)
        EVENTTYPE_CASE(FB::RefreshEvent, onWindowRefresh, FB::PluginWindow)
    END_PLUGIN_EVENT_MAP()

    /** BEGIN EVENTDEF -- DON'T CHANGE THIS LINE **/
    virtual bool onKeyDown(FB::KeyDownEvent *evt, FB::PluginWindow *);
    virtual bool onKeyUp(FB::KeyUpEvent *evt, FB::PluginWindow *);
    virtual bool onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *);
    virtual bool onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *);
    virtual bool onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *);
    virtual bool onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *);
    virtual bool onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *);
    virtual bool onWindowRefresh(FB::RefreshEvent *evt, FB::PluginWindow *);
    /** END EVENTDEF -- DON'T CHANGE THIS LINE **/
};


#endif

