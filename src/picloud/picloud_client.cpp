#include "picloud/picloud_client.h"
#include <cassert>
#include "wyc/util/util.h"
#include "wyc/util/base64.h"

namespace wyc
{

HINTERNET s_internet_handle=NULL;

void CALLBACK http_callback(
	HINTERNET hInternet,
	DWORD_PTR dwContext,
	DWORD dwInternetStatus,
	LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength
) 
{
	switch(dwInternetStatus)
	{
	case INTERNET_STATUS_COOKIE_SENT:
		wyc_print("[NET] Status: Cookie found and will be sent with request");
		break;
            
	case INTERNET_STATUS_COOKIE_RECEIVED:
		wyc_print("[NET] Status: Cookie Received");
		break;
            
	case INTERNET_STATUS_COOKIE_HISTORY:
		wyc_print("[NET] Status: Cookie History");
		break;
            
	case INTERNET_STATUS_CLOSING_CONNECTION:
		wyc_print("[NET] Status: Closing Connection");
		break;
            
	case INTERNET_STATUS_CONNECTED_TO_SERVER:
		wyc_print("[NET] Status: Connected to Server");
		break;
            
	case INTERNET_STATUS_CONNECTING_TO_SERVER:
		wyc_print("[NET] Status: Connecting to Server");
		break;
            
	case INTERNET_STATUS_CONNECTION_CLOSED:
		wyc_print("[NET] Status: Connection Closed");
		break;
            
	case INTERNET_STATUS_HANDLE_CLOSING:
		wyc_print("[NET] Status: Handle Closing");
		break;
            
	case INTERNET_STATUS_HANDLE_CREATED:
		assert(lpvStatusInformation);
		wyc_print("[NET] Handle created [%X]", ((LPINTERNET_ASYNC_RESULT)lpvStatusInformation)->dwResult);
		assert(dwContext);
		((xpicloud_client*)dwContext)->on_handle_created(((LPINTERNET_ASYNC_RESULT)lpvStatusInformation)->dwResult);
		break;
            
	case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
		wyc_print("[NET] Status: Intermediate response");
		break;
            
	case INTERNET_STATUS_RECEIVING_RESPONSE:
		wyc_print("[NET] Status: Receiving Response");    
		break;
            
	case INTERNET_STATUS_RESPONSE_RECEIVED:
		assert(lpvStatusInformation);
		assert(dwStatusInformationLength == sizeof(DWORD));
		wyc_print("[NET] Status: Response Received (%d Bytes)", *((LPDWORD)lpvStatusInformation));    
		break;

	case INTERNET_STATUS_REDIRECT:
		wyc_print("[NET] Status: Redirect");
		break;

	case INTERNET_STATUS_REQUEST_COMPLETE:
		wyc_print("[NET] Status: Request complete, success=%s",
			(((LPINTERNET_ASYNC_RESULT)lpvStatusInformation)->dwError==ERROR_SUCCESS)?"True":"False");
		assert(dwContext);
		((xpicloud_client*)dwContext)->on_request_complete();
		break;
            
	case INTERNET_STATUS_REQUEST_SENT:
		assert(lpvStatusInformation);
		assert(dwStatusInformationLength == sizeof(DWORD));
		wyc_print("[NET] Status: Request sent (%d Bytes)", *((LPDWORD)lpvStatusInformation));
		break;
            
	case INTERNET_STATUS_DETECTING_PROXY:
		wyc_print("[NET] Status: Detecting Proxy");
		break;            
            
	case INTERNET_STATUS_RESOLVING_NAME:
		wyc_print("[NET] Status: Resolving Name");
		break;
            
	case INTERNET_STATUS_NAME_RESOLVED:
		wyc_print("[NET] Status: Name Resolved");
		break;
            
	case INTERNET_STATUS_SENDING_REQUEST:
		wyc_print("[NET] Status: Sending request");
		break;
            
	case INTERNET_STATUS_STATE_CHANGE:
		wyc_print("[NET] Status: State Change");
		break;
            
	case INTERNET_STATUS_P3P_HEADER:
		wyc_print("[NET] Status: Received P3P header");
		break;
            
	default:
		wyc_print("[NET] Status: Unknown (%d)", dwInternetStatus);
		break;
	}
}


bool initialize_internet_connection(const wchar_t *client_name)
{
	if(NULL==s_internet_handle) {
		s_internet_handle = InternetOpen(client_name,INTERNET_OPEN_TYPE_DIRECT,NULL,NULL,INTERNET_FLAG_ASYNC);
		if(NULL==s_internet_handle)
			return false;
		InternetSetStatusCallback(s_internet_handle, (INTERNET_STATUS_CALLBACK)http_callback);
	}
	return true;
}

void cleanup_internet_connection()
{
	if(NULL!=s_internet_handle)
		InternetCloseHandle(s_internet_handle);
}

inline HINTERNET get_internet_handle()
{
	return s_internet_handle;
}

void log_internet_error()
{
	wchar_t info[512];
	DWORD size=512;
	DWORD error;
	if(!InternetGetLastResponseInfo(&error,info,&size)) {
		if(ERROR_INSUFFICIENT_BUFFER==GetLastError())
		{
			size+=1;
			wchar_t *buff = new wchar_t[size];
			if(InternetGetLastResponseInfo(&error,buff,&size))
				wyc_logw(wyc::LOG_ERROR,buff);
			delete [] buff;
			return;
		}
	}
	wyc_logw(wyc::LOG_ERROR,info);
}

void log_windows_error() 
{ 
	// Retrieve the system error message for the last-error code
	LPTSTR error_msg;
	DWORD error_code = GetLastError(); 
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &error_msg,
		0, NULL );
	wyc_logw(wyc::LOG_ERROR,L"[%d] %s",error_code,error_msg);
	LocalFree(error_msg);
}

