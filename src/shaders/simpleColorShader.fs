#version 330 core
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;

uniform vec3 color;
uniform sampler2D diffuseMap;
uniform float alpha;

void main() {
    FragColor = texture(diffuseMap, TexCoords) * vec4(color, alpha);
}