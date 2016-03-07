////------------------------------------------------------------------------------
////  gridaddon.cc
////  (C) 2012-2015 Individual contributors, see AUTHORS file
////------------------------------------------------------------------------------
//#include "stdneb.h"
//#include "terrainrenderer.h"
//
//
////#include "coregraphics/memoryvertexbufferloader.h"
////#include "coregraphics/memoryindexbufferloader.h"
////#include "coregraphics/memorytextureloader.h"
////#include "resources/resourcemanager.h"
//#include "renderutil/nodelookuputil.h"
////#include "visibility/visibilityprotocol.h"
////#include "graphics/graphicsinterface.h"
//
//using namespace CoreGraphics;
//using namespace Resources;
//using namespace Graphics;
//using namespace Math;
//
//namespace Terrain
//{
//	__ImplementClass(Terrain::TerrainRenderer, 'RTTR', Core::RefCounted);
//	__ImplementSingleton(Terrain::TerrainRenderer);
//
////------------------------------------------------------------------------------
///**
//*/
//TerrainRenderer::TerrainRenderer()
//{
//	__ConstructSingleton;
//}
//
////------------------------------------------------------------------------------
///**
//*/
//TerrainRenderer::~TerrainRenderer()
//{
//	__DestructSingleton;
//}
//
////------------------------------------------------------------------------------
///**
//*/
//void
//TerrainRenderer::Setup()
//{
//}
//
//void TerrainRenderer::Setup(Ptr<Graphics::ModelEntity> model_entity)
//{
//	//gets the buffers so we can modify
//	this->SetupTerrainModel(this->terrain_model_entity);
//
//	//setup vbo,ibo with grid data(vertexattribpointer, how to parse the data. type and steps)
//	//setup uniform buffer
//
//	////how to load a heightmap image and associate it to sampler?***************
//
//
//	//InitializeTexture(); //link a heightmap with sampler
//	//UpdateTexture();
//	//GetShaderVariables(); //uniform buffer
//	//UpdateTerrainMesh(); //generate grid vbo/ibo. initial bounding box for whole grid?
//	//UpdateWorldSize();
//}
//
////------------------------------------------------------------------------------
///**
//*/
//void
//TerrainRenderer::Discard()
//{
//
//}
//
//Ptr<Graphics::ModelEntity> 
//TerrainRenderer::CreateEntity()
//{
//	this->terrain_model_entity = ModelEntity::Create();
//	this->terrain_model_entity->SetResourceId(ResourceId("mdl:system/placeholder.n3"));
//	this->terrain_model_entity->SetLoadSynced(true);
//
//	return this->terrain_model_entity;
//}
//
//void TerrainRenderer::SetupTerrainModel(Ptr<Graphics::ModelEntity> model_entity)
//{
//	this->terrain_model_entity = model_entity;
//	this->terrain_model = this->terrain_model_entity->GetModelInstance()->GetModel();
//	this->terrain_shape_node_instance = RenderUtil::NodeLookupUtil::LookupStateNodeInstance(this->terrain_model_entity, "root/pCube1").cast<Models::ShapeNodeInstance>();
//	this->surface_instance = this->terrain_shape_node_instance->GetSurfaceInstance();
//
//	this->terrain_shape_node = this->terrain_shape_node_instance->GetModelNode().cast<Models::ShapeNode>();
//	this->terrain_mesh = this->terrain_shape_node->GetManagedMesh()->GetMesh();
//
//	this->vbo = terrain_mesh->GetVertexBuffer();
//	this->ibo = terrain_mesh->GetIndexBuffer();
//}
//
//} // namespace Grid