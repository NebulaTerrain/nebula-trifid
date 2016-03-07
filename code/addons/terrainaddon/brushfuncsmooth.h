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
#include "brushfunc.h"

namespace Terrain
{ 
class BrushSmooth : public BrushFunction
{
	__DeclareClass(BrushSmooth);
public:
	/// constructor
	BrushSmooth();
	/// destructor
	virtual ~BrushSmooth();
	virtual void ExecuteBrushFunction(const Ptr<Terrain::BrushTexture> brushtexture, const Math::float4& pos, float* destTextureBuffer, const Math::float2& destTextureSize, const float modifier);
	virtual void ExecuteBrushFunction(const Ptr<Terrain::BrushTexture> brushtexture, const Math::float4& pos, unsigned char* destTextureBuffer, const Math::float2& destTextureSize, const float modifier);
private:
	void GaussianBlur(float* source, float* dest, const int width, const int height, const float radius);
	float* BoxesForGauss(float radius, int n);
	void BoxBlur(float* source, float* dest, const int width, const int height, const int radius);
	void BoxBlurH(float* source, float* dest, const int width, const int height, const int radius);
	void BoxBlurT(float* source, float* dest, const int width, const int height, const int radius);
};
} // namespace Grid