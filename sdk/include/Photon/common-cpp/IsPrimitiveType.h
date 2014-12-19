/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2012 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#ifndef __IS_PRIMITIVE_TYPE_H
#define __IS_PRIMITIVE_TYPE_H

#ifndef _EG_BREW_PLATFORM
namespace ExitGames
{
#endif

	template<typename Etype> struct IsPrimitiveType{static const bool is = false;};
	template<> struct IsPrimitiveType<char>{static const bool is = true;};
	template<> struct IsPrimitiveType<signed char>{static const bool is = true;};
	template<> struct IsPrimitiveType<unsigned char>{static const bool is = true;};
	template<> struct IsPrimitiveType<short>{static const bool is = true;};
	template<> struct IsPrimitiveType<unsigned short>{static const bool is = true;};
	template<> struct IsPrimitiveType<int>{static const bool is = true;};
	template<> struct IsPrimitiveType<unsigned int>{static const bool is = true;};
	template<> struct IsPrimitiveType<long>{static const bool is = true;};
	template<> struct IsPrimitiveType<unsigned long>{static const bool is = true;};
	template<> struct IsPrimitiveType<long long>{static const bool is = true;};
	template<> struct IsPrimitiveType<unsigned long long>{static const bool is = true;};
	template<> struct IsPrimitiveType<float>{static const bool is = true;};
	template<> struct IsPrimitiveType<double>{static const bool is = true;};
	template<> struct IsPrimitiveType<long double>{static const bool is = true;};
	template<> struct IsPrimitiveType<bool>{static const bool is = true;};
	template<> struct IsPrimitiveType<wchar_t>{static const bool is = true;};

#ifndef _EG_BREW_PLATFORM
}
#endif

#endif