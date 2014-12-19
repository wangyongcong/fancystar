#ifndef __HEADER_WYC_PICLOUD_CLIENT
#define __HEADER_WYC_PICLOUD_CLIENT

#include <windows.h>
#include <wininet.h>

namespace wyc
{

bool initialize_internet_connection(const wchar_t *client_name);
void cleanup_internet_connection();

class xpicloud_client
{
	HINTERNET m_hconnect, m_hrequest;
public:
	xpicloud_client();
	~xpicloud_client();	
	bool load_config(const char *cfg);
	bool connect();
	void disconnect();
	bool send();
	void on_handle_created(DWORD_PTR handle);
	void on_request_complete();
};

}; // namespace wyc


#endif // __HEADER_WYC_PICLOUD_CLIENT


