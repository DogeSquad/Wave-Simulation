#version 330 core

smooth in float height;
smooth in vec2 coordinates;
smooth in vec3 normal;

uniform sampler2D backgroundTexture;

uniform float fogDensity;
uniform float fogRadius;
uniform float NEAR;
uniform float FAR;
uniform vec3 sunDir;

uniform vec3 waterColor;

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}
float noise(vec2 p){
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
		mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
	return res*res;
}
float fogFallOff(float depth)
{
    return smoothstep(fogDensity * fogRadius, fogRadius, depth);
}
void main()
{
    vec3 n = normalize(normal);

    vec4 diffuse = vec4(waterColor, 1.0f);
    //vec4 diffuse = vec4(mix(vec3(0.95f), color, n.y), 1.0f); // Foam
    float blending = max(0.0f, (1.0f - n.y - noise(75.0f * coordinates.xy)) * height / 6.0f);
    diffuse = mix(diffuse, vec4(vec3(0.95f), 1.0f), blending);
    diffuse *= vec4(diffuse.xyz * max(0.1f, dot(n, sunDir)), 1 - pow(blending, 5) + 0.1f); // Shadows

    float depth = (gl_FragCoord.z / gl_FragCoord.w) / (FAR - NEAR);

    gl_FragColor = mix(diffuse, vec4(texture(backgroundTexture, gl_FragCoord.xy).xyz, 1.0f), fogFallOff(depth));
}   