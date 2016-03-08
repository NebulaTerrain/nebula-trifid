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

	//now we update only the region 

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

void BrushFunction::ExecuteBrushFunction(const Ptr<Terrain::BrushTexture> brushtexture, const Math::float4& pos, unsigned char* destTextureBuffer, const Math::float2& destTextureSize, const float modifier)
{
	destTexWidth = (int)destTextureSize.x();
	destTexHeight = (int)destTextureSize.y();
	CalculateRegionToUpdate(pos, destTexWidth, destTexHeight, Terrain::BrushTool::Instance()->GetRadius());
	strength = Terrain::BrushTool::Instance()->GetStrength();
	maxHeight = Terrain::BrushTool::Instance()->GetMaxHeight();

	//now we update only the region 

	currentBrushIndex = 0;
	//start and ends are ranges of pixels
	currentChannel = Terrain::BrushTool::Instance()->GetCurrentChannel();
	//get index offset to the first value based on the current channel we need that to normalize the values
	int offset = currentChannel - 4;
	for (int y_start = y_startInit; y_start < y_end; y_start++)
	{
		currentColBufferIndex = destTexHeight*y_start * 4; //length of column * current number of column * number of bytes per pixel -> we get current column index in buffer
		currentColBrushIndex = brushtexture->size*y_brush_start;
		x_brush_start = x_brush_startInit;
		for (int x_start = x_startInit; x_start < x_end; x_start++)
		{
			currentBufferIndex = currentColBufferIndex + x_start * 4 + currentChannel; //current column + x_start * number of bytes per pixel + channel number -> current pixel index
			currentBrushIndex = currentColBrushIndex + x_brush_start;

			brushValue = brushtexture->sampledBrushBuffer[currentBrushIndex] / 255.f; //normalize to use as mask and to scale well with strength, brush values are from 0 - 255
			textureValue = destTextureBuffer[currentBufferIndex];

			textureValue += (strength*brushValue*modifier);
			//textureValue = Math::n_clamp(textureValue, 0.f, 255.f); //if we don't clamp now it will happen at the normalization anyway and this way if someone really wants to reduce/overwrite completly other channels he then can do it with one stroke at high strength
			destTextureBuffer[currentBufferIndex] = (unsigned char)textureValue;
			Math::float4 allChannels((float)(destTextureBuffer[currentBufferIndex + offset]), (float)(destTextureBuffer[currentBufferIndex + offset + 1]), (float)(destTextureBuffer[currentBufferIndex + offset + 2]), (float)(destTextureBuffer[currentBufferIndex + offset + 3]));
			allChannels[currentChannel] = textureValue;
			Math::float4 normalized = Math::float4::normalize(allChannels);
			destTextureBuffer[currentBufferIndex + offset] = (unsigned char)(normalized.x() * 255.f);
			destTextureBuffer[currentBufferIndex + offset + 1] = (unsigned char)(normalized.y() * 255.f);
			destTextureBuffer[currentBufferIndex + offset + 2] = (unsigned char)(normalized.z() * 255.f);
			destTextureBuffer[currentBufferIndex + offset + 3] = (unsigned char)(normalized.w() * 255.f);
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