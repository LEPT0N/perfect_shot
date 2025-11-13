// Copyright (c) Dan Mitchell
// vertex_shader.hlsl

struct s_vertex_shader_input
{
	float3 position : POSITION;
	float3 color : COLOR;
};

struct s_vertex_shader_output
{
	float4 position : SV_Position;
	float4 color : COLOR;
};

// ConstantBuffer<matrix> g_model_view_projection_matrix : register(b0);

s_vertex_shader_output main(s_vertex_shader_input input)
{
	s_vertex_shader_output output;
	// output.position = mul(g_model_view_projection_matrix, float4(input.position, 1.0f));
	output.position = float4(input.position, 1.0f);
	output.color = float4(input.color, 1.0f);

	return output;
}
