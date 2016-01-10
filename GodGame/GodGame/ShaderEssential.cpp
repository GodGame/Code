#include "stdafx.h"
#include "ShaderEssential.h"
#include "MyInline.h"
#include "Player.h"
#include <D3Dcompiler.h>


#pragma region PlayerShader


CSceneShader::CSceneShader() : CTexturedShader()
{
	m_pMesh = nullptr;
	m_pTexture = nullptr;
	m_pd3dDepthStencilState = nullptr;
	m_iDrawOption = 0;
	m_ppd3dMrtSrv = nullptr;

	m_pd3dPSOther = nullptr;
	m_pInfoScene = nullptr;

	m_pd3dShadowSrv = nullptr;
	m_pd3dLightPS = nullptr;
}

CSceneShader::~CSceneShader()
{
	if (m_pMesh) m_pMesh->Release();
	if (m_pTexture) m_pTexture->Release();
	if (m_pd3dDepthStencilState) m_pd3dDepthStencilState->Release();
	if (m_pd3dPSOther) m_pd3dPSOther->Release();
	if (m_pd3dShadowSrv) m_pd3dShadowSrv->Release();
	if (m_pd3dLightPS) m_pd3dLightPS->Release();
}

void CSceneShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSScreen", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSScreen", "ps_5_0", &m_pd3dPixelShader);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "InfoScreen", "ps_5_0", &m_pd3dPSOther);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "LightScreen", "ps_5_0", &m_pd3dLightPS);
}

void CSceneShader::BuildObjects(ID3D11Device *pd3dDevice, ID3D11ShaderResourceView ** ppd3dMrtSrv, int nMrtSrv)
{
	m_iDrawOption = nMrtSrv;
	m_ppd3dMrtSrv = ppd3dMrtSrv;

	m_pMesh = new CPlaneMesh(pd3dDevice, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

	m_pTexture = new CTexture(NUM_MRT - 1, 1, 17, 0, SET_SHADER_PS);
	m_pInfoScene = new CTexture(1, 0, 0, 0, SET_SHADER_PS);

	for (int i = 1; i < NUM_MRT; ++i) m_pTexture->SetTexture(i - 1, ppd3dMrtSrv[i]);

}

void CSceneShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera /*= nullptr*/)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);
	printf("Opt: %d \n", m_iDrawOption);
	//SetTexture(0, m_ppd3dMrtSrv[m_iDrawOption]);
	if (m_iDrawOption == 0)
	{
		UpdateShaders(pd3dDeviceContext);
	}
	else if (m_iDrawOption == 1)
	{
		m_pInfoScene->SetTexture(0, m_pd3dShadowSrv);
		pd3dDeviceContext->PSSetShader(m_pd3dLightPS, nullptr, 0);
		m_pInfoScene->UpdateShaderVariable(pd3dDeviceContext);
	}
	else if (m_iDrawOption < 0)
	{
		pd3dDeviceContext->PSSetShader(m_pd3dPSOther, nullptr, 0);
		m_pInfoScene->UpdateShaderVariable(pd3dDeviceContext);
	}
	else
	{
		SetInfoTextures(pd3dDeviceContext);
	}
	m_pMesh->Render(pd3dDeviceContext, uRenderState);
}

void CSceneShader::SetTexture(int index, ID3D11ShaderResourceView * m_pSceneSRV)
{
	m_pInfoScene->SetTexture(index, m_pSceneSRV);
}

void CSceneShader::SetInfoTextures(ID3D11DeviceContext * pd3dDeviceContext)
{
	m_pInfoScene->SetTexture(0, m_ppd3dMrtSrv[m_iDrawOption]);
	pd3dDeviceContext->PSSetShader(m_pd3dPSOther, nullptr, 0);
	m_pInfoScene->UpdateShaderVariable(pd3dDeviceContext);
}

void CSceneShader::UpdateShaders(ID3D11DeviceContext * pd3dDeviceContext)
{
	m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
}


CPlayerShader::CPlayerShader() : CTexturedIlluminatedShader()
{
}
CPlayerShader::~CPlayerShader()
{
}
void CPlayerShader::CreateShader(ID3D11Device *pd3dDevice)
{
	CTexturedIlluminatedShader::CreateShader(pd3dDevice);
}

void CPlayerShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain * pTerrain)
{
	CreateShader(pd3dDevice);

	m_nObjects = 1;
	m_ppObjects = new CGameObject*[m_nObjects];

	CTexture *pBrickTexture = new CTexture(1, 1, 0, 0);
	pBrickTexture->SetTexture(0,  TXMgr.GetShaderResourceView("srv_brick2_jpg"));
	pBrickTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));
	
	TXMgr.InsertObject(pBrickTexture, "PlayerTexture");

	CMaterial *pPlayerMaterial = new CMaterial();
	pPlayerMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	pPlayerMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	pPlayerMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f);
	pPlayerMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.2f, 1.0f);

	CCubeMeshTexturedIlluminated *pCubeMesh = new CCubeMeshTexturedIlluminated(pd3dDevice, 4.0f, 12.0f, 4.0f);
	CTerrainPlayer *pTerrainPlayer = new CTerrainPlayer(1);

	//플레이어의 위치가 변경될 때 지형의 정보에 따라 플레이어의 위치를 변경할 수 있도록 설정한다.
	pTerrainPlayer->SetPlayerUpdatedContext(pTerrain);
	//카메라의 위치가 변경될 때 지형의 정보에 따라 카메라의 위치를 변경할 수 있도록 설정한다.
	pTerrainPlayer->SetCameraUpdatedContext(pTerrain);
	/*지형의 xz-평면의 가운데에 플레이어가 위치하도록 한다. 플레이어의 y-좌표가 지형의 높이 보다 크고 중력이 작용하도록 플레이어를 설정하였으므로 플레이어는 점차적으로 하강하게 된다.*/

	pTerrainPlayer->InitPosition(XMFLOAT3(pTerrain->GetWidth()*0.5f, pTerrain->GetPeakHeight() + 1000.0f, 300));
	pTerrainPlayer->ChangeCamera(pd3dDevice, THIRD_PERSON_CAMERA, 0.0f);
	pTerrainPlayer->Rotate(0, 180, 0);

	pTerrainPlayer->SetMesh(pCubeMesh);
	pTerrainPlayer->SetMaterial(pPlayerMaterial);
	pTerrainPlayer->SetTexture(pBrickTexture);

	pBrickTexture->Release();
	m_ppObjects[0] = pTerrainPlayer;
}

void CPlayerShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);
	//XMFLOAT3 pos = m_ppObjects[0]->GetPosition();
	//printf("%0.2f %0.2f %0.2f \n", pos.x, pos.y, pos.z);
	//3인칭 카메라일 때 플레이어를 렌더링한다.
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		CShader::Render(pd3dDeviceContext, uRenderState);
	}
}
#pragma endregion PlayerShader



CTerrainShader::CTerrainShader() : CSplatLightingShader()
{
	m_nLayerNumber = 0;
	m_pptxLayerMap = nullptr;
}

void CTerrainShader::CreateShader(ID3D11Device *pd3dDevice)
{
#ifdef TS_TERRAIN
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 } };
	UINT nElements = ARRAYSIZE(d3dInputLayout);

	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSTerrain", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreateHullShaderFromFile(pd3dDevice, L"Effect.fx", "HSTerrain", "hs_5_0", &m_pd3dHullShader);
	CreateDomainShaderFromFile(pd3dDevice, L"Effect.fx", "DSTerrain", "ds_5_0", &m_pd3dDomainShader);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSTerrain", "ps_5_0", &m_pd3dPixelShader);
	CreateShaderVariables(pd3dDevice);
#else
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSSplatTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSSplatTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader);
#endif

}

CTerrainShader::~CTerrainShader()
{
	for (int i = 0; i < m_nLayerNumber; ++i)
	if (m_pptxLayerMap[i])
		m_pptxLayerMap[i]->Release();
	delete[] m_pptxLayerMap;
}

