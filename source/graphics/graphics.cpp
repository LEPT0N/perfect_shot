// Copyright (c) Dan Mitchell
// graphics.cpp

// ------- ------- ------- ------- ------- ------- -------
// headers

#include "common.h"
#include "graphics\graphics.h"

// ------- ------- ------- ------- ------- ------- -------
// constants

const int32 k_default_window_width = 1280;
const int32 k_default_window_height = 720;
const float k_clear_color[] = { 0.2f, 0.4f, 0.8f, 1.0f };

// ------- ------- ------- ------- ------- ------- -------
// public definitions

void c_d3d_globals::initialize(
	HINSTANCE instance,
	WNDPROC window_proc)
{
	assert(!m_initialized);

	m_window_width = k_default_window_width;
	m_window_height = k_default_window_height;

	enable_debug_layer();

	register_window_class(instance, window_proc);
	HWND window = create_window(instance, m_window_width, m_window_height);

	wrl::ComPtr<IDXGIAdapter4> dxgi_adapter = get_adapter();
	m_d3d_device = create_device(dxgi_adapter);
	m_direct_command_queue.initialize(
		m_d3d_device,
		D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_dxgi_swap_chain = create_swap_chain(
		window,
		m_window_width,
		m_window_height,
		m_direct_command_queue.get_d3d_command_queue());
	m_back_buffer_index = m_dxgi_swap_chain->GetCurrentBackBufferIndex();
	m_d3d_descriptor_heap = create_descriptor_heap(
		m_d3d_device,
		k_back_buffer_count,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_descriptor_size = m_d3d_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	update_render_target_views(
		m_d3d_device,
		m_dxgi_swap_chain,
		m_d3d_descriptor_heap,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		m_d3d_back_buffers,
		k_back_buffer_count);

	for (uint32 index = 0; index < k_back_buffer_count; index++)
	{
		m_d3d_command_allocators[index] = create_command_allocator(
			m_d3d_device,
			D3D12_COMMAND_LIST_TYPE_DIRECT);
	}

	m_d3d_command_list = create_command_list(
		m_d3d_device,
		m_d3d_command_allocators[m_back_buffer_index],
		D3D12_COMMAND_LIST_TYPE_DIRECT);

	m_initialized = true;

	ShowWindow(window, SW_SHOW);
}

void c_d3d_globals::destroy()
{
	assert(m_initialized);

	m_direct_command_queue.flush();
}

void c_d3d_globals::render()
{
	assert(m_initialized);

	wrl::ComPtr<ID3D12Resource> back_buffer = m_d3d_back_buffers[m_back_buffer_index];
	wrl::ComPtr<ID3D12CommandAllocator> command_allocator = m_d3d_command_allocators[m_back_buffer_index];

	command_allocator->Reset();
	m_d3d_command_list->Reset(command_allocator.Get(), nullptr);

	clear();

	present();
}

// ------- ------- ------- ------- ------- ------- -------
// private definitions

void c_d3d_globals::clear()
{
	wrl::ComPtr<ID3D12Resource> back_buffer = m_d3d_back_buffers[m_back_buffer_index];

	CD3DX12_RESOURCE_BARRIER d3dx_resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		back_buffer.Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_d3d_command_list->ResourceBarrier(1, &d3dx_resource_barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE d3dx_cpu_descriptor_handle(
		m_d3d_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
		m_back_buffer_index,
		m_descriptor_size);

	m_d3d_command_list->ClearRenderTargetView(
		d3dx_cpu_descriptor_handle,
		k_clear_color,
		0,
		nullptr);
}

void c_d3d_globals::present()
{
	wrl::ComPtr<ID3D12Resource> back_buffer = m_d3d_back_buffers[m_back_buffer_index];

	CD3DX12_RESOURCE_BARRIER d3dx_resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		back_buffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);

	m_d3d_command_list->ResourceBarrier(1, &d3dx_resource_barrier);

	m_frame_fence_values[m_back_buffer_index] =
		m_direct_command_queue.execute_command_list(m_d3d_command_list);

	throw_if_failed(m_dxgi_swap_chain->Present(1, 0));

	m_back_buffer_index = m_dxgi_swap_chain->GetCurrentBackBufferIndex();

	m_direct_command_queue.wait_for_fence_value(m_frame_fence_values[m_back_buffer_index]);
}
