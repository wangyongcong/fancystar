#ifndef __HEADER_WYC_PLATFORM
#define __HEADER_WYC_PLATFORM

#if (defined _WIN32) || (defined _WIN64)
/*
Minimum system required	Minimum value for _WIN32_WINNT and WINVER
Windows 7	_WIN32_WINNT_WIN7 (0x0601)
Windows Server 2008	_WIN32_WINNT_WS08 (0x0600)
Windows Vista	_WIN32_WINNT_VISTA (0x0600)
Windows Server 2003 with SP1, Windows XP with SP2	_WIN32_WINNT_WS03 (0x0502)
Windows Server 2003, Windows XP	_WIN32_WINNT_WINXP (0x0501)
 
Minimum version required	Minimum value of _WIN32_IE
Internet Explorer 8.0	_WIN32_IE_IE80 (0x0800)
Internet Explorer 7.0	_WIN32_IE_IE70 (0x0700)
Internet Explorer 6.0 SP2	_WIN32_IE_IE60SP2 (0x0603)
Internet Explorer 6.0 SP1	_WIN32_IE_IE60SP1 (0x0601)
Internet Explorer 6.0	_WIN32_IE_IE60 (0x0600)
Internet Explorer 5.5	_WIN32_IE_IE55 (0x0550)
Internet Explorer 5.01	_WIN32_IE_IE501 (0x0501)
Internet Explorer 5.0, 5.0a, 5.0b	_WIN32_IE_IE50 (0x0500)
*/

#define WINVER 0x500
#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <process.h>
#include <mmsystem.h>

// compiler intrinsit
// used for atomic operation
#include <intrin.h>

// user env
#include <userenv.h>
#pragma comment (lib, "userenv.lib")

// ignored warnings
// structure without a declarator as members of another structure or union
#pragma warning (disable: 4201) 
// the file contains a character that cannot be represented in the current code page
#pragma warning (disable: 4819)

#endif // WIN32

#endif // __HEADER_WYC_PLATFORM