void CTerrainShader::BuildObjects(ID3D11Device *pd3dDevice)
{
	m_nObjects = 1;
	m_ppObjects = new CGameObject*[m_nObjects];

	m_nLayerNumber = 1;
	m_pptxLayerMap = new CTexture *[m_nLayerNumber];

#ifdef TS_TERRAIN
	wchar_t  **ppHeigtName, ** ppTextureName;
	ppTextureName = new wchar_t *[m_nLayerNumber];
	ppHeigtName = new wchar_t *[m_nLayerNumber];

	for (int i = 0; i < m_nLayerNumber; ++i)
	{
		m_pptxLayerMap[i] = new CTexture(2, 2, 0, 0, (SET_SHADER_PS | SET_SHADER_VS | SET_SHADER_DS));
		ppTextureName[i] = new wchar_t[128];
		ppHeigtName[i] = new wchar_t[128];
	}
	ID3D11ShaderResourceView **ppd3dsrvHeight, **ppd3dsrvTexture;
	ppd3dsrvTexture = new ID3D11ShaderResourceView *[m_nLayerNumber];
	ppd3dsrvHeight = new ID3D11ShaderResourceView *[m_nLayerNumber];

	ppTextureName[0] = _T("../Assets/Image/Terrain/Detail_Texture_6.jpg");

	ppHeigtName[0] = _T("../Assets/Image/Terrain/HeightMap.jpg");

	for (int fileIndex = 0; fileIndex < m_nLayerNumber; fileIndex++)
	{
		D3DX11CreateShaderResourceViewFromFile(pd3dDevice, ppHeigtName[fileIndex], nullptr, nullptr, &ppd3dsrvHeight[fileIndex], nullptr);
		m_pptxLayerMap[fileIndex]->SetTexture(0, ppd3dsrvHeight[fileIndex]);
		m_pptxLayerMap[fileIndex]->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));
		ppd3dsrvHeight[fileIndex]->Release();

		D3DX11CreateShaderResourceViewFromFile(pd3dDevice, ppTextureName[fileIndex], nullptr, nullptr, &ppd3dsrvTexture[fileIndex], nullptr);
		m_pptxLayerMap[fileIndex]->SetTexture(1, ppd3dsrvTexture[fileIndex]);
		m_pptxLayerMap[fileIndex]->SetSampler(1, TXMgr.GetSamplerState("ss_linear_clamp"));
		ppd3dsrvTexture[fileIndex]->Release();
	}

	delete[] ppTextureName;
	delete[] ppd3dsrvHeight;
	delete[] ppd3dsrvTexture;
#else
	wchar_t ** ppTextureName, **ppAlphaName;
	ppTextureName = new wchar_t *[m_nLayerNumber];
	ppAlphaName = new wchar_t *[m_nLayerNumber];

	for (int i = 0; i < m_nLayerNumber; ++i)
	{
		m_pptxLayerMap[i] = new CTexture(2, 2, 0, 0, SET_SHADER_PS);
		ppTextureName[i] = new wchar_t[128];
		ppAlphaName[i] = new wchar_t[128];
	}
	ID3D11ShaderResourceView **ppd3dsrvTexture, **ppd3dsrvAlphaTexture;
	ppd3dsrvTexture = new ID3D11ShaderResourceView *[m_nLayerNumber];
	ppd3dsrvAlphaTexture = new ID3D11ShaderResourceView *[m_nLayerNumber];

	ppTextureName[0] = _T("../Assets/Image/Terrain/Detail_Texture_0.jpg");
	//ppTextureName[1] = _T("../Assets/Image/Terrain/Detail_Texture_6.jpg");
	//ppTextureName[2] = _T("../Assets/Image/Terrain/flower.jpg");

	ppAlphaName[0] = _T("../Assets/Image/Terrain/Alpha0.png");
	//ppAlphaName[1] = _T("../Assets/Image/Terrain/Alpha1.png");
	//ppAlphaName[2] = _T("../Assets/Image/Terrain/Alpha2.png");

	for (int fileIndex = 0; fileIndex < m_nLayerNumber; fileIndex++){
		D3DX11CreateShaderResourceViewFromFile(pd3dDevice, ppTextureName[fileIndex], nullptr, nullptr, &ppd3dsrvTexture[fileIndex], nullptr);
		m_pptxLayerMap[fileIndex]->SetTexture(0, ppd3dsrvTexture[fileIndex]);
		m_pptxLayerMap[fileIndex]->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));
		ppd3dsrvTexture[fileIndex]->Release();

		D3DX11CreateShaderResourceViewFromFile(pd3dDevice, ppAlphaName[fileIndex], nullptr, nullptr, &ppd3dsrvAlphaTexture[fileIndex], nullptr);
		m_pptxLayerMap[fileIndex]->SetTexture(1, ppd3dsrvAlphaTexture[fileIndex]);
		m_pptxLayerMap[fileIndex]->SetSampler(1, TXMgr.GetSamplerState("ss_linear_clamp"));
		ppd3dsrvAlphaTexture[fileIndex]->Release();
	}

	delete[] ppTextureName;
	delete[] ppAlphaName;
	delete[] ppd3dsrvTexture;
	delete[] ppd3dsrvAlphaTexture;
