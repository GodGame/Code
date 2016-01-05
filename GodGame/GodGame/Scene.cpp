#include "stdafx.h"
#include "MyInline.h"
#include "Scene.h"


CScene::CScene()
{
	m_ppShaders = nullptr;

	m_nShaders = 0;
	m_pCamera = nullptr;
	m_pSelectedObject = nullptr;


	//m_nRenderThreads = 0;

	//m_pRenderingThreadInfo = nullptr;
}

CScene::~CScene()
{
	//for (int i = 0; i < m_nRenderThreads; ++i)
	//{
	//	m_pRenderingThreadInfo[i].m_pd3dDeferredContext->Release();
	//	::CloseHandle(m_pRenderingThreadInfo[i].m_hRenderingBeginEvent);
	//	::CloseHandle(m_pRenderingThreadInfo[i].m_hRenderingEndEvent);
	//}

	//if (m_pRenderingThreadInfo) delete[] m_pRenderingThreadInfo;
	//if (m_hRenderingEndEvents) delete[] m_hRenderingEndEvents;
}

void CScene::BuildObjects(ID3D11Device *pd3dDevice, CSceneShader * pSceneShader)
{
	TXMgr.BuildResources(pd3dDevice);

	HRESULT hr;

	//텍스쳐 맵핑에 사용할 샘플러 상태 객체를 생성한다.
	ID3D11SamplerState *pd3dSamplerState = nullptr;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = 0;
	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);

	//텍스쳐 리소스를 생성한다.
	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	CTexture *pStoneTexture = new CTexture(1, 1, 0, 0);
	hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/Stone01.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	if (FAILED(hr)) printf("오류");
	pStoneTexture->SetTexture(0, pd3dsrvTexture);
	pStoneTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pBrickTexture = new CTexture(1, 1, 0, 0);
	hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/Brick01.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	if (FAILED(hr)) printf("오류");
	pBrickTexture->SetTexture(0, pd3dsrvTexture);
	pBrickTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();

	CTexture *pWoodTexture = new CTexture(1, 1, 0, 0);
	hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/Wood01.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	if (FAILED(hr)) printf("오류");
	pWoodTexture->SetTexture(0, pd3dsrvTexture);
	pWoodTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	pd3dSamplerState->Release();

	m_nShaders = NUM_SHADER;
	m_ppShaders = new CShader*[m_nShaders];

	//첫 번째로 그릴 객체는 스카이 박스이다.
	m_ppShaders[0] = new CSkyBoxShader();
	m_ppShaders[0]->CreateShader(pd3dDevice);
	m_ppShaders[0]->BuildObjects(pd3dDevice);

	m_ppShaders[1] = new CTerrainShader();
	m_ppShaders[1]->CreateShader(pd3dDevice);
	m_ppShaders[1]->BuildObjects(pd3dDevice);

	//재질을 생성한다.
	CMaterial *pRedMaterial = new CMaterial();
	pRedMaterial->m_Material.m_xcDiffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	pRedMaterial->m_Material.m_xcAmbient = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	pRedMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f);
	pRedMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	CMaterial *pGreenMaterial = new CMaterial();
	pGreenMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	pGreenMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	pGreenMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 3.0f);
	pGreenMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	CMaterial *pBlueMaterial = new CMaterial();
	pBlueMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	pBlueMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	pBlueMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 4.0f);
	pBlueMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	CMaterial *pWhiteMaterial = new CMaterial();
	pWhiteMaterial->m_Material.m_xcDiffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	pWhiteMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	pWhiteMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 4.0f);
	pWhiteMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);



	CStaticShader *pStaticObjectsShader = new CStaticShader();
	pStaticObjectsShader->CreateShader(pd3dDevice);
	pStaticObjectsShader->BuildObjects(pd3dDevice, GetTerrain(), pWhiteMaterial);
	m_ppShaders[2] = pStaticObjectsShader;

	//CNormalShader * pNmShader = new CNormalShader();
	//pNmShader->CreateShader(pd3dDevice);
	//pNmShader->BuildObjects(pd3dDevice, GetTerrain(), pWhiteMaterial);
	//m_ppShaders[2] = pNmShader;

	//CParticleShader *pParticleShader = new CParticleShader();
	//pParticleShader->CreateShader(pd3dDevice);
	//pParticleShader->BuildObjects(pd3dDevice, GetTerrain(), nullptr);
	//m_ppShaders[3] = pParticleShader;


	//CPointShader *pPointShader = new CPointShader();
	//pPointShader->CreateShader(pd3dDevice);
	//pPointShader->BuildObjects(pd3dDevice, GetTerrain(), pWhiteMaterial, pBrickTexture);
	//m_ppShaders[2] = pPointShader;


	//CWaterShader * pWaterShader = new CWaterShader();
	//pWaterShader->CreateShader(pd3dDevice);
	//pWaterShader->BuildObjects(pd3dDevice, nullptr);
	//m_ppShaders[4] = pWaterShader;

	//m_ppShaders[3] = pSceneShader;

	//	CreateStates(pd3dDevice);
	CreateShaderVariables(pd3dDevice);
	//	InitilizeThreads(pd3dDevice);
}

