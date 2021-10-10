// Copyright (c) Dan Mitchell
// string_utilities.h
// 

#ifndef __PERFECT_SHOT_STRING_UTILITIES_H__
#define __PERFECT_SHOT_STRING_UTILITIES_H__

#pragma once

// ------- ------- ------- ------- ------- ------- -------
// headers

#include <stdarg.h>

// ------- ------- ------- ------- ------- ------- -------
// declarations

template<int32 N> class c_string
{
public:

	c_string()
	{
		ZERO_MEMORY(m_buffer);
	};

	void printf(
		const wchar *format,
		...);

	const wchar* get_buffer() { return m_buffer; };

private:

	wchar m_buffer[N];
};

typedef c_string<64> c_temp_string;

// ------- ------- ------- ------- ------- ------- -------
// public definitions

template<int32 N> void c_string<N>::printf(
	const wchar* format,
	...)
{
	va_list arguments;
	va_start(arguments, format);

	_vsnwprintf_s(
		m_buffer,
		N,
		_TRUNCATE,
		format,
		arguments);

	va_end(arguments);
}

#endif // __PERFECT_SHOT_STRING_UTILITIES_H__
