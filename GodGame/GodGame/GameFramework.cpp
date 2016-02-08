#include "stdafx.h"
#include "GameFramework.h"
#include "MyInline.h"
#include "SceneInGame.h"
#include "SceneTitle.h"
//CRITICAL_SECTION m_cs;

bool		 gbChangeScene = false;
vector<CScene*> gSceneState;
CScene       * gpScene = nullptr;

void CGameFramework::ChangeGameScene(CScene * pScene)
{
	PopGameScene();
	PushGameScene(pScene);
}

void CGameFramework::PushGameScene(CScene * pScene)
{
	if (pScene)
	{
		gpScene = pScene;
		BuildObjects(gpScene);

		m_nRenderThreads = gpScene->GetRenderThreadNumber();

		if (m_nRenderThreads > 0)
		{
			InitilizeThreads();
			gbChangeScene = false;
		}
		gSceneState.push_back(pScene);
	}
}

void CGameFramework::PopGameScene()
{
	if (gSceneState.size() > 0)
	{
#ifdef _DEBUG
		system("cls");
#endif
		ReleaseThreads();

		CScene * pScene = *(gSceneState.end() - 1);
		ReleaseObjects(gpScene);
		delete gpScene;

		m_pPlayer       = nullptr;
		m_pSceneShader  = nullptr;
	}
	gSceneState.pop_back();
}

CGameFramework::CGameFramework()
{
	m_pd3dDevice     = nullptr;
	m_pDXGISwapChain = nullptr;

	for (int i = 0; i < NUM_MRT; ++i)
	{
		m_ppd3dRenderTargetView[i] = nullptr;
		m_pd3dMRTSRV[i]            = nullptr;
		m_ppd3dMRTtx[i]            = nullptr;
	}
	m_pd3dDepthStencilBuffer   = nullptr;
	m_pd3dDepthStencilView     = nullptr;
	m_pd3dDeviceContext        = nullptr;

	m_nWndClientWidth          = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight         = FRAME_BUFFER_HEIGHT;

	m_nRenderThreads           = 0;

	m_pRenderingThreadInfo     = nullptr;
	//m_pd3dPostProcessing     = nullptr;

	m_pd3dBackRenderTargetView = nullptr;

	gpScene                    = nullptr;
	gSceneState.reserve(4);

	m_pPlayerShader            = nullptr;
	m_pPlayer                  = nullptr;
	m_pCamera                  = nullptr;
	m_iDrawOption              = MRT_SCENE;
	m_pSceneShader             = nullptr;
	_tcscpy_s(m_pszBuffer, _T("__GodGame__("));

	//m_pd3dSSAOTargetView = nullptr;
	//m_pSSAOShader = nullptr;
	m_uRenderState = 0;
}

CGameFramework::~CGameFramework()
{
}

CGameFramework & CGameFramework::GetInstance()
{
	static CGameFramework instance;
	return instance;
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	//Direct3D 디바이스, 디바이스 컨텍스트, 스왑 체인 등을 생성하는 함수를 호출한다.
	if (!CreateDirect3DDisplay()) return(false);
	if (!CreateRenderTargetDepthStencilView()) return(false);

	CManagers::BuildManagers(m_pd3dDevice, m_pd3dDeviceContext);
	//PushGameScene(new CSceneInGame());
	PushGameScene(new CSceneTitle());

	//InitilizeThreads();
	return(true);
}

void CGameFramework::OnDestroy()
{
	//게임 객체를 소멸한다.
	ReleaseObjects(gpScene);
	delete gpScene;
	gSceneState.pop_back();

	for (auto it = gSceneState.begin(); it != gSceneState.end(); ++it)
	{
		(*it)->ReleaseObjects();
		delete *it;
	}

	//Direct3D와 관련된 객체를 소멸한다.
	if (m_pd3dDeviceContext) m_pd3dDeviceContext->ClearState();
	for (int i = 0; i < NUM_MRT; ++i)
	{
		if (m_ppd3dRenderTargetView[i]) m_ppd3dRenderTargetView[i]->Release();
		if (m_pd3dMRTSRV[i]) m_pd3dMRTSRV[i]->Release();
		if (m_ppd3dMRTtx[i]) m_ppd3dMRTtx[i]->Release();
		//if (m_pd3dMRTUAV[i]) m_pd3dMRTUAV[i]->Release();
	}
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDepthStencilView) m_pd3dDepthStencilView->Release();

	if (m_pDXGISwapChain) m_pDXGISwapChain->Release();
	if (m_pd3dDeviceContext) m_pd3dDeviceContext->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();

