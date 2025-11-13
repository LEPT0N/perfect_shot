// Copyright (c) Dan Mitchell
// pixel_shader.hlsl

struct s_pixel_shader_input
{
	float4 color : COLOR;
};

struct s_pixel_shader_output
{
	float4 color : SV_TARGET;
};

s_pixel_shader_output main(s_pixel_shader_input input)
{
	s_pixel_shader_output output;
	output.color = input.color;

	return output;
}
