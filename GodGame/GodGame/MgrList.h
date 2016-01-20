#pragma once

#ifndef __MGR_LIST
#define __MGR_LIST

#include "ResourceMgr.h"
#include "ShadowMgr.h"
#include "CollisionMgr.h"


class CManagers
{
public:
	static void BuildManagers(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext)
	{
		ViewMgr.BuildResources(pd3dDevice, pd3dDeviceContext);
		TXMgr.BuildResources(pd3dDevice);
		MaterialMgr.BuildResources(pd3dDevice);
		ShadowMgr.CreateShadowDevice(pd3dDevice);
	}
};

#endif