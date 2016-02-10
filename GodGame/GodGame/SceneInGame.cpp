#include "stdafx.h"
#include "MyInline.h"
#include "SceneInGame.h"
#include "SceneTitle.h"
#include "GameFramework.h"

bool bIsKeyDown = false;
//bool bIsMouseDown = false;

CSceneInGame::CSceneInGame() : CScene()
{
}

CSceneInGame::~CSceneInGame()
{
}

void CSceneInGame::BuildObjects(ID3D11Device *pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext, SceneShaderBuildInfo * SceneInfo)
{
	m_nMRT = NUM_MRT;
	m_nThread = NUM_THREAD;

	HRESULT hr = 0;

	//텍스쳐 맵핑에 사용할 샘플러 상태 객체를 생성한다.
	ID3D11SamplerState *pd3dSamplerState = nullptr;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	d3dSamplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD         = 0;
	d3dSamplerDesc.MaxLOD         = 0;
	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);

	//텍스쳐 리소스를 생성한다.
	//ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	//CTexture *pStoneTexture = new CTexture(1, 1, 0, 0);
	//hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/Stone01.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	//if (FAILED(hr)) printf("오류");
	//pStoneTexture->SetTexture(0, pd3dsrvTexture);
	//pStoneTexture->SetSampler(0, pd3dSamplerState);
	//pd3dsrvTexture->Release();

	//CTexture *pBrickTexture = new CTexture(1, 1, 0, 0);
	//hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/Brick01.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	//if (FAILED(hr)) printf("오류");
	//pBrickTexture->SetTexture(0, pd3dsrvTexture);
	//pBrickTexture->SetSampler(0, pd3dSamplerState);
	//pd3dsrvTexture->Release();

	//CTexture *pWoodTexture = new CTexture(1, 1, 0, 0);
	//hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/Wood01.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	//if (FAILED(hr)) printf("오류");
	//pWoodTexture->SetTexture(0, pd3dsrvTexture);
	//pWoodTexture->SetSampler(0, pd3dSamplerState);
	//pd3dsrvTexture->Release();
	//pd3dSamplerState->Release();

	//재질을 생성한다.
	CMaterial *pRedMaterial   = MaterialMgr.GetObjects("Red");
	CMaterial *pGreenMaterial = MaterialMgr.GetObjects("Green");
	CMaterial *pBlueMaterial  = MaterialMgr.GetObjects("Blue");
	CMaterial *pWhiteMaterial = MaterialMgr.GetObjects("White");
	{
		m_nShaders = NUM_SHADER;
		m_ppShaders = new CShader*[m_nShaders];

		//첫 번째로 그릴 객체는 스카이 박스이다.
		m_ppShaders[0] = new CSkyBoxShader();
		m_ppShaders[0]->CreateShader(pd3dDevice);
		m_ppShaders[0]->BuildObjects(pd3dDevice);

		m_ppShaders[1] = new CTerrainShader();
		m_ppShaders[1]->CreateShader(pd3dDevice);
		m_ppShaders[1]->BuildObjects(pd3dDevice);

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

		CPointInstanceShader *pPointShader = new CPointInstanceShader();
		pPointShader->CreateShader(pd3dDevice);
		pPointShader->BuildObjects(pd3dDevice, GetTerrain(), pWhiteMaterial);
		m_ppShaders[3] = pPointShader;

		CBillboardShader * pTrees = new CBillboardShader();
		pTrees->CreateShader(pd3dDevice);
		pTrees->BuildObjects(pd3dDevice, GetTerrain());
		m_ppShaders[4] = pTrees;

		CSceneShader * pSceneShader = new CSceneShader();
		pSceneShader->CreateShader(pd3dDevice);
		pSceneShader->BuildObjects(pd3dDevice, SceneInfo->ppMRTSRVArray, 0, SceneInfo->pd3dBackRTV);
		pSceneShader->CreateConstantBuffer(pd3dDevice, pd3dDeviceContext);
		m_pSceneShader = pSceneShader;
	}
	{
		m_ppShaders[3]->EntityAllStaticObjects();

		m_pPlayerShader = new CPlayerShader();
		m_pPlayerShader->CreateShader(pd3dDevice);
		m_pPlayerShader->BuildObjects(pd3dDevice, GetTerrain());

		SetCamera(m_pPlayerShader->GetPlayer()->GetCamera());
		m_pCamera->SetViewport(pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->GenerateViewMatrix();
	}
	CreateShaderVariables(pd3dDevice);
	BuildStaticShadowMap(pd3dDeviceContext);
}

