// Copyright (c) Dan Mitchell
// common.h

#ifndef __PERFECT_SHOT_COMMON_H__
#define __PERFECT_SHOT_COMMON_H__

#pragma once

// ------- ------- ------- ------- ------- ------- -------
// headers

#include <exception>
#include <wrl.h>

namespace wrl = Microsoft::WRL;

// ------- ------- ------- ------- ------- ------- -------
// declarations

#if defined(_DEBUG)
#define DEBUG_MODE
#endif

typedef wchar_t wchar;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef signed char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

inline void throw_if_failed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

inline void throw_if(bool value)
{
	if (value)
	{
		throw std::exception();
	}
}

#define ZERO_MEMORY(x) (memset(&x, 0, sizeof(x)))

#endif // __PERFECT_SHOT_COMMON_H__
