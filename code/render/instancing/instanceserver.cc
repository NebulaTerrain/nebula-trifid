//------------------------------------------------------------------------------
//  instanceserver.cc
//  (C) 2012 Gustav Sterbrant
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "instancing/instanceserver.h"
namespace Instancing
{
#if __DX11__
__ImplementClass(Instancing::InstanceServer, 'INSR', Instancing::D3D11InstanceServer);
#elif __OGL4__
__ImplementClass(Instancing::InstanceServer, 'INSR', Instancing::OGL4InstanceServer);
#else
#error "InstanceServer class not implemented on this platform!"
#endif

__ImplementSingleton(Instancing::InstanceServer);

//------------------------------------------------------------------------------
/**
*/
InstanceServer::InstanceServer()
{
	__ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
InstanceServer::~InstanceServer()
{
	__DestructSingleton;
}

} // namespace Instancing
