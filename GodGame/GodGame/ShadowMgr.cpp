#include "stdafx.h"
#include "ResourceMgr.h"
#include "ShadowMgr.h"
#include "Camera.h"
#include "MyInline.h"


CShadowMgr::CShadowMgr()
{
	m_iStaticMapSlot = TX_SLOT_STATIC_SHADOW;
	m_iMapSlot = TX_SLOT_SHADOWMAP;

	m_pd3dStaticDSVShadowMap = m_pd3dDSVShadowMap = nullptr;
	m_pd3dStaticSRVShadowMap = m_pd3dSRVShadowMap = nullptr;

	ZeroMemory(&m_d3dxShadowMapViewport, sizeof(D3D11_VIEWPORT));
	m_pd3dcbShadowMap = nullptr;
	m_pd3dShadowSamplerState = nullptr;
	m_pd3dShadowRS = nullptr;
	m_pd3dNormalRS = nullptr;

	Chae::XMFloat4x4Identity(&m_xmf44ShadowMap);
}

CShadowMgr::~CShadowMgr()
{
	if (m_pd3dDSVShadowMap) m_pd3dDSVShadowMap->Release();
	if (m_pd3dSRVShadowMap) m_pd3dSRVShadowMap->Release();
	if (m_pd3dStaticDSVShadowMap) m_pd3dStaticDSVShadowMap->Release();
	if (m_pd3dStaticSRVShadowMap) m_pd3dStaticSRVShadowMap->Release();

	if (m_pd3dcbShadowMap) m_pd3dcbShadowMap->Release();

	if (m_pd3dShadowSamplerState) m_pd3dShadowSamplerState->Release();
}

CShadowMgr & CShadowMgr::GetInstance()
{
	static CShadowMgr instance;
	return instance;
}

void CShadowMgr::CreateShadowDevice(ID3D11Device * pd3dDevice)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = SIZE_SHADOW_WIDTH;//FRAME_BUFFER_WIDTH;
	desc.Height = SIZE_SHADOW_HEIGHT;//FRAME_BUFFER_HEIGHT;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = desc.MiscFlags = 0;

	HRESULT hr;
	ID3D11Texture2D * pd3dShadowMap = nullptr;
	ASSERT(SUCCEEDED(hr = pd3dDevice->CreateTexture2D(&desc, nullptr, &pd3dShadowMap)));

	D3D11_DEPTH_STENCIL_VIEW_DESC d3dDSVDesc;
	d3dDSVDesc.Flags = d3dDSVDesc.Texture2D.MipSlice = 0;
	d3dDSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	ASSERT(SUCCEEDED(hr = pd3dDevice->CreateDepthStencilView(pd3dShadowMap, &d3dDSVDesc, &m_pd3dDSVShadowMap)));


	D3D11_SHADER_RESOURCE_VIEW_DESC d3dSRVDesc;
	d3dSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	d3dSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	d3dSRVDesc.Texture2D.MipLevels = desc.MipLevels;
	d3dSRVDesc.Texture2D.MostDetailedMip = 0;
	ASSERT(SUCCEEDED(hr = pd3dDevice->CreateShaderResourceView(pd3dShadowMap, &d3dSRVDesc, &m_pd3dSRVShadowMap)));
	pd3dShadowMap->Release();

	ASSERT(SUCCEEDED(hr = pd3dDevice->CreateTexture2D(&desc, nullptr, &pd3dShadowMap)));
	ASSERT(SUCCEEDED(hr = pd3dDevice->CreateDepthStencilView(pd3dShadowMap, &d3dDSVDesc, &m_pd3dStaticDSVShadowMap)));
	ASSERT(SUCCEEDED(hr = pd3dDevice->CreateShaderResourceView(pd3dShadowMap, &d3dSRVDesc, &m_pd3dStaticSRVShadowMap)));
	pd3dShadowMap->Release();
	
	TXMgr.InsertShaderResourceView(m_pd3dSRVShadowMap, "srv_ShaodwMap", 0, SET_SHADER_PS);
	TXMgr.InsertShaderResourceView(m_pd3dStaticSRVShadowMap, "srv_StaticShaodwMap", 0, SET_SHADER_PS);

	m_d3dxShadowMapViewport.TopLeftX = 0.0f;
	m_d3dxShadowMapViewport.TopLeftY = 0.0f;
	m_d3dxShadowMapViewport.Width = SIZE_SHADOW_WIDTH;//FRAME_BUFFER_WIDTH;
	m_d3dxShadowMapViewport.Height = SIZE_SHADOW_HEIGHT;//FRAME_BUFFER_HEIGHT;
	m_d3dxShadowMapViewport.MinDepth = 0.0f;
	m_d3dxShadowMapViewport.MaxDepth = 1.0f;

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT4X4);
	ASSERT(SUCCEEDED(hr = pd3dDevice->CreateBuffer(&d3dBufferDesc, nullptr, &m_pd3dcbShadowMap)));

#define PCF
#ifdef PCF
	m_pd3dShadowSamplerState = TXMgr.GetSamplerState("scs_point_border");
#else
	m_pd3dShadowSamplerState = TXMgr.GetSamplerState("ss_point_border");
