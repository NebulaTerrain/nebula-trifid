//------------------------------------------------------------------------------
//  shaderbufferbase.cc
//  (C) 2012-2014 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "shaderreadwritebufferbase.h"

namespace Base
{
__ImplementClass(Base::ShaderReadWriteBufferBase, 'SHBB', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
ShaderReadWriteBufferBase::ShaderReadWriteBufferBase() :
	isSetup(false),
    bufferIndex(0)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
ShaderReadWriteBufferBase::~ShaderReadWriteBufferBase()
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
void
ShaderReadWriteBufferBase::Setup(const SizeT numBackingBuffers)
{
	n_assert(!this->isSetup);
	n_assert(this->size > 0);
	this->isSetup = true;
	this->numBuffers = numBackingBuffers;
}

//------------------------------------------------------------------------------
/**
*/
void
ShaderReadWriteBufferBase::Discard()
{
	n_assert(this->isSetup);
	this->isSetup = false;
}

//------------------------------------------------------------------------------
/**
*/
void
ShaderReadWriteBufferBase::Update(void* data, uint offset, uint length)
{
	n_assert(offset < size);
	n_assert(length > 0);
	// implementation specific
}

//------------------------------------------------------------------------------
/**
*/
void
ShaderReadWriteBufferBase::CycleBuffers()
{
    this->bufferIndex = (this->bufferIndex + 1) % this->numBuffers;
}

} // namespace Base