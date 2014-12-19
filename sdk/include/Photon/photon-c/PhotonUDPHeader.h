/* Exit Games Photon - C Client Lib
 * Copyright (C) 2004-2012 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#ifndef __PHOTON_UDP_HEADER_H
#define __PHOTON_UDP_HEADER_H

typedef struct
{
	nByte  Ident;
	short Ticket;
	short DataLength;
	nByte  MessageType;
} PhotonUDPHeader; 

#endif