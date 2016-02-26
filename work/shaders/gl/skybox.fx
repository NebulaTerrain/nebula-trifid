//------------------------------------------------------------------------------
//  skybox.fx
//  (C) 2012 Gustav Sterbrant
//------------------------------------------------------------------------------

#include "lib/std.fxh"
#include "lib/shared.fxh"
#include "lib/util.fxh"
#include "lib/techniques.fxh"

float Contrast = 1.0f;
float Brightness = 1.0f;
float SkyBlendFactor = 0.0f;
float SkyRotationFactor = 0.03;

// declare two textures, one main texture and one blend texture together with a wrapping sampler
samplerCube SkyLayer1;
samplerCube SkyLayer2;

samplerstate SkySampler
{
	Samplers = { SkyLayer1, SkyLayer2 };
	AddressU = Wrap; 
	AddressV = Wrap;
	AddressW = Wrap;
	Filter = MinMagLinearMipPoint;
};

state SkyboxState
{
	CullMode = Front;
	DepthEnabled = true;
	DepthWrite = false;
	DepthFunc = Equal;
};

//------------------------------------------------------------------------------
/**
    Sky box vertex shader
*/
shader
void
vsMain(in vec3 position,
	in vec3 normal,
	in vec2 uv,
	in vec3 tangent,
	in vec3 binormal,
	out vec3 UV)
{
	vec3 tempPos = normalize(position);
	gl_Position = Projection * vec4(tempPos, 1);	
	float animationSpeed = TimeAndRandom.x * SkyRotationFactor;	
	mat3 rotMat = mat3( cos(animationSpeed), 0, sin(animationSpeed),
						0, 1, 0,
						-sin(animationSpeed), 0, cos(animationSpeed));
						
	float3 viewSample = (InvView * vec4(tempPos, 0)).xyz;
	UV = viewSample * rotMat;
}

//------------------------------------------------------------------------------
/**
    Skybox pixel shader
*/
shader
void
psMain(in vec3 UV,
	[color0] out vec4 Color)
{
	// rotate uvs around center with constant speed
	vec3 baseColor = textureLod(SkyLayer1, UV, 0).rgb;
	vec3 blendColor = textureLod(SkyLayer2, UV, 0).rgb;
	vec3 color = mix(baseColor, blendColor, SkyBlendFactor);		
	color = ((color - 0.5f) * Contrast) + 0.5f;
	color *= Brightness;	
		
	Color = EncodeHDR(vec4(color, 1));
	gl_FragDepth = 1.0f;
}

//------------------------------------------------------------------------------
/**
*/
SimpleTechnique(Default, "Static", vsMain(), psMain(), SkyboxState);