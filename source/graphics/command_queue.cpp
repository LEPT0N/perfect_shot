// Copyright (c) Dan Mitchell
// command_queue.cpp

// ------- ------- ------- ------- ------- ------- -------
// headers

#include "common.h"
#include "graphics\command_queue.h"

#include "graphics\graphics_utilities.h"

// ------- ------- ------- ------- ------- ------- -------
// public definitions

c_command_queue::c_command_queue()
{
	m_fence_event = nullptr;
	m_fence_value = 0;
	m_d3d_command_list_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
}

c_command_queue::~c_command_queue()
{
	CloseHandle(m_fence_event);
}

void c_command_queue::initialize(
	wrl::ComPtr<ID3D12Device2> d3d_device,
	D3D12_COMMAND_LIST_TYPE d3d_command_list_type)
{
	m_d3d_device = d3d_device;
	m_d3d_command_queue = create_command_queue(d3d_device, d3d_command_list_type);
	m_d3d_fence = create_fence(d3d_device);
	m_fence_event = create_fence_event();
	m_fence_value = 0;
	m_d3d_command_list_type = d3d_command_list_type;
}

void c_command_queue::flush()
{
	uint64 fence_value_for_signal = signal();
	wait_for_fence_value(fence_value_for_signal);
}

uint64 c_command_queue::signal()
{
	m_fence_value++;
	uint64 fence_value_for_signal = m_fence_value;

	throw_if_failed(m_d3d_command_queue->Signal(
		m_d3d_fence.Get(),
		fence_value_for_signal));

	return fence_value_for_signal;
}

bool c_command_queue::is_fence_complete(uint64 fence_value) const
{
	return (m_d3d_fence->GetCompletedValue() >= fence_value);
}

void c_command_queue::wait_for_fence_value(
	uint64 fence_value)
{
	if (!is_fence_complete(fence_value))
	{
		throw_if_failed(m_d3d_fence->SetEventOnCompletion(fence_value, m_fence_event));

		WaitForSingleObject(m_fence_event, INFINITE);
	}
}

wrl::ComPtr<ID3D12GraphicsCommandList> c_command_queue::get_command_list()
{
	// Either get a new command allocator or reuse one that is not being used anymore.
	wrl::ComPtr<ID3D12CommandAllocator> d3d_command_allocator;

	if (!m_command_allocator_queue.empty() && is_fence_complete(m_command_allocator_queue.front().fence_value))
	{
		d3d_command_allocator = m_command_allocator_queue.front().d3d_command_allocator;
		m_command_allocator_queue.pop();
		throw_if_failed(d3d_command_allocator.Reset());
	}
	else
	{
		d3d_command_allocator = create_command_allocator(
			m_d3d_device,
			m_d3d_command_list_type);
	}

	// Find or create a command list
	wrl::ComPtr<ID3D12GraphicsCommandList> d3d_command_list;

	if (!m_command_list_queue.empty())
	{
		d3d_command_list = m_command_list_queue.front();
		m_command_list_queue.pop();
		throw_if_failed(d3d_command_list->Reset(d3d_command_allocator.Get(), nullptr));
	}
	else
	{
		d3d_command_list = create_command_list(
			m_d3d_device,
			d3d_command_allocator,
			m_d3d_command_list_type);
	}

	// Hide the allocator inside the command list to keep it alive.
	// This increments the internal refcount of the command allocator.
	throw_if_failed(d3d_command_list->SetPrivateDataInterface(
		__uuidof(ID3D12CommandAllocator),
		d3d_command_allocator.Get()));

	return d3d_command_list;
}

uint64 c_command_queue::execute_command_list(
	wrl::ComPtr<ID3D12GraphicsCommandList> d3d_command_list)
{
	throw_if_failed(d3d_command_list->Close());

	ID3D12CommandAllocator* d3d_command_allocator;
	uint32 d3d_command_allocator_size = sizeof(d3d_command_allocator);
	throw_if_failed(d3d_command_list->GetPrivateData(
		__uuidof(ID3D12CommandAllocator),
		&d3d_command_allocator_size,
		&d3d_command_allocator));

	ID3D12CommandList* d3d_command_lists[] = { d3d_command_list.Get() };

	m_d3d_command_queue->ExecuteCommandLists(
		_countof(d3d_command_lists),
		d3d_command_lists);

	uint64 fence_value = signal();

	m_command_allocator_queue.push({fence_value, d3d_command_allocator });
	m_command_list_queue.push(d3d_command_list);

	// This decrements the refcount from SetPrivateDataInterface
	d3d_command_allocator->Release();

	return fence_value;
}

wrl::ComPtr<ID3D12CommandQueue> c_command_queue::get_d3d_command_queue()
{
	return m_d3d_command_queue;
}
