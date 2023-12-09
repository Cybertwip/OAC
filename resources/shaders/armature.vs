#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords1;
layout (location = 3) in vec2 aTexCoords2;
layout (location = 4) in vec3 tangent;
layout (location = 5) in vec3 bitangent; // Corrected location to 5
layout (location = 6) in ivec4 boneIds;  // Corrected location to 6
layout (location = 7) in vec4 weights;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

out vec2 TexCoords1;
out vec2 TexCoords2;
out vec3 Normal;
out vec3 FragPos;
flat out int boneId;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    TexCoords1 = aTexCoords1;
    TexCoords2 = aTexCoords2;
    FragPos = vec3(model * vec4(aPos, 1.0));
    boneId = boneIds.x;  // You may adjust this based on how you want to use boneIds.
}