#ifdef _THREAD
	for (int i = 0; i < m_nRenderThreads; ++i)
	{
		m_pRenderingThreadInfo[i].m_pd3dDeferredContext->Release();
		::CloseHandle(m_pRenderingThreadInfo[i].m_hRenderingBeginEvent);
		::CloseHandle(m_pRenderingThreadInfo[i].m_hRenderingEndEvent);

		//::_endthreadex(m_pRenderingThreadInfo[i].m_hRenderingThread[i]);
	}
#endif
	if (m_pd3dBackRenderTargetView) m_pd3dBackRenderTargetView->Release();

	//if (m_pd3dSSAOTargetView) m_pd3dSSAOTargetView->Release();
	if (m_pRenderingThreadInfo) delete[] m_pRenderingThreadInfo;
	if (m_hRenderingEndEvents)  delete[] m_hRenderingEndEvents;
}

bool CGameFramework::CreateRenderTargetDepthStencilView()
{
	HRESULT hResult = S_OK;
	//스왑 체인의 첫 번째 후면버퍼 인터페이스를 가져온다.
	ID3D11Texture2D *pd3dBackBuffer = nullptr;
	//스왑 체인의 첫 번째 후면버퍼에 대한 렌더 타겟 뷰를 생성한다.
	ASSERT(SUCCEEDED(hResult = m_pDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pd3dBackBuffer)));

	D3D11_RENDER_TARGET_VIEW_DESC d3dRTVDesc;
	d3dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	d3dRTVDesc.Texture2D.MipSlice = 0;
	d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateRenderTargetView(pd3dBackBuffer, &d3dRTVDesc, &m_pd3dBackRenderTargetView)));
	if (pd3dBackBuffer) pd3dBackBuffer->Release();

	D3D11_TEXTURE2D_DESC d3d2DBufferDesc;
	ZeroMemory(&d3d2DBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
	d3d2DBufferDesc.Width              = m_nWndClientWidth;
	d3d2DBufferDesc.Height             = m_nWndClientHeight;
	d3d2DBufferDesc.MipLevels          = 1;
	d3d2DBufferDesc.ArraySize          = 1;
	d3d2DBufferDesc.Format             = DXGI_FORMAT_R32_TYPELESS;//DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3d2DBufferDesc.SampleDesc.Count   = 1;
	d3d2DBufferDesc.SampleDesc.Quality = 0;
	d3d2DBufferDesc.Usage              = D3D11_USAGE_DEFAULT;
	d3d2DBufferDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;// | D3D11_BIND_SHADER_RESOURCE;
	d3d2DBufferDesc.CPUAccessFlags     = 0;
	d3d2DBufferDesc.MiscFlags          = 0;
	ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &m_pd3dDepthStencilBuffer/*m_ppd3dMRTtx[MRT_DEPTH]*/)));

	//생성한 깊이 버퍼(Depth Buffer)에 대한 뷰를 생성한다.
	D3D11_DEPTH_STENCIL_VIEW_DESC d3dViewDesc;
	ZeroMemory(&d3dViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	d3dViewDesc.Format             = DXGI_FORMAT_D32_FLOAT;//d3d2DBufferDesc.Format;
	d3dViewDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
	d3dViewDesc.Texture2D.MipSlice = 0;
	ASSERT(SUCCEEDED((hResult      = m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dViewDesc, &m_pd3dDepthStencilView))));

	D3D11_SHADER_RESOURCE_VIEW_DESC d3dSRVDesc;
	ZeroMemory(&d3dSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	d3dSRVDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
	d3dSRVDesc.Texture2D.MipLevels = 1;
	d3dSRVDesc.Format              = DXGI_FORMAT_R32_FLOAT;
	//if (FAILED(hResult = m_pd3dDevice->CreateShaderResourceView(m_ppd3dMRTtx[MRT_DEPTH], &d3dSRVDesc, &m_pd3dMRTSRV[MRT_DEPTH])))
	//	return(false);

	d3d2DBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	D3D11_UNORDERED_ACCESS_VIEW_DESC d3dUAVDesc;
	ZeroMemory(&d3dUAVDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	d3dUAVDesc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
	d3dUAVDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dUAVDesc.Texture2D.MipSlice = 0;

	d3dUAVDesc.Format             = d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;// DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R8G8B8A8_UNORM;
	d3d2DBufferDesc.Width         = m_nWndClientWidth * 0.25f;
	d3d2DBufferDesc.Height        = m_nWndClientHeight * 0.25f;

	d3dRTVDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	d3d2DBufferDesc.BindFlags     = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	d3d2DBufferDesc.Width         = m_nWndClientWidth;
	d3d2DBufferDesc.Height        = m_nWndClientHeight;
	d3dRTVDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
	//d3dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

	ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &m_ppd3dMRTtx[MRT_SCENE])));
	ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateShaderResourceView(m_ppd3dMRTtx[MRT_SCENE], &d3dSRVDesc, &m_pd3dMRTSRV[MRT_SCENE])));
	ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateRenderTargetView(m_ppd3dMRTtx[MRT_SCENE], &d3dRTVDesc, &m_ppd3dRenderTargetView[MRT_SCENE])));
	//	ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateUnorderedAccessView(m_ppd3dMRTtx[MRT_SCENE], &d3dUAVDesc, &m_pd3dPostUAV[1])));
	if (m_ppd3dMRTtx[MRT_SCENE]) m_ppd3dMRTtx[MRT_SCENE]->Release();

	d3dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	//ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &m_ppd3dMRTtx[0])));
	//ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateShaderResourceView(m_ppd3dMRTtx[0], &d3dSRVDesc, &m_pd3dMRTSRV[0])));

	d3d2DBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;// | D3D11_BIND_UNORDERED_ACCESS;