void CSceneInGame::ReleaseObjects()
{
	CScene::ReleaseObjects();
	QUADMgr.ReleaseQuadTree();
}

bool CSceneInGame::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//마우스 캡쳐를 하고 현재 마우스 위치를 가져온다.
		//SetCapture(hWnd);
		GetCursorPos(&m_ptOldCursorPos);
		//bIsMouseDown = true;
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		//마우스 캡쳐를 해제한다.
		//ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		//bIsMouseDown = false;
		break;
	}

	return false;
}
void CSceneInGame::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));
	//게임 월드 전체를 비추는 주변조명을 설정한다.
	m_pLights->m_xcGlobalAmbient             = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);

	//3개의 조명(점 광원, 스팟 광원, 방향성 광원)을 설정한다.
	m_pLights->m_pLights[0].m_bEnable        = 1.0f;
	m_pLights->m_pLights[0].m_nType          = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[0].m_xcAmbient      = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[0].m_xcDiffuse      = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights->m_pLights[0].m_xcSpecular     = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xv3Direction   = XMFLOAT3(-0.707f, -0.707f, 0.0f);

	m_pLights->m_pLights[1].m_bEnable        = 1.0f;
	m_pLights->m_pLights[1].m_nType          = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange         = 100.0f;
	m_pLights->m_pLights[1].m_xcAmbient      = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xcDiffuse      = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[1].m_xcSpecular     = XMFLOAT4(0.3f, 0.3f, 0.3f, 5.0f);
	m_pLights->m_pLights[1].m_xv3Position    = XMFLOAT3(500.0f, 300.0f, 500.0f);
	m_pLights->m_pLights[1].m_xv3Direction   = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xv3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[1].m_fFalloff       = 8.0f;
	m_pLights->m_pLights[1].m_fPhi           = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[1].m_fTheta         = (float)cos(XMConvertToRadians(20.0f));

	m_pLights->m_pLights[2].m_bEnable        = 0.0f;//1.0f;
	m_pLights->m_pLights[2].m_nType          = POINT_LIGHT;
	m_pLights->m_pLights[2].m_fRange         = 300.0f;
	m_pLights->m_pLights[2].m_xcAmbient      = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_pLights->m_pLights[2].m_xcDiffuse      = XMFLOAT4(0.8f, 0.8f, 0.8f, 10.0f);
	m_pLights->m_pLights[2].m_xcSpecular     = XMFLOAT4(0.5f, 0.5f, 0.5f, 10.0f);
	m_pLights->m_pLights[2].m_xv3Position    = XMFLOAT3(1098, 210, 350);
	m_pLights->m_pLights[2].m_xv3Direction   = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[2].m_xv3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[3].m_bEnable        = 1.0f;
	m_pLights->m_pLights[3].m_nType          = SPOT_LIGHT;
	m_pLights->m_pLights[3].m_fRange         = 60.0f;
	m_pLights->m_pLights[3].m_xcAmbient      = XMFLOAT4(0.2f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[3].m_xcDiffuse      = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[3].m_xcSpecular     = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_xv3Position    = XMFLOAT3(500.0f, 300.0f, 500.0f);
	m_pLights->m_pLights[3].m_xv3Direction   = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_pLights->m_pLights[3].m_xv3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[3].m_fFalloff       = 20.0f;
	m_pLights->m_pLights[3].m_fPhi           = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[3].m_fTheta         = (float)cos(XMConvertToRadians(15.0f));

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(d3dBufferDesc));
	d3dBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth      = sizeof(LIGHTS);
	d3dBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem        = m_pLights;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dcbLights);
}

