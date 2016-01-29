#include "stdafx.h"
#include "GameFramework.h"
#include "MyInline.h"
//CRITICAL_SECTION m_cs;

CGameFramework::CGameFramework()
{
	m_pPlayerShader = nullptr;

	m_pd3dDevice = nullptr;
	m_pDXGISwapChain = nullptr;
	for (int i = 0; i < NUM_MRT; ++i)
	{
		m_ppd3dRenderTargetView[i] = nullptr;
		m_pd3dMRTSRV[i] = nullptr;
		m_ppd3dMRTtx[i] = nullptr;
	}
	m_pd3dDepthStencilBuffer = nullptr;
	m_pd3dDepthStencilView = nullptr;
	m_pd3dDeviceContext = nullptr;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_nRenderThreads = 0;

	m_pRenderingThreadInfo = nullptr;
	//m_pd3dPostProcessing = nullptr;

	m_pd3dBackRenderTargetView = nullptr;

	m_pScene = nullptr;
	m_pPlayer = nullptr;
	m_pCamera = nullptr;
	m_iDrawOption = MRT_SCENE;
	m_pSceneShader = nullptr;
	_tcscpy_s(m_pszBuffer, _T("__GodGame__("));

	m_pd3dSSAOTargetView = nullptr;

	//m_pSSAOShader = nullptr;
	m_uRenderState = 0;
}


CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	//Direct3D 디바이스, 디바이스 컨텍스트, 스왑 체인 등을 생성하는 함수를 호출한다. 
	if (!CreateDirect3DDisplay()) return(false);
	if (!CreateRenderTargetDepthStencilView()) return(false);

	CManagers::BuildManagers(m_pd3dDevice, m_pd3dDeviceContext);
	BuildObjects();

	InitilizeThreads();
	return(true);
}