#endif
	if (nullptr == m_pd3dShadowSamplerState) ASSERT(E_FAIL);

	m_pd3dShadowSamplerState->AddRef();

	D3D11_RASTERIZER_DESC d3dRSDesc;
	ZeroMemory(&d3dRSDesc, sizeof(D3D11_RASTERIZER_DESC));
	d3dRSDesc.FillMode = D3D11_FILL_SOLID;
	d3dRSDesc.CullMode = D3D11_CULL_NONE;
	d3dRSDesc.DepthClipEnable = true;
	ASSERT(SUCCEEDED(hr = pd3dDevice->CreateRasterizerState(&d3dRSDesc, &m_pd3dNormalRS)));

	d3dRSDesc.CullMode = D3D11_CULL_NONE;
	d3dRSDesc.FrontCounterClockwise = false;
	d3dRSDesc.DepthBias = SHADOW_DEPTH_BIAS;
	d3dRSDesc.DepthBiasClamp = 0.0f;
	d3dRSDesc.SlopeScaledDepthBias = 1.0f;
	ASSERT(SUCCEEDED(hr = pd3dDevice->CreateRasterizerState(&d3dRSDesc, &m_pd3dShadowRS)));
}

void CShadowMgr::BuildShadowMap(ID3D11DeviceContext * pd3dDeviceContext, XMFLOAT3 & Target, XMFLOAT3 & LightPos, float fHalf)
{
	XMVECTOR xmvUp = XMVectorSet(1, 0, 0, 0);
	XMVECTOR xmvTarget = XMLoadFloat3(&Target);//XMVectorSet(fHalf - 1000, 0, fHalf, 0);// XMLoadFloat3(&m_pPlayer->GetPosition());
	XMVECTOR xmvLightPos = XMLoadFloat3(&LightPos);

	XMMATRIX xmtxShadowView = XMMatrixLookAtLH(xmvLightPos, xmvTarget, xmvUp);
	xmvLightPos = XMVector3TransformCoord(xmvLightPos, xmtxShadowView);

	XMFLOAT3 xmf3LightPos;
	XMStoreFloat3(&m_xmf3LightPos, xmvLightPos);

	float xl = m_xmf3LightPos.x - fHalf, xr = m_xmf3LightPos.x + fHalf;
	float yb = m_xmf3LightPos.y - fHalf, yt = m_xmf3LightPos.y + fHalf;
	float zn = m_xmf3LightPos.z - fHalf, zf = m_xmf3LightPos.z + fHalf;

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
}

void CShadowMgr::SetStaticShadowMap(ID3D11DeviceContext * pd3dDeviceContext, CCamera * pCamera)
{
	pCamera->UpdateShaderVariables(pd3dDeviceContext, m_xmf44ShadowVP/*xmf44LightViewProj*/, m_xmf3LightPos);
	pd3dDeviceContext->PSSetShader(nullptr, nullptr, 0);
	pd3dDeviceContext->RSSetState(m_pd3dShadowRS);
	pd3dDeviceContext->RSSetViewports(1, &m_d3dxShadowMapViewport);

	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	pd3dDeviceContext->OMSetRenderTargets(0, nullptr, m_pd3dStaticDSVShadowMap);
	pd3dDeviceContext->ClearDepthStencilView(m_pd3dStaticDSVShadowMap, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void CShadowMgr::ResetStaticShadowMap(ID3D11DeviceContext * pd3dDeviceContext, CCamera * pCamera)
{
	pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	pd3dDeviceContext->RSSetState(nullptr);

	pCamera->SetViewport(pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
}

void CShadowMgr::UpdateStaticShadowResource(ID3D11DeviceContext * pd3dDeviceContext)
{
#ifdef PCF
	pd3dDeviceContext->PSSetSamplers(SS_SLOT_SHADOWSAMPLE + 1, 1, &m_pd3dShadowSamplerState);
#else
	pd3dDeviceContext->PSSetSamplers(SS_SLOT_SHADOWSAMPLE, 1, &m_pd3dShadowSamplerState);
#endif
	pd3dDeviceContext->PSSetShaderResources(m_iStaticMapSlot, 1, &m_pd3dStaticSRVShadowMap);

	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbShadowMap, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	XMFLOAT4X4 * pd3dxmtxShadowTransform = (XMFLOAT4X4*)d3dMappedResource.pData;
	Chae::XMFloat4x4Transpose(pd3dxmtxShadowTransform, &m_xmf44ShadowMap);	//XMFLOAT4X4Transpose(&pcbViewProjection->m_xmf44View, &m_xmf44View);
	pd3dDeviceContext->Unmap(m_pd3dcbShadowMap, 0);

	pd3dDeviceContext->PSSetConstantBuffers(CB_SLOT_SHADOWMAP, 1, &m_pd3dcbShadowMap);
}

void CShadowMgr::SetDynamicShadowMap(ID3D11DeviceContext * pd3dDeviceContext, CCamera * pCamera)
{
	pCamera->UpdateShaderVariables(pd3dDeviceContext, m_xmf44ShadowVP/*xmf44LightViewProj*/, m_xmf3LightPos);
	pd3dDeviceContext->PSSetShader(nullptr, nullptr, 0);
	pd3dDeviceContext->RSSetState(m_pd3dShadowRS);
	pd3dDeviceContext->RSSetViewports(1, &m_d3dxShadowMapViewport);

	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	pd3dDeviceContext->OMSetRenderTargets(0, nullptr, m_pd3dDSVShadowMap);
	pd3dDeviceContext->ClearDepthStencilView(m_pd3dDSVShadowMap, D3D11_CLEAR_DEPTH, 1.0f, 0);

}

void CShadowMgr::ResetDynamicShadowMap(ID3D11DeviceContext * pd3dDeviceContext, CCamera * pCamera)
{
	pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	pd3dDeviceContext->RSSetState(nullptr);

	pCamera->SetViewport(pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
}

void CShadowMgr::UpdateDynamicShadowResource(ID3D11DeviceContext * pd3dDeviceContext)
{
	pd3dDeviceContext->PSSetShaderResources(m_iMapSlot, 1, &m_pd3dSRVShadowMap);
}

