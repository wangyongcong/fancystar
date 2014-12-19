/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2012 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#ifndef __TO_STRING_IMPLEMENTATION_H
#define __TO_STRING_IMPLEMENTATION_H

#include "ObjectToStringConverter.h"
#include "PrimitiveToStringConverter.h"

#ifndef _EG_BREW_PLATFORM
namespace ExitGames
{
#endif

	template<bool, typename Etype>
	struct ToStringImplementation
	{
		static ObjectToStringConverter<Etype> converter;
	};

	template<bool condition, typename Etype>
	struct ToStringImplementation<condition, Etype*>
	{
		static ObjectToStringConverter<Etype*> converter;
	};

	template<typename Etype>
	struct ToStringImplementation<true, Etype>
	{
		static PrimitiveToStringConverter<Etype> converter;
	};

	template<typename Etype>
	struct ToStringImplementation<true, Etype*>
	{
		static PrimitiveToStringConverter<Etype*> converter;
	};

#ifndef _EG_BREW_PLATFORM
}
#endif

#endif