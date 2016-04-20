#pragma once
#ifndef __INGAME__
#define __INGAME__

#include "Scene.h"

class CSceneInGame : public CScene
{
	//UINT m_nParticleShaderNum;
	CHAR m_nEffectShaderNum;

	POINT m_ptOldCursorPos;
	UINT  m_uHeightMapIndex;

public:
	CSceneInGame();
	virtual ~CSceneInGame();
	
	void InitializeRecv();

	virtual bool PacketProcess(LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool ProcessInput(HWND hWnd, float fTime, POINT & pt);

	virtual void BuildMeshes(ID3D11Device * pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext, ShaderBuildInfo * SceneInfo);
	virtual void ReleaseObjects();

	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, RENDER_INFO * pRenderInfo);
	virtual void UIRender(ID3D11DeviceContext *pd3dDeviceContext);

	virtual void CreateShaderVariables(ID3D11Device *pd3dDevice);
	virtual void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights);
	virtual void ReleaseShaderVariables();

	virtual void BuildStaticShadowMap(ID3D11DeviceContext * pd3dDeviceContext);
	virtual void PreProcessing(ID3D11DeviceContext * pd3dDeviceContext);

	virtual void GetGameMessage(CScene * byObj, eMessage eMSG, void * extra = nullptr);
	virtual void SendGameMessage(CScene * toObj, eMessage eMSG, void * extra = nullptr);
public:
#ifdef PICKING
	CGameObject *PickObjectPointedByCursor(int xClient, int yClient);
#endif
	//CHeightMapTerrain *GetTerrain();
};

#endif