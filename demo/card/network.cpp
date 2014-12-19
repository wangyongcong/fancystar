#include "network.h"
#include "wyc/util/util.h"

namespace wyc
{

typedef void (xcard_network::*pfn_response_handler)(const ExitGames::OperationResponse&);

typedef void (xcard_network::*pfn_event_handler)(const ExitGames::EventData&); 

struct op_slot_t
{
	int code;
	pfn_response_handler handler;
};

struct ev_slot_t
{
	int code;
	pfn_event_handler handler;
};

xcard_network::xcard_network()
{
	static const op_slot_t ls_response[] = 
	{
		{OP_ECHO, &xcard_network::ret_echo},
		{OP_LOGIN, &xcard_network::ret_login},
	};

/*	static const ev_slot_t ls_events[] = 
	{
		{EV_LOGIN_RESULT, &xcard_network::ev_login_result},
	};*/

	m_peer = 0;
	m_state= 0;
	m_cb_connect = 0;
	m_cb_login = 0;

	int count = sizeof(ls_response)/sizeof(op_slot_t);
	m_response_map.reserve(count);
	for(int i=0; i<count; ++i)
	{
		m_response_map.add(ls_response[i].code,(void*)(ls_response+i));
	}

/*	count = sizeof(ls_events)/sizeof(ev_slot_t);
	m_event_map.reserve(count);
	for(int i=0; i<count; ++i)
	{
		m_event_map.add(ls_events[i].code,(void*)(ls_events+i));
	}*/
}

xcard_network::~xcard_network()
{
	if(m_peer)
		delete m_peer;
}

void xcard_network::debugReturn(PhotonPeer_DebugLevel debugLevel, const ExitGames::JString& string)
{
	switch(debugLevel)
	{
	case DEBUG_LEVEL_ERRORS:
		wyc_error("[PHOTON] %s",string.ANSIRepresentation().cstr());
		break;
	case DEBUG_LEVEL_WARNINGS:
		wyc_warn("[PHOTON] %s",string.ANSIRepresentation().cstr());
		break;
	default:
		wyc_print("[PHOTON] %s",string.ANSIRepresentation().cstr());		
	}
}

void xcard_network::onStatusChanged(int statusCode)
{
	switch(statusCode)
	{
	case SC_CONNECT:
		set_connect_state(CONNECTED);
		wyc_print("[PHOTON] peer connected");
		if(m_cb_connect) {
			pfn_network_callback cb = m_cb_connect;
			m_cb_connect = 0;
			cb(0);
		}
		break;
	case SC_DISCONNECT:
		set_connect_state(DISCONNECTED);
		wyc_print("[PHOTON] peer disconnected");
		if(m_cb_connect) {
			pfn_network_callback cb = m_cb_connect;
			m_cb_connect = 0;
			cb(1);
		}
		break;
	default:
		break;
	}
}

void xcard_network::onOperationResponse(const ExitGames::OperationResponse& operationResponse)
{
	int code = operationResponse.getOperationCode();
	op_slot_t *op = (op_slot_t*) m_response_map.get(code);
	if(!op) {
		wyc_warn("[PHOTON] unknown response, code=%d",code);
		return;
	}
	(this->*(op->handler))(operationResponse);
}

void xcard_network::onEvent(const ExitGames::EventData& eventData)
{
	int code = eventData.getCode();
	wyc_print("[PHOTON] receive event, code=%d",code);
	ev_slot_t *ev = (ev_slot_t*) m_event_map.get(code);
	if(!ev) {
		wyc_warn("[PHOTON] unknown event, code=%d",code);
		return;
	}
	(this->*(ev->handler))(eventData);
}

bool xcard_network::connect(const char *address, pfn_network_callback cb)
{
	if(is_connected() || is_connecting())
		return true;
	if(!m_peer)
		m_peer = new ExitGames::LitePeer(this);
	nByte app_name[32];
	memset(app_name,0,sizeof(app_name));
	strcpy_s((char*)app_name,32,"demo-card");
	if(!m_peer->connect(address,app_name))
		return false;
	set_connect_state(CONNECTING);
	m_cb_connect=cb;
	wyc_print("[PHOTON] connecting server...");
	return true;
}

void xcard_network::disconnect()
{
	if(is_disconnecting() || is_disconnected())
		return;
	if(!m_peer) 
		return;
	m_peer->disconnect();
	set_connect_state(DISCONNECTING);
	wyc_print("[PHOTON] disconnecting...");
}

void xcard_network::update()
{
	if(m_peer) m_peer->service(true);
}

bool xcard_network::rpc_login(const char *user, const char *pwd, pfn_network_callback cb)
{
	ExitGames::OperationRequest req(OP_LOGIN);
	ExitGames::Hashtable &params = req.getParameters();
	params.put(ExitGames::KeyObject<nByte>(nByte(0)),ExitGames::ValueObject<ExitGames::JString>(user));
	params.put(ExitGames::KeyObject<nByte>(nByte(1)),ExitGames::ValueObject<ExitGames::JString>(pwd));
	m_cb_login = cb;
	return m_peer->opCustom(req,true);
}

void xcard_network::ret_login(const ExitGames::OperationResponse& response)
{
	const ExitGames::Object *obj = &response.getParameterForCode(0);
	int err=-1;
	if(obj && EG_INTEGER==obj->getType()) {
		err = *(int*)obj->getData();
	}
	if(m_cb_login) {
		pfn_network_callback cb = m_cb_login;
		m_cb_login=0;
		cb(err);
	}
	wyc_print("[PHOTON] login response: %d",err);
}

bool xcard_network::rpc_logout()
{
	ExitGames::OperationRequest req(OP_LOGOUT);
	return m_peer->opCustom(req,true);
}

bool xcard_network::rpc_echo(const char *words)
{
	ExitGames::OperationRequest req(OP_ECHO);
	ExitGames::Hashtable &ht = req.getParameters();
	ht.put(ExitGames::KeyObject<nByte>(nByte(0)),ExitGames::ValueObject<ExitGames::JString>(words));
	return m_peer->opCustom(req,true);
}

void xcard_network::ret_echo(const ExitGames::OperationResponse& response)
{
	const ExitGames::Object *obj = &response.getParameterForCode(0);
	if(obj && EG_STRING==obj->getType()) {
		ExitGames::JString *s = (ExitGames::JString*)obj->getData();
		wyc_logw(wyc::LOG_NORMAL,L"[PHOTON] echo: %s",s->cstr());
	}
}

} // namespace wyc
