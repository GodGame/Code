#pragma once
#ifndef __SCENE
#define __SCENE

#include "Shader.h"

#define MAX_LIGHTS		4
#define POINT_LIGHT		1.0f
#define SPOT_LIGHT		2.0f
#define DIRECTIONAL_LIGHT	3.0f

//1개의 조명을 표현하는 구조체이다.
struct LIGHT
{
	XMFLOAT4 m_xcAmbient;
	XMFLOAT4 m_xcDiffuse;
	XMFLOAT4 m_xcSpecular;
	XMFLOAT3 m_xv3Position;
	float m_fRange;
	XMFLOAT3 m_xv3Direction;
	float m_nType;
	XMFLOAT3 m_xv3Attenuation;
	float m_fFalloff;
	float m_fTheta; //cos(m_fTheta)
	float m_fPhi; //cos(m_fPhi)
	float m_bEnable;
	float padding;
};

struct LIGHTS
{
	LIGHT m_pLights[MAX_LIGHTS];
	XMFLOAT4 m_xcGlobalAmbient;
	XMFLOAT4 m_xv4CameraPosition;
};

struct RENDER_INFO
{
	ID3D11RenderTargetView ** ppd3dMrtRTV;
	CCamera * pCamera;
	int ThreadID;
	UINT * pRenderState;
};

struct SceneShaderBuildInfo
{
	ID3D11ShaderResourceView ** ppMRTSRVArray;
	ID3D11RenderTargetView  * pd3dBackRTV;
};

class CScene
{
protected:
	CSceneShader *  m_pSceneShader;
	CShader      *  m_pUIShader;
	CShader      ** m_ppShaders;
	int		m_nShaders : 8;
	int		m_nMRT : 8;
	int		m_nThread : 16;

	CPlayerShader * m_pPlayerShader;
	CCamera * m_pCamera;
	CGameObject *m_pSelectedObject;

	LIGHTS * m_pLights;
	ID3D11Buffer *m_pd3dcbLights;

	//렌더 타겟 뷰 인터페이스에 대한 포인터이다.
	//ID3D11RenderTargetView *m_pd3dRenderTargetView;
	//ID3D11DepthStencilView *m_pd3dDepthStencilView;

public:
	CScene();
	virtual ~CScene();

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool ProcessInput(HWND hWnd, float fTime);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext, SceneShaderBuildInfo * SceneInfo) = 0;
	virtual void ReleaseObjects();

	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, RENDER_INFO * pRenderInfo);
	virtual void UIRender(ID3D11DeviceContext *pd3dDeviceContext);

	virtual void CreateShaderVariables(ID3D11Device *pd3dDevice) = 0;
	virtual void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights);
	virtual void ReleaseShaderVariables();

public:
	virtual void GetGameMessage(CScene * byObj, eMessage eMSG);
	virtual void SendGameMessage(CScene * toObj, eMessage eMSG);
	static  void MessageObjToObj(CScene * byObj, CScene * toObj, eMessage eMSG);

	virtual void BuildStaticShadowMap(ID3D11DeviceContext * pd3dDeviceContext);
	virtual void OnCreateShadowMap(ID3D11DeviceContext * pd3dDeviceContext);
public:
#ifdef PICKING
	CGameObject *PickObjectPointedByCursor(int xClient, int yClient);
#endif
	void UpdateLights(ID3D11DeviceContext *pd3dDeviceContext);

	//void SetRenderTarget(ID3D11RenderTargetView *pd3dRenderTargetView){ m_pd3dRenderTargetView = pd3dRenderTargetView; }
	//void SetDepthStencilView(ID3D11DepthStencilView *pd3dDepthStencilView) { m_pd3dDepthStencilView = pd3dDepthStencilView; }

	void SetPlayerShader(CPlayerShader * pPlayerShader) { m_pPlayerShader = pPlayerShader; }
	CPlayerShader * GetPlayerShader() { return m_pPlayerShader; }
	CSceneShader  * GetSceneShader()  { return m_pSceneShader; }
	CShader       * GetUIShader()     { return m_pUIShader; }

	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	LIGHT   * GetLight(int index)  { return &m_pLights->m_pLights[index]; }
	CShader * GetShader(int index) { return m_ppShaders[index]; }

	int GetShaderNumber()       { return m_nShaders; }
	int GetMRTNumber()          { return m_nMRT; }
	int GetRenderThreadNumber() { return m_nThread; }
};
#endif