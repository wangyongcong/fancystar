#ifndef __HEADER_WYC_GLB_GAME
#define __HEADER_WYC_GLB_GAME

#include "wyc/obj/ressvr.h"

namespace wyc
{

/// get global resource path
void get_resource_path(std::string &file_path);

/// get global resource server
xressvr* get_resource_server();

}; // namespace wyc

#endif // __HEADER_WYC_GLB_GAME
