#version 330 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;

void main()
{
    gl_Position = proj_mat * view_mat * model_mat * vec4(aPos, 1.0);
}   