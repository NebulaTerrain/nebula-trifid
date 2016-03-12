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
	float GetBlurRadius();
	void SetTexture(Ptr<Terrain::BrushTexture> newTexture);
	void SetFunction(BrushFunctionType type);
	Ptr<Terrain::BrushTexture> GetCurrentBrushTexture();
	Ptr<Terrain::BrushFunction> GetCurrentBrushFunction();
	void ActivateSmoothBrush();
	void ActivateDefaultBrush();

	Util::Array<Ptr<Terrain::BrushTexture>> GetBrushTextures();
	void SetCurrentChannel(int channel);
	int GetCurrentChannel();
	void ChangeBrushTexture(int id);

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
	Util::Dictionary<Terrain::BrushFunctionType, Ptr<Terrain::BrushFunction>> brushFunctions;
};
} // namespace Grid