void CScene::ReleaseObjects()
{
	ReleaseShaderVariables();

	for (int j = 0; j < m_nShaders; j++)
	{
		if (m_ppShaders[j]) m_ppShaders[j]->ReleaseObjects();
		if (m_ppShaders[j]) delete m_ppShaders[j];
	}
	if (m_ppShaders) delete[] m_ppShaders;

}


bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		//		m_pSelectedObject = PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	return false;
}
void CScene::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));
	//게임 월드 전체를 비추는 주변조명을 설정한다.
	m_pLights->m_xcGlobalAmbient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);

	//3개의 조명(점 광원, 스팟 광원, 방향성 광원)을 설정한다.
	m_pLights->m_pLights[0].m_bEnable = 1.0f;
	m_pLights->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 300.0f;
	m_pLights->m_pLights[0].m_xcAmbient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xcDiffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xcSpecular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights->m_pLights[0].m_xv3Position = XMFLOAT3(0.0f, 300.0f, 0.0f);
	m_pLights->m_pLights[0].m_xv3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xv3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	m_pLights->m_pLights[1].m_bEnable = 1.0f;
	m_pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 100.0f;
	m_pLights->m_pLights[1].m_xcAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xcDiffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[1].m_xcSpecular = XMFLOAT4(0.3f, 0.3f, 0.3f, 5.0f);
	m_pLights->m_pLights[1].m_xv3Position = XMFLOAT3(500.0f, 300.0f, 500.0f);
	m_pLights->m_pLights[1].m_xv3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xv3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[1].m_fFalloff = 8.0f;
	m_pLights->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights->m_pLights[2].m_bEnable = 1.0f;
	m_pLights->m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[2].m_xcAmbient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights->m_pLights[2].m_xcDiffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	m_pLights->m_pLights[2].m_xcSpecular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[2].m_xv3Direction = XMFLOAT3(-0.707f, -0.707f, 0.0f);
	m_pLights->m_pLights[3].m_bEnable = 1.0f;
	m_pLights->m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[3].m_fRange = 60.0f;
	m_pLights->m_pLights[3].m_xcAmbient = XMFLOAT4(0.2f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[3].m_xcDiffuse = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[3].m_xcSpecular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_xv3Position = XMFLOAT3(500.0f, 300.0f, 500.0f);
	m_pLights->m_pLights[3].m_xv3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_pLights->m_pLights[3].m_xv3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[3].m_fFalloff = 20.0f;
	m_pLights->m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(15.0f));

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(d3dBufferDesc));
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth = sizeof(LIGHTS);
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pLights;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dcbLights);
}

void CScene::ReleaseShaderVariables()
{
	if (m_pLights) delete m_pLights;
	if (m_pd3dcbLights) m_pd3dcbLights->Release();
}
void CScene::UpdateLights(ID3D11DeviceContext *pd3dDeviceContext)
{
	if (m_pLights && m_pd3dcbLights) UpdateShaderVariable(pd3dDeviceContext, m_pLights);
}
void CScene::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbLights, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	LIGHTS *pcbLight = (LIGHTS *)d3dMappedResource.pData;
	memcpy(pcbLight, pLights, sizeof(LIGHTS));
	pd3dDeviceContext->Unmap(m_pd3dcbLights, 0);
	pd3dDeviceContext->PSSetConstantBuffers(PS_SLOT_LIGHT, 1, &m_pd3dcbLights);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'Z':
			((CParticleShader*)m_ppShaders[3])->SetParticle(1, &m_pPlayerShader->GetPlayer()->GetPosition());
			break;
		}
		break;
	}
	return(false);
}

