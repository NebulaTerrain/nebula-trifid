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
#include "coregraphics/streamtexturesaver.h"


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
TerrainAddon::TerrainAddon() : 
	terrainModelEnt(nullptr), 
	rHeightBuffer(nullptr),
	width(1024),
	height(1024),
	heightMapHeight(width + 1),
	heightMapWidth(height + 1),
	heightMultiplier(1.f),
	currentChannel(-1)
{
	
}

//------------------------------------------------------------------------------
/**
*/
TerrainAddon::~TerrainAddon()
{
}

void
TerrainAddon::Setup(Ptr<Graphics::ModelEntity> modelEntity)
{
	brushTool = Terrain::BrushTool::Create();
	brushTool->Setup();

	SetUpTerrainModel(modelEntity);

	UpdateTextureSizeVariables();

	InitializeTexture();

	CreateMaskTexutre("TextureMask_1");

	UpdateTexture();

	UpdateMasks();
		
	GetShaderVariables();

	UpdateTerrainMesh();

	UpdateWorldSize();
}

//------------------------------------------------------------------------------
/**
*/
void
TerrainAddon::Discard()
{
	this->ibo = 0;
	this->vbo = 0;

	this->heightMultiplierHandle = 0;
	this->samplerHeightMapHandle = 0;

	Memory::Free(Memory::DefaultHeap, this->rHeightBuffer);
	this->rHeightBuffer = 0;

	Resources::ResourceManager::Instance()->UnregisterUnmanagedResource(this->memoryHeightTexture.upcast<Resources::Resource>());
}


void 
TerrainAddon::InitializeTexture()
{
	this->memoryHeightTexture = CoreGraphics::Texture::Create();
	this->memoryHeightTexture->SetResourceId("heightMapMemTexture");
	Resources::ResourceManager::Instance()->RegisterUnmanagedResource(this->memoryHeightTexture.upcast<Resources::Resource>());

	currentTexture = memoryHeightTexture;
}

void 
TerrainAddon::UpdateTexture()
{
	Resources::ResourceId resName = memoryHeightTexture->GetResourceId();
	this->memoryHeightTexture->Unload();
	Memory::Free(Memory::DefaultHeap, this->rHeightBuffer);
	this->rHeightBuffer = 0;

	SizeT frameSize = (this->heightMapWidth) * (this->heightMapHeight)*sizeof(float);
	this->rHeightBuffer = (float*)Memory::Alloc(Memory::DefaultHeap, frameSize);
	Memory::Clear(this->rHeightBuffer, frameSize);

	Ptr<CoreGraphics::MemoryTextureLoader> loader = CoreGraphics::MemoryTextureLoader::Create();
	loader->SetImageBuffer(this->rHeightBuffer, this->heightMapWidth, this->heightMapHeight, CoreGraphics::PixelFormat::R32F);
	this->memoryHeightTexture->SetLoader(loader.upcast<Resources::ResourceLoader>());
	this->memoryHeightTexture->SetAsyncEnabled(false);
	this->memoryHeightTexture->SetResourceId(resName);
	this->memoryHeightTexture->Load();
	n_assert(this->memoryHeightTexture->IsLoaded());
	this->memoryHeightTexture->SetLoader(0);

	currentBuffer = rHeightBuffer;
}


Ptr<Graphics::ModelEntity> 
TerrainAddon::CreateTerrainEntity()
{
	this->terrainModelEnt = ModelEntity::Create();
	this->terrainModelEnt->SetResourceId(ResourceId("mdl:terrainmesh/plane.n3"));
	this->terrainModelEnt->SetLoadSynced(true);
	return terrainModelEnt;
}

void 
TerrainAddon::SetUpTerrainModel(Ptr<Graphics::ModelEntity> modelEntity)
{
	this->terrainModelEnt = modelEntity;
	this->terrainModel = terrainModelEnt->GetModelInstance()->GetModel();
	this->terrainShapeNodeInstance = RenderUtil::NodeLookupUtil::LookupStateNodeInstance(terrainModelEnt, "root/pCube1").cast<Models::ShapeNodeInstance>();
	this->surfaceInstance = terrainShapeNodeInstance->GetSurfaceInstance();
	
	this->terrainShapeNode = terrainShapeNodeInstance->GetModelNode().cast<Models::ShapeNode>();
	this->terrainMesh = terrainShapeNode->GetManagedMesh()->GetMesh();

	this->vbo = terrainMesh->GetVertexBuffer();
	this->ibo = terrainMesh->GetIndexBuffer();
}

