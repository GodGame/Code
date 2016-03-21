#pragma once

#ifndef __MGR_LIST
#define __MGR_LIST

#include "ResourceMgr.h"
#include "ShadowMgr.h"
#include "CollisionMgr.h"
#include "EventMgr.h"

class CManagers
{
public:
	static void BuildManagers(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext)
	{
		ViewMgr.BuildResources(pd3dDevice, pd3dDeviceContext);
		TXMgr.BuildResources(pd3dDevice);
		MaterialMgr.BuildResources(pd3dDevice);
		ShadowMgr.CreateShadowDevice(pd3dDevice);
		UIMgr.BuildResources();
		EVENTMgr.Initialize();
		//MESHMgr.BuildResources(pd3dDevice);
	}

	static void Update(CCamera * pCamera, float fFrameTime)
	{
		QUADMgr.Update(pCamera);
		EVENTMgr.Update(fFrameTime);
	}

	static void ReleaseManagers()
	{
		ViewMgr.ReleaseResources();
		TXMgr.ReleaseObjects();
		MaterialMgr.ReleaseObjects();
		ShadowMgr.ReleaseDevices();
		//UIMgr.R
	}
};

#endif