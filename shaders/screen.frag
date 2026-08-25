#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    // Sample the color from the FBO texture
    FragColor = texture(screenTexture, TexCoords);
}