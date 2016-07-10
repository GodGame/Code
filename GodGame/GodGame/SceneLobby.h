#pragma once
#ifndef __SCENE_LOBBY
#define __SCENE_LOBBY

#include "Scene.h"

class CSceneLobby : public CScene
{
public:
	CSceneLobby();
	virtual ~CSceneLobby();

	void InitializeRecv();

	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool ProcessInput(HWND hWnd, float fTime, POINT & pt);

	virtual void BuildMeshes(ID3D11Device * pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext, ShaderBuildInfo * SceneInfo);
	virtual void ReleaseObjects();

	//virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, RENDER_INFO * pRenderInfo);
	//virtual void UIRender(ID3D11DeviceContext *pd3dDeviceContext);

	virtual void CreateShaderVariables(ID3D11Device *pd3dDevice);
	virtual void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights);
	virtual void ReleaseShaderVariables();

	virtual void GetGameMessage(CScene * byObj, eMessage eMSG, void * extra = nullptr);
public:
#ifdef PICKING
	CGameObject *PickObjectPointedByCursor(int xClient, int yClient);
#endif
};

#endif