//	d3d2DBufferDesc.Width = FRAME_BUFFER_WIDTH * 0.5f;
//	d3d2DBufferDesc.Height = FRAME_BUFFER_HEIGHT * 0.5f;

	for (int i = 1; i < NUM_MRT; ++i)
	{
		//d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		switch (i)
		{
		case MRT_TXCOLOR:
			d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			break;
		case MRT_DIFFUSE:
		case MRT_SPEC:
		case MRT_POS:
			d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;// DXGI_FORMAT_R32G32B32A32_SINT;
			break;
		case MRT_NORMAL:
			d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_SNORM;
			break;
		default:
			d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &m_ppd3dMRTtx[i])));
		ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateShaderResourceView(m_ppd3dMRTtx[i], &d3dSRVDesc, &m_pd3dMRTSRV[i])));
		ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateRenderTargetView(m_ppd3dMRTtx[i], &d3dRTVDesc, &m_ppd3dRenderTargetView[i])));

		if (m_ppd3dMRTtx[i])
		{
			m_ppd3dMRTtx[i]->Release();
			m_ppd3dMRTtx[i] = nullptr;
		}
		m_ppd3dMRTtx[0] = nullptr;
	}
	d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//	ID3D11Texture2D * pdTx2D = nullptr;
	//	ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &pdTx2D)));
	//	ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateShaderResourceView(pdTx2D, &d3dSRVDesc, &m_pd3dSSAOSRV)));
	//	ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateRenderTargetView(pdTx2D, &d3dRTVDesc, &m_pd3dSSAOTargetView)));

	//	TXMgr.InsertShaderResourceView(m_pd3dSSAOSRV, "sr_rtvSSAO", 0);

	//	m_pd3dSSAOSRV->Release();
	//	pdTx2D->Release();

		//m_pd3dSSAOSRV = ViewMgr.GetSRV("sr2d_SSAO"); m_pd3dSSAOSRV->AddRef();
		//m_pd3dSSAOTargetView = ViewMgr.GetRTV("sr2d_SSAO"); m_pd3dSSAOTargetView->AddRef();

		//m_pd3dDeviceContext->OMSetRenderTargets(5, &m_pd3dRenderTargetView, m_pd3dDepthStencilView);

	return(true);
}

bool CGameFramework::CreateDirect3DDisplay()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

	UINT dwCreateDeviceFlags = 0;
