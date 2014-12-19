#ifndef __HEADER_WYC_NETWORK
#define __HEADER_WYC_NETWORK

#include "wyc/util/hash.h"

#include "LitePeer.h"
#include "protocol.h"

namespace wyc
{

typedef void (*pfn_network_callback) (int);

class xcard_network : public ExitGames::PhotonListener
{
public:
	xcard_network();
	~xcard_network();
	bool connect(const char *address, pfn_network_callback cb=0);
	void disconnect();
	void update();

    /* Summary
        called by the library as callback for debug messages in error
        case.
        Parameters
        debugLevel: the debug level, the message was created with
        string : the debug message
        Returns
        Nothing.                                                      
	*/
	virtual void debugReturn(PhotonPeer_DebugLevel debugLevel, const ExitGames::JString& string);

	/* Summary
	    called by the library as callback to operations. See <link CB_ON_OPERATION_RESPONSE>.
	    Parameters
	    operationResponse : operationResponse
	    Returns
	    Nothing.                                                                                                  
	*/
	virtual void onOperationResponse(const ExitGames::OperationResponse& operationResponse);

	/* Summary
	    called by the library as callback for peer
	    state-changes and errors. See <link CB_ON_STATUS_CHANGED>.
	    Parameters
	    statusCode :    the status code
	    Returns
	    Nothing.                                                                                                  
	*/
	virtual void onStatusChanged(int statusCode);

	/* Summary
	    called by the library as callback for events coming in. See <link CB_ON_EVENT>.
	    Parameters
	    event data : the event.
	    Returns
	    Nothing.                                                                                              
	*/
	virtual void onEvent(const ExitGames::EventData& eventData);


//-------------------------------------------
// Network state access
//-------------------------------------------

	bool is_connected() const
	{
		return (m_state & CONNECTED) == CONNECTED;
	}

	bool is_connecting() const
	{
		return (m_state & CONNECTING) == CONNECTING;
	}

	bool is_disconnected() const
	{
		return (m_state & CONNECT_STATE_MASK) == 0;
	}

	bool is_disconnecting() const
	{
		return (m_state & DISCONNECTING) == DISCONNECTING;
	}

	void set_connect_state(unsigned st)
	{
		m_state &= ~CONNECT_STATE_MASK;
		m_state |= st;
	}


//-------------------------------------------
// RPCs
//-------------------------------------------
	bool rpc_echo(const char *words);
	void ret_echo(const ExitGames::OperationResponse& response);
	bool rpc_login(const char *user, const char *pwd, pfn_network_callback cb=0);
	void ret_login(const ExitGames::OperationResponse& response);
	bool rpc_logout();

//-------------------------------------------
// Server events
//-------------------------------------------

private:
	ExitGames::LitePeer *m_peer;
	enum NETWORK_STATE
	{
		// connect state
		CONNECT_STATE_MASK = 3,
		DISCONNECTED = 0,
		CONNECTED = 1,
		DISCONNECTING = 2,
		CONNECTING= 3,
	};
	unsigned m_state;
	xdict m_response_map;
	xdict m_event_map;
	pfn_network_callback m_cb_connect;
	pfn_network_callback m_cb_login;
};

} // namespace wyc

#endif // __HEADER_WYC_NETWORK
