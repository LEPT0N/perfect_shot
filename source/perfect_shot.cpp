// Copyright (c) Dan Mitchell
// perfect_shot.cpp
//
// FPS where 'aim and reaction speed don't matter'.
// You equip two weapons. If an enemy is visible in the reticle, constant optimal DPS is done.
// Focus on positioning and movement abilities.
//
// Ideas:
//
// Being dead isn't fun, and not controlling your spawn isn't fun. When you die you teleport back to a 'spawn lounge' where you can pick your weapons, locomotion, and spawn point.
//
// TODO:
//
// Split into two projects: engine and perfect_shot

// ------- ------- ------- ------- ------- ------- -------
// headers

#include "common.h"

#include "graphics\graphics.h"
#include "utilities\time_utilities.h"

// ------- ------- ------- ------- ------- ------- -------
// private declarations

LRESULT CALLBACK window_proc(
	HWND window,
	UINT message,
	WPARAM wparam,
	LPARAM lparam);

void initialize_cube();
void render_cube();

void update();
void render();

// ------- ------- ------- ------- ------- ------- -------
// data

c_d3d_globals g_d3d_globals;
c_frame_timer g_timer;

// ------- ------- ------- ------- ------- ------- -------
// private definitions

int CALLBACK wWinMain(
	_In_ HINSTANCE instance,
	_In_opt_ HINSTANCE previous_instance,
	_In_ PWSTR command_line,
	_In_ int command_show)
{
	UNREFERENCED_PARAMETER(previous_instance);
	UNREFERENCED_PARAMETER(command_line);
	UNREFERENCED_PARAMETER(command_show);

	g_d3d_globals.initialize(instance, &window_proc);

	initialize_cube();

	MSG message = {};
	while (message.message != WM_QUIT)
	{
		if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	g_d3d_globals.destroy();

	return 0;
}

LRESULT CALLBACK window_proc(
	HWND window,
	UINT message,
	WPARAM wparam,
	LPARAM lparam)
{
	bool handled = false;

	if (g_d3d_globals.is_initialized())
	{
		switch (message)
		{
		case WM_PAINT:
			handled = true;
			update();
			render();
			break;

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		case WM_SYSCHAR:
		case WM_SIZE:
			handled = true;
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			handled = true;
			break;
		}
	}

	if (handled)
	{
		return 0;
	}
	else
	{
		return DefWindowProc(window, message, wparam, lparam);
	}
}

//          6----------------7
//         /|               /|
//        / |              / |
//       /  |             /  |
//      /   |            /   |
//     /    |           /    |
//    1----------------5     |
//    |     |          |     |
//    |     2----------|-----4
//    |    /           |    /
//    |   /            |   /
//    |  /             |  /
//    | /              | /
//    |/               |/
//    0----------------3
//
//
//


const s_vertex k_unit_cube_verticies[8] =
{
	{ {-1.0f, -1.0f, -1.0f}, {0.8f, 0.0f, 0.8f}, },
	{ {-1.0f, -1.0f, 1.0f}, {0.0f, 0.8f, 0.0f}, },
	{ {-1.0f, 1.0f, -1.0f}, {0.0f, 0.8f, 0.0f}, },
	{ {1.0f, -1.0f, -1.0f}, {0.0f, 0.8f, 0.0f}, },

	{ {1.0f, 1.0f, -1.0f}, {0.8f, 0.0f, 0.8f}, },
	{ {1.0f, -1.0f, 1.0f}, {0.8f, 0.0f, 0.8f}, },
	{ {-1.0f, 1.0f, 1.0f}, {0.8f, 0.0f, 0.8f}, },
	{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.8f, 0.0f}, },
};

const uint16 k_unit_cube_indicies[36] =
{
	0, 3, 1, 3, 5, 1,
	0, 1, 2, 1, 6, 2,
	0, 2, 3, 2, 4, 3,
	2, 6, 4, 6, 7, 4,
	1, 5, 6, 6, 5, 7,
	3, 4, 5, 4, 7, 5,
};

D3D12_RECT g_d3d_scissor_rectangle;
D3D12_VIEWPORT g_d3d_viewport;

wrl::ComPtr<ID3D12Resource> g_d3d_resource_cube_vertex_buffer;
D3D12_VERTEX_BUFFER_VIEW g_d3d_vertex_buffer_view;

wrl::ComPtr<ID3D12Resource> g_d3d_resource_cube_index_buffer;
D3D12_INDEX_BUFFER_VIEW g_d3d_index_buffer_view;

wrl::ComPtr<ID3D12DescriptorHeap> g_descriptor_heap;

wrl::ComPtr<ID3D12RootSignature> g_d3d_root_signature;

wrl::ComPtr<ID3D12PipelineState> g_d3d_pipeline_state;

