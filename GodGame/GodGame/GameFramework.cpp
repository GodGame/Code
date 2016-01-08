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
		m_ppd3dMRTView[i] = nullptr;
		m_ppd3dMRTtx[i] = nullptr;
	}
	m_pd3dDepthStencilBuffer = nullptr;
	m_pd3dDepthStencilView = nullptr;
	m_pd3dDeviceContext = nullptr;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_nRenderThreads = 0;

	m_pRenderingThreadInfo = nullptr;

	m_pScene = nullptr;
	m_pPlayer = nullptr;
	m_pCamera = nullptr;
	m_iDrawOption = MRT_SCENE;
	m_pSceneShader = nullptr;
	_tcscpy_s(m_pszBuffer, _T("ChaeProject("));

	m_pd3dStaticDSVShadowMap = m_pd3dDSVShadowMap = nullptr;
	m_pd3dStaticSRVShadowMap = m_pd3dSRVShadowMap = nullptr;

	ZeroMemory(&m_d3dxShadowMapViewport, sizeof(D3D11_VIEWPORT));
	m_pd3dcbShadowMap = nullptr;
	m_pd3dShadowSamplerState = nullptr;
	m_pd3dShadowRS = nullptr;
	m_pd3dNormalRS = nullptr;
	m_pd3dSSAOTargetView = nullptr;

	m_pSSAOShader = nullptr;
	m_uRenderState = 0;

	Chae::XMFloat4x4Identity(&m_xmf44ShadowMap);
	//Chae::XMFloat4x4Identity(&m_xmf44ShadowView);
	//Chae::XMFloat4x4Identity(&m_xmf44ShadowProject);
}


CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{

	m_hInstance = hInstance;
	m_hWnd = hMainWnd;


	//Direct3D ����̽�, ����̽� ���ؽ�Ʈ, ���� ü�� ���� �����ϴ� �Լ��� ȣ���Ѵ�. 
	if (!CreateDirect3DDisplay()) return(false);

	CManagers::BuildManagers(m_pd3dDevice);

	if (!CreateRenderTargetDepthStencilView()) return(false);
	if (!CreateShadowDevice()) return false;
	//�������� ��ü(���� ���� ��ü)�� �����Ѵ�. 
	BuildObjects();

	InitilizeThreads();
	return(true);
}