#endif

	//지형을 확대할 스케일 벡터이다. x-축과 z-축은 8배, y-축은 2배 확대한다.
	XMFLOAT3 xv3Scale(8.0f, 2.0f, 8.0f);
	/*지형을 높이 맵 이미지 파일을 사용하여 생성한다. 높이 맵 이미지의 크기는 가로x세로(257x257)이고 격자 메쉬의 크기는 가로x세로(17x17)이다.
	지형 전체는 가로 방향으로 16개, 세로 방향으로 16의 격자 메쉬를 가진다. 지형을 구성하는 격자 메쉬의 개수는 총 256(16x16)개가 된다.*/

	m_ppObjects[0] = new CHeightMapTerrain(pd3dDevice, _T("../Assets/Image/Terrain/HeightMap.raw"), 257, 257, 257, 257, xv3Scale);

	CMaterial *pTerrainMaterial = new CMaterial();
	pTerrainMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.7f, 0.8f, 0.7f, 1.0f);
	pTerrainMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	pTerrainMaterial->m_Material.m_xcSpecular = XMFLOAT4(3.0f, 3.0f, 3.0f, 5.0f);
	pTerrainMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	m_ppObjects[0]->SetMaterial(pTerrainMaterial);

}
void CTerrainShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);
	//float pBlendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	//pd3dDeviceContext->OMSetBlendState(m_pd3dSplatBlendState, pBlendFactor, 0xffffffff);
	//for (int i = 0; i < m_nLayerNumber; ++i)
	for (int i = m_nLayerNumber - 1; i >= 0; --i)
	{
		//m_ppObjects[0]->SetTexture(m_pptxLayerMap[0], false);
		m_pptxLayerMap[i]->UpdateShaderVariable(pd3dDeviceContext);
		m_ppObjects[i]->Render(pd3dDeviceContext, uRenderState, pCamera);
	}
	//pd3dDeviceContext->OMSetBlendState(nullptr, pBlendFactor, 0xffffffff);
}


CHeightMapTerrain *CTerrainShader::GetTerrain()
{
	return((CHeightMapTerrain *)m_ppObjects[0]);
}


CWaterShader::CWaterShader() : CTexturedShader()
{
	m_pd3dWaterBlendState = nullptr;
}

CWaterShader::~CWaterShader()
{
	if (m_pd3dWaterBlendState) m_pd3dWaterBlendState->Release();
}

void CWaterShader::CreateShader(ID3D11Device *pd3dDevice)
{
	CTexturedShader::CreateShader(pd3dDevice);
}

void CWaterShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain)
{
	//m_pTexture = pTexture;
	//if (pTexture) pTexture->AddRef();

	ID3D11SamplerState *pd3dSamplerState = nullptr;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = 0;
	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);

	ID3D11SamplerState *pd3dDetailSamplerState = nullptr;
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dDetailSamplerState);

	/////////////////////////////////////////////////

	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	/// 물 텍스쳐///

	CTexture *pWaterTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Terrain/water01.png"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	pWaterTexture->SetTexture(0, pd3dsrvTexture);
	pWaterTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	pd3dSamplerState->Release();

	//ID3D11ShaderResourceView *pd3dsrvDetailTexture = nullptr;
	//D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Terrain/Water_Detail_Texture_0.dds"), nullptr, nullptr, &pd3dsrvDetailTexture, nullptr);
	//pWaterTexture->SetTexture(1, pd3dsrvDetailTexture);
	//pWaterTexture->SetSampler(1, pd3dDetailSamplerState);
	//pd3dsrvDetailTexture->Release();
	//pd3dDetailSamplerState->Release();

	//CCubeMeshTexturedIlluminated *pCubeMesh = new CCubeMeshTexturedIlluminated(pd3dDevice, 12.0f, 12.0f, 12.0f);
	CSphereMeshTexturedIlluminated *pSphereMesh = new CSphereMeshTexturedIlluminated(pd3dDevice, 5.0f, 20, 20);

	m_nObjects = 1;
	m_ppObjects = new CGameObject*[m_nObjects];

	CRotatingObject *pObject = nullptr;

	/// 이상 스테틱 객체들

	m_ppObjects[0] = new CGameObject(1);
	CCubeMeshTexturedIlluminated * CWaterMesh = new CCubeMeshTexturedIlluminated(pd3dDevice, 2048, 100, 2048);
	m_ppObjects[0]->SetMesh(CWaterMesh);
	m_ppObjects[0]->SetTexture(pWaterTexture);
	m_ppObjects[0]->SetPosition(1024, 98, 1024);

	//CMaterial *pWaterMaterial = new CMaterial();
	//pWaterMaterial->m_Material.m_xcDiffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//pWaterMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.1f, 0.1f, 0.3f, 1.0f);
	//pWaterMaterial->m_Material.m_xcEmissive = XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f);
	//pWaterMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	//m_ppObjects[0]->SetMaterial(pWaterMaterial);

	SetBlendState(pd3dDevice);
}

void CWaterShader::SetBlendState(ID3D11Device *pd3dDevice)
{
	D3D11_BLEND_DESC	d3dBlendDesc;
	ZeroMemory(&d3dBlendDesc, sizeof(D3D11_BLEND_DESC));

	d3dBlendDesc.AlphaToCoverageEnable = false;
	d3dBlendDesc.IndependentBlendEnable = false;
	d3dBlendDesc.RenderTarget[0].BlendEnable = true;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;	// 파란색 위주로 한다.

	pd3dDevice->CreateBlendState(&d3dBlendDesc, &m_pd3dWaterBlendState);
}

void CWaterShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	float pBlendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pd3dDeviceContext->OMSetBlendState(m_pd3dWaterBlendState, pBlendFactor, 0xffffffff);

	m_ppObjects[0]->Render(pd3dDeviceContext, uRenderState, pCamera);
	pd3dDeviceContext->OMSetBlendState(nullptr, pBlendFactor, 0xffffffff);
}



CSkyBoxShader::CSkyBoxShader()
{
}

CSkyBoxShader::~CSkyBoxShader()
{
}

void CSkyBoxShader::CreateShader(ID3D11Device *pd3dDevice)
{
#define _WITH_SKYBOX_TEXTURE_CUBE
#ifdef _WITH_SKYBOX_TEXTURE_CUBE
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSSkyBoxTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSSkyBoxTexturedColor", "ps_5_0", &m_pd3dPixelShader);
#else
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSTexturedColor", "ps_5_0", &m_pd3dPixelShader);
#endif
}

void CSkyBoxShader::BuildObjects(ID3D11Device *pd3dDevice)
{
	m_nObjects = 1;
	m_ppObjects = new CGameObject*[m_nObjects];

	CSkyBox *pSkyBox = new CSkyBox(pd3dDevice);
	m_ppObjects[0] = pSkyBox;
}

void CSkyBoxShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	CShader::OnPrepareRender(pd3dDeviceContext, uRenderState);

	m_ppObjects[0]->Render(pd3dDeviceContext, uRenderState, pCamera);
}


CShadowShader::CShadowShader()
{

}

CShadowShader::~CShadowShader()
{
}

void CShadowShader::CreateShader(ID3D11Device * pd3dDevice)
{
}

void CShadowShader::BuildObjects(ID3D11Device * pd3dDevice)
{
}

void CShadowShader::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
}

CSSAOShader::CSSAOShader()
{
	m_pMesh = nullptr;
	m_pd3dSRVSSAO = nullptr;
	ZeroMemory(&m_ssao, sizeof(m_ssao));
}

