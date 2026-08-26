#version 330 core
layout (location = 0) out vec4 outNormal; 

in vec3 vNormal;
void main() {
    outNormal = vec4(normalize(vNormal) * 0.5 + 0.5, 1.0);
}