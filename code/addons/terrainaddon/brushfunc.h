#pragma once
//------------------------------------------------------------------------------
/**
@class Terrain::TerrainAddon

Brush 

(C) 2016 Individual contributors, see AUTHORS file
*/
//------------------------------------------------------------------------------
#include "core/refcounted.h"
#include "resources/managedtexture.h"
#include "brushtexture.h"

namespace Terrain
{ 
class BrushFunction : public Core::RefCounted
{
	__DeclareClass(BrushFunction);
public:
	/// constructor
	BrushFunction();
	/// destructor
	virtual ~BrushFunction();
	virtual void ExecuteBrushFunction(const Ptr<Terrain::BrushTexture> brushtexture, const Math::float4& pos, float* destTextureBuffer, const Math::float2& destTextureSize, const float modifier);
	virtual void ExecuteBrushFunction(const Ptr<Terrain::BrushTexture> brushtexture, const Math::float4& pos, unsigned char* destTextureBuffer, const Math::float2& destTextureSize, const float modifier);
	void CalculateRegionToUpdate(const Math::float4 &pos, const int width, const int height, const int radius);
	

private:

protected:
	//region variables
	int destTexWidth;
	int destTexHeight;
	int y_startInit;
	int y_end;
	int x_startInit;
	int x_end;
	int x_brush_startInit;
	int y_brush_start;
	int x;
	int y;
	//get variables
	float strength;
	float maxHeight;
	int radius;
	float blurRadius;
	//brush set variables
	int currentBrushIndex;
	int currentColBufferIndex;
	int currentColBrushIndex;
	int x_brush_start;
	int currentBufferIndex;
	float brushValue;
	float textureValue;
};
} // namespace Grid