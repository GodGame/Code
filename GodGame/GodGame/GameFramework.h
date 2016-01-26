#pragma once

#include "Timer.h"
#include "Player.h"
#include "Scene.h"

#ifndef MULTI_THREAD
#define MULTI_THREAD

#define MRT_SCENE	0
#define MRT_TXCOLOR	1
#define MRT_POS		2
#define MRT_DIFFUSE 3
#define MRT_SPEC	4
#define MRT_NORMAL	5

class CShader;
class CPlayer;
//class CGameFramework;
struct RenderingThreadInfo
{
	int		    m_nRenderingThreadID;
	HANDLE  m_hRenderingThread;
	HANDLE  m_hRenderingBeginEvent;
	HANDLE  m_hRenderingEndEvent;

	ID3D11DeviceContext * m_pd3dDeferredContext;
	ID3D11CommandList * m_pd3dCommandList;
	ID3D11DepthStencilView	* m_pd3dDepthStencilView;
	ID3D11RenderTargetView ** m_ppd3dRenderTargetView;
	//int		m_nShaders;
	CScene * m_pScene;
	CPlayer * m_pPlayer;
	UINT * m_puRenderState;
};

#endif
//class CSceneShader;
class CGameFramework
{
private:
	HINSTANCE	m_hInstance;		// ������ ���α׷� �ν��Ͻ� �ڵ�
	HWND		m_hWnd;			// ������ �ڵ�(Handle)

	int m_nWndClientWidth;
	int m_nWndClientHeight;

	CCamera *m_pCamera;

	//����̽� �������̽��� ���� �������̴�. �ַ� ���ҽ��� �����ϱ� ���Ͽ� �ʿ��ϴ�.
	ID3D11Device *m_pd3dDevice;
	//���� ü�� �������̽��� ���� �������̴�. �ַ� ���÷��̸� �����ϱ� ���Ͽ� �ʿ��ϴ�.
	IDXGISwapChain *m_pDXGISwapChain;
	//����̽� ���ؽ�Ʈ�� ���� �������̴�. �ַ� ���������� ������ �ϱ� ���Ͽ� �ʿ��ϴ�.
	ID3D11DeviceContext *m_pd3dDeviceContext;
	//���� Ÿ�� �� �������̽��� ���� �������̴�. 
	
	//ID3D11ShaderResourceView * m_pd3dSSAOSRV;
	//CSSAOShader * m_pSSAOShader;
	CSceneShader * m_pSceneShader;

	ID3D11RenderTargetView *m_pd3dBackRenderTargetView;

	ID3D11RenderTargetView *m_ppd3dRenderTargetView[NUM_MRT];
	ID3D11RenderTargetView *m_pd3dSSAOTargetView;
	//ID3D11RenderTargetView *m_pd3dPostProcessing;

	ID3D11ShaderResourceView *m_pd3dMRTSRV[NUM_MRT];

	ID3D11Texture2D * m_ppd3dMRTtx[NUM_MRT];

	ID3D11Texture2D	* m_pd3dDepthStencilBuffer;
	ID3D11DepthStencilView	* m_pd3dDepthStencilView;

private:	

	UINT m_uRenderState;

public:
	void BuildStaticShadowMap();
	void OnCreateShadowMap(CShader*ShaderList[], int nShaders);

private:
	int		m_iDrawOption;

	POINT	m_ptOldCursorPos;


	int m_nRenderThreads;
	RenderingThreadInfo * m_pRenderingThreadInfo;
	HANDLE * m_hRenderingEndEvents;

public:
	CPlayerShader *m_pPlayerShader;


	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void InitilizeThreads();
	static UINT WINAPI CGameFramework::RenderThread(LPVOID lpParameter);

	//����̽�, ���� ü��, ����̽� ���ؽ�Ʈ, ����̽��� ���õ� �並 �����ϴ� �Լ��̴�. 
	bool CreateRenderTargetDepthStencilView();;
	bool CreateDirect3DDisplay();
	//�������� �޽�, ��ü�� �����ϰ� �Ҹ��ϴ� �Լ��̴�. 
	void BuildObjects();
	void ReleaseObjects();

	//�����ӿ�ũ�� �ٽ�(����� �Է�, �ִϸ��̼�, ������)�� �����ϴ� �Լ��̴�. 
	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	//�������� �޽���(Ű����, ���콺 �Է�)�� ó���ϴ� �Լ��̴�. 
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	ID3D11Buffer			* m_pd3dcbColor;

	//������ ���� �����ӿ�ũ���� ����� Ÿ�̸��̴�.
	CGameTimer m_GameTimer;

	//������ ������ ���(Scene)�� �����ϴ� ��ü�� ���� �����͸� ��Ÿ����.
	CScene *m_pScene;
	CPlayer *m_pPlayer;
	//������ ������ ����Ʈ�� �� �������� ĸ�ǿ� ����ϱ� ���� ���ڿ��̴�.
	_TCHAR m_pszBuffer[50];
};

