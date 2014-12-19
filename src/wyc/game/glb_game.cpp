#include "fscorepch.h"
#include "wyc/game/game.h"
#include "wyc/game/glb_game.h"

void link_inner_module()
{
	IMPORT_MODULE(texture);
	IMPORT_MODULE(vertexbatch);
	IMPORT_MODULE(font);
}

GLEWContext* glewGetContext() 
{
	return wyc::xgame::singleton().get_renderer()->glew_context();
}

WGLEWContext* wglewGetContext()
{
	return wyc::xgame::singleton().get_renderer()->wglew_context();
}

namespace wyc
{

void get_resource_path(std::string &file_path)
{
	return xgame::singleton().get_resource_path(file_path);
}

xressvr* get_resource_server()
{
	return xgame::singleton().get_resource_server();
}

bool send_async_task(worker_function_t func, xrefobj *task, worker_callback_t cb, int hint)
{
	return xgame::singleton().send_async_task(func,task,cb,hint);
}

}; // namespace wyc
