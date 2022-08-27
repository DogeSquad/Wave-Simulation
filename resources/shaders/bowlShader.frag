#version 330 core

uniform sampler2D backgroundTexture;
uniform sampler2D grassTexture;

uniform float fogDensity;
uniform float fogRadius;
uniform float NEAR;
uniform float FAR;
uniform vec3 sunDir;

in vec3 normal;
in vec2 texCoords;
  
float fogFallOff(float depth)
{
    return smoothstep(fogDensity * fogRadius, fogRadius, depth);
}

void main()
{
    vec3 diffuse = max(0.1f, dot(normalize(normal), sunDir)) * texture(grassTexture, 50.0f * texCoords).xyz;

    float depth = (gl_FragCoord.z / gl_FragCoord.w) / (FAR - NEAR);

    gl_FragColor = vec4(mix(diffuse, texture(backgroundTexture, gl_FragCoord.xy).xyz, fogFallOff(depth)), 1.0f);
}   