//------------------------------------------------------------------------------
//  terrainaddon.cc
//  (C) 2012-2015 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "terrainaddon.h"
#include "coregraphics/memoryvertexbufferloader.h"
#include "coregraphics/memoryindexbufferloader.h"
#include "coregraphics/memorytextureloader.h"
#include "resources/resourcemanager.h"
#include "renderutil/nodelookuputil.h"
#include "visibility/visibilityprotocol.h"
#include "graphics/graphicsinterface.h"


using namespace CoreGraphics;
using namespace Resources;
using namespace Graphics;
using namespace Math;

namespace Terrain
{
	__ImplementClass(Terrain::TerrainAddon, 'TRAD', Core::RefCounted);

	//------------------------------------------------------------------------------
	/**
	*/
	TerrainAddon::TerrainAddon() : terrainModelEnt(nullptr)
	{
	}

	//------------------------------------------------------------------------------
	/**
	*/
	TerrainAddon::~TerrainAddon()
	{
	}

	void
		TerrainAddon::Setup(bool createAndAttachTerrainEntity)
	{
		width = 1024;
		height = 1024;
		heightMapHeight = height + 1;
		heightMapWidth = width + 1;
		heightMultiplier = 1.f;
		this->stage = GraphicsServer::Instance()->GetDefaultView()->GetStage();
		brushTool = Terrain::BrushTool::Create();
		brushTool->Setup();

		if (createAndAttachTerrainEntity) CreateTerrainEntity();
		else SetUpTerrainModel();

		InitializeTexture();
		
		LoadShader();

		UpdateTerrainMesh();

		UpdateWorldSize();
	}

	//------------------------------------------------------------------------------
	/**
	*/
	void
		TerrainAddon::Discard()
	{
		if (this->terrainModelEnt != nullptr)
		{
			stage->RemoveEntity(this->terrainModelEnt.cast<GraphicsEntity>());
			this->terrainModelEnt = 0;
		}
		else
		{
			terrainModel->DiscardInstance(terrainModelInstance);
			Resources::ResourceManager::Instance()->DiscardManagedResource(managedModel.upcast<Resources::ManagedResource>());
		}
		this->ibo = 0;
		this->vbo = 0;

		this->heightMultiplierHandle = 0;
		this->samplerHeightMapHandle = 0;

		Memory::Free(Memory::DefaultHeap, this->rHeightBuffer);
		this->rHeightBuffer = 0;



	}

	void TerrainAddon::InitializeTexture()
	{
		SizeT frameSize = (this->heightMapWidth) * (this->heightMapHeight)*sizeof(float);
		this->rHeightBuffer = (float*)Memory::Alloc(Memory::DefaultHeap, frameSize);
		Memory::Clear(this->rHeightBuffer, frameSize);

		//create texture
		this->memoryHeightTexture = CoreGraphics::Texture::Create();
		Ptr<CoreGraphics::MemoryTextureLoader> loader = CoreGraphics::MemoryTextureLoader::Create();
		loader->SetImageBuffer(this->rHeightBuffer, this->width+1, this->height+1, CoreGraphics::PixelFormat::R32F);
		this->memoryHeightTexture->SetLoader(loader.upcast<Resources::ResourceLoader>());
		this->memoryHeightTexture->SetAsyncEnabled(false);
		this->memoryHeightTexture->SetResourceId("heightMapMemTexture");
		this->memoryHeightTexture->Load();
		n_assert(this->memoryHeightTexture->IsLoaded());
		this->memoryHeightTexture->SetLoader(0);
		Resources::ResourceManager::Instance()->RegisterUnmanagedResource(this->memoryHeightTexture.upcast<Resources::Resource>());
	}

	void TerrainAddon::CreateTerrainEntity()
	{
		this->terrainModelEnt = ModelEntity::Create();
		this->terrainModelEnt->SetResourceId(ResourceId("mdl:system/terrainPlane.n3"));
		this->terrainModelEnt->SetLoadSynced(true);
		stage->AttachEntity(terrainModelEnt.cast<GraphicsEntity>());

		terrainModel = terrainModelEnt->GetModelInstance()->GetModel();

		terrainShapeNodeInstance = RenderUtil::NodeLookupUtil::LookupStateNodeInstance(terrainModelEnt, "root/pCube1").cast<Models::ShapeNodeInstance>();
		terrainShapeNode = terrainShapeNodeInstance->GetModelNode().cast<Models::ShapeNode>();
		surfaceInstance = terrainShapeNodeInstance->GetSurfaceInstance();
		Ptr<Resources::ManagedMesh> terrainManagedMesh = terrainShapeNode->GetManagedMesh();
		terrainMesh = terrainManagedMesh->GetMesh();
	}

