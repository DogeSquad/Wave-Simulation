#version 330 core 
#define M_PI 3.1415926535897932384626433832795f
in vec3 rayDir;

uniform vec3 skyColor;
uniform vec3 fogColor;
uniform vec3 sunColor;
uniform vec2 uRes;

uniform vec3 viewPos;
uniform vec3 sunPos;

float gradFunc(float y) 
{
    return min(1.0f, (y - 1.0f) * (y - 1.0f) * (y - 1.0f) * (y - 1.0f));
}
float greatCircleDistance(vec3 a, vec3 b)
{
    float euclideanDst = distance(a, b);
    float angle = asin(euclideanDst * 0.5f);  // Assumed R is 1.0f

    return acos(dot(a, b));
}

void main()
{   
    //vec3 sunPos_norm = normalize(sunPos);
    //if (greatCircleDistance(sunPos_norm, rayDir) < 0.02f) 
    //{
    //    gl_FragColor = vec4(sunColor, 1.0f);
    //    return;
    //}
    gl_FragColor = vec4(mix(skyColor, fogColor, gradFunc(rayDir.y)), 1.0f);
    //float sunPow = pow(max(0.0f, 40.0f * dot(rayDir, sunPos_norm) - 39.0f), 8);
    //gl_FragColor = mix(gl_FragColor, vec4(sunColor, 1.0f), sunPow);

    gl_FragDepth = 0.9999f;
}  