void initialize_cube()
{
	g_d3d_scissor_rectangle = CD3DX12_RECT(0, 0, INT32_MAX, INT32_MAX);
	g_d3d_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, 1280.0f, 720.0f);

	c_command_queue* copy_command_queue = g_d3d_globals.get_copy_command_queue();
	wrl::ComPtr<ID3D12GraphicsCommandList> d3d_command_list = copy_command_queue->get_command_list();

	// Copy vertex data to the GPU
	wrl::ComPtr<ID3D12Resource> g_d3d_resource_intermediate_vertex_bufer;
	create_resource(
		g_d3d_globals.get_device(),
		d3d_command_list,
		&g_d3d_resource_cube_vertex_buffer,
		&g_d3d_resource_intermediate_vertex_bufer,
		_countof(k_unit_cube_verticies), sizeof(s_vertex), k_unit_cube_verticies);

	g_d3d_vertex_buffer_view.BufferLocation = g_d3d_resource_cube_vertex_buffer->GetGPUVirtualAddress();
	g_d3d_vertex_buffer_view.SizeInBytes = sizeof(k_unit_cube_verticies);
	g_d3d_vertex_buffer_view.StrideInBytes = sizeof(s_vertex);

	// Copy index data to the GPU
	wrl::ComPtr<ID3D12Resource> g_d3d_resource_intermediate_index_bufer;
	create_resource(
		g_d3d_globals.get_device(),
		d3d_command_list,
		&g_d3d_resource_cube_index_buffer,
		&g_d3d_resource_intermediate_index_bufer,
		_countof(k_unit_cube_indicies), sizeof(uint16), k_unit_cube_indicies);

	g_d3d_index_buffer_view.BufferLocation = g_d3d_resource_cube_index_buffer->GetGPUVirtualAddress();
	g_d3d_index_buffer_view.Format = DXGI_FORMAT_R16_UINT;
	g_d3d_index_buffer_view.SizeInBytes = sizeof(k_unit_cube_indicies);

	// create the heap for the depth stencil view.
	g_descriptor_heap = create_descriptor_heap(
		g_d3d_globals.get_device(),
		1,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// Load the shaders.
	wrl::ComPtr<ID3DBlob> d3d_blob_vertex_shader;
	throw_if_failed(D3DReadFileToBlob(L"vertex_shader.cso", &d3d_blob_vertex_shader));

	wrl::ComPtr<ID3DBlob> d3d_blob_pixel_shader;
	throw_if_failed(D3DReadFileToBlob(L"pixel_shader.cso", &d3d_blob_pixel_shader));

	// define s_vertex_shader_input
	D3D12_INPUT_ELEMENT_DESC shader_input_layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// Get the root signature
	D3D12_FEATURE_DATA_ROOT_SIGNATURE d3d_feature_root_signature =
		get_root_signature_version(g_d3d_globals.get_device());

	D3D12_ROOT_SIGNATURE_FLAGS d3d_root_signature_flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// b0 can be interpreted as a matrix by the pixel shader
	CD3DX12_ROOT_PARAMETER1 d3d_root_parameters[1];
	d3d_root_parameters[0].InitAsConstants(sizeof(dx::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC d3dx_root_signature_description;
	d3dx_root_signature_description.Init_1_1(
		_countof(d3d_root_parameters),
		d3d_root_parameters,
		0,
		nullptr,
		d3d_root_signature_flags);

	// serialize the root signature
	wrl::ComPtr<ID3DBlob> d3d_blob_root_signature;
	wrl::ComPtr<ID3DBlob> d3d_blob_error;
	throw_if_failed(D3DX12SerializeVersionedRootSignature(
		&d3dx_root_signature_description,
		d3d_feature_root_signature.HighestVersion,
		&d3d_blob_root_signature,
		&d3d_blob_error));

	// Create the root signature
	throw_if_failed(g_d3d_globals.get_device()->CreateRootSignature(
		0,
		d3d_blob_root_signature->GetBufferPointer(),
		d3d_blob_root_signature->GetBufferSize(),
		IID_PPV_ARGS(&g_d3d_root_signature)));

	// Create the pipeline state stream

	D3D12_RT_FORMAT_ARRAY d3d_rt_format_array = {};
	d3d_rt_format_array.NumRenderTargets = 1;
	d3d_rt_format_array.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	struct s_pipeline_state_stream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE d3dx_root_signature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT d3dx_input_layout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY d3dx_primitive_topology;
		CD3DX12_PIPELINE_STATE_STREAM_VS d3dx_vertex_shader;
		CD3DX12_PIPELINE_STATE_STREAM_PS d3dx_pixel_shader;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT d3dx_depth_stencil_format;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS d3dx_render_target_formats;
	} pipeline_state_stream;

	pipeline_state_stream.d3dx_root_signature = g_d3d_root_signature.Get();
	pipeline_state_stream.d3dx_input_layout = { shader_input_layout , _countof(shader_input_layout) };
	pipeline_state_stream.d3dx_primitive_topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipeline_state_stream.d3dx_vertex_shader = CD3DX12_SHADER_BYTECODE(d3d_blob_vertex_shader.Get());
	pipeline_state_stream.d3dx_pixel_shader = CD3DX12_SHADER_BYTECODE(d3d_blob_pixel_shader.Get());
	pipeline_state_stream.d3dx_depth_stencil_format = DXGI_FORMAT_D32_FLOAT;
	pipeline_state_stream.d3dx_render_target_formats = d3d_rt_format_array;

	D3D12_PIPELINE_STATE_STREAM_DESC d3d_pipeline_state_stream_description =
	{
		sizeof(s_pipeline_state_stream),
		& pipeline_state_stream,
	};
	throw_if_failed(g_d3d_globals.get_device()->CreatePipelineState(
		&d3d_pipeline_state_stream_description,
		IID_PPV_ARGS(&g_d3d_pipeline_state)));

	// Wait for all of that to happen.
	uint64 fence_value = copy_command_queue->execute_command_list(d3d_command_list);
	copy_command_queue->wait_for_fence_value(fence_value);

	// content_loaded = true

	//ENDED_HERE; // ResizeDepthBuffer
}

void render_cube()
{
}

void update()
{
	g_timer.update();
}

void render()
{
	g_d3d_globals.render();

	render_cube();
}
