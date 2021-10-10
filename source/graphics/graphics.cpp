// Copyright (c) Dan Mitchell
// graphics.cpp

// ------- ------- ------- ------- ------- ------- -------
// headers

#include "common.h"
#include "graphics.h"

// ------- ------- ------- ------- ------- ------- -------
// constants

const wchar* k_window_class_name = L"DX12WindowClass";
const wchar* k_window_name = L"Perfect Shot";
const int32 k_window_width = 1280;
const int32 k_window_height = 720;
const float k_clear_color[] = { 0.2f, 0.4f, 0.8f, 1.0f };

// ------- ------- ------- ------- ------- ------- -------
// public definitions

void c_d3d_fence::initialize(
	wrl::ComPtr<ID3D12Device2> d3d_device)
{
	m_d3d_fence = create_fence(d3d_device);
	m_fence_event = create_fence_event();
	m_fence_value = 0;
}

void c_d3d_fence::destroy(
	wrl::ComPtr<ID3D12CommandQueue> d3d_command_queue)
{
	uint64 fence_value_for_signal = signal(d3d_command_queue);
	wait_for_fence_value(fence_value_for_signal);

	CloseHandle(m_fence_event);
}

uint64 c_d3d_fence::signal(
	wrl::ComPtr<ID3D12CommandQueue> d3d_command_queue)
{
	m_fence_value++;
	uint64 fence_value_for_signal = m_fence_value;

	throw_if_failed(d3d_command_queue->Signal(
		m_d3d_fence.Get(),
		fence_value_for_signal));

	return fence_value_for_signal;
}

void c_d3d_fence::wait_for_fence_value(
	uint64 fence_value)
{
	if (m_d3d_fence->GetCompletedValue() < fence_value)
	{
		throw_if_failed(m_d3d_fence->SetEventOnCompletion(fence_value, m_fence_event));

		WaitForSingleObject(m_fence_event, INFINITE);
	}
}

void c_d3d_globals::initialize(
	HINSTANCE instance,
	WNDPROC window_proc)
{
	enable_debug_layer();

	register_window_class(instance, window_proc);
	HWND window = create_window(instance);

	wrl::ComPtr<IDXGIAdapter4> dxgi_adapter = get_adapter();
	m_d3d_device = create_device(dxgi_adapter);
	m_d3d_command_queue = create_command_queue(m_d3d_device);
	m_dxgi_swap_chain = create_swap_chain(window, m_d3d_command_queue);
	m_back_buffer_index = m_dxgi_swap_chain->GetCurrentBackBufferIndex();
	m_d3d_descriptor_heap = create_descriptor_heap(
		m_d3d_device,
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

	m_fence.initialize(m_d3d_device);

	m_initialized = true;

	ShowWindow(window, SW_SHOW);
}

void c_d3d_globals::destroy()
{
	m_fence.destroy(m_d3d_command_queue);
}

void c_d3d_globals::render()
{
	wrl::ComPtr<ID3D12Resource> back_buffer = m_d3d_back_buffers[m_back_buffer_index];
	wrl::ComPtr<ID3D12CommandAllocator> command_allocator = m_d3d_command_allocators[m_back_buffer_index];

	command_allocator->Reset();
	m_d3d_command_list->Reset(command_allocator.Get(), nullptr);

	clear();

	present();
}

// ------- ------- ------- ------- ------- ------- -------
// private definitions

wrl::ComPtr<ID3D12Fence> c_d3d_fence::create_fence(
	wrl::ComPtr<ID3D12Device2> d3d_device)
{
	wrl::ComPtr<ID3D12Fence> d3d_fence;
	throw_if_failed(d3d_device->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&d3d_fence)));

	return d3d_fence;
}

HANDLE c_d3d_fence::create_fence_event()
{
	HANDLE fence_event = CreateEvent(nullptr, false, false, nullptr);
	throw_if_failed(fence_event != nullptr);
	return fence_event;
}

void c_d3d_globals::enable_debug_layer()
{
#if defined(DEBUG_MODE)
	wrl::ComPtr<ID3D12Debug> debug_interface;
	throw_if_failed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface)));
	debug_interface->EnableDebugLayer();
#endif
}

void c_d3d_globals::register_window_class(
	HINSTANCE instance,
	WNDPROC window_proc)
{
	WNDCLASSEX window_class = {};
	window_class.cbSize = sizeof(window_class);
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = window_proc;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = instance;
	window_class.hIcon = LoadIcon(instance, nullptr);
	window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
	window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	window_class.lpszMenuName = nullptr;
	window_class.lpszClassName = k_window_class_name;
	window_class.hIconSm = LoadIcon(instance, nullptr);

	ATOM atom = RegisterClassEx(&window_class);
	throw_if(atom == 0);
}