void CSceneInGame::ReleaseShaderVariables()
{
	if (m_pLights) delete m_pLights;
	if (m_pd3dcbLights) m_pd3dcbLights->Release();
}

void CSceneInGame::BuildStaticShadowMap(ID3D11DeviceContext * pd3dDeviceContext)
{
	// 방향성 광원은 위치 필요 없다.
	//LIGHT * pLight = m_pScene->GetLight(2);
	CHeightMapTerrain * pTerrain = GetTerrain();
	float fHalf = pTerrain->GetWidth() * 0.5;

	CShadowMgr * pSdwMgr = &ShadowMgr;
	pSdwMgr->BuildShadowMap(pd3dDeviceContext, XMFLOAT3(fHalf - 1000.0f, 0.0f, fHalf), XMFLOAT3(fHalf, fHalf, fHalf), fHalf);

	UINT uRenderState = (RS_SHADOWMAP);
	pSdwMgr->SetStaticShadowMap(pd3dDeviceContext, m_pCamera);

	m_ppShaders[1]->Render(pd3dDeviceContext, uRenderState, m_pCamera);
	m_ppShaders[4]->Render(pd3dDeviceContext, uRenderState, m_pCamera);

	pSdwMgr->ResetStaticShadowMap(pd3dDeviceContext, m_pCamera);
	//m_uRenderState = NULL;

	pSdwMgr->UpdateStaticShadowResource(pd3dDeviceContext);
	//pSdwMgr->UpdateDynamicShadowResource(m_pd3dDeviceContext);
	//m_pSceneShader->SetLightSRV(TXMgr.GetShaderResourceView("srv_StaticShaodwMap"));
}

void CSceneInGame::OnCreateShadowMap(ID3D11DeviceContext * pd3dDeviceContext)
{
	UINT uRenderState = (NOT_PSUPDATE | RS_SHADOWMAP);

	ShadowMgr.SetDynamicShadowMap(pd3dDeviceContext, m_pCamera);

	m_pPlayerShader->Render(pd3dDeviceContext, uRenderState, m_pCamera);
	m_ppShaders[2]->Render(pd3dDeviceContext, uRenderState, m_pCamera);

	ShadowMgr.ResetDynamicShadowMap(pd3dDeviceContext, m_pCamera);
	uRenderState = NULL;
}

void CSceneInGame::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbLights, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	LIGHTS *pcbLight = (LIGHTS *)d3dMappedResource.pData;
	memcpy(pcbLight, pLights, sizeof(LIGHTS));
	pd3dDeviceContext->Unmap(m_pd3dcbLights, 0);
	pd3dDeviceContext->PSSetConstantBuffers(CB_PS_SLOT_LIGHT, 1, &m_pd3dcbLights);
}

bool CSceneInGame::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	CMaterial * pMat = nullptr;

	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'Z':
			if (m_pLights->m_pLights[1].m_bEnable) m_pLights->m_pLights[1].m_bEnable = false;
			else m_pLights->m_pLights[1].m_bEnable = true;
			//((CParticleShader*)m_ppShaders[3])->SetParticle(1, &m_pPlayerShader->GetPlayer()->GetPosition());
			break;
		case 'N':
			pMat = MaterialMgr.GetObjects("White");
			pMat->m_Material.m_xcAmbient = XMFLOAT4(0, 0, 0, 0);
			pMat->m_Material.m_xcDiffuse = XMFLOAT4(0, 0, 0, 0);
			pMat = MaterialMgr.GetObjects("Terrain");
			pMat->m_Material.m_xcDiffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			pMat->m_Material.m_xcAmbient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			pMat->m_Material.m_xcSpecular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			pMat->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			break;
		case 'M':
			pMat = MaterialMgr.GetObjects("White");
			pMat->m_Material.m_xcAmbient = XMFLOAT4(1, 1, 1, 0);
			pMat->m_Material.m_xcDiffuse = XMFLOAT4(1, 1, 1, 0);

			pMat = MaterialMgr.GetObjects("Terrain");
			pMat->m_Material.m_xcDiffuse = XMFLOAT4(0.7f, 0.8f, 0.7f, 1.0f);
			pMat->m_Material.m_xcAmbient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
			pMat->m_Material.m_xcSpecular = XMFLOAT4(3.0f, 3.0f, 3.0f, 5.0f);
			pMat->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			break;
		case 'B':
			if (bIsKeyDown == false) bIsKeyDown = true;
			else bIsKeyDown = false;
			break;
			//case VK_SPACE:
			//	FRAMEWORK.ChangeGameScene(new CSceneTitle());
			//	break;
		}
		break;
	}
	return(false);
}

