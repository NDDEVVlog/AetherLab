#version 330 core
out vec4 FragColor;


in vec3 FragPos;  
in vec2 vTexCoord;

uniform vec3 lightColor;
uniform sampler2D texture1;

void main()
{
    vec4 texColor = texture(texture1, vTexCoord);
    FragColor = vec4(texColor.rgb * lightColor, texColor.a);
}
