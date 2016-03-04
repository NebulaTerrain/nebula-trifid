//------------------------------------------------------------------------------
//  terrainaddon.cc
//  (C) 2016 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "core\rttimacros.h"
#include "brushfuncsmooth.h"
#include "resources\resourcemanager.h"
#include "brushtool.h"
using namespace Math;

namespace Terrain
{
__ImplementClass(Terrain::BrushSmooth, 'TSBT', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
BrushSmooth::BrushSmooth()
{
		
}

//------------------------------------------------------------------------------
/**
*/
BrushSmooth::~BrushSmooth()
{
}

//------------------------------------------------------------------------------
/**
*/
void
BrushSmooth::ExecuteBrushFunction(const Ptr<Terrain::BrushTexture> brushtexture, const Math::float4& pos, float* destTextureBuffer, const Math::float2& destTextureSize, const float modifier)
{
	destTexWidth = (int)destTextureSize.x();
	destTexHeight = (int)destTextureSize.y();
	radius = Terrain::BrushTool::Instance()->GetRadius();
	CalculateRegionToUpdate(pos, destTexWidth, destTexHeight, radius);
	strength = Terrain::BrushTool::Instance()->GetStrength();
	maxHeight = Terrain::BrushTool::Instance()->GetMaxHeight();
	blurRadius = Terrain::BrushTool::Instance()->GetBlurStrength();

	//now we update only the region and we clamp it if big brush is close to the border

	//first i will need to get the region 
	//blur it
	//and apply

	//store region
	int diameterOfRegionToBlur = (radius * 2);
	int regionSize = diameterOfRegionToBlur*diameterOfRegionToBlur * 4;
	if (regionSize <= 0) return;
	float * regionToBlur = (float*)Memory::Alloc(Memory::DefaultHeap, regionSize);

	float* blurredRegion = (float*)Memory::Alloc(Memory::DefaultHeap, regionSize);

	int regionIndex = 0;
	for (int y_start = y_startInit; y_start < y_end; y_start++)
	{
		int currentColBufferIndex = destTexHeight*y_start;
		for (int x_start = x_startInit; x_start < x_end; x_start++)
		{
			int currentBufferIndex = currentColBufferIndex + x_start;
				
			regionToBlur[regionIndex] = destTextureBuffer[currentBufferIndex];
			regionIndex++;
		}
	}

	GaussianBlur(regionToBlur, blurredRegion, diameterOfRegionToBlur, diameterOfRegionToBlur, blurRadius);
	Memory::Free(Memory::DefaultHeap, regionToBlur);
		
	//apply the blurred data

	currentBrushIndex = 0;
	regionIndex = 0;
	for (int y_start = y_startInit; y_start < y_end; y_start++)
	{
		currentColBufferIndex = destTexHeight*y_start;
		currentColBrushIndex = brushtexture->size*y_brush_start;
		x_brush_start = x_brush_startInit;
		for (int x_start = x_startInit; x_start < x_end; x_start++)
		{
			currentBufferIndex = currentColBufferIndex + x_start;
			currentBrushIndex = currentColBrushIndex + x_brush_start;

			brushValue = brushtexture->sampledBrushBuffer[currentBrushIndex] / 255.f; //normalize to use as mask, brush values are from 0 - 255
			float bluredValue = blurredRegion[regionIndex];
			textureValue = destTextureBuffer[currentBufferIndex];
			float interpolatedValue = Math::n_lerp(textureValue, bluredValue, brushValue);
			destTextureBuffer[currentBufferIndex] = interpolatedValue;
			regionIndex++;
			x_brush_start++;
		}
		y_brush_start++;
	}
	Memory::Free(Memory::DefaultHeap, blurredRegion);
}

//linear time gaussian blur
void BrushSmooth::GaussianBlur(float* source, float* dest, const int width, const int height, const float radius)
{
	float* bxs = BoxesForGauss(radius, 3);
	BoxBlur(source, dest, width, height, (int)((bxs[0] - 1.f) / 2.f));
	BoxBlur(dest, source, width, height, (int)((bxs[1] - 1.f) / 2.f));
	BoxBlur(source, dest, width, height, (int)((bxs[2] - 1.f) / 2.f));
}

float* BrushSmooth::BoxesForGauss(float radius, int n)
{
	float wIdeal = Math::n_sqrt((12.f * radius*radius / (float)n) + 1.f);  // Ideal averaging filter width 
	float wl = Math::n_floor(wIdeal);  if ((int)wl % 2 == 0) wl--;
	float wu = wl + 2.f;

	float mIdeal = (12.f * radius*radius - (float)n*wl*wl - 4.f * (float)n*wl - 3.f * (float)n) / (-4.f * wl - 4.f);
	int m = (int)Math::n_floor(mIdeal + 0.5f);

	float* sizes = new float[n];  for (int i = 0; i < n; i++) sizes[i] = (i < m ? wl : wu);
	return sizes;
}

void BrushSmooth::BoxBlur(float* source, float* dest, const int width, const int height, const int radius)
{
	int size = width*height;
	for (int i = 0; i < size; i++) dest[i] = source[i];
	BoxBlurH(dest, source, width, height, radius);
	BoxBlurT(source, dest, width, height, radius);
}

void BrushSmooth::BoxBlurH(float* source, float* dest, const int width, const int height, const int radius)
{
	float iarr = 1.f / (float)(radius + radius + 1);
	for (int i = 0; i < height; i++) {
		int ti = i*width, li = ti, ri = ti + radius;
		float fv = source[ti], lv = source[ti + width - 1], val = ((float)radius + 1.f)*fv;
		for (int j = 0; j < radius; j++)				    val += source[ti + j];
		for (int j = 0; j <= radius; j++)				  { val += source[ri++] - fv;			  dest[ti++] = val*iarr; }
		for (int j = radius + 1; j < width - radius; j++) { val += source[ri++] - source[li++];   dest[ti++] = val*iarr; }
		for (int j = width - radius; j < width; j++)	  { val += lv - source[li++];			  dest[ti++] = val*iarr; }
	}
}

void BrushSmooth::BoxBlurT(float* source, float* dest, const int width, const int height, const int radius)
{
	float iarr = 1.f / (float)(radius + radius + 1);
	for (int i = 0; i < width; i++) {
		int ti = i, li = ti, ri = ti + radius*width;
		float fv = source[ti], lv = source[ti + width*(height - 1)], val = ((float)radius + 1.f)*fv;
		for (int j = 0; j < radius; j++)							 val += source[ti + j*width];
		for (int j = 0; j <= radius; j++)						   { val += source[ri] - fv;			dest[ti] = val*iarr;  ri += width; ti += width; }
		for (int j = radius + 1; j < height - radius; j++)		   { val += source[ri] - source[li];	dest[ti] = val*iarr;  li += width; ri += width; ti += width; }
		for (int j = height - radius; j < height; j++)			   { val += lv - source[li];			dest[ti] = val*iarr;  li += width; ti += width; }
	}
}

} // namespace Terrain