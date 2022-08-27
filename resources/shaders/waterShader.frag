#version 330 core

smooth in vec3 fragPos;
smooth in vec3 normal;

uniform sampler2D backgroundTexture;

uniform vec3 camPos;

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
    float blending = max(0.0f, (1.0f - n.y - noise(75.0f * fragPos.xz)) * fragPos.y / 6.0f);
    diffuse = mix(diffuse, vec4(vec3(0.95f), 1.0f), blending);
    diffuse *= vec4(diffuse.xyz * max(0.1f, dot(n, sunDir)), 1 - pow(blending, 5) + 0.1f); // Shadows
	
    float depth = (gl_FragCoord.z / gl_FragCoord.w) / (FAR - NEAR);
	
    gl_FragColor = mix(diffuse, vec4(texture(backgroundTexture, gl_FragCoord.xy).xyz, 1.0f), fogFallOff(depth));



	//vec3 upwelling = vec3(0f, 0.2f, 0.3f);
	//vec3 sky = vec3(0.69f,0.84f,1f);
	//vec3 air = vec3(0.1f,0.1f,0.1f);
	//float nSnell = 1.34f;
	//float Kdiffuse = 0.91f;
	//
	//float reflectivity;
	//vec3 nI = normalize(sunDir);            // Incident Ray (maybe switch)
	//vec3 nN = normalize(normal);			 // Surface Normal
	//float costhetai = abs(dot(nI, nN));
	//float thetai = acos(costhetai);
	//float sinthetat = sin(thetai)/nSnell;
	//float thetat = asin(sinthetat);
	//
	//if(thetai == 0.0)
	//{
	//	reflectivity = (nSnell - 1)/(nSnell + 1);
	//	reflectivity = reflectivity * reflectivity;
	//}
	//else
	//{
	//	float fs = sin(thetat - thetai) / sin(thetat + thetai);
	//	float ts = tan(thetat - thetai) / tan(thetat + thetai);
	//	reflectivity = 0.5 * ( fs*fs + ts*ts );
	//}
	//vec3 dPE = fragPos - camPos;
	//float dist = length(dPE) * Kdiffuse;
	//dist = exp(-dist);
	//
	//gl_FragColor = vec4(dist * (reflectivity * sky + (1-reflectivity) * upwelling) + (1-dist)* air, 1.0f);


}   