void CGameFramework::OnDestroy()
{
	//���� ��ü�� �Ҹ��Ѵ�. 
	ReleaseObjects();

	//Direct3D�� ���õ� ��ü�� �Ҹ��Ѵ�. 
	if (m_pd3dDeviceContext) m_pd3dDeviceContext->ClearState();
	for (int i = 0; i < NUM_MRT; ++i)
	{
		if (m_ppd3dRenderTargetView[i]) m_ppd3dRenderTargetView[i]->Release();
		if (m_ppd3dMRTView[i]) m_ppd3dMRTView[i]->Release();
		if (m_ppd3dMRTtx[i]) m_ppd3dMRTtx[i]->Release();
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

	if (m_pd3dSSAOTargetView) m_pd3dSSAOTargetView->Release();

	if (m_pRenderingThreadInfo) delete[] m_pRenderingThreadInfo;
	if (m_hRenderingEndEvents) delete[] m_hRenderingEndEvents;

	if (m_pd3dDSVShadowMap) m_pd3dDSVShadowMap->Release();
	if (m_pd3dSRVShadowMap) m_pd3dSRVShadowMap->Release();
	if (m_pd3dStaticDSVShadowMap) m_pd3dStaticDSVShadowMap->Release();
	if (m_pd3dStaticSRVShadowMap) m_pd3dStaticSRVShadowMap->Release();

	if (m_pd3dcbShadowMap) m_pd3dcbShadowMap->Release();

	if (m_pd3dShadowSamplerState) m_pd3dShadowSamplerState->Release();

}

bool CGameFramework::CreateRenderTargetDepthStencilView()
{
	HRESULT hResult = S_OK;
	//���� ü���� ù ��° �ĸ���� �������̽��� �����´�. 
	ID3D11Texture2D *pd3dBackBuffer;
	//���� ü���� ù ��° �ĸ���ۿ� ���� ���� Ÿ�� �並 �����Ѵ�.
	assert(SUCCEEDED(hResult = m_pDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pd3dBackBuffer)));
	
	D3D11_RENDER_TARGET_VIEW_DESC d3dRTVDesc;
	d3dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	d3dRTVDesc.Texture2D.MipSlice = 0;
	d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	assert(SUCCEEDED(hResult = m_pd3dDevice->CreateRenderTargetView(pd3dBackBuffer, &d3dRTVDesc, &m_ppd3dRenderTargetView[MRT_SCENE])));
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
	d3d2DBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	d3d2DBufferDesc.CPUAccessFlags = 0;
	d3d2DBufferDesc.MiscFlags = 0;
	assert(SUCCEEDED(hResult = m_pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &m_pd3dDepthStencilBuffer/*m_ppd3dMRTtx[MRT_DEPTH]*/)));

	//������ ���� ����(Depth Buffer)�� ���� �並 �����Ѵ�.
	D3D11_DEPTH_STENCIL_VIEW_DESC d3dViewDesc;
	ZeroMemory(&d3dViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	d3dViewDesc.Format = DXGI_FORMAT_D32_FLOAT;//d3d2DBufferDesc.Format;
	d3dViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	d3dViewDesc.Texture2D.MipSlice = 0;
	assert(SUCCEEDED((hResult = m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dViewDesc, &m_pd3dDepthStencilView))));

	D3D11_SHADER_RESOURCE_VIEW_DESC d3dSRVDesc;
	ZeroMemory(&d3dSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	d3dSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	d3dSRVDesc.Texture2D.MipLevels = 1;
	d3dSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	//if (FAILED(hResult = m_pd3dDevice->CreateShaderResourceView(m_ppd3dMRTtx[MRT_DEPTH], &d3dSRVDesc, &m_ppd3dMRTView[MRT_DEPTH]))) 
	//	return(false);

	d3d2DBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
//	d3d2DBufferDesc.Width = FRAME_BUFFER_WIDTH * 0.5f;
//	d3d2DBufferDesc.Height = FRAME_BUFFER_HEIGHT * 0.5f;
	for (int i = 1; i < NUM_MRT; ++i)
	{
		//d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		switch (i)
		{
		case MRT_TXCOLOR:
			d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case MRT_POS:
			d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;// DXGI_FORMAT_R32G32B32A32_SINT;
			break;
		case MRT_DIFFUSE:
			d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_B5G5R5A1_UNORM;// DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case MRT_SPEC:
			d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case MRT_NORMAL:
			d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_SNORM;
			break;
		default:
			d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		assert(SUCCEEDED(hResult = m_pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &m_ppd3dMRTtx[i])));
		assert(SUCCEEDED(hResult = m_pd3dDevice->CreateShaderResourceView(m_ppd3dMRTtx[i], &d3dSRVDesc, &m_ppd3dMRTView[i])));
		assert(SUCCEEDED(hResult = m_pd3dDevice->CreateRenderTargetView(m_ppd3dMRTtx[i], &d3dRTVDesc, &m_ppd3dRenderTargetView[i])));

		m_ppd3dMRTtx[i]->Release();
		m_ppd3dMRTtx[i] = nullptr;
	}
	d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	

	ID3D11Texture2D * pdTx2D;
	assert(SUCCEEDED(hResult = m_pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &pdTx2D)));
	assert(SUCCEEDED(hResult = m_pd3dDevice->CreateShaderResourceView(pdTx2D, &d3dSRVDesc, &m_pd3dSSAOSRV)));
	assert(SUCCEEDED(hResult = m_pd3dDevice->CreateRenderTargetView(pdTx2D, &d3dRTVDesc, &m_pd3dSSAOTargetView)));

	TXMgr.InsertShaderResourceView(m_pd3dSSAOSRV, "srv_rtvSSAO", 0);
	m_pd3dSSAOSRV->Release();
	pdTx2D->Release();
	

	//m_pd3dDeviceContext->OMSetRenderTargets(5, &m_pd3dRenderTargetView, m_pd3dDepthStencilView);

	return(true);
}


