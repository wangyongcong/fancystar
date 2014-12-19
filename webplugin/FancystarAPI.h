/**********************************************************\

  Auto-generated FancystarAPI.h

\**********************************************************/

#include <string>
#include <sstream>
#include <boost/weak_ptr.hpp>
#include "JSAPIAuto.h"
#include "BrowserHost.h"
#include "Fancystar.h"

#ifndef H_FancystarAPI
#define H_FancystarAPI

class FancystarAPI : public FB::JSAPIAuto
{
public:
    FancystarAPI(const FancystarPtr& plugin, const FB::BrowserHostPtr& host);
    virtual ~FancystarAPI();

    FancystarPtr getPlugin();

	//---------------------------------------
	// properties 
	//---------------------------------------
	std::string get_info();
	std::string get_game_name();

	//---------------------------------------
    // JavaScript events
	//---------------------------------------
    FB_JSAPI_EVENT(echo, 1, (const FB::variant&));

	//---------------------------------------
    // methods called by JavaScript
	//---------------------------------------
	bool load_game(const std::string &name);


private:
    FancystarWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;

    std::string m_gameName;
};

#endif // H_FancystarAPI

