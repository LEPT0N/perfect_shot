// Copyright (c) Dan Mitchell
// time_utilities.h
// 

#ifndef __PERFECT_SHOT_TIME_UTILITIES_H__
#define __PERFECT_SHOT_TIME_UTILITIES_H__

#pragma once

// ------- ------- ------- ------- ------- ------- -------
// headers

#include <chrono>

// ------- ------- ------- ------- ------- ------- -------
// constants

const double k_seconds_to_nanoseconds = 1e-9;

// ------- ------- ------- ------- ------- ------- -------
// declarations

class c_frame_timer
{
public:

	c_frame_timer()
	{
		m_previous_time = m_clock.now();
		m_frame_count = 0;
		m_time_elapsed = 0.0;
	};

	void update();

private:

	std::chrono::high_resolution_clock m_clock;
	std::chrono::steady_clock::time_point m_previous_time;

	uint64 m_frame_count;
	double m_time_elapsed;
};

#endif // __PERFECT_SHOT_TIME_UTILITIES_H__
