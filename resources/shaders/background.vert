#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 proj_mat;
uniform mat4 view_mat;

out vec3 rayDir;

void main()
{
    vec4 reverseVec;

    reverseVec = vec4(aPos.xy, 0.0f, 1.0f);
    reverseVec = inverse(proj_mat) * reverseVec;

    reverseVec.w = 0.0f;
    reverseVec *= view_mat;

    rayDir = normalize(vec3(reverseVec));
    gl_Position = vec4(aPos.xy, 0.0f, 1.0f);
}   