xpicloud_client::xpicloud_client()
{
	m_hconnect=NULL;
	m_hrequest=NULL;
}

xpicloud_client::~xpicloud_client()
{
}

void xpicloud_client::disconnect()
{
	if(m_hrequest) {
		InternetCloseHandle(m_hrequest);
		m_hrequest=NULL;
	}
	if(m_hconnect) {
		InternetCloseHandle(m_hconnect);
		m_hconnect=NULL;
	}
}

bool xpicloud_client::connect()
{
	HINTERNET hnet = get_internet_handle();
	if(NULL==hnet) {
		return false;
	}
	std::wstring s = L"Authorization: Basic NDc0NjoxZDZkY2RlMjEwZjc5MGZiOTQ1MDFiN2I0NGNiN2ZiNTZlNWU3MTgy\r\n";
	if(NULL==InternetOpenUrl(hnet,L"https://api.picloud.com/r/4896/ping/",s.c_str(),s.length(),
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE,
		(DWORD_PTR)this))
	{
		log_windows_error();
		return false;
	}
	return true;

	if(m_hconnect)
		// close the last handle
		InternetCloseHandle(m_hconnect);
	m_hconnect = InternetConnect(hnet,L"https://api.picloud.com/r/4896/ping/",INTERNET_DEFAULT_HTTPS_PORT,L"",L"",INTERNET_SERVICE_HTTP,0,(DWORD_PTR)this);
	if(NULL==m_hconnect) {
		log_internet_error();
		return false;
	}
	m_hrequest = HttpOpenRequest(m_hconnect,L"GET",L"",NULL,NULL,NULL,INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE,(DWORD_PTR)this);
	if(!m_hrequest)
	{
		log_windows_error();
		return false;
	}
	if(!HttpAddRequestHeaders(m_hrequest,s.c_str(),s.length(),HTTP_ADDREQ_FLAG_ADD))
	{
		log_windows_error();
		return false;
	}
	return true;
}

bool xpicloud_client::send()
{
	return true;
	if(!HttpSendRequest(m_hrequest,NULL,0,NULL,0)) {
		if(GetLastError()!=ERROR_IO_PENDING) {
			log_windows_error();
			return false;
		}
	}
	return true;
}

void xpicloud_client::on_request_complete()
{
	if(!m_hrequest)
		return;
	std::string ret;
	char buff[1024];
	DWORD size=1022, index=0, read, total_read=0;
	if(!HttpQueryInfo(m_hrequest,HTTP_QUERY_RAW_HEADERS_CRLF,buff,&size,&index))
		log_windows_error();
	else {
		const wchar_t *header = (wchar_t*)buff;
		wyc_logw(LOG_NORMAL,L"[NET] HTTP header:\n%s",header);
	}
	bool not_done=true;
	
	while(not_done) {
		INTERNET_BUFFERS inet_buffer;
		memset(&inet_buffer,0,sizeof(inet_buffer));
		inet_buffer.dwStructSize = sizeof(inet_buffer);
		inet_buffer.lpvBuffer=buff;
		inet_buffer.dwBufferLength=size-1;
		if(!InternetReadFileEx(m_hrequest,&inet_buffer,IRF_ASYNC,(DWORD_PTR)this)) {
			if(GetLastError() == ERROR_IO_PENDING)
				WaitForSingleObject(m_hrequest,INFINITE);
			else {
				log_windows_error();
				return;
			}
		}
		if(inet_buffer.dwBufferLength) {
			buff[inet_buffer.dwBufferLength]=0;
			wyc_print("[NET] Server response:\n%s",buff);
		}
		else 
			not_done=false;
	}
}

void xpicloud_client::on_handle_created(DWORD_PTR handle) 
{
	m_hrequest=(HINTERNET)handle;
}



}; // namespace wyc

