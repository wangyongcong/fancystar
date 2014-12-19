/* Exit Games Photon - C++ Client Lib
 * Copyright (C) 2004-2012 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#ifndef __PHOTON_PEER_H
#define __PHOTON_PEER_H

#include "PhotonListener.h"
#include "Utils.h"
#include "Hashtable.h"
#include "JVector.h"
#include "UTF8String.h"
#include "ANSIString.h"
#include "CustomType.h"
#include "OperationResponse.h"
#include "EventData.h"
#include "OperationRequest.h"

#ifndef _EG_BREW_PLATFORM
namespace ExitGames
{
#endif
	/* <title Photon: PhotonPeer class>
	   <toctitle Photon: PhotonPeer class>
	   
	   Summary
	   The PhotonPeer class provides a channel for reliable or
	   unreliable communication based on UDP.
	   Description
	   This class encapsulates Photon-related functions of the
	   public API as described in section <link UDP based functions, Photon functions>.
	   
	   PhotonPeer uses the callback interface <link PhotonListener>
	   that needs to be implemented by your application, to receive
	   results and events from the Photon Server.
	   
	   
	   
	   <b>How to establish a connection and to send data using the
	   PhotonPeer class:</b>
	   
	   
	   
	   \1. create an PhotonPeer instance.
	   
	   \2. PhotonPeer::Connect() to connect to the server;
	   
	   \3. regularly call PhotonPeer::service() to get new events
	   and to send commands. (best called within the game loop!)
	   
	   \4. wait for a callback to PhotonListener::PhotonPeerReturn()
	   with returnCode: SC_CONNECT;
	   
	   \5. call PhotonPeer::opJoin() to get into a game
	   
	   6 wait for "joined" return in
	   PhotonListener::PhotonPeerReturn() with opCode: OPC_RT_JOIN;
	   
	   \7. send in-game data by calling PhotonPeer::opRaiseEvent()
	   See <link MemoryManagement, Sending and receiving data> for
	   more information about Photon's serializable data structures
	   
	   \8. receive events by the
	   PhotonListener::PhotonPeerEventAction() callback
	   
	   \9. call PhotonPeer::opLeave() to quit/leave the game.
	   
	   \10. wait for callback PhotonListener::PhotonPeerReturn()
	   with opCode: OPC_RT_LEAVE;
	   
	   \11. disconnect by calling PhotonPeer::Disconnect()
	   
	   \12. check "disconnect" return in
	   PhotonListener::PhotonPeerReturn() with returnCode:
	   SC_DISCONNECT                                                                 */
	class PhotonPeer
	{
	public:
		#ifdef _EG_BREW_PLATFORM
		PhotonPeer(PhotonListener* listener, PlatformSpecific* pPlatform);
		#else
		PhotonPeer(PhotonListener* listener, bool useTcp=false);
		#endif
		virtual ~PhotonPeer(void);

		virtual bool connect(const JString& ipAddr, const nByte appID[APP_NAME_LENGTH]=NULL);
		virtual void disconnect(void);
		virtual void service(bool dispatchIncomingCommands=true);
		virtual void serviceBasic(void);
		virtual bool opCustom(const OperationRequest& operationRequest, bool sendReliable, nByte channelID=0, bool encrypt=false);
		virtual void sendOutgoingCommands(void);
		virtual bool dispatchIncomingCommands(void);
		virtual bool establishEncryption(void);
		virtual void fetchServerTimestamp(void);

		const PhotonBaseListener* getListener(void) const;
		int getServerTimeOffset(void) const;
		int getServerTime(void) const;
		int getBytesOut(void) const;
		int getBytesIn(void) const;
		PeerState getPeerState(void) const;
		int getSentCountAllowance(void) const;
		void setSentCountAllowance(int setSentCountAllowance);
		int getTimePingInterval(void) const;
		void setTimePingInterval(int setTimePingInterval);
		int getRoundTripTime(void) const;
		int getRoundTripTimeVariance(void) const;
		PhotonPeer_DebugLevel getDebugOutputLevel(void) const;
		bool setDebugOutputLevel(PhotonPeer_DebugLevel debugLevel);
		int getIncomingReliableCommandsCount(void) const;
		short getPeerId(void) const;
		int getSentTimeAllowance(void) const;
		void setSentTimeAllowance(int setSentTimeAllowance);
		int getQueuedIncomingCommands(void) const;
		int getQueuedOutgoingCommands(void) const;
		JString getServerAddress(void) const;
		bool getIsEncryptionAvailable(void) const;
		short getPeerCount(void) const;

		// internal debugging API, use macros below as public interface
		void sendDebugOutput(PhotonPeer_DebugLevel debugLevel, const EG_CHAR* const file, const EG_CHAR* const function, unsigned int line, const EG_CHAR* const dbgMsg, ...);
		void sendDebugOutput(PhotonPeer_DebugLevel debugLevel, const EG_CHAR* const file, const EG_CHAR* const function, unsigned int line, const EG_CHAR* const dbgMsg, va_list args);
	protected:
		SPhotonPeer* m_pPhotonPeer;
	private:
		PhotonListener* m_pListener;

		static void onOperationResult(PhotonPeer* pPeer, COperationResponse* cOperationResponse);
		static void onPeerStatus(PhotonPeer* pPeer, int statusCode);
		static void onEventAction(PhotonPeer* pPeer, CEventData* cEventData);
		static void onDebugReturn(PhotonPeer* pPeer, PhotonPeer_DebugLevel debugLevel, EG_CHAR* szDebugString);
	};
#ifndef _EG_BREW_PLATFORM
}
#endif

// public debugging API
#undef PhotonPeer_sendDebugOutput
#undef PhotonPeer_vsendDebugOutput
#ifdef EG_DEBUGGER
#	define PhotonPeer_sendDebugOutput(peer, debugLevel, ...) (peer)->sendDebugOutput(debugLevel, __WFILE__, __WFUNCTION__, __LINE__, __VA_ARGS__)
#	define PhotonPeer_vsendDebugOutput(peer, debugLevel, dbgMsg, args) (peer)->sendDebugOutput(debugLevel, __WFILE__, __WFUNCTION__, __LINE__, dbgMsg, args)
#else
#	define PhotonPeer_sendDebugOutput(peer, debugLevel, ...) ((void)0)
#	define PhotonPeer_vsendDebugOutput(peer, debugLevel, dbgMsg, args) ((void)0)
#endif

#endif