	void TerrainAddon::SetUpTerrainModel()
	{
		managedModel = Resources::ResourceManager::Instance()->CreateManagedResource(Models::Model::RTTI, Resources::ResourceId("mdl:system/terrainPlane.n3"), 0, true).downcast<Models::ManagedModel>();
		this->terrainModel = managedModel->GetModel();
		this->terrainModelInstance = terrainModel->CreateInstance();

		this->terrainShapeNodeInstance = terrainModelInstance->LookupNodeInstance("root/pCube1").cast<Models::ShapeNodeInstance>();
		this->surfaceInstance = terrainShapeNodeInstance->GetSurfaceInstance();

		this->terrainShapeNode = terrainShapeNodeInstance->GetModelNode().cast<Models::ShapeNode>();
		this->terrainMesh = terrainShapeNode->GetManagedMesh()->GetMesh();
	}

	void TerrainAddon::LoadShader()
	{
		//const Ptr<Materials::SurfaceInstance>& surface = terrainShapeNodeInstance->GetSurfaceInstance();

		this->heightMultiplierHandle = surfaceInstance->GetConstant("HeightMultiplier");
		this->samplerHeightMapHandle = surfaceInstance->GetConstant("HeightMap");
		
		this->samplerHeightMapHandle->SetTexture(this->memoryHeightTexture);
		this->heightMultiplierHandle->SetValue((float)this->heightMultiplier);
	}

	void TerrainAddon::GenerateTerrainBasedOnResolution()
	{
		int vertexCount = width * height;
		int squares = (width - 1) * (height - 1);
		int triangles = squares * 2;
		int indicesCount = triangles * 3;
		// Create the structure to hold the mesh data.
		vertexData.Reserve(vertexCount);
		indices.Reserve(indicesCount);

		//Generate indices for specified resolution
		//we store columns in the array
		//current column is 0
		//current row is 0

		//since i store the points column wise the next column starts at index = current column * height

		//so how do we traverse?
		//we traverse with squares

		//we can traverse column wise
		//so first loop loops columns
		//second loop loops rows
		//first is column 0
		//first row is 0
		//then row is 1
		//we never do the last row nor last column, we don't do that with borders since they are already a part of pre-border edges

		//face 1
		//vertex 0 is current column and current row
		//vertex 1 is current column and current row + 1
		//vertex 2 is current column + 1 and current row + 1

		//face 2
		//vertex 0 is current column + 1 and current row + 1 i.e. same as face 1 vertex 2
		//vertex 1 is current column + 1 and current row
		//vertex 2 is current column and current row

		//indices are 0 1 4, 5 4 0
		//let's walk around the square
		//must traverse triangle vertices in same direction for all triangles f.ex. all face vertices are traversed counter-clockwise
		for (int col = 0; col < width; col++)
		{
			for (int row = 0; row < height; row++)
			{
				//since i store the points column wise the next column starts at index = current column * height
				int currentColumn = height * col;
				vertexData.Append(VertexData((float)col, (float)row, ((float)col / (float)height), ((float)row / (float)width)));
				//we never do the last row nor last column, we don't do that with borders since they are already a part border faces that were build in previous loop
				if (col == width - 1 || row == height - 1) continue; //this might be more expensive than writing another for loop set just for indices

				//face 1
				//vertex 0 is current column and current row
				//vertex 1 is current column and current row + 1
				//vertex 2 is current column + 1 and current row + 1
				int nextColumn = height * (col + 1); //or currentColumn + height //will use that later
				indices.Append(currentColumn + row);
				indices.Append(currentColumn + row + 1);
				indices.Append(nextColumn + row + 1); //we need to calculate the next column here
				//face 2
				//vertex 0 is current column + 1 and current row + 1 i.e. same as face 1 vertex 2
				//vertex 1 is current column + 1 and current row
				//vertex 2 is current column and current row i.e. same as face 1 vertex 1
				indices.Append(nextColumn + row + 1);
				indices.Append(nextColumn + row);
				indices.Append(currentColumn + row);
			}
		}
	}

