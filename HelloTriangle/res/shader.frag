#version 450

layout(location = 0) out vec4 oColor;

layout(location = 0) in vec3 fragColor;

void main() {
	oColor = vec4(fragColor, 1.0);
}