/* Exit Games Photon - C Client Lib
 * Copyright (C) 2004-2012 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#ifndef __COMMAND_H
#define __COMMAND_H

typedef struct
{
	nByte  Command;
	nByte  ChannelID;
	nByte  Flags;
	nByte  Reserved;
	int   CommandLength;
	int   ReliableSequenceNumber;
} CommandHeader; 

#endif