#pragma once
//------------------------------------------------------------------------------
/**
	@class Grid::GridAddon
	
	Renders an infinite grid in X-Z space.
	
	(C) 2012-2015 Individual contributors, see AUTHORS file
*/
//------------------------------------------------------------------------------
#include "core/refcounted.h"
#include "core/singleton.h"

namespace Terrain
{
class TerrainRenderer : public Core::RefCounted
{
	__DeclareClass(TerrainRenderer);
	__DeclareSingleton(TerrainRenderer);
public:
	/// constructor
	TerrainRenderer();
	/// destructor
	virtual ~TerrainRenderer();

	/// setup
	void Setup();
	/// discard
	void Discard();

private:

};
} // namespace Grid