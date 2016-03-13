//------------------------------------------------------------------------------
//  terrainaddon.cc
//  (C) 2016 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "core\rttimacros.h"
#include "resources\resourcemanager.h"
#include "coregraphics\pixelformat.h"
#include "brushtool.h"
#include "io\ioserver.h"


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
	LoadBrushTextures("tex:terrainbrushes/");
	brushFunctions.Add(Terrain::BrushFunctionType::Smooth, Terrain::BrushSmooth::Create());
	brushFunctions.Add(Terrain::BrushFunctionType::Standard, Terrain::BrushFunction::Create());
	SetTexture(brushTextures.Front());
	SetFunction(BrushFunctionType::Standard);
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


void BrushTool::ChangeBrushTexture(int id)
{
	SetTexture(brushTextures[id]);
}


void BrushTool::SetFunction(BrushFunctionType type)
{
	function = brushFunctions[type];
}

void BrushTool::ActivateSmoothBrush()
{
	function = brushFunctions[BrushFunctionType::Smooth];
}

void BrushTool::ActivateDefaultBrush()
{
	function = brushFunctions[BrushFunctionType::Standard];
}

Util::Array<Ptr<Terrain::BrushTexture>>
BrushTool::LoadBrushTextures(const Util::String& dir)
{
	Util::Array<Util::String> brushList = IO::IoServer::Instance()->ListFiles(dir, "*");
	for (int i = 0; i < brushList.Size(); i++)
	{
		Ptr<Terrain::BrushTexture> texture = Terrain::BrushTexture::Create();
		texture->Setup(Util::String::Sprintf("%s%s", dir.AsCharPtr(), brushList[i].AsCharPtr()));
		brushTextures.Append(texture);
	}

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

Ptr<Terrain::BrushTexture> BrushTool::GetCurrentBrushTexture()
{
	return brushTexture;
}

Ptr<Terrain::BrushFunction> BrushTool::GetCurrentBrushFunction()
{
	return function;
}

} // namespace Terrain