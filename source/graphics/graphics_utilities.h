// Copyright (c) Dan Mitchell
// graphics_utilities.h

#ifndef __PERFECT_SHOT_GRAPHICS_UTILITIES_H__
#define __PERFECT_SHOT_GRAPHICS_UTILITIES_H__

#pragma once

// ------- ------- ------- ------- ------- ------- -------
// constants

const uint32 k_back_buffer_count = 2;

// ------- ------- ------- ------- ------- ------- -------
// declarations

struct s_vertex
{
	dx::XMFLOAT3 position;
	dx::XMFLOAT3 color;
};

void enable_debug_layer();
void register_window_class(
	HINSTANCE instance,
	WNDPROC window_proc);
HWND create_window(
	HINSTANCE instance,
	int32 window_width,
	int32 window_height);
wrl::ComPtr<IDXGIAdapter4> get_adapter();
wrl::ComPtr<ID3D12Device2> create_device(
	wrl::ComPtr<IDXGIAdapter4> dxgi_adapter);
wrl::ComPtr<ID3D12CommandQueue> create_command_queue(
	wrl::ComPtr<ID3D12Device2> d3d_device);
bool is_tearing_supported();
wrl::ComPtr<IDXGISwapChain4> create_swap_chain(
	HWND window,
	int32 window_width,
	int32 window_height,
	wrl::ComPtr<ID3D12CommandQueue> d3d_command_queue);
wrl::ComPtr<ID3D12DescriptorHeap> create_descriptor_heap(
	wrl::ComPtr<ID3D12Device2> d3d_device,
	uint32 count,
	D3D12_DESCRIPTOR_HEAP_TYPE d3d_descriptor_heap_type);
void update_render_target_views(
	wrl::ComPtr<ID3D12Device2> d3d_device,
	wrl::ComPtr<IDXGISwapChain4> dxgi_swap_chain,
	wrl::ComPtr<ID3D12DescriptorHeap> d3d_descriptor_heap,
	D3D12_DESCRIPTOR_HEAP_TYPE d3d_descriptor_heap_type,
	wrl::ComPtr<ID3D12Resource>* d3d_back_buffers,
	uint32 back_buffer_count);
void create_resource(
	wrl::ComPtr<ID3D12Device2> d3d_device,
	wrl::ComPtr<ID3D12GraphicsCommandList> d3d_command_list,
	ID3D12Resource** destination_resource,
	ID3D12Resource** intermediate_resource,
	int32 element_count,
	int32 element_size,
	const void* resource_data);
D3D12_FEATURE_DATA_ROOT_SIGNATURE get_root_signature_version(
	wrl::ComPtr<ID3D12Device2> d3d_device);
wrl::ComPtr<ID3D12CommandAllocator> create_command_allocator(
	wrl::ComPtr<ID3D12Device2> d3d_device,
	D3D12_COMMAND_LIST_TYPE d3d_command_list_type);
wrl::ComPtr<ID3D12GraphicsCommandList> create_command_list(
	wrl::ComPtr<ID3D12Device2> d3d_device,
	wrl::ComPtr<ID3D12CommandAllocator> d3d_command_allocator,
	D3D12_COMMAND_LIST_TYPE d3d_command_list_type);
wrl::ComPtr<ID3D12CommandQueue> create_command_queue(
	wrl::ComPtr<ID3D12Device2> d3d_device,
	D3D12_COMMAND_LIST_TYPE d3d_command_list_type);
wrl::ComPtr<ID3D12Fence> create_fence(
	wrl::ComPtr<ID3D12Device2> d3d_device);
HANDLE create_fence_event();

#endif // __PERFECT_SHOT_GRAPHICS_UTILITIES_H__
