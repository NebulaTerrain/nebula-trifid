//------------------------------------------------------------------------------
//  gridaddon.cc
//  (C) 2012-2015 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "terrainrenderer.h"

namespace Terrain
{
	__ImplementClass(Terrain::TerrainRenderer, 'RTTR', Core::RefCounted);
	__ImplementSingleton(Terrain::TerrainRenderer);

//------------------------------------------------------------------------------
/**
*/
TerrainRenderer::TerrainRenderer()
{
	__ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
TerrainRenderer::~TerrainRenderer()
{
	__DestructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
void
TerrainRenderer::Setup()
{
}

//------------------------------------------------------------------------------
/**
*/
void
TerrainRenderer::Discard()
{

}


} // namespace Grid