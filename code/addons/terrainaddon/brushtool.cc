//------------------------------------------------------------------------------
//  terrainaddon.cc
//  (C) 2016 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "core\rttimacros.h"
#include "resources\resourcemanager.h"
#include "coregraphics\pixelformat.h"
#include "brushtool.h"


namespace Terrain
{
__ImplementClass(Terrain::BrushTool, 'TBTL', Core::RefCounted);
__ImplementSingleton(Terrain::BrushTool);

//------------------------------------------------------------------------------
/**
*/
BrushTool::BrushTool() : 
	radius(32),
	blurPrecStrength(1.f),
	strength(10.f),
	maxHeight(1024.f),
	currentChannel(0)
{
	__ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
BrushTool::~BrushTool()
{
	__DestructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
void
	BrushTool::Setup()
{
	LoadBrushTextures();
	brushSmooth = Terrain::BrushSmooth::Create();
	brushDefaultAddRemove = Terrain::BrushFunction::Create();
	SetTexture(brushTextures.Front());
	SetFunction(brushDefaultAddRemove);
}

//------------------------------------------------------------------------------
/**
*/
void
	BrushTool::Discard()
{

}

void BrushTool::SetRadius(int newRadius)
{
	if (newRadius < 1) radius = 1; //if someone has magically set radius to 0
	else radius = newRadius;
	brushTexture->ResizeTexture(radius * 2);
}

int BrushTool::GetRadius()
{
	return radius;
}

void BrushTool::SetStrength(float newStrength)
{
	strength = newStrength;
}

float BrushTool::GetStrength()
{
	return strength;
}

void BrushTool::SetTexture(Ptr<Terrain::BrushTexture> newTexture)
{
	brushTexture = newTexture;
	int properBrushSize = this->radius * 2;
	if (brushTexture->size != properBrushSize)
	{
		brushTexture->ResizeTexture(properBrushSize);
	}
}

void BrushTool::SetFunction(Ptr<Terrain::BrushFunction> newFunction)
{
	function = newFunction;
}

void BrushTool::Paint(const Math::float4& pos, float* destTextureBuffer, const Math::float2& textureSize, const float modifier)
{

	function->ExecuteBrushFunction(brushTexture, pos, destTextureBuffer, textureSize, modifier);
}

void BrushTool::Paint(const Math::float4& pos, unsigned char* destTextureBuffer, const Math::float2& textureSize, const float modifier)
{

	function->ExecuteBrushFunction(brushTexture, pos, destTextureBuffer, textureSize, modifier);
}

void BrushTool::ActivateSmoothBrush()
{
	function = brushSmooth.cast<BrushFunction>();
}

void BrushTool::ActivateDefaultBrush()
{
	function = brushDefaultAddRemove.cast<BrushFunction>();
}

Util::Array<Ptr<Terrain::BrushTexture>>
BrushTool::LoadBrushTextures()
{
	Ptr<Terrain::BrushTexture> texture = Terrain::BrushTexture::Create();
	texture->Setup("tex:system/lightcones.dds");
	brushTextures.Append(texture);
	return brushTextures;
}

void BrushTool::SetMaxHeight(float newMaxHeight)
{
	this->maxHeight = newMaxHeight;
}

float BrushTool::GetMaxHeight()
{
	return maxHeight;
}

void BrushTool::SetBlurStrength(float newBlurStrength)
{
	blurPrecStrength = newBlurStrength;
}

float BrushTool::GetBlurRadius()
{
	//n_assert(blurPrecStrength <= 1.f);
	return (radius - 1)*blurPrecStrength;
}

Util::Array<Ptr<Terrain::BrushTexture>> 
BrushTool::GetBrushTextures()
{
	return brushTextures;
}

void BrushTool::SetCurrentChannel(int channel)
{
	currentChannel = channel;
}

int BrushTool::GetCurrentChannel()
{
	return currentChannel;
}

} // namespace Terrain