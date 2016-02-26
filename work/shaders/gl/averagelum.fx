//------------------------------------------------------------------------------
//  averagelum.fx
//  (C) 2013 Gustav Sterbrant
//------------------------------------------------------------------------------

#include "lib/std.fxh"
#include "lib/techniques.fxh"
#include "lib/util.fxh"

sampler2D ColorSource;
sampler2D PreviousLum;

samplerstate LuminanceSampler
{
	Samplers = { ColorSource, PreviousLum };
	Filter = Point;
};

float TimeDiff;

state AverageLumState
{
	CullMode = Back;
	DepthEnabled = false;
	DepthWrite = false;
};


//------------------------------------------------------------------------------
/**
*/
shader
void
vsMain(in vec3 position,
	[slot=2] in vec2 uv,
	out vec2 UV) 
{
	gl_Position = vec4(position, 1);
	UV = uv;
}

//------------------------------------------------------------------------------
/**
	Performs a 2x2 kernel downscale, will only render 1 pixel
*/
shader
void
psMain(vec2 UV,
	[color0] out vec4 result)
{
	vec2 pixelSize = GetPixelSize(ColorSource);
	
	// source should be a 512x512 texture, so we sample the 8'th mip of the texture
	vec4 sample1 = texelFetch(ColorSource, ivec2(1, 0), 8);
	vec4 sample2 = texelFetch(ColorSource, ivec2(0, 1), 8);
	vec4 sample3 = texelFetch(ColorSource, ivec2(1, 1), 8);
	vec4 sample4 = texelFetch(ColorSource, ivec2(0, 0), 8);
	vec4 currentLum = (sample1 + sample2 + sample3 + sample4) * 0.25f;
	vec4 lastLum = texelFetch(PreviousLum, ivec2(0, 0), 0);
	
/*	float	sigma = 0.04/(0.04 + Clum.x);
	float	tau = sigma*0.4 + (1.0 - sigma)*0.1;
	vec3	col = Alum + (Clum - Alum) * (1.0 - exp(-dtime/tau));
*/

	//vec3	col = pow( pow( Alum, 0.25 ) + ( pow( Clum, 0.25 ) - pow( Alum, 0.25 ) ) * ( 1.0 - pow( 0.98, 30 * dtime ) ), 4.0);
	vec4 Color = lastLum + (currentLum - lastLum) * (1.0 - pow(0.98, 30.0 * TimeDiff));
	Color.x = clamp(Color.x, 0.25f, 5.0f);
	//Color.y = clamp(Color.y, 0.5f, 2);
	//gl_FragColor = vec4(col.x,col.y, 0.0, 1.0);
	result = vec4(Color.x, Color.y, 0, 1);
}

//------------------------------------------------------------------------------
/**
*/
PostEffect(vsMain(), psMain(), AverageLumState);
