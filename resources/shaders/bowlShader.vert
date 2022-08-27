#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 normal;
out vec2 texCoords;

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;

void main()
{
    normal = aNormal;
    texCoords = aTexCoords;
    gl_Position = proj_mat * view_mat * model_mat * vec4(aPos, 1.0f);
}   