void 
TerrainAddon::GetShaderVariables()
{
	//const Ptr<Materials::SurfaceInstance>& surface = terrainShapeNodeInstance->GetSurfaceInstance();

	//get
	this->heightMultiplierHandle = surfaceInstance->GetConstant("HeightMultiplier");
	this->samplerHeightMapHandle = surfaceInstance->GetConstant("HeightMap");
	this->terrainSizeHandle = surfaceInstance->GetConstant("TerrainSize");
	//set
	this->samplerHeightMapHandle->SetTexture(this->memoryHeightTexture);
	this->heightMultiplierHandle->SetValue((float)this->heightMultiplier);
	this->terrainSizeHandle->SetValue((float)width);

	for (int i = 0; i < maskHandles.Size(); i++)
	{
		//mask handles are generated when mask is created
		//update if necessary:
		this->maskHandles[i] = surfaceInstance->GetConstant(this->maskVarNames[i]);
		//set
		this->maskHandles[i]->SetTexture(maskTextures[i]);
	}
}

void 
TerrainAddon::GenerateTerrainBasedOnResolution()
{
	vertexData.Clear();
	indices.Clear();

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
			vertexData.Append(VertexData((float)col, (float)row));
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

void 
TerrainAddon::SetUpVBO()
{
	vbo->Unload();
	ibo->Unload();

	// setup VBO
	Util::Array<VertexComponent> components;
	components.Append(VertexComponent(VertexComponent::Position, 0, VertexComponent::Float2, 0));
	Ptr<MemoryVertexBufferLoader> vboLoader = MemoryVertexBufferLoader::Create();
	int vertCount = vertexData.Size();
	int sizeofstruct = sizeof(VertexData);
	vboLoader->Setup(components, vertCount, vertexData.Begin(), vertCount*sizeof(VertexData), VertexBuffer::UsageImmutable, VertexBuffer::AccessNone);
		
	this->vbo->SetLoader(vboLoader.upcast<ResourceLoader>());
	this->vbo->SetAsyncEnabled(false);
	this->vbo->Load();
	n_assert(this->vbo->IsLoaded());
	this->vbo->SetLoader(NULL);

	Ptr<MemoryIndexBufferLoader> iboLoader = MemoryIndexBufferLoader::Create();
	int indicesCount = indices.Size();
	iboLoader->Setup(IndexType::Index32, indicesCount, indices.Begin(), indicesCount*sizeof(int));
		
	this->ibo->SetLoader(iboLoader.upcast<ResourceLoader>());
	this->ibo->SetAsyncEnabled(false);
	this->ibo->Load();
	n_assert(this->ibo->IsLoaded());
	this->ibo->SetLoader(NULL);

	primitiveGroups.Clear();
	CoreGraphics::PrimitiveGroup primitive;
	primitive.SetBaseIndex(0);
	primitive.SetNumVertices(vertCount);
	primitive.SetBaseIndex(0);
	primitive.SetNumIndices(indicesCount);
	primitive.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
	primitiveGroups.Append(primitive);
}
	

Ptr<Graphics::ModelEntity> 
TerrainAddon::GetTerrainEntity()
{
	return terrainModelEnt;
}

Ptr<CoreGraphics::Mesh> 
TerrainAddon::GetTerrainMesh()
{
	return terrainMesh;
}

void 
TerrainAddon::UpdateTerrainWithNewSize(int width, int height)
{
	this->width = width;
	this->height = height;
	this->terrainSizeHandle->SetValue((float)width);
	UpdateTextureSizeVariables();
	UpdateTerrainMesh();		
	UpdateTexture();
	UpdateMasks();
	UpdateWorldSize();
}

void 
TerrainAddon::UpdateTerrainMesh()
{
	UpdateBoundingBox();

	//matrix44 transform = matrix44::translation(-(width / 2.0f), 0, -(height / 2.0f));
	//this->terrainModelEnt->SetTransform(transform);
	GenerateTerrainBasedOnResolution();
	SetUpVBO();
	terrainMesh->SetPrimitiveGroups(primitiveGroups);
}

void 
TerrainAddon::UpdateWorldSize()
{
	Ptr<Visibility::ChangeVisibilityBounds> msg = Visibility::ChangeVisibilityBounds::Create();
	msg->SetWorldBoundingBox(boundingBox);
	msg->SetStageName(GraphicsServer::Instance()->GetDefaultView()->GetStage()->GetName().AsString());
	Graphics::GraphicsInterface::Instance()->Send(msg.upcast<Messaging::Message>());
}

void 
TerrainAddon::UpdateTerrainAtPos(const Math::float4& pos, const float modifier)
{
	if (currentChannel == InvalidIndex) //height map has different pixel format than texture masks
	{
		brushTool->GetCurrentBrushFunction()->ExecuteBrushFunction(float2(pos.x(),pos.z()), (float*)currentBuffer, float2((float)heightMapWidth, (float)heightMapHeight), modifier);
	}
	else
	{
		brushTool->GetCurrentBrushFunction()->ExecuteBrushFunction(float2(pos.x(), pos.z()), (unsigned char*)currentBuffer, float2((float)heightMapWidth, (float)heightMapHeight), modifier);
	}
	currentTexture->Update(currentBuffer, 0, heightMapWidth, heightMapHeight, 0, 0, 0);
}

Ptr<Terrain::BrushTool> 
TerrainAddon::GetBrushTool()
{
	return brushTool;
}

void 
TerrainAddon::UpdateHeightMultiplier(float multiplier)
{
	heightMultiplier = multiplier;
	this->heightMultiplierHandle->SetValue(this->heightMultiplier);
}

void 
TerrainAddon::FillChannel(float newValue)
{
	if (currentChannel == InvalidIndex)
	{
		int frameSize = heightMapWidth*heightMapHeight;
		for (int i = 0; i < frameSize; i++)
		{
			rHeightBuffer[i] = newValue;
		}
	}
	else
	{
		int numberOfChannels = 4;
		int frameSize = heightMapWidth* heightMapHeight * numberOfChannels;
		int offset = -currentChannel;
		unsigned char* castBuffer = (unsigned char*)currentBuffer;
		for (int i = currentChannel; i < frameSize; i += numberOfChannels)
		{
			//currentBuffer[i] = (unsigned char)Math::n_clamp(newValue, 0.f, 255.f); //this new code below is def slower but will adapt with other channels and reduce them if necessary
			Math::float4 allChannels((float)(castBuffer[i + offset]), (float)(castBuffer[i + offset + 1]), (float)(castBuffer[i + offset + 2]), (float)(castBuffer[i + offset + 3]));
			allChannels[currentChannel] = newValue;
			Math::float4 normalized = Math::float4::normalize(allChannels);
			castBuffer[i + offset] = (unsigned char)(normalized.x() * 255.f);
			castBuffer[i + offset + 1] = (unsigned char)(normalized.y() * 255.f);
			castBuffer[i + offset + 2] = (unsigned char)(normalized.z() * 255.f);
			castBuffer[i + offset + 3] = (unsigned char)(Math::n_clamp(normalized.w(), 0.f, 1.f) * 255.f);//for some reason component w explodes sometimes
		}
	}
	currentTexture->Update(currentBuffer, 0, heightMapWidth, heightMapHeight, 0, 0, 0);
}


void TerrainAddon::FullBlurCurrentChannel()
{
	brushTool->ActivateSmoothBrush();
	int prevRadius = brushTool->GetRadius();
	brushTool->SetRadius(heightMapWidth/2);
	UpdateTerrainAtPos(Math::float4(heightMapWidth / 2.f, 0, heightMapWidth / 2.f, 1.f), 1.f);
	brushTool->SetRadius(prevRadius);
	brushTool->ActivateDefaultBrush();
}

void 
TerrainAddon::ApplyHeightMultiplier()
{
	int frameSize = heightMapWidth*heightMapHeight;
	for (int i = 0; i < frameSize; i++)
	{
		rHeightBuffer[i] *= heightMultiplier;
	}
	currentTexture->Update(currentBuffer, 0, heightMapWidth, heightMapHeight, 0, 0, 0);
}

void 
TerrainAddon::UpdateBoundingBox()
{
	boundingBox = Math::bbox(Math::point((width - 1.f) / 2.f, 10.f, (height - 1.f) / 2.f), Math::vector((width - 1.f) / 2.f, 10.f, (height - 1.f) / 2.f));
	terrainShapeNode->SetBoundingBox(boundingBox);
	terrainModel->SetBoundingBox(boundingBox);
}

Ptr<Materials::SurfaceInstance> TerrainAddon::GetSurfaceInstance()
{
	return surfaceInstance;
}

void TerrainAddon::CreateMaskTexutre(Util::String matVarName)
{
	Ptr<CoreGraphics::Texture> maskTexture =  CoreGraphics::Texture::Create();
	Util::String resName = Util::String::Sprintf("heightMapTextureMask_%d", maskTextures.Size() + 1); //+ 1 just to be consistent since they are named in shader starting from 1 and saved from 1
	maskTexture->SetResourceId(resName);
	Resources::ResourceManager::Instance()->RegisterUnmanagedResource(maskTexture.upcast<Resources::Resource>());

	unsigned char* maskBuffer = nullptr;

	maskTextures.Append(maskTexture);
	maskBuffers.Append(maskBuffer);

	maskHandles.Append(surfaceInstance->GetConstant(matVarName));
	maskVarNames.Append(matVarName);
}

void TerrainAddon::SwitchChannel(int mask, int channel)
{
	//here current channel is used to cast currentBuffer to the right format when updating with brush check above UpdateTextureAtPos
	currentChannel = channel; 
	if (channel == InvalidIndex)
	{
		currentTexture = memoryHeightTexture;
		currentBuffer = rHeightBuffer;
		brushTool->SetCurrentChannel(0);
	}
	else
	{
		currentTexture = maskTextures[mask];
		currentBuffer = maskBuffers[mask];
		currentChannel = channel;
		brushTool->SetCurrentChannel(channel);
	}
	
}

void TerrainAddon::UpdateMasks()
{
	for (int i = 0; i < maskTextures.Size(); i++)
	{
		Resources::ResourceId resName = maskTextures[i]->GetResourceId();

		maskTextures[i]->Unload();
		Memory::Free(Memory::DefaultHeap, maskBuffers[i]);
		maskBuffers[i] = 0;

		int numberOfChannels = 4;
		SizeT frameSize = (this->heightMapWidth) * (this->heightMapHeight) * sizeof(unsigned char) * numberOfChannels; //4 channels 1 byte per channel - 8 bit channels
		maskBuffers[i] = (unsigned char*)Memory::Alloc(Memory::DefaultHeap, frameSize);
		Memory::Clear(maskBuffers[i], frameSize);
		//for normalizing to work at least one channel has to be set to 255
		int channelToSetMaskToOne = 0;
		for (int j = 0; j < frameSize; j += numberOfChannels)
		{
			maskBuffers[i][j + channelToSetMaskToOne] = 255;
		}
		Ptr<CoreGraphics::MemoryTextureLoader> loader = CoreGraphics::MemoryTextureLoader::Create();
		loader->SetImageBuffer(maskBuffers[i], this->heightMapWidth, this->heightMapHeight, CoreGraphics::PixelFormat::SRGBA8);
		maskTextures[i]->SetLoader(loader.upcast<Resources::ResourceLoader>());
		maskTextures[i]->SetAsyncEnabled(false);
		maskTextures[i]->SetResourceId(resName);
		maskTextures[i]->Load();
		n_assert(maskTextures[i]->IsLoaded());
		maskTextures[i]->SetLoader(0);
	}
}

void TerrainAddon::UpdateTextureSizeVariables()
{
	this->heightMapWidth = width + 1;
	this->heightMapHeight = height + 1;
}

void TerrainAddon::SaveHeightMap(Util::String path)
{
	Ptr<CoreGraphics::StreamTextureSaver> saver = CoreGraphics::StreamTextureSaver::Create();
	Ptr<IO::Stream> stream = IO::IoServer::Instance()->CreateStream(path);
	saver->SetFormat(CoreGraphics::ImageFileFormat::PNG);
	saver->SetMipLevel(0);
	saver->SetStream(stream);
	memoryHeightTexture->SetSaver(saver.upcast<Resources::ResourceSaver>());
	n_assert(memoryHeightTexture->Save());
}

void TerrainAddon::SaveMasks(Util::String path)
{
	for (int i = 0; i < maskTextures.Size(); i++)
	{
		Util::String resName = Util::String::Sprintf("%s%d.png", path.AsCharPtr(), i+1);
		Ptr<CoreGraphics::StreamTextureSaver> saver = CoreGraphics::StreamTextureSaver::Create();
		Ptr<IO::Stream> stream = IO::IoServer::Instance()->CreateStream(resName);
		saver->SetFormat(CoreGraphics::ImageFileFormat::PNG);
		saver->SetMipLevel(0);
		saver->SetStream(stream);
		maskTextures[i]->SetSaver(saver.upcast<Resources::ResourceSaver>());
		n_assert(maskTextures[i]->Save());
	}
	
}

} // namespace Terrain