bool CGameFramework::CreateShadowDevice()
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = 1024;//FRAME_BUFFER_WIDTH;
	desc.Height = 1024;//FRAME_BUFFER_HEIGHT;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = desc.MiscFlags = 0;

	HRESULT hr;
	ID3D11Texture2D * pd3dShadowMap = nullptr;
	assert(SUCCEEDED(hr = m_pd3dDevice->CreateTexture2D(&desc, nullptr, &pd3dShadowMap)));

	D3D11_DEPTH_STENCIL_VIEW_DESC d3dDSVDesc;
	d3dDSVDesc.Flags = d3dDSVDesc.Texture2D.MipSlice = 0;
	d3dDSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; 
	assert(SUCCEEDED(hr = m_pd3dDevice->CreateDepthStencilView(pd3dShadowMap, &d3dDSVDesc, &m_pd3dDSVShadowMap)));
	

	D3D11_SHADER_RESOURCE_VIEW_DESC d3dSRVDesc;
	d3dSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	d3dSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	d3dSRVDesc.Texture2D.MipLevels = desc.MipLevels;
	d3dSRVDesc.Texture2D.MostDetailedMip = 0;
	assert(SUCCEEDED(hr = m_pd3dDevice->CreateShaderResourceView(pd3dShadowMap, &d3dSRVDesc, &m_pd3dSRVShadowMap)));
	pd3dShadowMap->Release();

	assert(SUCCEEDED(hr = m_pd3dDevice->CreateTexture2D(&desc, nullptr, &pd3dShadowMap)));
	assert(SUCCEEDED(hr = m_pd3dDevice->CreateDepthStencilView(pd3dShadowMap, &d3dDSVDesc, &m_pd3dStaticDSVShadowMap)));
	assert(SUCCEEDED(hr = m_pd3dDevice->CreateShaderResourceView(pd3dShadowMap, &d3dSRVDesc, &m_pd3dStaticSRVShadowMap)));
	pd3dShadowMap->Release();
	//pd3dShadowMap->Release();

	m_d3dxShadowMapViewport.TopLeftX = 0.0f;
	m_d3dxShadowMapViewport.TopLeftY = 0.0f;
	m_d3dxShadowMapViewport.Width = 1024;//FRAME_BUFFER_WIDTH;
	m_d3dxShadowMapViewport.Height = 1024;//FRAME_BUFFER_HEIGHT;
	m_d3dxShadowMapViewport.MinDepth = 0.0f;
	m_d3dxShadowMapViewport.MaxDepth = 1.0f;

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT4X4);
	assert(SUCCEEDED(hr = m_pd3dDevice->CreateBuffer(&d3dBufferDesc, nullptr, &m_pd3dcbShadowMap)));

#define PCF
#ifdef PCF
	m_pd3dShadowSamplerState = TXMgr.GetSamplerState("scs_point_border");
#else
	m_pd3dShadowSamplerState = TXMgr.GetSamplerState("ss_point_border");
#endif
	if (nullptr == m_pd3dShadowSamplerState) assert(E_FAIL);

	m_pd3dShadowSamplerState->AddRef();

	D3D11_RASTERIZER_DESC d3dRSDesc;
	ZeroMemory(&d3dRSDesc, sizeof(D3D11_RASTERIZER_DESC));
	d3dRSDesc.FillMode = D3D11_FILL_SOLID;
	d3dRSDesc.CullMode = D3D11_CULL_NONE;
	d3dRSDesc.DepthClipEnable = true;
	assert(SUCCEEDED(hr = m_pd3dDevice->CreateRasterizerState(&d3dRSDesc, &m_pd3dNormalRS)));

	d3dRSDesc.CullMode = D3D11_CULL_NONE;
	d3dRSDesc.FrontCounterClockwise = false;
	d3dRSDesc.DepthBias = 5000;
	d3dRSDesc.DepthBiasClamp = 0.0f;
	d3dRSDesc.SlopeScaledDepthBias = 2.0f;
	assert(SUCCEEDED(hr = m_pd3dDevice->CreateRasterizerState(&d3dRSDesc, &m_pd3dShadowRS)));

	return true;
}