CSSAOShader::~CSSAOShader()
{
	if(m_pd3dSRVSSAO) m_pd3dSRVSSAO->Release();
}

void CSSAOShader::CreateShader(ID3D11Device * pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"SSAO.fx", "VSSCeneSpaceAmbient", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"SSAO.fx", "PSSCeneSpaceAmbient", "ps_5_0", &m_pd3dPixelShader);
}

void CSSAOShader::BuildObjects(ID3D11Device * pd3dDevice)
{
	BuildSSAO(pd3dDevice);

	CPlaneMesh * pMesh = new CPlaneMesh(pd3dDevice, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	m_pMesh = pMesh;

}

void CSSAOShader::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);
	UpdateShaderVariable(pd3dDeviceContext, pCamera);
	TXMgr.UpdateShaderVariable(pd3dDeviceContext, "srv_random1d");
	//TXMgr.UpdateShaderVariable(pd3dDeviceContext, "srv_rtvSSAO");

	m_pMesh->Render(pd3dDeviceContext, uRenderState);
}

void CSSAOShader::BuildSSAO(ID3D11Device * pd3dDevice)
{
	float aspect = (float)FRAME_BUFFER_WIDTH / (float)FRAME_BUFFER_HEIGHT;
	float farZ = 1000.0f;
	float halfHeight = farZ * tanf(XMConvertToRadians(0.5f * 60.0f));
	float halfWidth = aspect * halfHeight;
	
	m_ssao.m_gFrustumCorners[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	m_ssao.m_gFrustumCorners[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	m_ssao.m_gFrustumCorners[2] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);
	m_ssao.m_gFrustumCorners[3] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);

	int index = 0;
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < NUM_SSAO_OFFSET; ++i)
	{
		float s = Chae::RandomFloat(0.25, 1.0);

		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&m_ssao.m_gOffsetVectors[i]));
		XMStoreFloat4(&m_ssao.m_gOffsetVectors[i], v);
	}
}

void CSSAOShader::CreateShaderVariable(ID3D11Device * pd3dDevice)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(CB_SSAO_INFO);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = pd3dDevice->CreateBuffer(&bd, nullptr, &m_pd3dcbSSAOInfo);
	if (FAILED(hr))
		printf("오류입니다!!");
}

void CSSAOShader::UpdateShaderVariable(ID3D11DeviceContext * pd3dDeviceContext, CCamera * pCamera)
{
	float fw = 0.5f; //FRAME_BUFFER_WIDTH * 0.5f;
	float fh = 0.5f; //FRAME_BUFFER_HEIGHT * 0.5f;
	static const XMMATRIX T(
		+fw, 0.0f, 0.0f, 0.0f,
		0.0f, -fh, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		+fw, +fh, 0.0f, 1.0f);

	XMMATRIX VP = XMLoadFloat4x4(&pCamera->GetViewProjectionMatrix());// camera.Proj();
	XMMATRIX PT = XMMatrixMultiply(VP, T);
	XMStoreFloat4x4(&m_ssao.m_gViewToTexSpace, PT);

	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbSSAOInfo, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	CB_SSAO_INFO *pcbSSAO = (CB_SSAO_INFO *)d3dMappedResource.pData;
	memcpy(pcbSSAO->m_gFrustumCorners, m_ssao.m_gFrustumCorners, sizeof(m_ssao.m_gFrustumCorners));
	memcpy(pcbSSAO->m_gOffsetVectors, m_ssao.m_gOffsetVectors, sizeof(m_ssao.m_gOffsetVectors));
	Chae::XMFloat4x4Transpose(&pcbSSAO->m_gViewToTexSpace, &m_ssao.m_gViewToTexSpace);
	pd3dDeviceContext->Unmap(m_pd3dcbSSAOInfo, 0);

	//상수 버퍼를 디바이스의 슬롯(CB_SLOT_WORLD_MATRIX)에 연결한다.
	pd3dDeviceContext->VSSetConstantBuffers(CB_SLOT_SSAO, 1, &m_pd3dcbSSAOInfo);
	pd3dDeviceContext->PSSetConstantBuffers(CB_SLOT_SSAO, 1, &m_pd3dcbSSAOInfo);
}