#ifdef _DEBUG
	dwCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//디바이스를 생성하기 위하여 시도할 드라이버 유형의 순서를 나타낸다.
	D3D_DRIVER_TYPE d3dDriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT nDriverTypes = sizeof(d3dDriverTypes) / sizeof(D3D10_DRIVER_TYPE);

	//디바이스를 생성하기 위하여 시도할 특성 레벨의 순서를 나타낸다.
	D3D_FEATURE_LEVEL d3dFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	UINT nFeatureLevels = sizeof(d3dFeatureLevels) / sizeof(D3D_FEATURE_LEVEL);

	//생성할 스왑 체인을 서술하는 구조체이다.
	/*스왑 체인을 생성하기 위한 스왑 체인 구조체 DXGI_SWAP_CHAIN_DESC를 설정한다. 스왑 체인의 버퍼 크기는 주 윈도우의 클라이언트 영역의 크기로 설정하고 출력 윈도우는 주 윈도우로 설정한다. 스왑 체인의 출력 모드는 윈도우 모드로 설정한다.*/
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount                        = 1;
	dxgiSwapChainDesc.BufferDesc.Width                   = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height                  = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow                       = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count                   = 1;
	dxgiSwapChainDesc.SampleDesc.Quality                 = 0;
	dxgiSwapChainDesc.Windowed                           = TRUE;

	/*원하는 디바이스 드라이버 유형 순서로 스왑 체인과 디바이스, 디바이스 컨텍스트를 생성하기 위해 드라이버 유형 배열(d3dDriverTypes[])의 각 원소에 대하여 D3D11CreateDeviceAndSwapChain() 함수 호출을 통해 디바이스와 스왑체인 생성을 시도한다.*/
	D3D_DRIVER_TYPE nd3dDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL nd3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	HRESULT hResult = S_OK;
	//디바이스의 드라이버 유형과 특성 레벨을 지원하는 디바이스와 스왑 체인을 생성한다. 고수준의 디바이스 생성을 시도하고 실패하면 다음 수준의 디바이스를 생성한다.
	for (UINT i = 0; i < nDriverTypes; i++)
	{
		nd3dDriverType = d3dDriverTypes[i];
		if (SUCCEEDED(hResult = D3D11CreateDeviceAndSwapChain(nullptr, nd3dDriverType, nullptr, dwCreateDeviceFlags, d3dFeatureLevels, nFeatureLevels, D3D11_SDK_VERSION, &dxgiSwapChainDesc, &m_pDXGISwapChain, &m_pd3dDevice, &nd3dFeatureLevel, &m_pd3dDeviceContext))) break;
	}
	if (!m_pDXGISwapChain || !m_pd3dDevice || !m_pd3dDeviceContext) return(false);
	//디바이스가 생성되면 렌더 타겟 뷰를 생성하기 위해 CreateRenderTargetView() 함수를 호출한다.
	//렌더 타겟 뷰를 생성하는 함수를 호출한다.

	return(true);
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//마우스 캡쳐를 하고 현재 마우스 위치를 가져온다.
		SetCapture(hWnd);
		//GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		//마우스 캡쳐를 해제한다.
		ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}

	if (gpScene) gpScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (gpScene) gpScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'Q':
			if (m_pSceneShader) 
				static_cast<CSceneShader*>(m_pSceneShader)->SetDrawOption((m_iDrawOption = 0));
			break;
		case 'W':
			if (m_pSceneShader)
				static_cast<CSceneShader*>(m_pSceneShader)->SetDrawOption((m_iDrawOption = 1));
			break;
		case 'E':
			if (m_pSceneShader)
				static_cast<CSceneShader*>(m_pSceneShader)->SetDrawOption((m_iDrawOption = 2));
			break;
		case 'R':
			if (m_pSceneShader)
				static_cast<CSceneShader*>(m_pSceneShader)->SetDrawOption((m_iDrawOption = 3));
			break;
		case 'T':
			if (m_pSceneShader)
				static_cast<CSceneShader*>(m_pSceneShader)->SetDrawOption((m_iDrawOption = 4));
			break;
		case 'Y':
			if (m_pSceneShader)
				static_cast<CSceneShader*>(m_pSceneShader)->SetDrawOption((m_iDrawOption = 5));
			break;
			//		case 'Z':
			//			m_pSceneShader->SetDrawOption((m_iDrawOption = -1));
			//			m_pSceneShader->SetTexture(0, m_pd3dSSAOSRV/*TXMgr.GetShaderResourceView("srv_rtvSSAO")*/);
			break;
			//case 'P':
			//	m_pSceneShader->SetDrawOption((m_iDrawOption = -1));
			//	m_pSceneShader->SetTexture(0, m_pd3dsrvShadowMap);
			//	break;
		}
		break;

	case WM_KEYUP:
		switch (wParam)
		{
			/*‘F1’ 키를 누르면 1인칭 카메라, ‘F2’ 키를 누르면 스페이스-쉽 카메라로 변경한다, ‘F3’ 키를 누르면 3인칭 카메라로 변경한다.*/
		case VK_F1:
		case VK_F2:
		case VK_F3:
			if (m_pPlayer)
			{
				m_pPlayer->ChangeCamera(m_pd3dDevice, (wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
				m_pCamera = m_pPlayer->GetCamera();
				//씬에 현재 카메라를 설정한다.
				gpScene->SetCamera(m_pCamera);
			}
			break;

		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
		/*윈도우의 크기가 변경될 때(현재는 “Alt+Enter“ 전체 화면 모드와 윈도우 모드로 전환될 때) 스왑 체인의 후면버퍼 크기를 조정하고 후면버퍼에 대한 렌더 타겟 뷰를 다시 생성한다. */
	//case WM_SIZE:
	//{
	//				m_nWndClientWidth = LOWORD(lParam);
	//				m_nWndClientHeight = HIWORD(lParam);

	//				m_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	//				//게임 객체를 소멸한다.
	//				ReleaseObjects();

	//				//Direct3D와 관련된 객체를 소멸한다.
	//				if (m_pd3dDeviceContext) m_pd3dDeviceContext->ClearState();
	//				for (int i = 0; i < NUM_MRT; ++i)
	//				{
	//					if (m_ppd3dRenderTargetView[i]) m_ppd3dRenderTargetView[i]->Release();
	//					if (m_pd3dMRTSRV[i]) m_pd3dMRTSRV[i]->Release();
	//					if (m_ppd3dMRTtx[i]) m_ppd3dMRTtx[i]->Release();
	//				}
	//				if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	//				if (m_pd3dDepthStencilView) m_pd3dDepthStencilView->Release();

	//				if (m_pRenderingThreadInfo) delete[] m_pRenderingThreadInfo;
	//				if (m_hRenderingEndEvents) delete[] m_hRenderingEndEvents;

	//				//if (m_pd3dStencilMirrorState) m_pd3dStencilMirrorState->Release();
	//				//if (m_pd3dDepthStencilReflectState) m_pd3dDepthStencilReflectState->Release();

	//				//if (m_pd3dNoWriteBlendState) m_pd3dNoWriteBlendState->Release();
	//				//if (m_pd3dCullCWRasterizerState) m_pd3dCullCWRasterizerState->Release();

	//				m_pDXGISwapChain->ResizeBuffers(2, m_nWndClientWidth, m_nWndClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	//				CreateRenderTargetDepthStencilView();

	//				CCamera * pCamera = m_pPlayer->GetCamera();
	//				if (pCamera) pCamera->SetViewport(m_pd3dDeviceContext, 0, 0, m_nWndClientWidth, m_nWndClientHeight, 0.0f, 1.0f);

	//				break;
	//}

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

void CGameFramework::InitilizeThreads()
{
#ifdef _THREAD
	//InitializeCriticalSection(&m_cs);

	m_nRenderThreads = gpScene->GetRenderThreadNumber();
	m_pRenderingThreadInfo = new RenderingThreadInfo[m_nRenderThreads];
	m_hRenderingEndEvents = new HANDLE[m_nRenderThreads];

	for (int i = 0; i < m_nRenderThreads; ++i)
	{
		//m_pRenderingThreadInfo[i].m_nShaders = m_nShaders;
		m_pRenderingThreadInfo[i].m_nRenderingThreadID    = i;
		m_pRenderingThreadInfo[i].m_pPlayer               = m_pPlayer;
		m_pRenderingThreadInfo[i].m_pScene                = gpScene;
		m_pRenderingThreadInfo[i].m_pd3dCommandList       = nullptr;
		m_pRenderingThreadInfo[i].m_hRenderingBeginEvent  = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		m_pRenderingThreadInfo[i].m_hRenderingEndEvent    = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		m_pRenderingThreadInfo[i].m_pd3dDepthStencilView  = m_pd3dDepthStencilView;
		m_pRenderingThreadInfo[i].m_ppd3dRenderTargetView = m_ppd3dRenderTargetView;
		m_pRenderingThreadInfo[i].m_puRenderState         = &m_uRenderState;
		//m_pRenderingThreadInfo[i].m_pbInGame              = &m_bInGame;
		m_hRenderingEndEvents[i]                          = m_pRenderingThreadInfo[i].m_hRenderingEndEvent;
		// 디퍼드 컨텍스트 생성
		m_pd3dDevice->CreateDeferredContext(0, &m_pRenderingThreadInfo[i].m_pd3dDeferredContext);

		if (m_pPlayer)
			m_pRenderingThreadInfo[i].m_pPlayer->GetCamera()->SetViewport(m_pRenderingThreadInfo[i].m_pd3dDeferredContext, 0, 0, FRAME_BUFFER_WIDTH /** 0.5f*/, FRAME_BUFFER_HEIGHT /** 0.5f*/, 0.0f, 1.0f);
		else
			CCamera::SetViewport(m_pRenderingThreadInfo[i].m_pd3dDeferredContext, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

		m_pRenderingThreadInfo[i].m_pd3dDeferredContext->OMSetRenderTargets(NUM_MRT - 1,
			&m_ppd3dRenderTargetView[1], m_pd3dDepthStencilView);

		//m_pRenderingThreadInfo[i].m_hRenderingThread    = (HANDLE)::CreateThread(nullptr, 0, CGameFramework::RenderThread, &m_pRenderingThreadInfo[i], 0/*CREATE_SUSPENDED*/, nullptr);
		m_pRenderingThreadInfo[i].m_hRenderingThread = (HANDLE)::_beginthreadex(nullptr, 0,
			CGameFramework::RenderThread, &m_pRenderingThreadInfo[i], CREATE_SUSPENDED, nullptr);

		//gThreadState.push_back(m_pRenderingThreadInfo[i].m_hRenderingThread);
		::ResumeThread(m_pRenderingThreadInfo[i].m_hRenderingThread);
	}
#endif
}

void CGameFramework::ReleaseThreads()
{
	gbChangeScene = true;

	for (int i = 0; i < m_nRenderThreads; ++i)
		::SetEvent(m_pRenderingThreadInfo[i].m_hRenderingBeginEvent);

	::WaitForMultipleObjects(m_nRenderThreads, m_hRenderingEndEvents, TRUE, INFINITE);

	gbChangeScene = false;
}

void CGameFramework::BuildObjects(CScene * pScene)
{
	CShader::CreateShaderVariables(m_pd3dDevice);
	CIlluminatedShader::CreateShaderVariables(m_pd3dDevice);

	SceneShaderBuildInfo info;
	info.pd3dBackRTV = m_pd3dBackRenderTargetView;
	info.ppMRTSRVArray = m_pd3dMRTSRV;

	pScene->BuildObjects(m_pd3dDevice, m_pd3dDeviceContext, &info);
	m_pPlayerShader = pScene->GetPlayerShader();

	if (m_pPlayerShader)
	{
		m_pPlayer = m_pPlayerShader->GetPlayer();
		m_pCamera = m_pPlayer->GetCamera();
	}
	else
		CCamera::SetViewport(m_pd3dDeviceContext, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT); // FRAME_BUFFER_WIDTH * 0.5f, FRAME_BUFFER_HEIGHT * 0.5f);

	m_pSceneShader = pScene->GetSceneShader();
}

void CGameFramework::ReleaseObjects(CScene * pScene)
{
	//CShader 클래스의 정적(static) 멤버 변수로 선언된 상수 버퍼를 반환한다.
	CShader::ReleaseShaderVariables();
	CIlluminatedShader::ReleaseShaderVariables();

	pScene->ReleaseObjects();
}

void CGameFramework::ProcessInput()
{
	bool bProcessedByScene = false;
	if (gpScene) bProcessedByScene = gpScene->ProcessInput(m_hWnd, m_GameTimer.GetTimeElapsed());
}

void CGameFramework::AnimateObjects()
{
	float fFrameTime = m_GameTimer.GetTimeElapsed();
	gpScene->AnimateObjects(fFrameTime);
	if (m_pSceneShader) m_pSceneShader->AnimateObjects(fFrameTime);
}

void CGameFramework::Render()
{
	gpScene->OnCreateShadowMap(m_pd3dDeviceContext);

	float fClearColor[4] = { 0.0f, 0.125f, 0.3f, 0.0f };	//렌더 타겟 뷰를 채우기 위한 색상을 설정한다.
															/* 렌더 타겟 뷰를 fClearColor[] 색상으로 채운다. 즉, 렌더 타겟 뷰에 연결된 스왑 체인의 첫 번째 후면버퍼를 fClearColor[] 색상으로 지운다. */
	if (gpScene->GetMRTNumber() > 1)
	{
		m_pd3dDeviceContext->OMSetRenderTargets(NUM_MRT - 1, &m_ppd3dRenderTargetView[1], m_pd3dDepthStencilView);
		m_pd3dDeviceContext->ClearRenderTargetView(m_pd3dBackRenderTargetView, fClearColor);
		//for (int i = 0; i < NUM_MRT; ++i)
		//{
		//	m_pd3dDeviceContext->ClearRenderTargetView(m_ppd3dRenderTargetView[i], fClearColor);
		//}
		m_pd3dDeviceContext->ClearDepthStencilView(m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	else
	{
		m_pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dBackRenderTargetView, m_pd3dDepthStencilView);
		m_pd3dDeviceContext->ClearRenderTargetView(m_pd3dBackRenderTargetView, fClearColor);
		m_pd3dDeviceContext->ClearDepthStencilView(m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	#ifdef _THREAD
	if (m_nRenderThreads <= 1) ImmediateRender();
		else DeferredRender();
	#else
		ImmediateRender();
	#endif
}

void CGameFramework::DeferredRender()
{
	m_nRenderThreads = gpScene->GetRenderThreadNumber();

	for (int i = 0; i < m_nRenderThreads; ++i)
	{
		::SetEvent(m_pRenderingThreadInfo[i].m_hRenderingBeginEvent);
	}
	::WaitForMultipleObjects(m_nRenderThreads, m_hRenderingEndEvents, TRUE, INFINITE);

	for (int i = 0; i < m_nRenderThreads; ++i)
	{
		//UpdateShadowResource();
		m_pd3dDeviceContext->ExecuteCommandList(m_pRenderingThreadInfo[i].m_pd3dCommandList, TRUE);
		m_pRenderingThreadInfo[i].m_pd3dCommandList->Release();
	}
}

void CGameFramework::ImmediateRender()
{
	RENDER_INFO RenderInfo;
	RenderInfo.pCamera      = m_pCamera;
	RenderInfo.ppd3dMrtRTV  = m_ppd3dRenderTargetView;
	RenderInfo.ThreadID     = -1;
	RenderInfo.pRenderState = &m_uRenderState;

	if (m_pPlayer)
		m_pPlayer->UpdateShaderVariables(m_pd3dDeviceContext);

	gpScene->UpdateLights(m_pd3dDeviceContext);
	gpScene->Render(m_pd3dDeviceContext, &RenderInfo);

	if (m_pPlayerShader)
		m_pPlayerShader->Render(m_pd3dDeviceContext, *RenderInfo.pRenderState, RenderInfo.pCamera);
}

void CGameFramework::PostProcess()
{
	if (m_pSceneShader)
	{
#pragma region IF_SCENESHADER_REDNER_IN_FRAMEWORK
		//m_pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dSSAOTargetView, nullptr);
		//m_pd3dDeviceContext->PSSetShaderResources(21, 1, &m_pd3dMRTSRV[MRT_NORMAL]);
		//m_pSSAOShader->Render(m_pd3dDeviceContext, NULL, m_pCamera);

		//ShadowMgr.UpdateStaticShadowResource(m_pd3dDeviceContext);
		//ShadowMgr.UpdateDynamicShadowResource(m_pd3dDeviceContext);

		//gpScene->UpdateLights(m_pd3dDeviceContext);
		//m_pd3dDeviceContext->OMSetRenderTargets(1, &m_ppd3dRenderTargetView[MRT_SCENE], nullptr);
		//m_pSceneShader->Render(m_pd3dDeviceContext, 0, m_pCamera);
#pragma endregion
		m_pSceneShader->PostProcessingRender(m_pd3dDeviceContext, 0, m_pCamera);
	}
	gpScene->UIRender(m_pd3dDeviceContext);
}

void CGameFramework::FrameAdvance()
{
	//타이머의 시간이 갱신되도록 하고 프레임 레이트를 계산한다.
	m_GameTimer.Tick();
	//사용자 입력을 처리하기 위한 ProcessInput() 함수를 호출한다.
	ProcessInput();
	//게임 객체의 애니메이션을 처리하기 위한 AnimateObjects() 함수를 호출한다.
	AnimateObjects();
	// 렌더링
	Render();
	// 후처리 기법
	PostProcess();
	//스왑 체인의 후면버퍼의 내용이 디스플레이에 출력되도록 프리젠트한다.
	m_pDXGISwapChain->Present(0, 0);

	m_GameTimer.GetFrameRate(m_pszBuffer + 12, 37);
	::SetWindowText(m_hWnd, m_pszBuffer);
}

UINT WINAPI CGameFramework::RenderThread(LPVOID lpParameter)
{
	RenderingThreadInfo * pRenderingThreadInfo = (RenderingThreadInfo*)lpParameter;
	CScene * pScene                            = pRenderingThreadInfo->m_pScene;
	CPlayer * pPlayer                          = pRenderingThreadInfo->m_pPlayer;
	ID3D11DeviceContext * pd3dDeferredContext  = pRenderingThreadInfo->m_pd3dDeferredContext;

	RENDER_INFO RenderInfo;
	RenderInfo.pCamera                         = (pPlayer) ? pPlayer->GetCamera() : nullptr;
	RenderInfo.ppd3dMrtRTV                     = pRenderingThreadInfo->m_ppd3dRenderTargetView;
	RenderInfo.ThreadID                        = pRenderingThreadInfo->m_nRenderingThreadID;
	RenderInfo.pRenderState                    = pRenderingThreadInfo->m_puRenderState;

	if (NUM_THREAD == 1) RenderInfo.ThreadID = -1;

	pd3dDeferredContext->OMSetDepthStencilState(nullptr, 1);
	while (true)
	{
		::WaitForSingleObject(pRenderingThreadInfo->m_hRenderingBeginEvent, INFINITE);
		if (gbChangeScene) break;

		if (RenderInfo.ThreadID == 0)
		{
			//pFramework->UpdateShadowResource();
			pd3dDeferredContext->ClearDepthStencilView(pRenderingThreadInfo->m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		}
		//pd3dDeferredContext->OMSetRenderTargets(NUM_MRT-1, &pRenderingThreadInfo->m_ppd3dRenderTargetView[1], pRenderingThreadInfo->m_pd3dDepthStencilView);
		//pd3dDeferredContext->ClearDepthStencilView(pRenderingThreadInfo->m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0.0f);
		//pd3dDeferredContext->OMSetDepthStencilState(nullptr, 1);

		if (pPlayer) pPlayer->UpdateShaderVariables(pd3dDeferredContext);

		pScene->Render(pd3dDeferredContext, &RenderInfo);

		pd3dDeferredContext->FinishCommandList(TRUE, &pRenderingThreadInfo->m_pd3dCommandList);
		::SetEvent(pRenderingThreadInfo->m_hRenderingEndEvent);
	}
	cout << pRenderingThreadInfo ->m_nRenderingThreadID << "번 Thread를 종료합니다. " << endl;
	::SetEvent(pRenderingThreadInfo->m_hRenderingEndEvent);

	::CloseHandle(pRenderingThreadInfo->m_hRenderingBeginEvent);
	::CloseHandle(pRenderingThreadInfo->m_hRenderingEndEvent);
	::CloseHandle(pRenderingThreadInfo->m_hRenderingThread);

	return 0;
}