void CGameFramework::BuildStaticShadowMap(ID3D11DeviceContext * pd3dDeviceContext)
{
	// ���⼺ ������ ��ġ �ʿ� ����.
	//LIGHT * pLight = m_pScene->GetLight(2);
	CHeightMapTerrain * pTerrain = m_pScene->GetTerrain();
	float fHalf = pTerrain->GetWidth() * 0.5;

	XMVECTOR xmvUp = XMVectorSet(1, 0, 0, 0);
	XMVECTOR xmvTarget = XMVectorSet(fHalf - 1000, 0, fHalf, 0);// XMLoadFloat3(&m_pPlayer->GetPosition());
	XMVECTOR xmvLightPos = XMVectorSet(fHalf, fHalf, fHalf, 0);

	XMMATRIX xmtxShadowView = XMMatrixLookAtLH(xmvLightPos, xmvTarget, xmvUp);
	xmvLightPos = XMVector3TransformCoord(xmvLightPos, xmtxShadowView);

	XMFLOAT3 xmf3LightPos;
	XMStoreFloat3(&xmf3LightPos, xmvLightPos);


	float xl = xmf3LightPos.x - fHalf, xr = xmf3LightPos.x + fHalf;
	float yb = xmf3LightPos.y - fHalf, yt = xmf3LightPos.y + fHalf;
	float zn = xmf3LightPos.z - fHalf, zf = xmf3LightPos.z + fHalf;

	XMMATRIX xmtxShadowProj = XMMatrixOrthographicOffCenterLH(xl, xr, yb, yt, zn, zf);
	static const XMMATRIX xmtxCoord
		(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f		
			);
	XMMATRIX xmtxShadowMap = xmtxShadowView * xmtxShadowProj * xmtxCoord;
	XMStoreFloat4x4(&m_xmf44ShadowMap, xmtxShadowMap);

	XMStoreFloat4x4(&m_xmf44ShadowVP, (xmtxShadowView * xmtxShadowProj));


	m_uRenderState = NOT_PSUPDATE;
	m_pCamera->UpdateShaderVariables(m_pd3dDeviceContext, m_xmf44ShadowVP/*xmf44LightViewProj*/);
	m_pd3dDeviceContext->PSSetShader(nullptr, nullptr, 0);
	m_pd3dDeviceContext->RSSetState(m_pd3dShadowRS);
	m_pd3dDeviceContext->RSSetViewports(1, &m_d3dxShadowMapViewport);

	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	m_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, m_pd3dStaticDSVShadowMap);
	m_pd3dDeviceContext->ClearDepthStencilView(m_pd3dStaticDSVShadowMap, D3D11_CLEAR_DEPTH, 1.0f, 0);

	(*m_pScene->GetShader(1))->Render(m_pd3dDeviceContext, m_uRenderState, m_pCamera);

	m_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_pd3dDeviceContext->RSSetState(nullptr);
	m_uRenderState = NULL;

	m_pSceneShader->SetLightSRV(m_pd3dStaticSRVShadowMap);
	//m_pCamera->SetViewport(m_pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);

	UpdateStaticShadowResource();
}