HWND c_d3d_globals::create_window(
	HINSTANCE instance)
{
	HWND window = CreateWindowEx(
		0,
		k_window_class_name,
		k_window_name,
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		k_window_width,
		k_window_height,
		nullptr,
		nullptr,
		instance,
		nullptr);
	throw_if(window == nullptr);

	return window;
}

wrl::ComPtr<IDXGIAdapter4> c_d3d_globals::get_adapter()
{
	uint32 create_factory_flags = 0;
#if defined(DEBUG_MODE)
	create_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	wrl::ComPtr<IDXGIFactory4> dxgi_factory;
	throw_if_failed(CreateDXGIFactory2(
		create_factory_flags,
		IID_PPV_ARGS(&dxgi_factory)));

	wrl::ComPtr<IDXGIAdapter1> dxgi_adapter1;
	throw_if_failed(dxgi_factory->EnumWarpAdapter(IID_PPV_ARGS(&dxgi_adapter1)));

	wrl::ComPtr<IDXGIAdapter4> dxgi_adapter4;
	throw_if_failed(dxgi_adapter1.As(&dxgi_adapter4));

	return dxgi_adapter4;
}

wrl::ComPtr<ID3D12Device2> c_d3d_globals::create_device(
	wrl::ComPtr<IDXGIAdapter4> dxgi_adapter)
{
	wrl::ComPtr<ID3D12Device2> d3d_device;
	throw_if_failed(D3D12CreateDevice(
		dxgi_adapter.Get(),
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(&d3d_device)));

#if defined(DEBUG_MODE)
	wrl::ComPtr<ID3D12InfoQueue> d3d_info_queue;
	throw_if_failed(d3d_device.As(&d3d_info_queue));

	d3d_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
	d3d_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
	d3d_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

	// D3D12_MESSAGE_CATEGORY suppressed_categories[] = {};
	// D3D12_MESSAGE_SEVERITY suppressed_severities[] = {};
	// D3D12_MESSAGE_ID suppressed_ids[] = {};

	// D3D12_INFO_QUEUE_FILTER d3d_info_queue_filter = {};
	// d3d_info_queue_filter.DenyList.NumCategories = _countof(suppressed_categories);
	// d3d_info_queue_filter.DenyList.pCategoryList = suppressed_categories;
	// d3d_info_queue_filter.DenyList.NumSeverities = _countof(suppressed_severities);
	// d3d_info_queue_filter.DenyList.pSeverityList = suppressed_severities;
	// d3d_info_queue_filter.DenyList.NumIDs = _countof(suppressed_ids);
	// d3d_info_queue_filter.DenyList.pIDList = suppressed_ids;

	// throw_if_failed(d3d_info_queue->PushStorageFilter(&d3d_info_queue_filter));
#endif

	return d3d_device;
}

wrl::ComPtr<ID3D12CommandQueue> c_d3d_globals::create_command_queue(
	wrl::ComPtr<ID3D12Device2> d3d_device)
{
	D3D12_COMMAND_QUEUE_DESC d3d_command_queue_description = {};
	d3d_command_queue_description.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	d3d_command_queue_description.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	d3d_command_queue_description.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	wrl::ComPtr<ID3D12CommandQueue> d3d_command_queue;
	throw_if_failed(d3d_device->CreateCommandQueue(
		&d3d_command_queue_description,
		IID_PPV_ARGS(&d3d_command_queue)));

	return d3d_command_queue;
}

bool c_d3d_globals::is_tearing_supported()
{
	bool supported = false;

	//wrl::ComPtr<IDXGIFactory5> dxgi_factory;
	//throw_if_failed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory)));

	//throw_if_failed(dxgi_factory->CheckFeatureSupport(
	//	DXGI_FEATURE_PRESENT_ALLOW_TEARING,
	//	&supported, sizeof(supported)));

	return supported;
}

wrl::ComPtr<IDXGISwapChain4> c_d3d_globals::create_swap_chain(
	HWND window,
	wrl::ComPtr<ID3D12CommandQueue> d3d_command_queue)
{
	uint32 create_factory_flags = 0;
#if defined(DEBUG_MODE)
	create_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	wrl::ComPtr<IDXGIFactory4> dxgi_factory;
	throw_if_failed(CreateDXGIFactory2(
		create_factory_flags,
		IID_PPV_ARGS(&dxgi_factory)));

	DXGI_SWAP_CHAIN_DESC1 dxgi_swap_chain_description = {};
	dxgi_swap_chain_description.Width = k_window_width;
	dxgi_swap_chain_description.Height = k_window_height;
	dxgi_swap_chain_description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgi_swap_chain_description.Stereo = false;
	dxgi_swap_chain_description.SampleDesc.Count = 1;
	dxgi_swap_chain_description.SampleDesc.Quality = 0;
	dxgi_swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgi_swap_chain_description.BufferCount = k_back_buffer_count;
	dxgi_swap_chain_description.Scaling = DXGI_SCALING_NONE;
	dxgi_swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgi_swap_chain_description.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgi_swap_chain_description.Flags = is_tearing_supported() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	wrl::ComPtr<IDXGISwapChain1> dxgi_swap_chain1;
	throw_if_failed(dxgi_factory->CreateSwapChainForHwnd(
		d3d_command_queue.Get(),
		window,
		&dxgi_swap_chain_description,
		nullptr, nullptr, &dxgi_swap_chain1));

	wrl::ComPtr<IDXGISwapChain4> dxgi_swap_chain4;
	throw_if_failed(dxgi_swap_chain1.As(&dxgi_swap_chain4));

	return dxgi_swap_chain4;
}

