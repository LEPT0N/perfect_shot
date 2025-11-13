// Copyright (c) Dan Mitchell
// graphics.h
// 
// https://www.3dgep.com/learning-directx-12-1
//
//    Z
//    |     Y
//    |    /
//    |   / 
//    |  /
//    | /
//    |/
//    o------------ X
//
//

#ifndef __PERFECT_SHOT_GRAPHICS_H__
#define __PERFECT_SHOT_GRAPHICS_H__

#pragma once

// ------- ------- ------- ------- ------- ------- -------
// headers

#include "graphics\command_queue.h"
#include "graphics\graphics_utilities.h"

// ------- ------- ------- ------- ------- ------- -------
// declarations

class c_d3d_globals
{
public:

	c_d3d_globals()
	{
		ZERO_MEMORY(m_frame_fence_values);
		m_back_buffer_index = 0;
		m_descriptor_size = 0;
		m_window_width = 0;
		m_window_height = 0;
		m_initialized = false;
	};

	void initialize(
		HINSTANCE instance,
		WNDPROC window_proc);
	void destroy();

	bool is_initialized() const { return m_initialized; };

	// TODO: consider any use of this should be a method here.
	wrl::ComPtr<ID3D12Device2> get_device() { return m_d3d_device; };

	c_command_queue* get_copy_command_queue() { return &m_copy_command_queue; };

	void render();

private:

	void clear();
	void present();

	wrl::ComPtr<ID3D12Device2> m_d3d_device;
	wrl::ComPtr<IDXGISwapChain4> m_dxgi_swap_chain;
	wrl::ComPtr<ID3D12DescriptorHeap> m_d3d_descriptor_heap;
	wrl::ComPtr<ID3D12Resource> m_d3d_back_buffers[k_back_buffer_count];
	wrl::ComPtr<ID3D12CommandAllocator> m_d3d_command_allocators[k_back_buffer_count];
	wrl::ComPtr<ID3D12GraphicsCommandList> m_d3d_command_list;
	
	uint64 m_frame_fence_values[k_back_buffer_count];
	c_command_queue m_direct_command_queue;
	c_command_queue m_copy_command_queue;

	uint32 m_back_buffer_index;
	uint32 m_descriptor_size;

	int32 m_window_width;
	int32 m_window_height;

	bool m_initialized;
};

#endif // __PERFECT_SHOT_GRAPHICS_H__
