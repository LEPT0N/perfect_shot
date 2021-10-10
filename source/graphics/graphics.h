// Copyright (c) Dan Mitchell
// graphics.h
// 
// https://www.3dgep.com/learning-directx-12-1

#ifndef __PERFECT_SHOT_GRAPHICS_H__
#define __PERFECT_SHOT_GRAPHICS_H__

#pragma once

// ------- ------- ------- ------- ------- ------- -------
// headers

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>

// ------- ------- ------- ------- ------- ------- -------
// constants

const uint32 k_back_buffer_count = 2;

// ------- ------- ------- ------- ------- ------- -------
// declarations

class c_d3d_fence
{
public:

	c_d3d_fence()
	{
		m_fence_event = nullptr;
		m_fence_value = 0;
	};

	void initialize(
		wrl::ComPtr<ID3D12Device2> d3d_device);
	void destroy(
		wrl::ComPtr<ID3D12CommandQueue> d3d_command_queue);

private:

	static wrl::ComPtr<ID3D12Fence> create_fence(
		wrl::ComPtr<ID3D12Device2> d3d_device);
	static HANDLE create_fence_event();

	uint64 signal_fence(
		wrl::ComPtr<ID3D12CommandQueue> d3d_command_queue);
	void wait_for_fence_value(
		uint64 fence_value);

	wrl::ComPtr<ID3D12Fence> m_d3d_fence;
	HANDLE m_fence_event;
	uint64 m_fence_value;

};

class c_d3d_globals
{
public:

	c_d3d_globals()
	{
		m_back_buffer_index = 0;
		m_descriptor_size = 0;
		m_initialized = false;
	};

	void initialize(
		HINSTANCE instance,
		WNDPROC window_proc);
	void destroy();

	bool is_initialized() const { return m_initialized; };

private:

	static void enable_debug_layer();
	static void register_window_class(
		HINSTANCE instance,
		WNDPROC window_proc);
	static HWND create_window(
		HINSTANCE instance);
	static wrl::ComPtr<IDXGIAdapter4> get_adapter();
	static wrl::ComPtr<ID3D12Device2> create_device(
		wrl::ComPtr<IDXGIAdapter4> dxgi_adapter);
	static wrl::ComPtr<ID3D12CommandQueue> create_command_queue(
		wrl::ComPtr<ID3D12Device2> d3d_device);
	static bool is_tearing_supported();
	static wrl::ComPtr<IDXGISwapChain4> create_swap_chain(
		HWND window,
		wrl::ComPtr<ID3D12CommandQueue> d3d_command_queue);
	static wrl::ComPtr<ID3D12DescriptorHeap> create_descriptor_heap(
		wrl::ComPtr<ID3D12Device2> d3d_device,
		D3D12_DESCRIPTOR_HEAP_TYPE d3d_descriptor_heap_type);
	static void update_render_target_views(
		wrl::ComPtr<ID3D12Device2> d3d_device,
		wrl::ComPtr<IDXGISwapChain4> dxgi_swap_chain,
		wrl::ComPtr<ID3D12DescriptorHeap> d3d_descriptor_heap,
		D3D12_DESCRIPTOR_HEAP_TYPE d3d_descriptor_heap_type,
		wrl::ComPtr<ID3D12Resource> *d3d_back_buffers,
		uint32 back_buffer_count);
	static wrl::ComPtr<ID3D12CommandAllocator> create_command_allocator(
		wrl::ComPtr<ID3D12Device2> d3d_device,
		D3D12_COMMAND_LIST_TYPE d3d_command_list_type);
	static wrl::ComPtr<ID3D12GraphicsCommandList> create_command_list(
		wrl::ComPtr<ID3D12Device2> d3d_device,
		wrl::ComPtr<ID3D12CommandAllocator> d3d_command_allocator,
		D3D12_COMMAND_LIST_TYPE d3d_command_list_type);

	wrl::ComPtr<ID3D12Device2> m_d3d_device;
	wrl::ComPtr<ID3D12CommandQueue> m_d3d_command_queue;
	wrl::ComPtr<IDXGISwapChain4> m_dxgi_swap_chain;
	wrl::ComPtr<ID3D12DescriptorHeap> m_d3d_descriptor_heap;
	wrl::ComPtr<ID3D12Resource> m_d3d_back_buffers[k_back_buffer_count];
	wrl::ComPtr<ID3D12CommandAllocator> m_d3d_command_allocators[k_back_buffer_count];
	wrl::ComPtr<ID3D12GraphicsCommandList> m_d3d_command_list;
	c_d3d_fence m_fence;

	uint32 m_back_buffer_index;
	uint32 m_descriptor_size;

	bool m_initialized;
};

#endif // __PERFECT_SHOT_GRAPHICS_H__
