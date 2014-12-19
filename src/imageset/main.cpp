#include "winpch.h"
#include "xutil.h"
#include "xconsole.h"
#include "psd_loader.h"

using namespace wyc;

#pragma warning (disable : 4819)

#ifdef _DEBUG
	#pragma comment (lib, "fsutil_d.lib")
	#pragma comment (lib, "fshash_d.lib")
	#pragma comment (lib, "fsstring_d.lib")
	#pragma comment (lib, "fsconsole_d.lib")
#else
	#pragma comment (lib, "fsutil.lib")
	#pragma comment (lib, "fshash.lib")
	#pragma comment (lib, "fsstring.lib")
	#pragma comment (lib, "fsconsole.lib")
#endif

int GenerateImageset(const char *argv[], int argn)
{
	std::string input, output;
	if(argn<2)
		return 1;
	input=argv[1];
	if(input.empty()) 
		return 2;
	if(argn<3) {
		std::string::size_type pos=input.rfind('.');
		if(pos==std::string::npos || pos==0)
			output=input;
		else {
			output=input.substr(0,pos);
		}
		output+=".imageset";
	}
	else output=argv[2];
	xpsd_loader loader;
	if(!loader.load(input.c_str()))
		return 3;
	if(!loader.save_as_imageset(output.c_str()))
		return 4;
	return 0;
}

int __stdcall wWinMain (HINSTANCE, HINSTANCE, LPTSTR, int)
{
	// 创建控制台
	wyc::xconsole::create_console();
	wyc::xconsole &console=wyc::xconsole::singleton();
	console.regcmd("imageset",(wyc::xconsole::pfnCommandHandler)&GenerateImageset);
	console.process_input();
	return 0;
}


