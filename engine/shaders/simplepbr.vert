﻿#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoords;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

out vec2 vTexcoords; 
out vec3 vNormal;
out vec3 vTanget;
out vec3 vPosition;

uniform mat4 world;
uniform mat4 worldInverseTranspose;
uniform mat4 view; 
uniform mat4 projection; 

void main()
{
    vTexcoords = texcoords;
	vNormal = (worldInverseTranspose * vec4(normal, 0.0)).xyz;
	vec4 worldPosition = world * vec4(position, 1.0f);
	vPosition = worldPosition.xyz;
	vTanget = (worldInverseTranspose * vec4(tangent, 0.0)).xyz;
    gl_Position = projection * view * worldPosition;
}