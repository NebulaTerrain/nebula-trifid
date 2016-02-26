#pragma once
//------------------------------------------------------------------------------
/**
    @class ToolkitUtil::ModelPhysics
    
    Implements the physics counterpart of models.
    
    (C) 2012 Johannes Hirche
*/
//------------------------------------------------------------------------------
#include "core/refcounted.h"
#include "base/exporttypes.h"
#include "io/stream.h"
#include "physics/model/templates.h"


namespace ToolkitUtil
{
class ModelPhysics : public Core::RefCounted
{
	__DeclareClass(ModelPhysics);
public:
	/// constructor
	ModelPhysics();
	/// destructor
	virtual ~ModelPhysics();

	/// sets the name
	void SetName(const Util::String& name);
	/// gets the name
	const Util::String& GetName() const;

	/// sets mesh node
	void SetMeshMode(Physics::MeshTopologyType flags);
	/// gets mesh node
	Physics::MeshTopologyType GetMeshMode();

	/// sets export mode
	void SetExportMode(PhysicsExportMode flags);
	/// gets export mode
	PhysicsExportMode GetExportMode();

	/// sets physics mesh
	void SetPhysicsMesh(const Util::String & mesh);
	/// gets physics mesh
	const Util::String& GetPhysicsMesh() const;

	
	/// clears attributes
	void Clear();

	/// saves attributes to file
	void Save(const Ptr<IO::Stream>& stream);
	/// loads attributes from file
	void Load(const Ptr<IO::Stream>& stream);


private:
	PhysicsExportMode physicsMode;
	Physics::MeshTopologyType meshMode;	
	Util::String name;
	Util::String checksum;
	Util::String physicsMesh;

	static const short Version = 1;
}; 

//------------------------------------------------------------------------------
/**
*/
inline void
ModelPhysics::SetName(const Util::String& name)
{
	n_assert(name.IsValid());
	this->name = name;
}

//------------------------------------------------------------------------------
/**
*/
inline const Util::String& 
ModelPhysics::GetName() const
{
	return this->name;
}

//------------------------------------------------------------------------------
/**
*/
inline void
ModelPhysics::SetPhysicsMesh(const Util::String& name)
{
	n_assert(name.IsValid());
	this->physicsMesh = name;
}

//------------------------------------------------------------------------------
/**
*/
inline const Util::String& 
ModelPhysics::GetPhysicsMesh() const
{
	return this->physicsMesh;
}
} // namespace ToolkitUtil
//------------------------------------------------------------------------------