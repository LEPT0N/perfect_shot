// Copyright (c) Dan Mitchell
// time_utilities.cpp

// ------- ------- ------- ------- ------- ------- -------
// headers

#include "common.h"
#include "utilities\time_utilities.h"

#include "utilities\string_utilities.h"

// ------- ------- ------- ------- ------- ------- -------
// public definitions

void c_frame_timer::update()
{
	std::chrono::steady_clock::time_point now = m_clock.now();
	std::chrono::nanoseconds delta = now - m_previous_time;
	m_previous_time = now;

	m_frame_count++;
	m_time_elapsed += delta.count() * k_seconds_to_nanoseconds;
	if (m_time_elapsed > 1.0)
	{
		double frames_per_second = m_frame_count / m_time_elapsed;

		c_temp_string output;
		output.printf(L"FPS: %.02f\n", frames_per_second);
		OutputDebugString(output.get_buffer());

		m_frame_count = 0;
		m_time_elapsed = 0.0;
	}
}
