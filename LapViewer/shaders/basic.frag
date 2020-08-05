#version 330 core

uniform vec3 color = vec3(1.0, 1.0, 1.0);

out vec4 outColor;

void main(){
  outColor = vec4(color, 1.0);
}