#version 330 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0

smooth out float height;
smooth out vec2 coordinates;
smooth out vec3 normal;

uniform sampler1D waveParamsTex;

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;

uniform vec3 sunDir;
uniform vec2 tileSize;

uniform float time;

uniform float g;

//const int m = 5;
//uniform float amplitudes[m];
//uniform float phases[m];
//uniform vec2 directions[m];
float meanDepth = 0.0f;

vec3 wavePointPosition(float alpha, float beta)
{
    vec3 position = vec3(0.0f);
    for (int i = 0; i < textureSize(waveParamsTex, 0); i++)
    {
        vec4 params = texelFetch(waveParamsTex, i, 0);
        float k = length(params.zw); 
        float omega = sqrt(g * k); // For deep water
        //float omega = sqrt(g * k * tanh(k * meanDepth)); // For shallow water
        float delta = params.z * alpha + params.w * beta - omega * time - params.y;

        position.x += (params.z / k) * (params.x  /* / tanh(k * meanDepth) */) * sin(delta);
        position.z += (params.w / k) * (params.x /* / tanh(k * meanDepth) */) * sin(delta);
        position.y += params.x * cos(delta);
    }
    return vec3(alpha - position.x, position.y, beta - position.z);
}

void main()
{
    vec3 wavePoint = wavePointPosition(aPos.x, aPos.z);
    
    vec3 deltaAlpha = wavePointPosition(aPos.x + tileSize.x, aPos.z) - wavePoint;
    vec3 deltaBeta = wavePointPosition(aPos.x, aPos.z + tileSize.y) - wavePoint;

    normal = normalize(cross(deltaAlpha, deltaBeta));
    gl_Position = proj_mat * view_mat * model_mat * vec4(wavePoint, 1.0f);
    height = wavePoint.y;
    coordinates = wavePoint.xz;
}   