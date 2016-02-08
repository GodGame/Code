#pragma once
#ifndef __SCENE_TITLE
#define __SCENE_TITLE

#include "Scene.h"

class CSceneTitle : public CScene
{
public:
	CSceneTitle();
	virtual ~CSceneTitle();

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool ProcessInput(HWND hWnd, float fTime);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext, SceneShaderBuildInfo * SceneInfo);
	virtual void ReleaseObjects();

	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, RENDER_INFO * pRenderInfo);
	virtual void UIRender(ID3D11DeviceContext *pd3dDeviceContext);

	virtual void CreateShaderVariables(ID3D11Device *pd3dDevice);
	virtual void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights);
	virtual void ReleaseShaderVariables();
public:
#ifdef PICKING
	CGameObject *PickObjectPointedByCursor(int xClient, int yClient);
#endif
	//CHeightMapTerrain *GetTerrain();
};

#endif