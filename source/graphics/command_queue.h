// Copyright (c) Dan Mitchell
// command_queue.h

#ifndef __PERFECT_SHOT_COMMAND_QUEUE_H__
#define __PERFECT_SHOT_COMMAND_QUEUE_H__

#pragma once

// ------- ------- ------- ------- ------- ------- -------
// declarations

class c_command_queue
{
public:

	c_command_queue();
	~c_command_queue();

	void initialize(
		wrl::ComPtr<ID3D12Device2> d3d_device,
		D3D12_COMMAND_LIST_TYPE d3d_command_list_type);

	void flush();
	uint64 signal();
	bool is_fence_complete(uint64 fence_value) const;
	void wait_for_fence_value(
		uint64 fence_value);

	wrl::ComPtr<ID3D12GraphicsCommandList> get_command_list();

	uint64 execute_command_list(
		wrl::ComPtr<ID3D12GraphicsCommandList> d3d_command_list);
	wrl::ComPtr<ID3D12CommandQueue> get_d3d_command_queue();

private:

	struct command_allocator_queue_entry
	{
		uint64 fence_value;
		wrl::ComPtr<ID3D12CommandAllocator> d3d_command_allocator;
	};

	wrl::ComPtr<ID3D12Device2> m_d3d_device;
	wrl::ComPtr<ID3D12CommandQueue> m_d3d_command_queue;
	wrl::ComPtr<ID3D12Fence> m_d3d_fence;
	HANDLE m_fence_event;
	uint64 m_fence_value;
	D3D12_COMMAND_LIST_TYPE m_d3d_command_list_type;
	std::queue<command_allocator_queue_entry> m_command_allocator_queue;
	std::queue<wrl::ComPtr<ID3D12GraphicsCommandList> > m_command_list_queue;

};

#endif // __PERFECT_SHOT_COMMAND_QUEUE_H__
