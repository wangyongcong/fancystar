/* Exit Games Photon - C Client Lib
 * Copyright (C) 2004-2012 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#ifndef __ENET_CHANNEL_H
#define __ENET_CHANNEL_H

#include "Constants.h"
#include "data_structures.h"
#include "EnetCommand.h"

namespace ExitGames
{
	class EnetChannel
	{
	public:
		EnetChannel(nByte channelNumber);
		~EnetChannel(void);
		EnetCommand* getReliableCommandFromQueue(int reliableSequenceNumber);
		EnetCommand* getUnreliableCommandFromQueue(int unreliableSequenceNumber);
		bool removeReliableCommandFromQueue(int reliableSequenceNumber);
		bool removeUnreliableCommandFromQueue(int unreliableSequenceNumber);
	private:
		nByte channelNumber;

		EG_Vector* incomingReliableCommands;
		EG_Vector* incomingUnreliableCommands;

		int incomingReliableSequenceNumber;		// sequencenr of last dispatched command
		int incomingUnreliableSequenceNumber;

		EG_Vector* outgoingReliableCommands;
		EG_Vector* outgoingUnreliableCommands;

		int outgoingReliableSequenceNumber;
		int outgoingUnreliableSequenceNumber;

		friend class PeerBase;
		friend class EnetPeer;
	};
}

#endif