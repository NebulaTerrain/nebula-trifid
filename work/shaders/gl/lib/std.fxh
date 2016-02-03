//------------------------------------------------------------------------------
//  std.fxh
//  (C) 2013 Gustav Sterbrant
//------------------------------------------------------------------------------

#ifndef STD_FXH
#define STD_FXH

/// define global macros
#if GLSL
	#define TEXTURE_SIZE(tex, mip) textureSize(tex, mip)
#elif HLSL
	#define TEXTURE_SIZE(tex, mip) tex.GetDimensions(mip)
#elif PS3
#elif WII
#endif

/// define a set of macros to convert from HLSL to GLSL
#if GLSL
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define uint2 uvec2
#define uint3 uvec3
#define uint4 uvec4
#define bool2 bvec2
#define bool3 bvec3
#define bool4 bvec4
#define matrix mat4
#define float4x4 mat4
#define float3x3 mat3
#define float2x2 mat2
#define float2x3 mat2x3
#define float2x4 mat2x4
#define float3x2 mat3x2
#define float3x4 mat3x4
#define float4x2 mat4x2
#define float4x3 mat4x3
#define Texture1D sampler1D
#define Texture1DArray sampler1DArray
#define Texture2D sampler2D
#define Texture2DArray sampler2DArray
#define Texture3D sampler3D
#define TextureCube samplerCube
#define TextureCubeArray samplerCubeArray

void sincos(float angle, out float sinus, out float cosinus)
{
	sinus = sin(angle);
	cosinus = cos(angle);
}

#define ddx dFdx
#define ddy dFdy
#define lerp mix
#define frac fract
#define rsqrt(x) inversesqrt(x)
#define fmod mod
#define saturate(x) clamp(x, 0.0f, 1.0f)
#define mul(x, y) y * x

#elif HLSL
int2 NPixelSize(Texture2D tex, int lod)
{
	int2 val;
	tex.GetDimensions(lod, val.x, val.y);
	val.xy = 1 / val.xy;
	return val;
}

#elif PS3

#elif Wii

#endif // platform

#endif // STD_H