bool CSceneInGame::ProcessInput(HWND hWnd, float fFrameTime)
{
	static UCHAR pKeyBuffer[256];
	//GetKeyboardState(pKeyBuffer);

	CPlayer * pPlayer = m_pCamera->GetPlayer();
	{
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
		if (GetCapture() == hWnd)
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
		if (dwDirection || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				/*cxDelta는 y-축의 회전을 나타내고 cyDelta는 x-축의 회전을 나타낸다. 오른쪽 마우스 버튼이 눌려진 경우 cxDelta는 z-축의 회전을 나타낸다.*/
				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
					pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
			/*플레이어를 dwDirection 방향으로 이동한다(실제로는 속도 벡터를 변경한다). 이동 거리는 시간에 비례하도록 한다. 플레이어의 이동 속력은 (50/초)로 가정한다. 만약 플레이어의 이동 속력이 있다면 그 값을 사용한다.*/
			if (dwDirection)
				pPlayer->Move(dwDirection, 50.0f * fFrameTime, true);
		}
		//플레이어를 실제로 이동하고 카메라를 갱신한다. 중력과 마찰력의 영향을 속도 벡터에 적용한다.
		pPlayer->Update(fFrameTime);
	}

	if (pKeyBuffer[VK_SPACE] & 0xF0)
	{
		FRAMEWORK.ChangeGameScene(new CSceneTitle());

		ZeroMemory(pKeyBuffer, 256);
		SetKeyboardState(pKeyBuffer);
	}
	return false;
}

void CSceneInGame::AnimateObjects(float fTimeElapsed)
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
		//XMStoreFloat3(&m_pLights->m_pLights[0].m_xv3Position, XMVectorAdd(xvTerrainCenter, xvRotated));
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
#ifdef _QUAD_TREE
	QUADMgr.Update(m_pCamera);
#endif
}

void CSceneInGame::Render(ID3D11DeviceContext*pd3dDeviceContext, RENDER_INFO * pRenderInfo)
{
	//UpdateLights(pd3dDeviceContext);
	//CPlayer * pPlayer = m_pCamera->GetPlayer();
#ifdef _THREAD
	int index = pRenderInfo->ThreadID;
	//if (index == 3 && bIsKeyDown == true)
	//	return;
	if (index == m_nShaders - 1)
	{
		m_pPlayerShader->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
	}
//#ifndef _DEBUG
	//if (index == m_nThread - 1)
	//{
	//	UpdateLights(pd3dDeviceContext);
	//	pd3dDeviceContext->OMSetRenderTargets(1, &(pRenderInfo->ppd3dMrtRTV[MRT_SCENE]), nullptr);
	//	ShadowMgr.UpdateStaticShadowResource(pd3dDeviceContext);
	//	m_pSceneShader->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
	//	return;
	//}
//#endif
	m_ppShaders[index]->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
#else
	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
	}
#endif
	//m_ppShaders[m_nShaders - 1]->Render(pd3dDeviceContext, pCamera);
}

void CSceneInGame::UIRender(ID3D11DeviceContext * pd3dDeviceContext)
{
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

CHeightMapTerrain *CSceneInGame::GetTerrain()
{
	CTerrainShader *pTerrainShader = (CTerrainShader *)m_ppShaders[1];
	return(pTerrainShader->GetTerrain());
}