wrl::ComPtr<ID3D12DescriptorHeap> c_d3d_globals::create_descriptor_heap(
	wrl::ComPtr<ID3D12Device2> d3d_device,
	D3D12_DESCRIPTOR_HEAP_TYPE d3d_descriptor_heap_type)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3d_descriptor_heap_description = {};
	d3d_descriptor_heap_description.NumDescriptors = k_back_buffer_count;
	d3d_descriptor_heap_description.Type = d3d_descriptor_heap_type;

	wrl::ComPtr<ID3D12DescriptorHeap> d3d_descriptor_heap;
	throw_if_failed(d3d_device->CreateDescriptorHeap(
		&d3d_descriptor_heap_description,
		IID_PPV_ARGS(&d3d_descriptor_heap)));

	return d3d_descriptor_heap;
}

void c_d3d_globals::update_render_target_views(
	wrl::ComPtr<ID3D12Device2> d3d_device,
	wrl::ComPtr<IDXGISwapChain4> dxgi_swap_chain,
	wrl::ComPtr<ID3D12DescriptorHeap> d3d_descriptor_heap,
	D3D12_DESCRIPTOR_HEAP_TYPE d3d_descriptor_heap_type,
	wrl::ComPtr<ID3D12Resource>* d3d_back_buffers,
	uint32 back_buffer_count)
{
	uint32 descriptor_size = d3d_device->GetDescriptorHandleIncrementSize(d3d_descriptor_heap_type);

	CD3DX12_CPU_DESCRIPTOR_HANDLE d3d_cpu_descriptor_handle(d3d_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

	for (uint32 index = 0; index < back_buffer_count; index++)
	{
		wrl::ComPtr<ID3D12Resource> back_buffer;
		throw_if_failed(dxgi_swap_chain->GetBuffer(index, IID_PPV_ARGS(&back_buffer)));

		d3d_device->CreateRenderTargetView(back_buffer.Get(), nullptr, d3d_cpu_descriptor_handle);

		d3d_back_buffers[index] = back_buffer;

		d3d_cpu_descriptor_handle.Offset(descriptor_size);
	}
}

wrl::ComPtr<ID3D12CommandAllocator> c_d3d_globals::create_command_allocator(
	wrl::ComPtr<ID3D12Device2> d3d_device,
	D3D12_COMMAND_LIST_TYPE d3d_command_list_type)
{
	wrl::ComPtr<ID3D12CommandAllocator> d3d_command_allocator;
	throw_if_failed(d3d_device->CreateCommandAllocator(
		d3d_command_list_type,
		IID_PPV_ARGS(&d3d_command_allocator)));

	return d3d_command_allocator;
}

wrl::ComPtr<ID3D12GraphicsCommandList> c_d3d_globals::create_command_list(
	wrl::ComPtr<ID3D12Device2> d3d_device,
	wrl::ComPtr<ID3D12CommandAllocator> d3d_command_allocator,
	D3D12_COMMAND_LIST_TYPE d3d_command_list_type)
{
	wrl::ComPtr<ID3D12GraphicsCommandList> d3d_command_list;
	throw_if_failed(d3d_device->CreateCommandList(
		0,
		d3d_command_list_type,
		d3d_command_allocator.Get(),
		nullptr,
		IID_PPV_ARGS(&d3d_command_list)));

	throw_if_failed(d3d_command_list->Close());

	return d3d_command_list;
}

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

	throw_if_failed(m_d3d_command_list->Close());

	ID3D12CommandList* d3d_command_lists[] = { m_d3d_command_list.Get() };

	m_d3d_command_queue->ExecuteCommandLists(
		_countof(d3d_command_lists),
		d3d_command_lists);

	throw_if_failed(m_dxgi_swap_chain->Present(1, 0));

	m_frame_fence_values[m_back_buffer_index] = m_fence.signal(m_d3d_command_queue);

	m_back_buffer_index = m_dxgi_swap_chain->GetCurrentBackBufferIndex();

	m_fence.wait_for_fence_value(m_frame_fence_values[m_back_buffer_index]);
}
