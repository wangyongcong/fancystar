/* Exit Games Photon - C++ Client Lib
 * Copyright (C) 2004-2012 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#ifndef __PHOTON_BASE_LISTENER_H
#define __PHOTON_BASE_LISTENER_H

#include "Photon.h"
#include "JString.h"

#ifndef _EG_BREW_PLATFORM
namespace ExitGames
{
#endif
	class PhotonBaseListener
	{
	public:
		virtual ~PhotonBaseListener(void){}
        /* Summary
           called by the library as callback for debug messages in error
           case.
           Parameters
           debugLevel: the debug level, the message was created with
           string : the debug message
           Returns
           Nothing.                                                      */
		virtual void debugReturn(PhotonPeer_DebugLevel debugLevel, const JString& string) = 0;
	};
#ifndef _EG_BREW_PLATFORM
}
#endif

#endif