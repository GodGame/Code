#pragma once
#ifndef __INGAME__
#define __INGAME__

#include "Scene.h"

class CSceneInGame : public CScene
{
	//렌더 타겟 뷰 인터페이스에 대한 포인터이다. 
	//ID3D11RenderTargetView *m_pd3dRenderTargetView;
	//ID3D11DepthStencilView *m_pd3dDepthStencilView;
public:
	CSceneInGame();
	virtual ~CSceneInGame();

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext, CSceneShader * pSceneShader);
	virtual void ReleaseObjects();

	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, RENDER_INFO * pRenderInfo);

	virtual void CreateShaderVariables(ID3D11Device *pd3dDevice);
	virtual void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights);
	virtual void ReleaseShaderVariables();
public:
#ifdef PICKING
	CGameObject *PickObjectPointedByCursor(int xClient, int yClient);
#endif
	CHeightMapTerrain *GetTerrain();
};

#endif