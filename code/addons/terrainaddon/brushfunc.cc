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
BrushFunction::ExecuteBrushFunction(const Math::float2& pos, float* destTextureBuffer, const Math::float2& destTextureSize, const float modifier)
{
	destTexWidth = (int)destTextureSize.x();
	destTexHeight = (int)destTextureSize.y();
	CalculateRegionToUpdate(pos, destTexWidth, destTexHeight, Terrain::BrushTool::Instance()->GetRadius());
	strength = Terrain::BrushTool::Instance()->GetStrength();
	maxHeight = Terrain::BrushTool::Instance()->GetMaxHeight();
	Ptr<Terrain::BrushTexture> brushtexture = Terrain::BrushTool::Instance()->GetCurrentBrushTexture();
	//now we update only the region 

	currentBrushIndex = 0;
	for (int y_start = y_startInit; y_start < y_end; y_start++)
	{
		currentColBufferIndex = destTexHeight * y_start;
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

void BrushFunction::ExecuteBrushFunction(const Math::float2& pos, unsigned char* destTextureBuffer, const Math::float2& destTextureSize, const float modifier)
{
	destTexWidth = (int)destTextureSize.x();
	destTexHeight = (int)destTextureSize.y();
	CalculateRegionToUpdate(pos, destTexWidth, destTexHeight, Terrain::BrushTool::Instance()->GetRadius());
	strength = Terrain::BrushTool::Instance()->GetStrength();
	maxHeight = Terrain::BrushTool::Instance()->GetMaxHeight();
	Ptr<Terrain::BrushTexture> brushtexture = Terrain::BrushTool::Instance()->GetCurrentBrushTexture();
	//now we update only the region 

	currentBrushIndex = 0;
	//start and ends are ranges of pixels
	currentChannel = Terrain::BrushTool::Instance()->GetCurrentChannel(); //only needed when we have multiple channels
	//get index offset to the first value based on the current channel we need that to normalize the values
	int offset = -currentChannel;
	int numberOfChannels = 4; //only needed when we have multiple channels

	for (int y_start = y_startInit; y_start < y_end; y_start++)
	{
		currentColBufferIndex = destTexHeight * y_start * numberOfChannels; //length of column * current number of column * number of channels -> we get current column index in buffer
		currentColBrushIndex = brushtexture->size*y_brush_start;
		x_brush_start = x_brush_startInit;
		for (int x_start = x_startInit; x_start < x_end; x_start++)
		{
			currentBufferIndex = currentColBufferIndex + x_start * numberOfChannels + currentChannel; //current column + x_start * number of channels + channel number -> current pixel index
			currentBrushIndex = currentColBrushIndex + x_brush_start;

			brushValue = brushtexture->sampledBrushBuffer[currentBrushIndex] / 255.f; //normalize to use as mask and to scale well with strength, brush values are from 0 - 255
			textureValue = destTextureBuffer[currentBufferIndex];

			textureValue += (strength*brushValue*modifier);
			//textureValue = Math::n_clamp(textureValue, 0.f, 255.f); //if we don't clamp now it will happen at the normalization anyway and this way if someone really wants to reduce/overwrite completly other channels he then can do it with one stroke at high strength
			//destTextureBuffer[currentBufferIndex] = (unsigned char)textureValue;
			Math::float4 allChannels((float)(destTextureBuffer[currentBufferIndex + offset]), (float)(destTextureBuffer[currentBufferIndex + offset + 1]), (float)(destTextureBuffer[currentBufferIndex + offset + 2]), (float)(destTextureBuffer[currentBufferIndex + offset + 3]));
			allChannels[currentChannel] = textureValue;
			Math::float4 normalized = Math::float4::normalize(allChannels);
			destTextureBuffer[currentBufferIndex + offset] = (unsigned char)(normalized.x() * 255.f);
			destTextureBuffer[currentBufferIndex + offset + 1] = (unsigned char)(normalized.y() * 255.f);
			destTextureBuffer[currentBufferIndex + offset + 2] = (unsigned char)(normalized.z() * 255.f);
			destTextureBuffer[currentBufferIndex + offset + 3] = (unsigned char)(Math::n_clamp(normalized.w(),0.f,1.f) * 255.f);//for some reason component w explodes sometimes
			x_brush_start++;
		}
		y_brush_start++;
	}
}

void BrushFunction::ExecuteBrushFunction(const Math::float2& pos, unsigned short* destTextureBuffer, const Math::float2& destTextureSize, const float modifier)
{
	destTexWidth = (int)destTextureSize.x();
	destTexHeight = (int)destTextureSize.y();
	CalculateRegionToUpdate(pos, destTexWidth, destTexHeight, Terrain::BrushTool::Instance()->GetRadius());
	strength = Terrain::BrushTool::Instance()->GetStrength();
	maxHeight = Terrain::BrushTool::Instance()->GetMaxHeight();
	Ptr<Terrain::BrushTexture> brushtexture = Terrain::BrushTool::Instance()->GetCurrentBrushTexture();
	//now we update only the region 

	currentBrushIndex = 0;
	//start and ends are ranges of pixels
	for (int y_start = y_startInit; y_start < y_end; y_start++)
	{
		currentColBufferIndex = destTexHeight*y_start; //length of column * current number of column -> we get current column index in buffer
		currentColBrushIndex = brushtexture->size*y_brush_start;
		x_brush_start = x_brush_startInit;
		for (int x_start = x_startInit; x_start < x_end; x_start++)
		{
			currentBufferIndex = currentColBufferIndex + x_start; //current column + x_start -> current pixel index
			currentBrushIndex = currentColBrushIndex + x_brush_start;

			brushValue = brushtexture->sampledBrushBuffer[currentBrushIndex] / 255.f; //normalize to use as mask and to scale well with strength, brush values are from 0 - 255
			textureValue = destTextureBuffer[currentBufferIndex];

			textureValue += (strength*brushValue*modifier);
			textureValue = Math::n_clamp(textureValue, 0.f, maxHeight);
			destTextureBuffer[currentBufferIndex] = (unsigned short)textureValue;
			
			x_brush_start++;
		}
		y_brush_start++;
	}
}

void BrushFunction::CalculateRegionToUpdate(const Math::float2 &pos, const int width, const int height, const int radius)
{
	//using the attributes update the cpu buffer of texture
	x = (int)pos.x();
	y = (int)pos.y();

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