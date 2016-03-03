#pragma once
//------------------------------------------------------------------------------
/**
@class Terrain::TerrainAddon

Renders terrain with specified resolution based on height-map

(C) 2016 Individual contributors, see AUTHORS file
*/
//------------------------------------------------------------------------------
#include "core/refcounted.h"
#include "graphics/modelentity.h"
#include "models/nodes/shapenodeinstance.h"
#include "models/nodes/shapenode.h"
#include "brushtool.h"

namespace Terrain
{
	class TerrainAddon : public Core::RefCounted
	{
		__DeclareClass(TerrainAddon);
	public:
		/// constructor
		TerrainAddon();
		/// destructor
		virtual ~TerrainAddon();

		/// setup
		void Setup(Ptr<Graphics::ModelEntity> modelEntity);

		/// discard
		void Discard();

		void GetShaderVariables();

		void GenerateTerrainBasedOnResolution();
		void UpdateTerrainMesh();
		void SetUpVBO();
		void InitializeTexture();
		void UpdateTexture();
		void UpdateTexture(void* data, SizeT size, SizeT width, SizeT height, IndexT left, IndexT top, IndexT mip);
		void SetUpTerrainModel(Ptr<Graphics::ModelEntity> modelEntity);
		Ptr<Graphics::ModelEntity> CreateTerrainEntity();
		Ptr<Graphics::ModelEntity> GetTerrainEntity();
		Ptr<CoreGraphics::Mesh> GetTerrainMesh();

		void UpdateTerrainWithNewSize(int width, int height);
		void UpdateWorldSize();

		void UpdateTerrainAtPos(const Math::float4& pos, const float modifier);
		void UpdateHeightMultiplier(float multiplier);
		Ptr<Terrain::BrushTool> GetBrushTool();
		void FlattenTerrain(float newTerrainHeight);
		void ApplyHeightMultiplier();
	private:
		#pragma pack (push)
		#pragma pack(1)
		struct VertexData
		{
			float x, z;
			float u1, v1;
			//float u2, v2;
			VertexData(){}
			//VertexData(float x1, float z1, float u_1, float v_1, float u_2, float v_2) : x(x1), z(z1), u1(u_1), v1(v_1), u2(u_2), v2(v_2)
			VertexData(float x1, float z1, float u_1, float v_1) : x(x1), z(z1), u1(u_1), v1(v_1)
			{}
		};
		#pragma pack (pop) 

		int width, height;
		int heightMapWidth, heightMapHeight;
		float heightMultiplier;

		// mesh
		Util::Array<int> indices;
		Util::Array<VertexData> vertexData;

		Util::Array<CoreGraphics::PrimitiveGroup> primitiveGroups;
		Ptr<CoreGraphics::VertexBuffer> vbo;
		Ptr<CoreGraphics::IndexBuffer> ibo;

		// shader variables
		Ptr<Materials::SurfaceConstant> heightMultiplierHandle;
		Ptr<Materials::SurfaceConstant> samplerHeightMapHandle;
		Ptr<Materials::SurfaceConstant> transformHandle;

		Ptr<CoreGraphics::Texture> memoryHeightTexture;
		float *rHeightBuffer;

		Ptr<Graphics::ModelEntity> terrainModelEnt;
		Ptr<CoreGraphics::Mesh> terrainMesh;
		Ptr<Materials::SurfaceInstance> surfaceInstance;
		
		Ptr<Models::ShapeNodeInstance> terrainShapeNodeInstance;
		Ptr<Models::ShapeNode> terrainShapeNode;
		Ptr<Models::Model> terrainModel;
		Ptr<Terrain::BrushTool> brushTool;
		Ptr<Models::ModelInstance> terrainModelInstance;
		Math::bbox boundingBox;
	};
} // namespace Grid