//------------------------------------------------------------------------------
//  ogl4uniformbuffer.cc
//  (C) 2015 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "ogl4uniformbuffer.h"
#include "coregraphics/shadervariable.h"
#include "coregraphics/constantbuffer.h"
#include "coregraphics/bufferlock.h"
#include "coregraphics/renderdevice.h"


namespace OpenGL4
{
__ImplementClass(OpenGL4::OGL4UniformBuffer, 'O4UB', Base::ConstantBufferBase);

//------------------------------------------------------------------------------
/**
*/
OGL4UniformBuffer::OGL4UniformBuffer() : 
	ogl4Buffer(0),
	handle(NULL),
	bufferLock(NULL)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
OGL4UniformBuffer::~OGL4UniformBuffer()
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
void
OGL4UniformBuffer::Setup(const SizeT numBackingBuffers)
{
    ConstantBufferBase::Setup(numBackingBuffers);
    glGenBuffers(1, &this->ogl4Buffer);

    GLint alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);

    // calculate aligned size
    this->size = (this->size + alignment - 1) - (this->size + alignment - 1) % alignment;

    glBindBuffer(GL_UNIFORM_BUFFER, this->ogl4Buffer);
    if (!this->sync)
    {
        GLenum mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		glBufferStorage(GL_UNIFORM_BUFFER, this->size * this->numBuffers, NULL, mapFlags);
        this->buffer = glMapBufferRange(GL_UNIFORM_BUFFER, 0, this->size * this->numBuffers, mapFlags);
    }
    else
    {
        //glBufferStorage(GL_UNIFORM_BUFFER, this->size * this->NumBuffers, NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
		glBufferData(GL_UNIFORM_BUFFER, this->size * this->numBuffers, NULL, GL_STREAM_DRAW);
        this->buffer = n_new_array(byte, this->size);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// create new bufferlock
	this->bufferLock = OGL4BufferLock::Create();

    // setup handle
    this->handle = n_new(AnyFX::OpenGLBufferBinding);
    this->handle->size = this->size;
    this->handle->handle = this->ogl4Buffer;
    this->handle->bindRange = true;
    this->handle->offset = 0;
}

//------------------------------------------------------------------------------
/**
*/
void
OGL4UniformBuffer::Discard()
{
	// first step, remove buffer lock which should 
	this->bufferLock = 0;

    if (!this->sync)
    {
#if OGL4_BINDLESS
        glUnmapNamedBuffer(this->ogl4Buffer);
#else
        glBindBuffer(GL_UNIFORM_BUFFER, this->ogl4Buffer);
        glUnmapBuffer(GL_UNIFORM_BUFFER);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
#endif
    }
    else
    {
        n_delete_array(this->buffer);
    }
    
    this->buffer = 0;
	n_delete(this->handle);
	this->handle = 0;
    glDeleteBuffers(1, &this->ogl4Buffer);
	this->ogl4Buffer = 0;
    ConstantBufferBase::Discard();
}

//------------------------------------------------------------------------------
/**
*/
void
OGL4UniformBuffer::SetupFromBlockInShader(const Ptr<CoreGraphics::Shader>& shader, const Util::String& blockName, const SizeT numBackingBuffers)
{
    n_assert(!this->isSetup);
    AnyFX::EffectVarblock* block = shader->GetOGL4Effect()->GetVarblockByName(blockName.AsCharPtr());
    this->size = block->GetSize();

    // setup buffer which initializes GL buffer
    this->Setup(numBackingBuffers);

	this->BeginUpdateSync();
    const eastl::vector<AnyFX::VarblockVariableBinding>& perFrameBinds = block->GetVariables();
    for (unsigned i = 0; i < perFrameBinds.size(); i++)
    {
        const AnyFX::VarblockVariableBinding& binding = perFrameBinds[i];
        Ptr<CoreGraphics::ShaderVariable> var = CoreGraphics::ShaderVariable::Create();
        Ptr<OGL4UniformBuffer> thisPtr(this);
        var->BindToUniformBuffer(thisPtr.downcast<CoreGraphics::ConstantBuffer>(), binding.offset, binding.size, binding.value);

        // add to variable dictionary
        this->variables.Append(var);
        this->variablesByName.Add(binding.name.c_str(), var);
    }
	this->EndUpdateSync();
}

//------------------------------------------------------------------------------
/**
*/
void
OGL4UniformBuffer::CycleBuffers()
{
    ConstantBufferBase::CycleBuffers();
    this->handle->offset = this->size * this->bufferIndex;
}

//------------------------------------------------------------------------------
/**
*/
void
OGL4UniformBuffer::EndUpdateSync()
{
	if (this->sync)
	{
		// only sync if we made changes
		if (this->isDirty)
		{
			//glInvalidateBufferSubData(this->ogl4Buffer, this->handle->offset, this->size);
#if OGL4_BINDLESS
			glNamedBufferSubData(this->ogl4Buffer, this->handle->offset, this->size, this->buffer);
#else
			glBindBuffer(GL_UNIFORM_BUFFER, this->ogl4Buffer);
			glBufferSubData(GL_UNIFORM_BUFFER, this->handle->offset, this->size, this->buffer);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
#endif
		}
	}
	else
	{
		CoreGraphics::RenderDevice::EnqueueBufferLockIndex(this->bufferLock.downcast<CoreGraphics::BufferLock>(), this->bufferIndex);
	}
	
	ConstantBufferBase::EndUpdateSync();
}

} // namespace OpenGL4 