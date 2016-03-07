//#pragma once
////------------------------------------------------------------------------------
///**
//	@class Grid::GridAddon
//	
//	Renders an infinite grid in X-Z space.
//	
//	(C) 2012-2015 Individual contributors, see AUTHORS file
//*/
////------------------------------------------------------------------------------
//#include "core/refcounted.h"
//#include "core/singleton.h"
//
//#include "graphics/modelentity.h"
//#include "models/nodes/shapenodeinstance.h"
//#include "models/nodes/shapenode.h"
//
//namespace Terrain
//{
//class TerrainRenderer : public Core::RefCounted
//{
//	__DeclareClass(TerrainRenderer);
//	__DeclareSingleton(TerrainRenderer);
//public:
//	/// constructor
//	TerrainRenderer();
//	/// destructor
//	virtual ~TerrainRenderer();
//
//	/// setup
//	void Setup();
//	/// discard
//	void Discard();
//
//	Ptr<Graphics::ModelEntity> CreateEntity();
//
//	//setup buffers with grid data
//	void Setup(Ptr<Graphics::ModelEntity> model_entity);
//
//	//update offsets with camera posiition
//	//void UpdateLevelOffsets(camera position);
//
//	//Gather instance data. directly modify shader array with mapping
//	//Draw with offset instance
//
//
//private:
//	Ptr<Graphics::ModelEntity> terrain_model_entity;
//	Ptr<CoreGraphics::Mesh> terrain_mesh;
//	Ptr<Materials::SurfaceInstance> surface_instance;
//
//	Ptr<Models::ShapeNodeInstance> terrain_shape_node_instance;
//	Ptr<Models::ShapeNode> terrain_shape_node;
//	Ptr<Models::Model> terrain_model;
//	//Ptr<Models::ModelInstance> terrainModelInstance;
//	Math::bbox boundingBox;
//
//	//Util::Array<CoreGraphics::PrimitiveGroup> primitiveGroups;
//	Ptr<CoreGraphics::VertexBuffer> vbo;
//	Ptr<CoreGraphics::IndexBuffer> ibo;
//
//	void SetupTerrainModel(Ptr<Graphics::ModelEntity> model_entity);
//};
//} // namespace Grid