	void TerrainAddon::SetUpVBO()
	{
		// setup VBO
		Util::Array<VertexComponent> components;
		components.Append(VertexComponent(VertexComponent::Position, 0, VertexComponent::Float2, 0));
		components.Append(VertexComponent(VertexComponent::TexCoord1, 0, VertexComponent::Float2, 0));
		Ptr<MemoryVertexBufferLoader> vboLoader = MemoryVertexBufferLoader::Create();
		int vertCount = vertexData.Size();
		int sizeofstruct = sizeof(VertexData);
		vboLoader->Setup(components, vertCount, vertexData.Begin(), vertCount*sizeof(VertexData), VertexBuffer::UsageImmutable, VertexBuffer::AccessNone);

		this->vbo = VertexBuffer::Create();
		this->vbo->SetLoader(vboLoader.upcast<ResourceLoader>());
		this->vbo->SetAsyncEnabled(false);
		this->vbo->Load();
		n_assert(this->vbo->IsLoaded());
		this->vbo->SetLoader(NULL);

		Ptr<MemoryIndexBufferLoader> iboLoader = MemoryIndexBufferLoader::Create();
		int indicesCount = indices.Size();
		iboLoader->Setup(IndexType::Index32, indicesCount, indices.Begin(), indicesCount*sizeof(int));

		this->ibo = IndexBuffer::Create();
		this->ibo->SetLoader(iboLoader.upcast<ResourceLoader>());
		this->ibo->SetAsyncEnabled(false);
		this->ibo->Load();
		n_assert(this->ibo->IsLoaded());
		this->ibo->SetLoader(NULL);

		// setup ibo
		this->vertexLayout = this->vbo->GetVertexLayout();

		primitiveGroups.Clear();
		CoreGraphics::PrimitiveGroup primitive;
		primitive.SetBaseIndex(0);
		primitive.SetNumVertices(vertCount);
		primitive.SetBaseIndex(0);
		primitive.SetNumIndices(indicesCount);
		primitive.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		primitiveGroups.Append(primitive);
	}

	void TerrainAddon::UpdateTexture(void* data, SizeT size, SizeT width, SizeT height, IndexT left, IndexT top, IndexT mip)
	{
		this->memoryHeightTexture->Update(data, size, heightMapWidth, heightMapHeight, left, top, mip);
	}
	

	Ptr<Graphics::ModelEntity> TerrainAddon::GetTerrainEntity()
	{
		return terrainModelEnt;
	}

	Ptr<CoreGraphics::Mesh> TerrainAddon::GetTerrainMesh()
	{
		return terrainMesh;
	}

	void TerrainAddon::UpdateTerrainMesh()
	{
		Math::bbox boundingBox = Math::bbox(Math::point((float)width / 2.f, (float)height, (float)height / 2.f), Math::vector((float)width / 2.f, (float)height, (float)height / 2.f));
		terrainShapeNode->SetBoundingBox(boundingBox);
		terrainModel->SetBoundingBox(boundingBox);
		//matrix44 transform = matrix44::translation(-(width / 2.0f), 0, -(height / 2.0f));
		//this->terrainModelEnt->SetTransform(transform);
		GenerateTerrainBasedOnResolution();
		SetUpVBO();
		terrainMesh->GetVertexBuffer()->Unload();
		terrainMesh->GetIndexBuffer()->Unload();
		terrainMesh->SetVertexBuffer(vbo);
		terrainMesh->SetIndexBuffer(ibo);
		terrainMesh->SetPrimitiveGroups(primitiveGroups);
	}

	void TerrainAddon::UpdateTerrainWithNewSize(int width, int height)
	{
		this->width = width;
		this->height = height;
		this->heightMapWidth = width + 1;
		this->heightMapHeight = height + 1;
		UpdateTerrainMesh();
		Memory::Free(Memory::DefaultHeap, this->rHeightBuffer);
		this->rHeightBuffer = 0;
		InitializeTexture();
		UpdateWorldSize();
	}

	void TerrainAddon::UpdateWorldSize()
	{
		Math::bbox box = Math::bbox(Math::point(0, (float)height, 0), Math::vector((float)width / 2.f, (float)height, (float)height / 2.f));

		Ptr<Visibility::ChangeVisibilityBounds> msg = Visibility::ChangeVisibilityBounds::Create();
		msg->SetWorldBoundingBox(box);
		msg->SetStageName(stage->GetName().AsString());
		Graphics::GraphicsInterface::Instance()->Send(msg.upcast<Messaging::Message>());
	}

	void TerrainAddon::UpdateTerrainAtPos(const Math::float4& pos, const float modifier)
	{
		//n_printf("\nmousePos %f %f\n", pos.x(), pos.z());
		brushTool->Paint(pos, rHeightBuffer, float2((float)heightMapWidth, (float)heightMapHeight), modifier);
		memoryHeightTexture->Update(rHeightBuffer, (heightMapWidth)*(heightMapHeight)*sizeof(float), heightMapWidth, heightMapHeight, 0, 0, 0);
	}

	Ptr<Terrain::BrushTool> TerrainAddon::GetBrushTool()
	{
		return brushTool;
	}

	void TerrainAddon::UpdateHeightMultiplier(float multiplier)
	{
		heightMultiplier = multiplier;
		this->heightMultiplierHandle->SetValue(this->heightMultiplier);
	}

} // namespace Terrain