void CGameFramework::OnDestroy()
{
	//게임 객체를 소멸한다. 
	ReleaseObjects();

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
	//if (m_pd3dPostProcessing) m_pd3dPostProcessing->Release();
	if (m_pd3dSSAOTargetView) m_pd3dSSAOTargetView->Release();
	

	if (m_pRenderingThreadInfo) delete[] m_pRenderingThreadInfo;
	if (m_hRenderingEndEvents) delete[] m_hRenderingEndEvents;
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
	d3d2DBufferDesc.Width = m_nWndClientWidth;
	d3d2DBufferDesc.Height = m_nWndClientHeight;
	d3d2DBufferDesc.MipLevels = 1;
	d3d2DBufferDesc.ArraySize = 1;
	d3d2DBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;//DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3d2DBufferDesc.SampleDesc.Count = 1;
	d3d2DBufferDesc.SampleDesc.Quality = 0;
	d3d2DBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3d2DBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;// | D3D11_BIND_SHADER_RESOURCE;
	d3d2DBufferDesc.CPUAccessFlags = 0;
	d3d2DBufferDesc.MiscFlags = 0;
	ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &m_pd3dDepthStencilBuffer/*m_ppd3dMRTtx[MRT_DEPTH]*/)));

	//생성한 깊이 버퍼(Depth Buffer)에 대한 뷰를 생성한다.
	D3D11_DEPTH_STENCIL_VIEW_DESC d3dViewDesc;
	ZeroMemory(&d3dViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	d3dViewDesc.Format = DXGI_FORMAT_D32_FLOAT;//d3d2DBufferDesc.Format;
	d3dViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	d3dViewDesc.Texture2D.MipSlice = 0;
	ASSERT(SUCCEEDED((hResult = m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dViewDesc, &m_pd3dDepthStencilView))));

	D3D11_SHADER_RESOURCE_VIEW_DESC d3dSRVDesc;
	ZeroMemory(&d3dSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	d3dSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	d3dSRVDesc.Texture2D.MipLevels = 1;
	d3dSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	//if (FAILED(hResult = m_pd3dDevice->CreateShaderResourceView(m_ppd3dMRTtx[MRT_DEPTH], &d3dSRVDesc, &m_pd3dMRTSRV[MRT_DEPTH]))) 
	//	return(false);

	d3d2DBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	D3D11_UNORDERED_ACCESS_VIEW_DESC d3dUAVDesc;
	ZeroMemory(&d3dUAVDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	d3dUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	d3dUAVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dUAVDesc.Texture2D.MipSlice = 0;

	d3dUAVDesc.Format = d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;// DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R8G8B8A8_UNORM;
	d3d2DBufferDesc.Width = m_nWndClientWidth * 0.25f;
	d3d2DBufferDesc.Height = m_nWndClientHeight * 0.25f;
		
	
	d3dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	d3d2DBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	d3d2DBufferDesc.Width = m_nWndClientWidth;
	d3d2DBufferDesc.Height = m_nWndClientHeight;
	d3dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
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

		case MRT_DIFFUSE:
			d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case MRT_SPEC:
			d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			break;
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
//		ASSERT(SUCCEEDED(hResult = m_pd3dDevice->CreateUnorderedAccessView(m_ppd3dMRTtx[i], &d3dUAVDesc, &m_pd3dMRTUAV[i])));

		if (m_ppd3dMRTtx[i]) 
		{
			m_ppd3dMRTtx[i]->Release();
			m_ppd3dMRTtx[i] = nullptr;
		}
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


void CGameFramework::BuildStaticShadowMap()
{
	// 방향성 광원은 위치 필요 없다.
	//LIGHT * pLight = m_pScene->GetLight(2);
	CHeightMapTerrain * pTerrain = m_pScene->GetTerrain();
	float fHalf = pTerrain->GetWidth() * 0.5;

	CShadowMgr * pSdwMgr = &ShadowMgr;
	pSdwMgr->BuildShadowMap(m_pd3dDeviceContext, XMFLOAT3(fHalf - 1000.0f, 0.0f, fHalf), XMFLOAT3(fHalf, fHalf, fHalf), fHalf);

	m_uRenderState = NOT_PSUPDATE;
	pSdwMgr->SetStaticShadowMap(m_pd3dDeviceContext, m_pCamera);

	m_pScene->GetShader(1)->Render(m_pd3dDeviceContext, m_uRenderState, m_pCamera);

	pSdwMgr->ResetStaticShadowMap(m_pd3dDeviceContext, m_pCamera);
	m_uRenderState = NULL;

	pSdwMgr->UpdateStaticShadowResource(m_pd3dDeviceContext);
	m_pSceneShader->SetLightSRV(TXMgr.GetShaderResourceView("srv_StaticShaodwMap"));
}

void CGameFramework::OnCreateShadowMap(CShader*ShaderList[], int nShaders)
{
	m_uRenderState = NOT_PSUPDATE;

	ShadowMgr.SetDynamicShadowMap(m_pd3dDeviceContext, m_pCamera);

	for (int i = 0; i < nShaders; ++i)
		ShaderList[i]->Render(m_pd3dDeviceContext, m_uRenderState, m_pCamera);

	ShadowMgr.ResetDynamicShadowMap(m_pd3dDeviceContext, m_pCamera);
	m_uRenderState = NULL;

	//(*m_pScene->GetShader(1))->Render(m_pd3dDeviceContext, m_uRenderState, nullptr);
	//m_pPlayerShader->Render(m_pd3dDeviceContext, m_uRenderState, m_pCamera);
	//(*m_pScene->GetShader(2))->Render(m_pd3dDeviceContext, m_uRenderState, nullptr);

//#ifdef _THREAD
//	for (int i = 1; i < 3; ++i)
//	{
//		::SetEvent(m_pRenderingThreadInfo[i].m_hRenderingBeginEvent);
//	}
//	::WaitForMultipleObjects(2, &m_hRenderingEndEvents[1], TRUE, INFINITE);
//#endif
//	ID3D11RenderTargetView* renderTargets[1] = { 0 };
//	m_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, m_pd3ddsvShadowMap);
//	m_pd3dDeviceContext->ClearDepthStencilView(m_pd3ddsvShadowMap, D3D11_CLEAR_DEPTH, 1.0f, 0);
//#ifdef _THREAD
//	for (int i = 1; i < 3; ++i)
//	{
//		m_pd3dDeviceContext->ExecuteCommandList(m_pRenderingThreadInfo[i].m_pd3dCommandList, TRUE);
//		m_pRenderingThreadInfo[i].m_pd3dCommandList->Release();
//	}
//#else
//	if (m_pScene) m_pScene->Render(m_pd3dDeviceContext, m_pCamera);
//#endif


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
	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.Windowed = TRUE;

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

	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//마우스 캡쳐를 하고 현재 마우스 위치를 가져온다.
		SetCapture(hWnd);
		GetCursorPos(&m_ptOldCursorPos);
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

}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'Q':
			m_pSceneShader->SetDrawOption((m_iDrawOption = 0));
			break;
		case 'W':
			m_pSceneShader->SetDrawOption((m_iDrawOption = 1));
			break;
		case 'E':
			m_pSceneShader->SetDrawOption((m_iDrawOption = 2));
			break;
		case 'R':
			m_pSceneShader->SetDrawOption((m_iDrawOption = 3));
			break;
		case 'T':
			m_pSceneShader->SetDrawOption((m_iDrawOption = 4));
			break;
		case 'Y':
			m_pSceneShader->SetDrawOption((m_iDrawOption = 5));
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
				m_pScene->SetCamera(m_pCamera);
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

void CGameFramework::BuildObjects()
{
	CShader::CreateShaderVariables(m_pd3dDevice);
	CIlluminatedShader::CreateShaderVariables(m_pd3dDevice);

	m_pSceneShader = new CSceneShader();
	m_pSceneShader->CreateShader(m_pd3dDevice);
	m_pSceneShader->BuildObjects(m_pd3dDevice, m_pd3dMRTSRV, m_iDrawOption, m_pd3dBackRenderTargetView);

	m_pScene = new CScene();
	m_pScene->SetDepthStencilView(m_pd3dDepthStencilView);
	m_pScene->BuildObjects(m_pd3dDevice, m_pSceneShader);

	CHeightMapTerrain *pTerrain = m_pScene->GetTerrain();

	m_pPlayerShader = new CPlayerShader();
	m_pPlayerShader->CreateShader(m_pd3dDevice);
	m_pPlayerShader->BuildObjects(m_pd3dDevice, pTerrain);
	m_pPlayer = m_pPlayerShader->GetPlayer();

	m_pCamera = m_pPlayer->GetCamera();
	m_pCamera->SetViewport(m_pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pCamera->GenerateViewMatrix();

	m_pScene->SetCamera(m_pCamera);
	m_pScene->SetPlayerShader(m_pPlayerShader);

//	m_pSSAOShader = new CSSAOShader();
//	m_pSSAOShader->CreateShaderVariable(m_pd3dDevice);
//	m_pSSAOShader->CreateShader(m_pd3dDevice);
//	m_pSSAOShader->BuildObjects(m_pd3dDevice);

	m_pSceneShader->CreateConstantBuffer(m_pd3dDevice, m_pd3dDeviceContext);
	BuildStaticShadowMap();
}

void CGameFramework::InitilizeThreads()
{
#ifdef _THREAD
	//InitializeCriticalSection(&m_cs);

	m_nRenderThreads = NUM_THREAD;
	m_pRenderingThreadInfo = new RenderingThreadInfo[m_nRenderThreads];
	m_hRenderingEndEvents = new HANDLE[m_nRenderThreads];

	for (int i = 0; i < m_nRenderThreads; ++i)
	{
		//m_pRenderingThreadInfo[i].m_nShaders = m_nShaders;
		m_pRenderingThreadInfo[i].m_nRenderingThreadID = i;
		m_pRenderingThreadInfo[i].m_pPlayer = m_pPlayer;
		m_pRenderingThreadInfo[i].m_pScene = m_pScene;
		m_pRenderingThreadInfo[i].m_pd3dCommandList = nullptr;
		m_pRenderingThreadInfo[i].m_hRenderingBeginEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		m_pRenderingThreadInfo[i].m_hRenderingEndEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		m_pRenderingThreadInfo[i].m_pd3dDepthStencilView = m_pd3dDepthStencilView;
		m_pRenderingThreadInfo[i].m_ppd3dRenderTargetView = m_ppd3dRenderTargetView;
		m_pRenderingThreadInfo[i].m_puRenderState = &m_uRenderState;
		m_hRenderingEndEvents[i] = m_pRenderingThreadInfo[i].m_hRenderingEndEvent;
		// 디퍼드 컨텍스트 생성
		m_pd3dDevice->CreateDeferredContext(0, &m_pRenderingThreadInfo[i].m_pd3dDeferredContext);
		m_pRenderingThreadInfo[i].m_pPlayer->GetCamera()->SetViewport
			(m_pRenderingThreadInfo[i].m_pd3dDeferredContext, 0, 0, FRAME_BUFFER_WIDTH /** 0.5f*/, FRAME_BUFFER_HEIGHT /** 0.5f*/, 0.0f, 1.0f);

		m_pRenderingThreadInfo[i].m_pd3dDeferredContext->OMSetRenderTargets(NUM_MRT - 1,
			&m_ppd3dRenderTargetView[1], m_pd3dDepthStencilView);

		//m_pRenderingThreadInfo[i].m_hRenderingThread = (HANDLE)::CreateThread(nullptr, 0,
		//	CGameFramework::RenderThread, &m_pRenderingThreadInfo[i], 0/*CREATE_SUSPENDED*/, nullptr);

		m_pRenderingThreadInfo[i].m_hRenderingThread = (HANDLE)::_beginthreadex(nullptr, 0,
			CGameFramework::RenderThread, &m_pRenderingThreadInfo[i], CREATE_SUSPENDED, nullptr);
		::ResumeThread(m_pRenderingThreadInfo[i].m_hRenderingThread);
	}
#endif
}

void CGameFramework::ReleaseObjects()
{
	//CShader 클래스의 정적(static) 멤버 변수로 선언된 상수 버퍼를 반환한다.
	CShader::ReleaseShaderVariables();
	CIlluminatedShader::ReleaseShaderVariables();

	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;


	if (m_pSceneShader) m_pSceneShader->ReleaseObjects();
	if (m_pSceneShader) delete m_pSceneShader;
	m_pSceneShader = nullptr;

	//if (m_pSSAOShader) m_pSSAOShader->ReleaseObjects();
	//if (m_pSSAOShader) delete m_pSSAOShader;
	//m_pSSAOShader = nullptr;
}

void CGameFramework::ProcessInput()
{
	bool bProcessedByScene = false;
	if (m_pScene) bProcessedByScene = m_pScene->ProcessInput();
	if (!bProcessedByScene)
	{
		static UCHAR pKeyBuffer[256];
		DWORD dwDirection = 0;
		/*키보드의 상태 정보를 반환한다. 화살표 키(‘→’, ‘←’, ‘↑’, ‘↓’)를 누르면 플레이어를 오른쪽/왼쪽(로컬 x-축), 앞/뒤(로컬 z-축)로 이동한다. ‘Page Up’과 ‘Page Down’ 키를 누르면 플레이어를 위/아래(로컬 y-축)로 이동한다.*/
		if (GetKeyboardState(pKeyBuffer))
		{
			if (pKeyBuffer[VK_UP] & 0xF0) dwDirection |= DIR_FORWARD;
			if (pKeyBuffer[VK_DOWN] & 0xF0) dwDirection |= DIR_BACKWARD;
			if (pKeyBuffer[VK_LEFT] & 0xF0) dwDirection |= DIR_LEFT;
			if (pKeyBuffer[VK_RIGHT] & 0xF0) dwDirection |= DIR_RIGHT;
			if (pKeyBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
			if (pKeyBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;
		}
		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		/*마우스를 캡쳐했으면 마우스가 얼마만큼 이동하였는 가를 계산한다. 마우스 왼쪽 또는 오른쪽 버튼이 눌러질 때의 메시지(WM_LBUTTONDOWN, WM_RBUTTONDOWN)를 처리할 때 마우스를 캡쳐하였다. 그러므로 마우스가 캡쳐된 것은 마우스 버튼이 눌려진 상태를 의미한다. 마우스를 좌우 또는 상하로 움직이면 플레이어를 x-축 또는 y-축으로 회전한다.*/
		if (GetCapture() == m_hWnd)
		{
			//마우스 커서를 화면에서 없앤다(보이지 않게 한다).
			SetCursor(nullptr);
			//현재 마우스 커서의 위치를 가져온다.
			GetCursorPos(&ptCursorPos);
			//마우스 버튼이 눌린 채로 이전 위치에서 현재 마우스 커서의 위치까지 움직인 양을 구한다.
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}
		//플레이어를 이동하거나(dwDirection) 회전한다(cxDelta 또는 cyDelta).
		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				/*cxDelta는 y-축의 회전을 나타내고 cyDelta는 x-축의 회전을 나타낸다. 오른쪽 마우스 버튼이 눌려진 경우 cxDelta는 z-축의 회전을 나타낸다.*/
				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
			/*플레이어를 dwDirection 방향으로 이동한다(실제로는 속도 벡터를 변경한다). 이동 거리는 시간에 비례하도록 한다. 플레이어의 이동 속력은 (50/초)로 가정한다. 만약 플레이어의 이동 속력이 있다면 그 값을 사용한다.*/
			if (dwDirection) m_pPlayer->Move(dwDirection, 50.0f * m_GameTimer.GetTimeElapsed(), true);
		}
	}
	//플레이어를 실제로 이동하고 카메라를 갱신한다. 중력과 마찰력의 영향을 속도 벡터에 적용한다.
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::AnimateObjects()
{
	float fFrameTime = m_GameTimer.GetTimeElapsed();
	if (m_pScene) m_pScene->AnimateObjects(fFrameTime);
	if (m_pSceneShader) m_pSceneShader->AnimateObjects(fFrameTime);
}

void CGameFramework::FrameAdvance()
{
	//타이머의 시간이 갱신되도록 하고 프레임 레이트를 계산한다. 
	m_GameTimer.Tick();
	//사용자 입력을 처리하기 위한 ProcessInput() 함수를 호출한다.  
	ProcessInput();
	//게임 객체의 애니메이션을 처리하기 위한 AnimateObjects() 함수를 호출한다.  
	AnimateObjects();


	CShader* shaderList[] = { m_pPlayerShader, m_pScene->GetShader(2) };
	OnCreateShadowMap(shaderList, 2);

	float fClearColor[4] = { 0.0f, 0.125f, 0.3f, 0.0f };	//렌더 타겟 뷰를 채우기 위한 색상을 설정한다.  
	/* 렌더 타겟 뷰를 fClearColor[] 색상으로 채운다. 즉, 렌더 타겟 뷰에 연결된 스왑 체인의 첫 번째 후면버퍼를 fClearColor[] 색상으로 지운다. */
	m_pd3dDeviceContext->OMSetRenderTargets(NUM_MRT -1, &m_ppd3dRenderTargetView[1], m_pd3dDepthStencilView);
	m_pd3dDeviceContext->ClearRenderTargetView(m_pd3dBackRenderTargetView, fClearColor);

	for (int i = 0; i < NUM_MRT; ++i)
	{
		if (m_ppd3dRenderTargetView[i]) m_pd3dDeviceContext->ClearRenderTargetView(m_ppd3dRenderTargetView[i], fClearColor);
	}
	if (m_pd3dDepthStencilView) m_pd3dDeviceContext->ClearDepthStencilView(m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

#ifdef _THREAD
	for (int i = 0; i < m_nRenderThreads; ++i)
	{
		::SetEvent(m_pRenderingThreadInfo[i].m_hRenderingBeginEvent);
	}
	::WaitForMultipleObjects(m_nRenderThreads, m_hRenderingEndEvents, TRUE, INFINITE);
#endif

	//m_pScene->UpdateLights(m_pd3dDeviceContext);
	//if (m_pPlayer) m_pPlayer->UpdateShaderVariables(m_pd3dDeviceContext);


#ifdef _THREAD
	//	if (m_pd3dDepthStencilView) m_pd3dDeviceContext->ClearDepthStencilView(m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	for (int i = 0; i < m_nRenderThreads; ++i)
	{
		//UpdateShadowResource();
		m_pd3dDeviceContext->ExecuteCommandList(m_pRenderingThreadInfo[i].m_pd3dCommandList, TRUE);
		m_pRenderingThreadInfo[i].m_pd3dCommandList->Release();
	}
#else
	RENDER_INFO RenderInfo;
	RenderInfo.pCamera = m_pCamera;
	RenderInfo.ppd3dMrtRTV = m_ppd3dRenderTargetView;
	RenderInfo.ThreadID = -1;
	RenderInfo.pRenderState = &m_uRenderState;

	if (m_pPlayer) m_pPlayer->UpdateShaderVariables(m_pd3dDeviceContext);
	m_pScene->UpdateLights(m_pd3dDeviceContext);
	if (m_pScene) m_pScene->Render(m_pd3dDeviceContext, &RenderInfo);
	if (m_pPlayerShader) m_pPlayerShader->Render(m_pd3dDeviceContext, *RenderInfo.pRenderState, RenderInfo.pCamera);
#endif

	//m_pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dSSAOTargetView, nullptr);
	//m_pd3dDeviceContext->PSSetShaderResources(21, 1, &m_pd3dMRTSRV[MRT_NORMAL]);	
	//m_pSSAOShader->Render(m_pd3dDeviceContext, NULL, m_pCamera);

	//ShadowMgr.UpdateStaticShadowResource(m_pd3dDeviceContext);
	ShadowMgr.UpdateDynamicShadowResource(m_pd3dDeviceContext);

	m_pScene->UpdateLights(m_pd3dDeviceContext);
	m_pd3dDeviceContext->OMSetRenderTargets(1, &m_ppd3dRenderTargetView[MRT_SCENE], nullptr);
	m_pSceneShader->Render(m_pd3dDeviceContext, 0, m_pCamera);

	//if (m_iDrawOption >= 0)
	//{
	//	printf("opt : %d \n", m_iDrawOption);
	//	m_pSceneShader->SetTexture(0, m_pd3dMRTSRV[m_iDrawOption]);
	//}
	//m_pCamera->SetViewport(m_pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	


	//스왑 체인의 후면버퍼의 내용이 디스플레이에 출력되도록 프리젠트한다.  
	m_pDXGISwapChain->Present(0, 0);

	m_GameTimer.GetFrameRate(m_pszBuffer + 12, 37);
	::SetWindowText(m_hWnd, m_pszBuffer);
}


UINT WINAPI CGameFramework::RenderThread(LPVOID lpParameter)
{
	RenderingThreadInfo * pRenderingThreadInfo = (RenderingThreadInfo*)lpParameter;
	CScene * pScene = pRenderingThreadInfo->m_pScene;
	ID3D11DeviceContext * pd3dDeferredContext = pRenderingThreadInfo->m_pd3dDeferredContext;

	RENDER_INFO RenderInfo;
	RenderInfo.pCamera = pRenderingThreadInfo->m_pPlayer->GetCamera();
	RenderInfo.ppd3dMrtRTV = pRenderingThreadInfo->m_ppd3dRenderTargetView;
	RenderInfo.ThreadID = pRenderingThreadInfo->m_nRenderingThreadID;
	RenderInfo.pRenderState = pRenderingThreadInfo->m_puRenderState;

	if (NUM_THREAD == 1) RenderInfo.ThreadID = -1;

	pd3dDeferredContext->OMSetDepthStencilState(nullptr, 1);
	while (true)
	{
		::WaitForSingleObject(pRenderingThreadInfo->m_hRenderingBeginEvent, INFINITE);
		if (RenderInfo.ThreadID == 0)
		{
			//pFramework->UpdateShadowResource();
			pd3dDeferredContext->ClearDepthStencilView(pRenderingThreadInfo->m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		}
		//pd3dDeferredContext->OMSetRenderTargets(NUM_MRT-1, &pRenderingThreadInfo->m_ppd3dRenderTargetView[1], pRenderingThreadInfo->m_pd3dDepthStencilView);
		//pd3dDeferredContext->ClearDepthStencilView(pRenderingThreadInfo->m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0.0f);
		//pd3dDeferredContext->OMSetDepthStencilState(nullptr, 1);

		pRenderingThreadInfo->m_pPlayer->UpdateShaderVariables(pd3dDeferredContext);

		pScene->Render(pd3dDeferredContext, &RenderInfo);

		pd3dDeferredContext->FinishCommandList(TRUE, &pRenderingThreadInfo->m_pd3dCommandList);
		::SetEvent(pRenderingThreadInfo->m_hRenderingEndEvent);
	}
}