bool CScene::ProcessInput()
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	if (m_pLights && m_pd3dcbLights)
	{
		//현재 카메라의 위치 벡터를 조명을 나타내는 상수 버퍼에 설정한다.
		XMFLOAT3 xv3CameraPosition = m_pCamera->GetPosition();
		m_pLights->m_xv4CameraPosition = XMFLOAT4(xv3CameraPosition.x, xv3CameraPosition.y, xv3CameraPosition.z, 1.0f);

		//점 조명이 지형의 중앙을 중심으로 회전하도록 설정한다.
		CHeightMapTerrain *pTerrain = GetTerrain();
		static XMVECTOR	xvRotated = XMVectorSet(pTerrain->GetWidth()*0.3f, 0.0f, 0.0f, 0.0f);
		//XMVECTOR	xvRotated = XMLoadFloat3(&xmf3Rotated);
		XMMATRIX xmtxRotate = XMMatrixRotationRollPitchYaw((float)XMConvertToRadians(30.0f*fTimeElapsed), 0.0f, 0.0f);
		//XMFLOAT4X4RotationYawPitchRoll(&xmtxRotate, (float)XMConvertToRadians(30.0f*fTimeElapsed), 0.0f, 0.0f);
		xvRotated = XMVector3TransformCoord(xvRotated, xmtxRotate); // xv3ec3TransformCoord(&xvRotated, &xvRotated, &xmtxRotate);

		XMVECTOR xvTerrainCenter = XMVectorSet(pTerrain->GetWidth()*0.5f, pTerrain->GetPeakHeight() + 10.0f, pTerrain->GetLength()*0.5f, 0.0f);
		XMStoreFloat3(&m_pLights->m_pLights[0].m_xv3Position, XMVectorAdd(xvTerrainCenter, xvRotated));
		m_pLights->m_pLights[0].m_fRange = pTerrain->GetPeakHeight();

		/*두 번째 조명은 플레이어가 가지고 있는 손전등(스팟 조명)이다. 그러므로 플레이어의 위치와 방향이 바뀌면 현재 플레이어의 위치와 z-축 방향 벡터를 스팟 조명의 위치와 방향으로 설정한다.*/
		CPlayer *pPlayer = m_pCamera->GetPlayer();
		m_pLights->m_pLights[1].m_xv3Position = pPlayer->GetPosition();
		m_pLights->m_pLights[1].m_xv3Direction = pPlayer->GetLookVector();

		//m_pLights->m_pLights[3].m_xv3Position = pPlayer->GetPosition();
		//m_pLights->m_pLights[3].m_xv3Position.y += 40.0f; //+XMFLOAT3(0.0f, 40.0f, 0.0f);
	}

	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	}
}

void CScene::Render(ID3D11DeviceContext*pd3dDeviceContext, RENDER_INFO * pRenderInfo)
{
	//UpdateLights(pd3dDeviceContext);
	//CPlayer * pPlayer = m_pCamera->GetPlayer();

#ifdef _THREAD
	int index = pRenderInfo->ThreadID;

	//if (index == m_nShaders - 1)
	//{
//
	//	UpdateLights(pd3dDeviceContext);
	//	pd3dDeviceContext->OMSetRenderTargets(1, &(pRenderInfo->ppd3dMrtRTV[0]), nullptr);
	//}
	if (index == m_nShaders - 1)
	{			
		m_pPlayerShader->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
	}
	
	m_ppShaders[index]->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
#else
	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
	}
#endif


	//m_ppShaders[m_nShaders - 1]->Render(pd3dDeviceContext, pCamera);
}

#ifdef PICKING
CGameObject *CScene::PickObjectPointedByCursor(int xClient, int yClient)
{
	if (!m_pCamera) return(nullptr);

	XMFLOAT4X4 xmtxView = m_pCamera->GetViewMatrix();
	XMFLOAT4X4 xmtxProjection = m_pCamera->GetProjectionMatrix();
	D3D11_VIEWPORT d3dViewport = m_pCamera->GetViewport();

	XMFLOAT3 xv3PickPosition;
	/*화면 좌표계의 점 (xClient, yClient)를 화면 좌표 변환의 역변환과 투영 변환의 역변환을 한다. 그 결과는 카메라 좌표계의 점이다. 투영 평면이 카메라에서 z-축으로 거리가 1이므로 z-좌표는 1로 설정한다.*/
	xv3PickPosition.x = (((2.0f * xClient) / d3dViewport.Width) - 1) / xmtxProjection._11;
	xv3PickPosition.y = -(((2.0f * yClient) / d3dViewport.Height) - 1) / xmtxProjection._22;
	xv3PickPosition.z = 1.0f;

	int nIntersected = 0;
	float fNearHitDistance = FLT_MAX;
	MESHINTERSECTINFO d3dxIntersectInfo;
	CGameObject *pIntersectedObject = nullptr, *pNearestObject = nullptr;
	//씬의 모든 쉐이더 객체에 대하여 픽킹을 처리하여 카메라와 가장 가까운 픽킹된 객체를 찾는다.
	for (int i = 0; i < m_nShaders; i++)
	{
		pIntersectedObject = m_ppShaders[i]->PickObjectByRayIntersection(&xv3PickPosition, &xmtxView, &d3dxIntersectInfo);
		if (pIntersectedObject && (d3dxIntersectInfo.m_fDistance < fNearHitDistance))
		{
			fNearHitDistance = d3dxIntersectInfo.m_fDistance;
			pNearestObject = pIntersectedObject;
		}
	}
	return(pNearestObject);
}
#endif

CHeightMapTerrain *CScene::GetTerrain()
{
	CTerrainShader *pTerrainShader = (CTerrainShader *)m_ppShaders[1];
	return(pTerrainShader->GetTerrain());
}

