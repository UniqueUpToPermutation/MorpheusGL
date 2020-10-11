#version 410 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec2 vert_out_vec_u[];
in vec2 vert_out_vec_v[];
in vec4 vert_out_color[];
in vec2 vert_out_uv_lower[];
in vec2 vert_out_uv_upper[];
in int vert_out_tex_id[];

flat out vec4 color;
flat out int tex_id;

out vec2 uv;

void main() {    
	color = vert_out_color[0];
	tex_id = vert_out_tex_id[0];

	vec2 uv_lower = vert_out_uv_lower[0];
	vec2 uv_upper = vert_out_uv_upper[0];

	vec4 vec_u = vec4(vert_out_vec_u[0], 0.0, 0.0);
	vec4 vec_v = vec4(vert_out_vec_v[0], 0.0, 0.0);

	// Produce a quad with correct uv's
	uv = uv_lower;
    gl_Position = gl_in[0].gl_Position;

	EmitVertex();

	uv = vec2(uv_lower.x, uv_upper.y);
	gl_Position = gl_in[0].gl_Position + vec_v;

	EmitVertex();

	uv = vec2(uv_upper.x, uv_lower.y);
	gl_Position = gl_in[0].gl_Position + vec_u;

	EmitVertex();

	uv = uv_upper;
	gl_Position = gl_in[0].gl_Position + vec_u + vec_v;

	EmitVertex();
	EndPrimitive();
}  