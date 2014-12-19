/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2012 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#ifndef __JVECTOR_H
#define __JVECTOR_H

#include "JVectorBase.h"
#include "IsPrimitiveType.h"
#include "ToStringImplementation.h"

#ifndef _EG_BREW_PLATFORM
namespace ExitGames
{
#endif

	template<typename Etype>
	class JVector : public JVectorBase<Etype>
	{
	public:
		using JVectorBase<Etype>::toString;

		JVector(unsigned int initialCapacity=40, unsigned int capacityIncrement=10);
		~JVector(void);

		JVector(const JVector<Etype>& rhv);

		JString& toString(JString& retStr, bool withTypes=false) const;
	private:
		JString primitiveToString(void) const;
		JString objectToString(void) const;
	};

	template<typename Etype>
	class JVector<Etype*> : public JVectorBase<Etype*>
	{
	public:
		using JVectorBase<Etype*>::toString;

		JVector(unsigned int initialCapacity=40, unsigned int capacityIncrement=10);
		~JVector(void);

		JVector(const JVector<Etype*>& rhv);

		JString& toString(JString& retStr, bool withTypes=false) const;
	private:
		JString primitiveToString(void) const;
		JString objectToString(void) const;
	};

	template<> JString& JVector<JString>::toString(JString& retStr, bool withTypes) const;
	template<> JString& JVector<JString*>::toString(JString& retStr, bool withTypes) const;



	template<typename Etype>
	JVector<Etype>::JVector(unsigned int initialCapacity, unsigned int capacityIncrement) : JVectorBase<Etype>(initialCapacity, capacityIncrement)
	{
	}

	template<typename Etype>
	JVector<Etype*>::JVector(unsigned int initialCapacity, unsigned int capacityIncrement) : JVectorBase<Etype*>(initialCapacity, capacityIncrement)
	{
	}

	template<typename Etype>
	JVector<Etype>::~JVector(void)
	{
	}

	template<typename Etype>
	JVector<Etype*>::~JVector(void)
	{
	}

	template<typename Etype>
	JVector<Etype>::JVector(const JVector<Etype>& rhv) : JVectorBase<Etype>(rhv)
	{
	}

	template<typename Etype>
	JVector<Etype*>::JVector(const JVector<Etype*>& rhv) : JVectorBase<Etype*>(rhv)
	{
	}

	template<typename Etype>
	JString& JVector<Etype>::toString(JString& retStr, bool /*withTypes*/) const
	{
		return retStr += ToStringImplementation<IsPrimitiveType<Etype>::is, Etype>::converter.toString(this->mpData, this->size());
	}

	template<typename Etype>
	JString& JVector<Etype*>::toString(JString& retStr, bool /*withTypes*/) const
	{
		return retStr += ToStringImplementation<IsPrimitiveType<Etype*>::is, Etype*>::converter.toString(this->mpData, this->size());
	}

#ifndef _EG_BREW_PLATFORM
}
#endif

#endif