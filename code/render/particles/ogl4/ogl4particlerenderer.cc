//------------------------------------------------------------------------------
//  OGL4particlerenderer.cc
//  (C) 2008 Radon Labs GmbH
//------------------------------------------------------------------------------
#include "stdneb.h"

#include "particles/ogl4/ogl4particlerenderer.h"
#include "coregraphics/vertexcomponent.h"
#include "coregraphics/memoryvertexbufferloader.h"
#include "coregraphics/memoryindexbufferloader.h"
#include "coregraphics/renderdevice.h"
#include "coregraphics/vertexlayoutserver.h"

namespace OpenGL4
{
__ImplementClass(OpenGL4::OGL4ParticleRenderer, 'O4PR', Base::ParticleRendererBase);
__ImplementSingleton(OpenGL4::OGL4ParticleRenderer);

using namespace Util;
using namespace Base;
using namespace CoreGraphics;
using namespace Resources;
using namespace Particles;
using namespace Math;

//------------------------------------------------------------------------------
/**
*/
OGL4ParticleRenderer::OGL4ParticleRenderer() :
    curParticleVertexIndex(0),
	curParticleIndex(0),
    mappedVertices(0),
    curVertexPtr(0),
	bufferIndex(0)
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
OGL4ParticleRenderer::~OGL4ParticleRenderer()
{
    if (this->IsValid())
    {
        this->Discard();
    }
    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
void
OGL4ParticleRenderer::Setup()
{
    n_assert(!this->IsValid());
    n_assert(!this->particleVertexBuffer.isvalid());
    n_assert(!this->cornerVertexBuffer.isvalid());
    n_assert(!this->cornerIndexBuffer.isvalid());

    ParticleRendererBase::Setup();
    this->curParticleVertexIndex = 0;
    this->mappedVertices = 0;
    this->curVertexPtr = 0;

    // we need to create 2 vertex buffers and 1 index buffer:
    //  - one static vertex buffer which contains 4 corner vertices
    //  - one dynamic vertex buffer with one vertex per particle
    //  - one index buffer which 6 indices, forming 2 triangles

    // setup the corner vertex buffer
    Array<VertexComponent> cornerComponents;
    cornerComponents.Append(VertexComponent((VertexComponentBase::SemanticName)0, 0, VertexComponent::Float2, 0));
    float cornerVertexData[] = { 0, 0,  1, 0,  1, 1,  0, 1 };
    Ptr<MemoryVertexBufferLoader> cornerVBLoader = MemoryVertexBufferLoader::Create();
    cornerVBLoader->Setup(cornerComponents, 4, cornerVertexData, sizeof(cornerVertexData), VertexBuffer::UsageImmutable, VertexBuffer::AccessNone);

    this->cornerVertexBuffer = VertexBuffer::Create();
    this->cornerVertexBuffer->SetLoader(cornerVBLoader.upcast<ResourceLoader>());
    this->cornerVertexBuffer->SetAsyncEnabled(false);
    this->cornerVertexBuffer->Load();
    if (!this->cornerVertexBuffer->IsLoaded())
    {
        n_error("OGL4ParticleRenderer: Failed to setup corner vertex buffer!");
    }
    this->cornerVertexBuffer->SetLoader(0);

    // setup the corner index buffer
	ushort cornerIndexData[] = { 0, 1, 2, 2, 3, 0 };
    Ptr<MemoryIndexBufferLoader> cornerIBLoader = MemoryIndexBufferLoader::Create();
    cornerIBLoader->Setup(IndexType::Index16, 6, cornerIndexData, sizeof(cornerIndexData), IndexBuffer::UsageImmutable, IndexBuffer::AccessNone);

    this->cornerIndexBuffer = IndexBuffer::Create();
    this->cornerIndexBuffer->SetLoader(cornerIBLoader.upcast<ResourceLoader>());
    this->cornerIndexBuffer->SetAsyncEnabled(false);
    this->cornerIndexBuffer->Load();
    if (!this->cornerIndexBuffer->IsLoaded())
    {
        n_error("OGL4ParticleRenderer: Failed to setup corner index buffer!");
    }
    this->cornerIndexBuffer->SetLoader(0);

    // setup the cornerPrimitiveGroup which describes one particle instance
    this->primGroup.SetBaseVertex(0);
    this->primGroup.SetNumVertices(4);
    this->primGroup.SetBaseIndex(0);
    this->primGroup.SetNumIndices(6);
    this->primGroup.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

    // setup the dynamic particle vertex buffer (contains one vertex per particle)
    Array<VertexComponent> particleComponents;
    particleComponents.Append(VertexComponent((VertexComponentBase::SemanticName)1, 0, VertexComponent::Float4, 1, VertexComponent::PerInstance, 1));   // Particle::position
	particleComponents.Append(VertexComponent((VertexComponentBase::SemanticName)2, 0, VertexComponent::Float4, 1, VertexComponent::PerInstance, 1));   // Particle::stretchPosition
	particleComponents.Append(VertexComponent((VertexComponentBase::SemanticName)3, 0, VertexComponent::Float4, 1, VertexComponent::PerInstance, 1));   // Particle::color
	particleComponents.Append(VertexComponent((VertexComponentBase::SemanticName)4, 0, VertexComponent::Float4, 1, VertexComponent::PerInstance, 1));   // Particle::uvMinMax
	particleComponents.Append(VertexComponent((VertexComponentBase::SemanticName)5, 0, VertexComponent::Float4, 1, VertexComponent::PerInstance, 1));   // x: Particle::rotation, y: Particle::size

    Ptr<MemoryVertexBufferLoader> particleVBLoader = MemoryVertexBufferLoader::Create();
    particleVBLoader->Setup(particleComponents, MaxNumRenderedParticles * 3, NULL, 0, VertexBuffer::UsageDynamic, VertexBuffer::AccessWrite, VertexBuffer::SyncingCoherentPersistent);

    this->particleVertexBuffer = VertexBuffer::Create();
    this->particleVertexBuffer->SetLoader(particleVBLoader.upcast<ResourceLoader>());
    this->particleVertexBuffer->SetAsyncEnabled(false);
    this->particleVertexBuffer->Load();
    if (!this->particleVertexBuffer->IsLoaded())
    {
        n_error("OGL4ParticleRenderer: Failed to setup particle vertex buffer!");
    }
    this->particleVertexBuffer->SetLoader(0);

	// map buffer
	this->mappedVertices = this->particleVertexBuffer->Map(VertexBuffer::MapWrite);

	// create buffer lock
	this->particleBufferLock = BufferLock::Create();

	Array<VertexComponent> components = cornerComponents;
	components.AppendArray(particleComponents);
	this->vertexLayout = VertexLayout::Create();
	this->vertexLayout->SetStreamBuffer(0, this->cornerVertexBuffer->GetOGL4VertexBuffer());
	this->vertexLayout->SetStreamBuffer(1, this->particleVertexBuffer->GetOGL4VertexBuffer());
	this->vertexLayout->Setup(components);
}

//------------------------------------------------------------------------------
/**
*/
void
OGL4ParticleRenderer::Discard()
{
    n_assert(this->IsValid());

    this->cornerVertexBuffer->Unload();
    this->cornerVertexBuffer = 0;

    this->cornerIndexBuffer->Unload();
    this->cornerIndexBuffer->SetLoader(0);
    this->cornerIndexBuffer = 0;

	this->particleVertexBuffer->Unmap();
	this->mappedVertices = 0;
    this->particleVertexBuffer->Unload();
    this->particleVertexBuffer = 0;

	this->vertexLayout->Discard();
    this->vertexLayout = 0;

    ParticleRendererBase::Discard();
}

//------------------------------------------------------------------------------
/**
    This method is called once per frame before visible particle systems are
    attached.
*/
void
OGL4ParticleRenderer::BeginAttach()
{
    n_assert(!this->inAttach);
	n_assert(0 != this->mappedVertices);
    this->inAttach = true;
    this->curVertexPtr = ((float*)this->mappedVertices) + this->bufferIndex * MaxNumRenderedParticles * 20;
    this->curParticleVertexIndex = this->bufferIndex * MaxNumRenderedParticles;
	this->curParticleIndex = 0;

	// make sure to wait for our buffer to be done with the writing until we let our particles render
	this->particleBufferLock->WaitForBuffer(this->bufferIndex);
}

//------------------------------------------------------------------------------
/**
    This method is called once per frame after visible particle systems are
    attached.
*/
void
OGL4ParticleRenderer::EndAttach()
{
    n_assert(this->inAttach);
    n_assert(0 != this->mappedVertices);
    this->inAttach = false;
    this->curVertexPtr = 0;
    this->curParticleVertexIndex = 0;
	
	// lock this segment of the buffer and traverse to our next buffer (we are using triple buffering)
	RenderDevice::EnqueueBufferLockIndex(this->particleBufferLock, this->bufferIndex);
	this->bufferIndex = (this->bufferIndex + 1) % 3;
}

} // namespace OpenGL4
