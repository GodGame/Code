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
	HINSTANCE	m_hInstance;		// 윈도우 프로그램 인스턴스 핸들
	HWND		m_hWnd;			// 윈도우 핸들(Handle)

	int m_nWndClientWidth;
	int m_nWndClientHeight;

	CCamera *m_pCamera;

	//디바이스 인터페이스에 대한 포인터이다. 주로 리소스를 생성하기 위하여 필요하다.
	ID3D11Device *m_pd3dDevice;
	//스왑 체인 인터페이스에 대한 포인터이다. 주로 디스플레이를 제어하기 위하여 필요하다.
	IDXGISwapChain *m_pDXGISwapChain;
	//디바이스 컨텍스트에 대한 포인터이다. 주로 파이프라인 설정을 하기 위하여 필요하다.
	ID3D11DeviceContext *m_pd3dDeviceContext;
	//렌더 타겟 뷰 인터페이스에 대한 포인터이다. 
	
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

	//디바이스, 스왑 체인, 디바이스 컨텍스트, 디바이스와 관련된 뷰를 생성하는 함수이다. 
	bool CreateRenderTargetDepthStencilView();;
	bool CreateDirect3DDisplay();
	//렌더링할 메쉬, 객체를 생성하고 소멸하는 함수이다. 
	void BuildObjects();
	void ReleaseObjects();

	//프레임워크의 핵심(사용자 입력, 애니메이션, 렌더링)을 구성하는 함수이다. 
	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	//윈도우의 메시지(키보드, 마우스 입력)를 처리하는 함수이다. 
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	ID3D11Buffer			* m_pd3dcbColor;

	//다음은 게임 프레임워크에서 사용할 타이머이다.
	CGameTimer m_GameTimer;

	//다음은 게임의 장면(Scene)을 관리하는 객체에 대한 포인터를 나타낸다.
	CScene *m_pScene;
	CPlayer *m_pPlayer;
	//다음은 프레임 레이트를 주 윈도우의 캡션에 출력하기 위한 문자열이다.
	_TCHAR m_pszBuffer[50];
};

