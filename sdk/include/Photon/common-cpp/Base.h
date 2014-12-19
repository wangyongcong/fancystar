/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2012 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#ifndef __BASE_H
#define __BASE_H

#include "BaseListener.h"
#include "ToString.h"

#ifndef _EG_BREW_PLATFORM
namespace ExitGames
{
#endif
	/* Summary
	   This is the base-class of all Utility-classes except of <link JString>,
	   used by both Neutron and Photon.
	   Description
	   This class provides a common callback interface for transmitting debug
	   messages from all utility classes to your application. Please refer to
	   <link Base::setListener@BaseListener*, setListener()> for more information.
	   See Also
	   <link BaseListener> , <link Base::setListener@BaseListener*, setListener()>                                */
	class Base : public ToString
	{
	public:
		static void setListener(const BaseListener* const baseListener);
	protected:
		static void debugReturn(const JString& string);
		static BaseListener* mpBaseListener;
	};
#ifndef _EG_BREW_PLATFORM
}
#endif

#endif