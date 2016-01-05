#pragma once
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

class CScene
{
private:
	CShader **m_ppShaders;
	int		m_nShaders;

	//CShader * m_pMirrorShader;

	CPlayerShader * m_pPlayerShader;
	CCamera *m_pCamera;
	CGameObject *m_pSelectedObject;

	LIGHTS *m_pLights;
	ID3D11Buffer *m_pd3dcbLights;

	//렌더 타겟 뷰 인터페이스에 대한 포인터이다. 
	ID3D11RenderTargetView *m_pd3dRenderTargetView;
	ID3D11DepthStencilView *m_pd3dDepthStencilView;

	//	ID3D11DepthStencilState  * m_pd3dStencilMirrorState;	// 거울 스텐실 버퍼에 기록하기 위해 거울 렌더링 전용
	//	ID3D11DepthStencilState  * m_pd3dDepthStencilReflectState;	// 반사될 물체들을 가시적인 부분만 그릴때 사용

	//	ID3D11BlendState	* m_pd3dNoWriteBlendState;	// 블렌드 스테이트
	//	ID3D11RasterizerState * m_pd3dCullCWRasterizerState;	// 반시계방향 전용 RS
	//int m_nRenderThreads;
	//RenderingThreadInfo * m_pRenderingThreadInfo;
	//HANDLE * m_hRenderingEndEvents;
public:

	CScene();
	~CScene();

	void UpdateLights(ID3D11DeviceContext *pd3dDeviceContext);

	void SetRenderTarget(ID3D11RenderTargetView *pd3dRenderTargetView){ m_pd3dRenderTargetView = pd3dRenderTargetView; }
	void SetDepthStencilView(ID3D11DepthStencilView *pd3dDepthStencilView) { m_pd3dDepthStencilView = pd3dDepthStencilView; }
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//void InitilizeThreads(ID3D11Device *pd3dDevice);
	void BuildObjects(ID3D11Device *pd3dDevice, CSceneShader * pSceneShader);
	void ReleaseObjects();
	//	void CreateStates(ID3D11Device *pd3dDevice);
	void SetPlayerShader(CPlayerShader * pPlayerShader) { m_pPlayerShader = pPlayerShader; }

	bool ProcessInput();
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D11DeviceContext *pd3dDeviceContext, RENDER_INFO * pRenderInfo);
	//	void RenderReflected(ID3D11DeviceContext *pd3dDeviceContext, CCamera * pCamera, XMMATRIX * pxmtxReflect);
#ifdef PICKING
	CGameObject *PickObjectPointedByCursor(int xClient, int yClient);
#endif
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	CHeightMapTerrain *GetTerrain();

	void CreateShaderVariables(ID3D11Device *pd3dDevice);
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights);
	void ReleaseShaderVariables();

	LIGHT * GetLight(int index) { return &m_pLights->m_pLights[index]; }

	//static DWORD WINAPI RenderThread(LPVOID lpParameter);
	CShader ** GetShader(int index){ return &m_ppShaders[index]; }
};
