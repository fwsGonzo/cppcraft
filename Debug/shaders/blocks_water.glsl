#version 130
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
//precision highp float;
precision mediump float;

uniform mat4 matproj;
uniform mat4 matview;
uniform vec3 vtrans;

uniform vec3 worldOffset;
uniform vec3 lightVector;
uniform float frameCounter;

in vec3 in_vertex;
in vec3 in_normal;
in vec3 in_texture;
in vec4 in_biome;
in vec4 in_color;
in vec4 in_color2;

out vec4 lightdata;
out vec4 torchlight;

out float vertdist;
out vec3 v_eye;
out vec3 v_ldir;
out vec3 v_half;
out vec3 v_normal;
out vec3 l_normal;
out vec3 l_reflect;

out vec4 waves;

const float VERTEX_SCALE

void main(void)
{
	vec4 position = vec4(in_vertex / VERTEX_SCALE + vtrans, 1.0);
	position = matview * position;
	vertdist = length(position.xyz);
	gl_Position = matproj * position;
	
	// light and eye direction in view space
	v_eye = -position.xyz / vertdist;
	v_ldir = mat3(matview) * lightVector;
	v_half = normalize(v_eye + v_ldir);
	// reflect light in view space using view-normal
	l_normal = in_normal;
	v_normal = mat3(matview) * in_normal;
	
	// local reflection vector
	l_reflect = reflect(-v_eye, v_normal) * mat3(matview);
	// galactic coordinates
	vec3 w_vertex  = position.xyz * mat3(matview) - worldOffset;
	
	// Current water vertex position, scaled down a lot, then motion added in one direction	
	// output to fragment shader
	float timer = frameCounter / 35.0;
	
	waves.xy  = w_vertex.xz * vec2(0.10, 0.02) + timer * vec2(0.2, 0.0);
	//waves.xy = vec2(cos(waves.x), sin(waves.x));
	waves.zw  = w_vertex.xz * vec2(0.25, 0.05) + timer * vec2(0.25, 0.05);
	//waves.zw = vec2(cos(waves.y), sin(waves.y));
	//waves.xy  = w_vertex.xz * vec2(0.10, 0.02) + timer * vec2(0.2, 0.0);
	//waves.zw  = w_vertex.xz * vec2(0.25, 0.05) + timer * vec2(0.25, 0.05);
	
	lightdata   = in_color;
	torchlight  = in_color2;
}

#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable
precision highp float;

uniform sampler2D underwatermap;
uniform sampler2D reflectionmap;
uniform samplerCube skymap;

uniform mat4 matview;
uniform vec3 screendata;

uniform float frameCounter;
uniform float daylight;
uniform vec4  playerLight;
uniform float modulation;
uniform int playerSubmerged;

in vec4 lightdata;
in vec4 torchlight;

in float vertdist;
in vec3 v_ldir;
in vec3 v_half;
in vec3 v_eye;
in vec3 v_normal;

in vec3 l_normal;
in vec3 l_reflect;
in vec4 waves; // wave positions

const float ZFAR

#include "srdnoise.glsl"

void main(void)
{
	// derivative simplex noise
	vec2 grad, grad2;
	srdnoise(waves.xy, 0.0, grad);
	srdnoise(waves.zw, 0.0, grad2);
	
	// final x/z values
	grad = grad * 0.7 + grad2 * 0.3;
	
	vec3 tx = vec3(0.0, 1.0, grad.x);
	vec3 ty = vec3(1.0, 0.0, grad.y);
	
	vec3 Normal = vec3(grad.x, grad.y, 0.0);
	Normal.y += 32; // make sure its extremely positive
	Normal = normalize(Normal);
	
	vec3 vNormal = mat3(matview) * Normal;
	#define viewNormal vNormal
	#define fNormal    vNormal
	
	vec3 vReflect   = l_reflect + Normal * 0.125;
	
	
	//#define vEye   v_eye
	//#define vLight v_ldir
	vec3 vEye   = normalize(v_eye);
	vec3 vLight = normalize(v_ldir);
	
	//----- fresnel term -----
	
	float fresnel = dot(vEye, fNormal);
	fresnel = 0.02 + 0.97 * pow(1.0 - fresnel, 5.0);
	
	//----- REFRACTION -----
	
	// screenspace tex coordinates
	vec2 texCoord = gl_FragCoord.xy / screendata.xy;
	
	// start with underwater texture
	float wdepth = vertdist / ZFAR;
	
	// fake refraction
	//refcoord.y -= viewNormal.z * 0.25 * wdepth;
	// wave modulation
	vec2 refcoord = texCoord + Normal.xz * 0.01 * (0.25 + 0.75 * wdepth);
	
	// read underwater, use as base color
	vec4 underw = texture2D(underwatermap, refcoord);
	
	//----- REFLECTIONS -----
	
	vec4 wreflection = vec4(0.0);
	if (playerSubmerged == 0)
	{
		// sky reflection
		//vec3 reflection = textureCube(skymap, vReflect).rgb * daylight;
		//color.rgb = mix(color.rgb, reflection, 0.3 * fresnel);
		
		// world/terrain reflection
		wreflection = texture2D(reflectionmap, refcoord);
		wreflection.rgb = mix(wreflection.rgb, vec3(0.7), 0.25);
	}
	
	//----- SEACOLOR -----
	
	const vec3 deepwater    = vec3(0.26, 0.42, 0.50);
	const vec3 shallowwater = vec3(0.35, 0.55, 0.60);
	
	float dep = vertdist / ZFAR;
	// if player is underwater, we need to see the sky properly:
	dep = underw.a - dep;
	
	// minimum depth: 0.06, grows very quickly
	if (playerSubmerged == 0)
	dep = 1.0 - smoothstep(0.0, 0.06, dep) * (1.0 - fresnel);
	
	// create water "color"
	vec3 waterColor = mix(deepwater, shallowwater, dep) * daylight;
	// mix water color and underwater / reflection
	waterColor = mix(waterColor, underw.rgb, dep) * daylight;
	// create final color value
	vec4 color = vec4(waterColor, 1.0);
	
	// add reflections
	if (playerSubmerged == 0)
	{
		float reflevel = min(1.0, 0.7 * fresnel + underw.a * 0.3);
		color.rgb = mix(color.rgb, wreflection.rgb, reflevel);
	}
	
	//- lighting (we need shadow later) -//
	#include "lightw.glsl"
	
	// fake waves
	//color.rgb *= 0.6 + pow(dot(Normal, l_normal), 2.0) * 0.5 * daylight;
	
	// sun / specular
	const vec3  SUNCOLOR = vec3(1.0, 0.56, 0.23);
	const float SUNSPEC  = 0.55; // specular
	const float SUNSHINE = 2.0; // shininess
	
	float shine = max(0.0, dot(fNormal, normalize(v_half)) );
	float spec  = max(0.0, dot(reflect(-vLight, fNormal), vEye) );
	
	vec3 specular = SUNCOLOR * SUNSPEC * pow(spec, 16.0) + pow(spec, 4.0) * 0.3;
	color.rgb += specular * pow(shine, SUNSHINE) * daylight * shadow * shadow;
	
	#include "horizonfade.glsl"
	
	// final color
	#include "finalcolor.glsl"
}

#endif
