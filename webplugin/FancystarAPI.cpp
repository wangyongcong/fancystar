/**********************************************************\

  Auto-generated FancystarAPI.cpp

\**********************************************************/
#include "FancystarPCH.h"

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"

#include "FancystarAPI.h"

///////////////////////////////////////////////////////////////////////////////
/// @fn FancystarAPI::FancystarAPI(const FancystarPtr& plugin, const FB::BrowserHostPtr host)
///
/// @brief  Constructor for your JSAPI object.  You should register your methods, properties, and events
///         that should be accessible to Javascript from here.
///
/// @see FB::JSAPIAuto::registerMethod
/// @see FB::JSAPIAuto::registerProperty
/// @see FB::JSAPIAuto::registerEvent
///////////////////////////////////////////////////////////////////////////////
FancystarAPI::FancystarAPI(const FancystarPtr& plugin, const FB::BrowserHostPtr& host) : m_plugin(plugin), m_host(host)
{
	registerMethod("load_game", make_method(this, &FancystarAPI::load_game));

	// properties
	registerProperty("info", make_property(this,&FancystarAPI::get_info));
	registerProperty("game_name", make_property(this,&FancystarAPI::get_game_name));

	// attributes
	registerAttribute("version", "0.1", true);
	registerAttribute("author", "ycwang", true);
	registerAttribute("engine", "Fancystar", true);
}

///////////////////////////////////////////////////////////////////////////////
/// @fn FancystarAPI::~FancystarAPI()
///
/// @brief  Destructor.  Remember that this object will not be released until
///         the browser is done with it; this will almost definitely be after
///         the plugin is released.
///////////////////////////////////////////////////////////////////////////////
FancystarAPI::~FancystarAPI()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @fn FancystarPtr FancystarAPI::getPlugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
FancystarPtr FancystarAPI::getPlugin()
{
    FancystarPtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}


//---------------------------------------
// properties 
//---------------------------------------

std::string FancystarAPI::get_game_name()
{
    return m_gameName;
}

std::string FancystarAPI::get_info()
{
    return "Powered by Fancystar Game Engine";
}

//---------------------------------------
// methods called by JavaScript
//---------------------------------------
bool FancystarAPI::load_game(const std::string &name)
{
	FancystarPtr pfs=m_plugin.lock();
	pfs->load_game(name);
	return true;
}

