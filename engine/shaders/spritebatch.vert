#version 410 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 origin_extents;
layout(location = 2) in vec4 uv_lower_upper;
layout(location = 3) in vec4 color;
layout(location = 4) in int tex_id;

uniform mat4 transformMatrix;

out vec2 vert_out_vec_u;
out vec2 vert_out_vec_v;
out vec4 vert_out_color;
out vec2 vert_out_uv_lower;
out vec2 vert_out_uv_upper;
out int vert_out_tex_id;

void main() {
	vec2 position2D = position.xy;
	vec2 origin = origin_extents.xy;
	vec2 extents = origin_extents.zw;
	vec2 uv_lower = uv_lower_upper.xy;
	vec2 uv_upper = uv_lower_upper.zw;

	// Rotate sprite around the origin
	vec2 rect_tr = position2D + vec2(extents.x, 0.0);
	vec2 rect_bl = position2D + vec2(0.0, extents.y);

	float rotation = position.w;

	mat2 rot_mat = mat2(cos(rotation), -sin(rotation), 
		sin(rotation), cos(rotation));
	
	vec2 origin_world_space = position2D + origin;

	position2D -= origin_world_space;
	rect_tr -= origin_world_space;
	rect_bl -= origin_world_space;

	position2D = rot_mat * position2D;
	rect_tr = rot_mat * rect_tr;
	rect_bl = rot_mat * rect_bl;

	position2D += origin_world_space;
	rect_tr += origin_world_space;
	rect_bl += origin_world_space;

	// Transform the sprite via the transform matrix
	vec4 transformed_pos = transformMatrix * vec4(position2D, position.z, 1.0);
	vec4 transformed_tr = transformMatrix * vec4(rect_tr, position.z, 1.0);
	vec4 transformed_bl = transformMatrix * vec4(rect_bl, position.z, 1.0);

	// Invert the y-axis so that (0, 0) is the top left corner
	transformed_pos.y = -transformed_pos.y;
	transformed_tr.y = -transformed_tr.y;
	transformed_bl.y = -transformed_bl.y;

	// Pass results to geometry shader
	gl_Position = transformed_pos;
	vert_out_vec_u = (transformed_tr - transformed_pos).xy;
	vert_out_vec_v = (transformed_bl - transformed_pos).xy;
	vert_out_color = color;
	vert_out_uv_lower = uv_lower;
	vert_out_uv_upper = uv_upper;
	vert_out_tex_id = tex_id;
}