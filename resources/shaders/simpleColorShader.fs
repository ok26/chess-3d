#version 330 core
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;

uniform vec3 color;
uniform sampler2D diffuseMap;

void main() {
    FragColor = texture(diffuseMap, TexCoords) * vec4(color, 0.3);
}