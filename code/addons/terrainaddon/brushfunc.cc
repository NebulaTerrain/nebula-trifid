//------------------------------------------------------------------------------
//  terrainaddon.cc
//  (C) 2016 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "core\rttimacros.h"
#include "brushfunc.h"
#include "resources\resourcemanager.h"
#include "brushtool.h"
using namespace Math;

namespace Terrain
{
__ImplementClass(Terrain::BrushFunction, 'TBFN', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
BrushFunction::BrushFunction()
{
	
}

//------------------------------------------------------------------------------
/**
*/
BrushFunction::~BrushFunction()
{
}

//------------------------------------------------------------------------------
/**
*/
void
BrushFunction::ExecuteBrushFunction(const Ptr<Terrain::BrushTexture> brushtexture, const Math::float4& pos, float* destTextureBuffer, const Math::float2& destTextureSize, const float modifier)
{
	destTexWidth = (int)destTextureSize.x();
	destTexHeight = (int)destTextureSize.y();
	CalculateRegionToUpdate(pos, destTexWidth, destTexHeight, Terrain::BrushTool::Instance()->GetRadius());
	strength = Terrain::BrushTool::Instance()->GetStrength();
	maxHeight = Terrain::BrushTool::Instance()->GetMaxHeight();

	//now we update only the region and we clamp it if big brush is close to the border

	currentBrushIndex = 0;
	for (int y_start = y_startInit; y_start < y_end; y_start++)
	{
		currentColBufferIndex = destTexHeight*y_start;
		currentColBrushIndex = brushtexture->size*y_brush_start;
		x_brush_start = x_brush_startInit;
		for (int x_start = x_startInit; x_start < x_end; x_start++)
		{
			currentBufferIndex = currentColBufferIndex + x_start;
			currentBrushIndex = currentColBrushIndex + x_brush_start;

			brushValue = brushtexture->sampledBrushBuffer[currentBrushIndex] / 255.f; //normalize to use as mask and to scale well with strength, brush values are from 0 - 255
			textureValue = destTextureBuffer[currentBufferIndex];

			textureValue += (strength*brushValue*modifier);
			textureValue = Math::n_clamp(textureValue, 0.f, maxHeight);
			destTextureBuffer[currentBufferIndex] = textureValue;
			
			x_brush_start++;
		}
		y_brush_start++;
	}
}

void BrushFunction::CalculateRegionToUpdate(const Math::float4 &pos, const int width, const int height, const int radius)
{
	//using the attributes update the cpu buffer of texture
	x = (int)pos.x();
	y = (int)pos.z();

	//calculate region
	y_startInit = y - radius;
	x_startInit = x - radius;

	y_end = y + radius;
	x_end = x + radius;

	//calculate brush size, we need to clamp at borders
	y_brush_start = 0;
	x_brush_startInit = 0;

	//have to clamp the range of x if the brush pos is at the border
	if (x_startInit < 0)
	{
		x_brush_startInit = x_brush_startInit - x_startInit;
		x_startInit = 0;
	}
	x_end = Math::n_min(width, x_end);
	if (y_startInit < 0)
	{
		y_brush_start = y_brush_start - y_startInit; //no need to calculate ends for brush
		y_startInit = 0;
	}
	y_end = Math::n_min(height, y_end);
}

} // namespace Terrain