void CGameFramework::OnCreateShadowMap(ID3D11DeviceContext * pd3dDeviceContext, CShader*ShaderList[], int nShaders)
{

	m_uRenderState = NOT_PSUPDATE;
	m_pCamera->UpdateShaderVariables(m_pd3dDeviceContext, m_xmf44ShadowVP/*xmf44LightViewProj*/);
	m_pd3dDeviceContext->PSSetShader(nullptr, nullptr, 0);
	m_pd3dDeviceContext->RSSetState(m_pd3dShadowRS);
	m_pd3dDeviceContext->RSSetViewports(1, &m_d3dxShadowMapViewport);

	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	m_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, m_pd3dDSVShadowMap);
	m_pd3dDeviceContext->ClearDepthStencilView(m_pd3dDSVShadowMap, D3D11_CLEAR_DEPTH, 1.0f, 0);

	for (int i = 0; i < nShaders; ++i)
		ShaderList[i]->Render(m_pd3dDeviceContext, m_uRenderState, m_pCamera);

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

	m_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_pd3dDeviceContext->RSSetState(nullptr);
	m_uRenderState = NULL;

	m_pSceneShader->SetLightSRV(m_pd3dStaticSRVShadowMap);
	m_pCamera->SetViewport(m_pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
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

	//����̽��� �����ϱ� ���Ͽ� �õ��� ����̹� ������ ������ ��Ÿ����.
	D3D_DRIVER_TYPE d3dDriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT nDriverTypes = sizeof(d3dDriverTypes) / sizeof(D3D10_DRIVER_TYPE);

	//����̽��� �����ϱ� ���Ͽ� �õ��� Ư�� ������ ������ ��Ÿ����.
	D3D_FEATURE_LEVEL d3dFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	UINT nFeatureLevels = sizeof(d3dFeatureLevels) / sizeof(D3D_FEATURE_LEVEL);

	//������ ���� ü���� �����ϴ� ����ü�̴�.
	/*���� ü���� �����ϱ� ���� ���� ü�� ����ü DXGI_SWAP_CHAIN_DESC�� �����Ѵ�. ���� ü���� ���� ũ��� �� �������� Ŭ���̾�Ʈ ������ ũ��� �����ϰ� ��� ������� �� ������� �����Ѵ�. ���� ü���� ��� ���� ������ ���� �����Ѵ�.*/
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

	/*���ϴ� ����̽� ����̹� ���� ������ ���� ü�ΰ� ����̽�, ����̽� ���ؽ�Ʈ�� �����ϱ� ���� ����̹� ���� �迭(d3dDriverTypes[])�� �� ���ҿ� ���Ͽ� D3D11CreateDeviceAndSwapChain() �Լ� ȣ���� ���� ����̽��� ����ü�� ������ �õ��Ѵ�.*/
	D3D_DRIVER_TYPE nd3dDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL nd3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	HRESULT hResult = S_OK;
	//����̽��� ����̹� ������ Ư�� ������ �����ϴ� ����̽��� ���� ü���� �����Ѵ�. ������� ����̽� ������ �õ��ϰ� �����ϸ� ���� ������ ����̽��� �����Ѵ�.
	for (UINT i = 0; i < nDriverTypes; i++)
	{
		nd3dDriverType = d3dDriverTypes[i];
		if (SUCCEEDED(hResult = D3D11CreateDeviceAndSwapChain(nullptr, nd3dDriverType, nullptr, dwCreateDeviceFlags, d3dFeatureLevels, nFeatureLevels, D3D11_SDK_VERSION, &dxgiSwapChainDesc, &m_pDXGISwapChain, &m_pd3dDevice, &nd3dFeatureLevel, &m_pd3dDeviceContext))) break;
	}
	if (!m_pDXGISwapChain || !m_pd3dDevice || !m_pd3dDeviceContext) return(false);
	//����̽��� �����Ǹ� ���� Ÿ�� �並 �����ϱ� ���� CreateRenderTargetView() �Լ��� ȣ���Ѵ�. 
	//���� Ÿ�� �並 �����ϴ� �Լ��� ȣ���Ѵ�.

	return(true);
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{

	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//���콺 ĸ�ĸ� �ϰ� ���� ���콺 ��ġ�� �����´�.
		SetCapture(hWnd);
		GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		//���콺 ĸ�ĸ� �����Ѵ�.
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
			m_pSceneShader->SetTexture(0, m_pd3dSRVShadowMap);
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
		case 'Z':
			m_pSceneShader->SetDrawOption((m_iDrawOption = -1));
			m_pSceneShader->SetTexture(0, m_pd3dSSAOSRV/*TXMgr.GetShaderResourceView("srv_rtvSSAO")*/);
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
			/*��F1�� Ű�� ������ 1��Ī ī�޶�, ��F2�� Ű�� ������ �����̽�-�� ī�޶�� �����Ѵ�, ��F3�� Ű�� ������ 3��Ī ī�޶�� �����Ѵ�.*/
		case VK_F1:
		case VK_F2:
		case VK_F3:
			if (m_pPlayer)
			{
				m_pPlayer->ChangeCamera(m_pd3dDevice, (wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
				m_pCamera = m_pPlayer->GetCamera();
				//���� ���� ī�޶� �����Ѵ�.
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
		/*�������� ũ�Ⱑ ����� ��(����� ��Alt+Enter�� ��ü ȭ�� ���� ������ ���� ��ȯ�� ��) ���� ü���� �ĸ���� ũ�⸦ �����ϰ� �ĸ���ۿ� ���� ���� Ÿ�� �並 �ٽ� �����Ѵ�. */
	//case WM_SIZE:
	//{
	//				m_nWndClientWidth = LOWORD(lParam);
	//				m_nWndClientHeight = HIWORD(lParam);

	//				m_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	//				//���� ��ü�� �Ҹ��Ѵ�. 
	//				ReleaseObjects();

	//				//Direct3D�� ���õ� ��ü�� �Ҹ��Ѵ�. 
	//				if (m_pd3dDeviceContext) m_pd3dDeviceContext->ClearState();
	//				for (int i = 0; i < NUM_MRT; ++i)
	//				{
	//					if (m_ppd3dRenderTargetView[i]) m_ppd3dRenderTargetView[i]->Release();
	//					if (m_ppd3dMRTView[i]) m_ppd3dMRTView[i]->Release();
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
	m_pSceneShader->BuildObjects(m_pd3dDevice, m_ppd3dMRTView, m_iDrawOption);

	m_pScene = new CScene();
	//m_pScene->SetRenderTarget(m_pd3dRenderTargetView);
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

	m_pSSAOShader = new CSSAOShader();
	m_pSSAOShader->CreateShaderVariable(m_pd3dDevice);
	m_pSSAOShader->CreateShader(m_pd3dDevice);
	m_pSSAOShader->BuildObjects(m_pd3dDevice);

	BuildStaticShadowMap(m_pd3dDeviceContext);
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
		// ���۵� ���ؽ�Ʈ ����
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
	//CShader Ŭ������ ����(static) ��� ������ ����� ��� ���۸� ��ȯ�Ѵ�.
	CShader::ReleaseShaderVariables();
	CIlluminatedShader::ReleaseShaderVariables();

	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;


	if (m_pSceneShader) m_pSceneShader->ReleaseObjects();
	if (m_pSceneShader) delete m_pSceneShader;
	m_pSceneShader = nullptr;

	if (m_pSSAOShader) m_pSSAOShader->ReleaseObjects();
	if (m_pSSAOShader) delete m_pSSAOShader;
	m_pSSAOShader = nullptr;
}

void CGameFramework::ProcessInput()
{
	bool bProcessedByScene = false;
	if (m_pScene) bProcessedByScene = m_pScene->ProcessInput();
	if (!bProcessedByScene)
	{
		static UCHAR pKeyBuffer[256];
		DWORD dwDirection = 0;
		/*Ű������ ���� ������ ��ȯ�Ѵ�. ȭ��ǥ Ű(���桯, ���硯, ���衯, ���顯)�� ������ �÷��̾ ������/����(���� x-��), ��/��(���� z-��)�� �̵��Ѵ�. ��Page Up���� ��Page Down�� Ű�� ������ �÷��̾ ��/�Ʒ�(���� y-��)�� �̵��Ѵ�.*/
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
		/*���콺�� ĸ�������� ���콺�� �󸶸�ŭ �̵��Ͽ��� ���� ����Ѵ�. ���콺 ���� �Ǵ� ������ ��ư�� ������ ���� �޽���(WM_LBUTTONDOWN, WM_RBUTTONDOWN)�� ó���� �� ���콺�� ĸ���Ͽ���. �׷��Ƿ� ���콺�� ĸ�ĵ� ���� ���콺 ��ư�� ������ ���¸� �ǹ��Ѵ�. ���콺�� �¿� �Ǵ� ���Ϸ� �����̸� �÷��̾ x-�� �Ǵ� y-������ ȸ���Ѵ�.*/
		if (GetCapture() == m_hWnd)
		{
			//���콺 Ŀ���� ȭ�鿡�� ���ش�(������ �ʰ� �Ѵ�).
			SetCursor(nullptr);
			//���� ���콺 Ŀ���� ��ġ�� �����´�.
			GetCursorPos(&ptCursorPos);
			//���콺 ��ư�� ���� ä�� ���� ��ġ���� ���� ���콺 Ŀ���� ��ġ���� ������ ���� ���Ѵ�.
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}
		//�÷��̾ �̵��ϰų�(dwDirection) ȸ���Ѵ�(cxDelta �Ǵ� cyDelta).
		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				/*cxDelta�� y-���� ȸ���� ��Ÿ���� cyDelta�� x-���� ȸ���� ��Ÿ����. ������ ���콺 ��ư�� ������ ��� cxDelta�� z-���� ȸ���� ��Ÿ����.*/
				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
			/*�÷��̾ dwDirection �������� �̵��Ѵ�(�����δ� �ӵ� ���͸� �����Ѵ�). �̵� �Ÿ��� �ð��� ����ϵ��� �Ѵ�. �÷��̾��� �̵� �ӷ��� (50/��)�� �����Ѵ�. ���� �÷��̾��� �̵� �ӷ��� �ִٸ� �� ���� ����Ѵ�.*/
			if (dwDirection) m_pPlayer->Move(dwDirection, 50.0f * m_GameTimer.GetTimeElapsed(), true);
		}
	}
	//�÷��̾ ������ �̵��ϰ� ī�޶� �����Ѵ�. �߷°� �������� ������ �ӵ� ���Ϳ� �����Ѵ�.
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::AnimateObjects()
{
	if (m_pScene) m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::FrameAdvance()
{
	//Ÿ�̸��� �ð��� ���ŵǵ��� �ϰ� ������ ����Ʈ�� ����Ѵ�. 
	m_GameTimer.Tick();
	//����� �Է��� ó���ϱ� ���� ProcessInput() �Լ��� ȣ���Ѵ�.  
	ProcessInput();
	//���� ��ü�� �ִϸ��̼��� ó���ϱ� ���� AnimateObjects() �Լ��� ȣ���Ѵ�.  
	AnimateObjects();

	//BuildStaticShadowMap(m_pd3dDeviceContext);
	CShader* shaderList[] = { m_pPlayerShader, *m_pScene->GetShader(2) };
	OnCreateShadowMap(m_pd3dDeviceContext, shaderList, 2);
	//UpdateShadowResource();

	float fClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };	//���� Ÿ�� �並 ä��� ���� ������ �����Ѵ�.  
	/* ���� Ÿ�� �並 fClearColor[] �������� ä���. ��, ���� Ÿ�� �信 ����� ���� ü���� ù ��° �ĸ���۸� fClearColor[] �������� �����. */
	m_pd3dDeviceContext->OMSetRenderTargets(NUM_MRT -1, &m_ppd3dRenderTargetView[1], m_pd3dDepthStencilView);
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
	//if (m_pPlayerShader) m_pPlayerShader->Render(m_pd3dDeviceContext, m_pCamera);
	//ID3D11ShaderResourceView * pNullPtr[] = { nullptr, nullptr, nullptr,nullptr, nullptr, nullptr };
	//m_pd3dDeviceContext->PSSetShaderResources(16, 6, pNullPtr);

	//m_pSceneShader->UpdateShaders(m_pd3dDeviceContext);

//	m_pd3dDeviceContext->ClearRenderTargetView(m_pd3dSSAOTargetView, color);
	m_pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dSSAOTargetView, nullptr);
	m_pd3dDeviceContext->PSSetShaderResources(21, 1, &m_ppd3dMRTView[MRT_NORMAL]);	
	m_pSSAOShader->Render(m_pd3dDeviceContext, NULL, m_pCamera);

	m_pScene->UpdateLights(m_pd3dDeviceContext);
	m_pd3dDeviceContext->OMSetRenderTargets(1, &m_ppd3dRenderTargetView[0], nullptr);
	UpdateShadowResource();

	//if (m_iDrawOption >= 0)
	//{
	//	printf("opt : %d \n", m_iDrawOption);
	//	m_pSceneShader->SetTexture(0, m_ppd3dMRTView[m_iDrawOption]);
	//}
	//m_pCamera->SetViewport(m_pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pSceneShader->Render(m_pd3dDeviceContext, NULL);


	//���� ü���� �ĸ������ ������ ���÷��̿� ��µǵ��� ������Ʈ�Ѵ�.  
	m_pDXGISwapChain->Present(0, 0);

	m_GameTimer.GetFrameRate(m_pszBuffer + 12, 37);
	::SetWindowText(m_hWnd, m_pszBuffer);
}

void CGameFramework::UpdateShadowResource()
{
	m_pd3dDeviceContext->PSSetShaderResources(TX_SLOT_SHADOWMAP, 1, &m_pd3dSRVShadowMap);
}

void CGameFramework::UpdateStaticShadowResource()
{
#ifdef PCF
	m_pd3dDeviceContext->PSSetSamplers(SS_SLOT_SHADOWSAMPLE + 1, 1, &m_pd3dShadowSamplerState);
#else
	m_pd3dDeviceContext->PSSetSamplers(SS_SLOT_SHADOWSAMPLE, 1, &m_pd3dShadowSamplerState);
#endif
	m_pd3dDeviceContext->PSSetShaderResources(TX_SLOT_STATIC_SHADOW, 1, &m_pd3dStaticSRVShadowMap);

	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	m_pd3dDeviceContext->Map(m_pd3dcbShadowMap, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	XMFLOAT4X4 * pd3dxmtxShadowTransform = (XMFLOAT4X4*)d3dMappedResource.pData;
	Chae::XMFloat4x4Transpose(pd3dxmtxShadowTransform, &m_xmf44ShadowMap);	//XMFLOAT4X4Transpose(&pcbViewProjection->m_xmf44View, &m_xmf44View);
	m_pd3dDeviceContext->Unmap(m_pd3dcbShadowMap, 0);

	m_pd3dDeviceContext->PSSetConstantBuffers(CB_SLOT_SHADOWMAP, 1, &m_pd3dcbShadowMap);
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
