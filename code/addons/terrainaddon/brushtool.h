#pragma once
//------------------------------------------------------------------------------
/**
@class Terrain::TerrainAddon

Brush 

(C) 2016 Individual contributors, see AUTHORS file
*/
//------------------------------------------------------------------------------
#include "core/refcounted.h"
#include "brushtexture.h"
#include "brushfunc.h"
#include "brushfuncsmooth.h"

namespace Terrain
{
class BrushTool : public Core::RefCounted
{
	__DeclareClass(BrushTool);
	__DeclareSingleton(BrushTool);
public:
	/// constructor
	BrushTool();
	/// destructor
	virtual ~BrushTool();

	/// setup
	void Setup();
	/// discard
	void Discard();
		
	void SetRadius(int newRadius);
	int GetRadius();
	void SetStrength(float newStrength);
	float GetStrength();
	void SetMaxHeight(float newMaxHeight);
	float GetMaxHeight();
	void SetBlurStrength(float newBlurStrength);
	float GetBlurStrength();
	void SetTexture(Ptr<Terrain::BrushTexture> newTexture);
	void SetFunction(Ptr<Terrain::BrushFunction> newFunction);
	void ActivateSmoothBrush();
	void ActivateDefaultBrush();
	void Paint(const Math::float4& pos, float* heightTextureBuffer, const Math::float2& heightTextureSize, const float modifier);
	void Paint(const Math::float4& pos, unsigned char* heightTextureBuffer, const Math::float2& heightTextureSize, const float modifier);
	Util::Array<Ptr<Terrain::BrushTexture>> GetBrushTextures();
	void SetCurrentChannel(int channel);
	int GetCurrentChannel();
private:

	Ptr<Terrain::BrushTexture> brushTexture;
	Ptr<Terrain::BrushFunction> function;
	int radius;
	float blurPrecStrength;
	float strength;
	float maxHeight;
	int currentChannel;

	Util::Array<Ptr<Terrain::BrushTexture>> brushTextures;
	Util::Array<Ptr<Terrain::BrushTexture>> LoadBrushTextures();
	
	Ptr<Terrain::BrushFunction> brushDefaultAddRemove;
	Ptr<Terrain::BrushSmooth> brushSmooth;
};
} // namespace Grid