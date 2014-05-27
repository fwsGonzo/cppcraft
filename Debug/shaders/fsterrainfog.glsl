#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform vec2 nearPlaneHalfSize;

in  vec2 in_vertex;
out vec2 texCoord;
out vec4 eye_direction;

void main(void)
{
	texCoord = in_vertex;
	gl_Position = vec4(in_vertex * 2.0 - 1.0, 0.0, 1.0);
	
	eye_direction = vec4(gl_Position.xy * nearPlaneHalfSize, -1.0, 1.0);
}
#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D terrain;
uniform sampler2D skytexture;
uniform sampler2D depthtexture;

uniform vec3 sunAngle;

uniform mat4 matview;
uniform vec3 cameraPos;
uniform vec3 worldOffset;
uniform float timeElapsed;

uniform mat4 matviewport;
uniform mat4 matmvp;
uniform mat4 matproj;

in vec2 texCoord;
in vec4 eye_direction;

const float ZFAR
const float ZNEAR

#include "noise3.glsl"

float fogDensity(in vec3  ray,
				 in vec3  point,
				 in float depth,
				 in float fogheight)
{
	// distance in fog is calculated with a simple intercept theorem
	float len = max(0.0, fogheight - point.y) / max(0.1, ray.y);
	float fogdepth = min(ZFAR * 0.5, len) / (ZFAR * 0.5);
	
	// how far are we from some arbitrary height?
	float foglevel = 1.0 - min(1.0, abs(point.y - fogheight) / 64.0);
	
	float above = step(point.y, fogheight);
	foglevel = (1.0 - above) * foglevel + above * pow(foglevel, 4.0);
	
	float noise = snoise(point * 0.01) + snoise(point * 0.04);
	noise = (noise + 2.0) * 0.25;
	
	return (noise + fogdepth) * 0.5 * foglevel;
}

float linearizeDepth(in vec2 uv)
{
	float d = texture2D(depthtexture, uv).x;
	return ZNEAR / (ZFAR - d * (ZFAR - ZNEAR)) * ZFAR;
}

void main()
{
	// base color
	vec4 color = texture2D(terrain, texCoord);
	// depth from alpha
	#define depth  color.a
	
	// reconstruct eye coordinates
	vec4 cofs = eye_direction * matview;
	cofs.xyz *= linearizeDepth(texCoord);
	vec3 ray = normalize(-cofs.xyz);
	// to world coordinates
	vec3 wpos = cofs.xyz + vec3(0.0, cameraPos.y, 0.0) - worldOffset;
	
	// volumetric fog
	float foglevel  = fogDensity(ray, wpos, depth, 76.0);
	foglevel *= 2.0 * depth;
	
	const vec3 fogBaseColor = vec3(0.9);
	const vec3 sunBaseColor = vec3(1.0, 0.8, 0.5);
	
	float sunAmount = max(0.0, dot(-ray, sunAngle)) * 0.8 * depth;
	float fogAmount = foglevel * (1.0 - sunAmount);
	color.rgb = mix(color.rgb, fogBaseColor, fogAmount);
	color.rgb = mix(color.rgb, sunBaseColor, sunAmount);
	
	//color.rgb = wpos.xyz / ZFAR;
	//color.rgb = vec3(foglevel);
	
	// mix in fog
	vec3 skyColor = texture2D(skytexture, texCoord).rgb;
	color.rgb = mix(color.rgb, skyColor, smoothstep(0.5, 1.0, depth));
	
	gl_FragData[0] = color;
}

#endif
