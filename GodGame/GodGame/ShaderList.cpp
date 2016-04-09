#include "stdafx.h"
#include "MyInline.h"
#include "ShaderList.h"
#include <D3Dcompiler.h>

/////////////////////////////////////////////////////////////////////////////

///////////////////////////////
#pragma region InstancingShader
CInstancingShader::CInstancingShader() : CInstanceShader(), CTexturedShader()
{
	m_nInstanceBufferOffset = 0;
	m_nInstanceBufferStride = 0;

	m_pd3dCubeInstanceBuffer = nullptr;
	m_pd3dSphereInstanceBuffer = nullptr;

	m_pMaterial = nullptr;
	m_pTexture = nullptr;
}

CInstancingShader::~CInstancingShader()
{
	if (m_pd3dCubeInstanceBuffer) m_pd3dCubeInstanceBuffer->Release();
	if (m_pd3dSphereInstanceBuffer) m_pd3dSphereInstanceBuffer->Release();

	if (m_pMaterial) m_pMaterial->Release();
	if (m_pTexture) m_pTexture->Release();
}

void CInstancingShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEPOS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEPOS", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEPOS", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSInstancedTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSInstancedTexturedColor", "ps_5_0", &m_pd3dPixelShader);
}

void CInstancingShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial *pMaterial, CTexture *pTexture, int k)
{
	m_pMaterial = pMaterial;
	if (pMaterial) pMaterial->AddRef();

	m_pTexture = pTexture;
	if (pTexture) pTexture->AddRef();

	m_nInstanceBufferStride = sizeof(VS_VB_INSTANCE);
	m_nInstanceBufferOffset = 0;

	CCubeMeshTextured *pCubeMesh = new CCubeMeshTextured(pd3dDevice, 12.0f, 12.0f, 12.0f);
	//CSphereMeshTextured *pSphereMesh = new CSphereMeshTextured(pd3dDevice, 12.0f, 20, 20);

	float fxPitch = 18.0f * 3.5f;
	float fyPitch = 18.0f * 3.5f;
	float fzPitch = 18.0f * 3.5f;

	float fTerrainWidth = pHeightMapTerrain->GetWidth();
	float fTerrainLength = pHeightMapTerrain->GetLength();

	/*두 가지(직육면체와 구) 객체들을 지형에 일정한 간격으로 배치한다. 지형의 표면에 직육면체를 배치하고 직육면체 위에 구가 배치된다. 직육면체와 구는 빨강색, 녹색, 파랑색이 반복되도록 배치된다.*/
	int xObjects = int(fTerrainWidth / (fxPitch * 3.0f)), yObjects = 2, zObjects = int(fTerrainLength / (fzPitch * 3.0f)), i = 0;
	m_nObjects = xObjects * zObjects;

	m_ppObjects = new CGameObject*[m_nObjects];

	XMFLOAT3 xv3RotateAxis;
	CRotatingObject *pRotatingObject = nullptr;

	///*구는 3가지 종류(재질에 따라)이다. 다른 재질의 구들이 번갈아 나열되도록 한다. 재질의 종류에 따라 k가 0, 1, 2의 값을 가지고 k에 따라 객체의 위치를 다르게 설정한다.*/
	for (int x = 0; x < xObjects; x++)
	{
		for (int z = 0; z < zObjects; z++)
		{
			pRotatingObject = new CRotatingObject();
			pRotatingObject->SetMesh(pCubeMesh);
			float xPosition = (k * fxPitch) + (x * fxPitch * 3.0f);
			float zPosition = (k * fzPitch) + (z * fxPitch * 3.0f);
			float fHeight = pHeightMapTerrain->GetHeight(xPosition, zPosition);
			pRotatingObject->SetPosition(xPosition, fHeight + (fyPitch * 2), zPosition);
			pRotatingObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
			pRotatingObject->SetRotationSpeed(36.0f * (i % 10) + 36.0f);
			pRotatingObject->AddRef();
			m_ppObjects[i++] = pRotatingObject;
		}
	}
	//pRotatingObject = new CRotatingObject();
	//pRotatingObject->SetMesh(pSphereMesh);
	//pRotatingObject->SetPosition(1105, 270, 1290);
	//m_ppObjects[0] = pRotatingObject;

	//pRotatingObject = new CRotatingObject();
	//pRotatingObject->SetMesh(pSphereMesh);
	//pRotatingObject->SetPosition(1050, 270, 1190);
	//m_ppObjects[1] = pRotatingObject;

	m_pd3dCubeInstanceBuffer = CreateInstanceBuffer(pd3dDevice, m_nObjects, m_nInstanceBufferStride, nullptr);
	//m_pd3dSphereInstanceBuffer = CreateInstanceBuffer(pd3dDevice, xObjects * zObjects, m_nInstanceBufferStride, nullptr);
	pCubeMesh->AssembleToVertexBuffer(1, &m_pd3dCubeInstanceBuffer, &m_nInstanceBufferStride, &m_nInstanceBufferOffset);

	/*for (int x = 0; x < xObjects; x++)
	{
	for (int z = 0; z < zObjects; z++)
	{
	pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pCubeMesh);
	float xPosition = (k * fxPitch) + (x * fxPitch * 3.0f);
	float zPosition = (k * fzPitch) + (z * fxPitch * 3.0f);
	float fHeight = pHeightMapTerrain->GetHeight(xPosition, zPosition);
	pRotatingObject->SetPosition(xPosition, fHeight + 6.0f, zPosition);
	XMFLOAT3 xv3SurfaceNormal = pHeightMapTerrain->GetNormal(xPosition, zPosition);
	xv3ec3Cross(&xv3RotateAxis, &XMFLOAT3(0.0f, 1.0f, 0.0f), &xv3SurfaceNormal);
	float fAngle = acos(xv3ec3Dot(&XMFLOAT3(0.0f, 1.0f, 0.0f), &xv3SurfaceNormal));
	pRotatingObject->Rotate(&xv3RotateAxis, (float)D3DXToDegree(fAngle));
	pRotatingObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	pRotatingObject->SetRotationSpeed(18.0f * (i % 10) + 10.0f);
	m_ppObjects[i++] = pRotatingObject;
	}
	}
	m_pd3dCubeInstanceBuffer = CreateInstanceBuffer(pd3dDevice, xObjects * zObjects, m_nInstanceBufferStride, nullptr);
	pCubeMesh->AssembleToVertexBuffer(1, &m_pd3dCubeInstanceBuffer, &m_nInstanceBufferStride, &m_nInstanceBufferOffset);*/
}

void CInstancingShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);
	if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);
	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);

	int nCubeObjects = m_nObjects;

	int nCubeInstances = 0;
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dCubeInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	VS_VB_INSTANCE *pnCubeInstances = (VS_VB_INSTANCE *)d3dMappedResource.pData;
	for (int j = 0; j < nCubeObjects; j++)
	{
		if (m_ppObjects[j]->IsVisible(pCamera))
		{
			Chae::XMFloat4x4Transpose(&pnCubeInstances[nCubeInstances].m_d3dxTransform, &m_ppObjects[j]->m_xmf44World); // XMFLOAT4X4Transpose(&pnSphereInstances[nSphereInstances++].m_d3dxTransform, &m_ppObjects[j]->m_xmf44World);
			nCubeInstances++;
		}
	}
	pd3dDeviceContext->Unmap(m_pd3dCubeInstanceBuffer, 0);

	CMesh *pSphereMesh = m_ppObjects[0]->GetMesh();
	pSphereMesh->RenderInstanced(pd3dDeviceContext, uRenderState, nCubeInstances, 0);

	//int nCubeInstances = 0;
	//pd3dDeviceContext->Map(m_pd3dCubeInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	//VS_VB_INSTANCE *pCubeInstances = (VS_VB_INSTANCE *)d3dMappedResource.pData;
	//for (int j = nSphereObjects; j < m_nObjects; j++)
	//{
	//	if (m_ppObjects[j])
	//	{
	//		if (m_ppObjects[j]->IsVisible(pCamera))
	//		{
	//			XMFLOAT4X4Transpose(&pCubeInstances[nCubeInstances++].m_d3dxTransform, &m_ppObjects[j]->m_xmf44World);
	//		}
	//	}
	//}
	//pd3dDeviceContext->Unmap(m_pd3dCubeInstanceBuffer, 0);

	//CMesh *pCubeMesh = m_ppObjects[m_nObjects - 1]->GetMesh();
	//pCubeMesh->RenderInstanced(pd3dDeviceContext, nCubeInstances, 0);
}

#pragma endregion

#pragma region BillboardShader
CBillboardShader::CBillboardShader() : CShader(), CInstanceShader()
{
	m_pTexture = nullptr;

	m_nTrees = 0;
	m_pd3dTreeInstanceBuffer = nullptr;
}

CBillboardShader::~CBillboardShader()
{
	if (m_pd3dTreeInstanceBuffer) m_pd3dTreeInstanceBuffer->Release();
	if (m_pTexture) m_pTexture->Release();
}

void CBillboardShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTANCE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/BillBoard.fx", "VSBillboard", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/BillBoard.fx", "PSBillboardTextureArray", "ps_5_0", &m_pd3dPixelShader);
	CreateGeometryShaderFromFile(pd3dDevice, L"fx/BillBoard.fx", "GSBillboard", "gs_5_0", &m_pd3dGeometryShader);
}

void CBillboardShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain)
{
	m_nObjects = m_nTrees = 100;

	XMFLOAT3 xmf3Pos;
	XMFLOAT2 xmf2Size = XMFLOAT2(40, 40);

	m_nInstanceBufferStride = sizeof(VS_VB_WORLD_POSITION);
	m_nInstanceBufferOffset = 0;

	CBillboardObject * pTree = nullptr;
	m_ppObjects = new CGameObject*[m_nObjects];

	CHeightMapTerrain * pTerrain = pHeightMapTerrain;
	int cxTerrain = pTerrain->GetWidth();
	int czTerrain = pTerrain->GetLength();

	CBillBoardVertex * pTreeMesh = new CBillBoardVertex(pd3dDevice, xmf2Size.x, xmf2Size.y);
	for (int i = 0; i < m_nTrees; ++i) 
	{
		float fxTerrain = xmf3Pos.x = rand() % cxTerrain;
		float fzTerrain = xmf3Pos.z = rand() % czTerrain;
		xmf3Pos.y = pTerrain->GetHeight(fxTerrain, fzTerrain, false) + 18;
		pTree = new CBillboardObject(xmf3Pos, i, xmf2Size);
		pTree->SetMesh(pTreeMesh);
		pTree->SetCollide(true);
		pTree->AddRef();
		m_ppObjects[i] = pTree;
	}
	m_ppObjects[0]->SetPosition(XMFLOAT3(1006, 200, 308));

	m_pd3dTreeInstanceBuffer = CreateInstanceBuffer(pd3dDevice, m_nTrees, m_nInstanceBufferStride, nullptr);
	pTreeMesh->AssembleToVertexBuffer(1, &m_pd3dTreeInstanceBuffer, &m_nInstanceBufferStride, &m_nInstanceBufferOffset);

	m_pTexture = new CTexture(1, 1, TX_SLOT_TEXTURE_ARRAY, 0, SET_SHADER_PS);
	// 크기 동일'
	//ID3D11ShaderResourceView * pd3dsrvArray = nullptr;
	ID3D11ShaderResourceView * pd3dsrvArray = CTexture::CreateTexture2DArraySRV(pd3dDevice, _T("../Assets/Image/Objects/bill"), _T("png"), 4);

	m_pTexture->SetTexture(0, pd3dsrvArray);
	m_pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));

	pd3dsrvArray->Release();

	EntityAllStaticObjects();
}

void CBillboardShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);
	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);

	if (uRenderState & RS_SHADOWMAP)
	{
		AllRender(pd3dDeviceContext, uRenderState, pCamera);
		return;
	}
	//	pCamera->UpdateCameraPositionCBBuffer(pd3dDeviceContext);

	int nTreeInstance = 0;
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dTreeInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	XMFLOAT4 *pnTreeInstances = (XMFLOAT4 *)d3dMappedResource.pData;

	CBillboardObject * pTree = nullptr;
	//XMFLOAT4 xmfInstanceData;
	for (int j = 0; j < m_nTrees; ++j)
	{
		pTree = (CBillboardObject*)m_ppObjects[j];

		//pTree->SetActive(true);
		if (pTree->IsVisible())
		{
			pnTreeInstances[nTreeInstance] = pTree->GetInstanceData();
		//	printf("%0.2f %0.2f %0.2f \n", pnTreeInstances[nTreeInstance].m_xv3Position.x, pnTreeInstances[nTreeInstance].m_xv3Position.y, pnTreeInstances[nTreeInstance].m_xv3Position.z);
			nTreeInstance++;
		}
	}
	pd3dDeviceContext->Unmap(m_pd3dTreeInstanceBuffer, 0);

	CMesh *pTreeMesh = m_ppObjects[0]->GetMesh();
	pTreeMesh->RenderInstanced(pd3dDeviceContext, uRenderState, nTreeInstance, 0);
}

void CBillboardShader::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera)
{
	//D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	//pd3dDeviceContext->Map(m_pd3dcbCameraPos, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	//VS_CB_CAMERAPOS *pcbViewProjection = (VS_CB_CAMERAPOS *)d3dMappedResource.pData;
	//pd3dDeviceContext->Unmap(m_pd3dcbCameraPos, 0);
	//pd3dDeviceContext->VSSetConstantBuffers(CB_SLOT_CAMERAPOS, 1, &m_pd3dcbCameraPos);

	////월드 변환 행렬을 상수 버퍼에 복사한다.
	//D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	//pd3dDeviceContext->Map(m_pd3dcbWorldMatrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	//VS_CB_CAMERAPOS *pcbWorldMatrix = (VS_CB_WORLD_MATRIX *)d3dMappedResource.pData;
	//Chae::XMFloat4x4Transpose(&pcbWorldMatrix->m_d3dxTransform, pxmtxWorld); //XMFLOAT4X4Transpose(&pcbWorldMatrix->m_d3dxTransform, pxmtxWorld);
	//pd3dDeviceContext->Unmap(m_pd3dcbWorldMatrix, 0);

	////상수 버퍼를 디바이스의 슬롯(CB_SLOT_WORLD_MATRIX)에 연결한다.
	//pd3dDeviceContext->VSSetConstantBuffers(CB_SLOT_WORLD_MATRIX, 1, &m_pd3dcbWorldMatrix);
}

void CBillboardShader::AnimateObjects(float fTimeElapsed)
{
}
void CBillboardShader::AllRender(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dTreeInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	XMFLOAT4 *pnTreeInstances = (XMFLOAT4 *)d3dMappedResource.pData;

	CBillboardObject * pTree = nullptr;
	XMFLOAT4 xmfInstanceData;
	for (int j = 0; j < m_nTrees; ++j)
	{
		pTree = (CBillboardObject*)m_ppObjects[j];
		pnTreeInstances[j] = pTree->GetInstanceData();
	}
	pd3dDeviceContext->Unmap(m_pd3dTreeInstanceBuffer, 0);

	CMesh *pTreeMesh = m_ppObjects[0]->GetMesh();
	pTreeMesh->RenderInstanced(pd3dDeviceContext, uRenderState, m_nTrees, 0);
}
#pragma endregion

CStaticShader::CStaticShader() : CShader()
{
	//m_pTexture = nullptr;
	m_pMaterial = nullptr;
}

CStaticShader::~CStaticShader()
{
	//if (m_pTexture) m_pTexture->Release();
	if (m_pMaterial) m_pMaterial->Release();
}

void CStaticShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);

	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSNormalAndSF", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSNormalAndSF", "ps_5_0", &m_pd3dPixelShader);

	//D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	//{
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	//};
	//UINT nElements = ARRAYSIZE(d3dInputElements);
	//CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	//CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader);
	//CreateShaderVariables(pd3dDevice);
}

void CStaticShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr)
{
	m_pMaterial = pMaterial;
	if (pMaterial) pMaterial->AddRef();

	TEXTURE_MGR & txmgr = SceneMgr.mgrTexture;
	MESH_MGR & meshmgr = SceneMgr.mgrMesh;

	m_nObjects = 2;
	m_ppObjects = new CGameObject*[m_nObjects];

	char ManagerNames[2][50] = { { "scene_skull" }, {"scene_warrok"} };//, { "scene_man_death" }, { "scene_player_0" }, { "scene_skull_0" }};
	//int Pos[5][2] = { {1085, 220}, {1085, 260}, {} };

	CGameObject *pObject = nullptr;

	float fHeight = 0;
	//fHeight = pHeightMapTerrain->GetHeight(1085, 220, true);
	//m_ppObjects[0] = new CGameObject(1);
	//m_ppObjects[0]->AddRef();
	//m_ppObjects[0]->SetPosition(1085, fHeight + 5, 220);//(1105, 200, 250);

	fHeight = pHeightMapTerrain->GetHeight(1085, 260, true);
	CMonster * pAnimatedObject = new CMonster(1);
	pAnimatedObject->SetAnimationCycleTime(0, 2.0f);
	pAnimatedObject->SetMesh(meshmgr.GetObjects("scene_skull_0"));
	
	m_ppObjects[0] = pAnimatedObject;
	m_ppObjects[0]->SetPosition(1085, fHeight, 260);
	m_ppObjects[0]->AddRef();


	char AnimNames[6][50] = { "scene_warrok_idle", "scene_warrok_run", "scene_warrok_roar", "scene_warrok_punch", "scene_warrok_swiping", "scene_warrok_death"};

	fHeight = pHeightMapTerrain->GetHeight(1115, 275, false);
	pAnimatedObject = new CWarrock(CWarrock::eANI_WARROCK_ANIM_NUM);

	for (int i = 0; i < 6; ++i)
		pAnimatedObject->SetMesh(meshmgr.GetObjects(AnimNames[i]), i);

	pAnimatedObject->InitializeAnimCycleTime();
	//static_cast<CWarrock*>(pAnimatedObject)->BuildObject(pAnimatedObject);

	m_ppObjects[1] = pAnimatedObject;
	m_ppObjects[1]->SetPosition(1115, fHeight, 275);
	m_ppObjects[1]->Rotate(0, 90.f, 0);
	m_ppObjects[1]->AddRef();

	//fHeight = pHeightMapTerrain->GetHeight(1140, 255, false);
	//pAnimatedObject = new CAnimatedObject(1);
	//pAnimatedObject->SetAnimationCycleTime(0, 2.0f);
	//m_ppObjects[3] = pAnimatedObject;
	//m_ppObjects[3]->SetPosition(1140, fHeight + 5, 255);
	//m_ppObjects[3]->AddRef();
	////m_ppObjects[3]->SetPosition(1100, 170, 255);
	//
	//fHeight = pHeightMapTerrain->GetHeight(1180, 255, false);
	//pAnimatedObject = new CAnimatedObject(1);
	//pAnimatedObject->SetAnimationCycleTime(0, 2.0f);
	//m_ppObjects[4] = pAnimatedObject;
	//m_ppObjects[4]->SetPosition(1180, fHeight, 255);
	//m_ppObjects[4]->AddRef();


	for (int i = 0; i < m_nObjects; ++i)
	{
		m_ppObjects[i]->SetTexture(txmgr.GetObjects(ManagerNames[i]));

		CCharacter * pAnimObject = nullptr;
		if (pAnimObject = dynamic_cast<CCharacter*>(m_ppObjects[i]))
		{
			pAnimObject->SetCollide(true);
			pAnimObject->UpdateFramePerTime();
			pAnimObject->SetUpdatedContext(pHeightMapTerrain);
		}
	}
	/// 이상 스테틱 객체들
}

void CStaticShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	ID3D11ShaderResourceView * pd3dNullSRV[] = { nullptr, nullptr };
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);

	for (int j = 0; j < m_nObjects; j++)
	{
		//카메라의 절두체에 포함되는 객체들만을 렌더링한다.
		//m_ppObjects[j]->SetActive(true);
		if (m_ppObjects[j]->IsVisible(pCamera))
		{
			pd3dDeviceContext->PSSetShaderResources(2, 2, pd3dNullSRV);
			m_ppObjects[j]->Render(pd3dDeviceContext, uRenderState, pCamera);
		}
	}
}

void CStaticShader::GetGameMessage(CShader * byObj, eMessage eMSG, void * extra)
{
	if (eMSG == eMessage::MSG_PASS_PLAYERPTR)
	{
		for (int i = 0; i < m_nObjects; ++i)
			static_cast<CMonster*>(m_ppObjects[i])->BuildObject(static_cast<CCharacter*>(extra));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
CCharacterShader::CCharacterShader()
{
	m_pMaterial = nullptr;
}

CCharacterShader::~CCharacterShader()
{
	if (m_pMaterial) m_pMaterial->Release();
}

void CCharacterShader::CreateShader(ID3D11Device * pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);

	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSNormalAndSF", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSNormalAndSF", "ps_5_0", &m_pd3dPixelShader);
}

void CCharacterShader::BuildObjects(ID3D11Device * pd3dDevice, CHeightMapTerrain * pHeightMapTerrain, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr)
{
	m_pMaterial = pMaterial;
	if (pMaterial) pMaterial->AddRef();

	TEXTURE_MGR & txmgr = SceneMgr.mgrTexture;
	MESH_MGR & meshmgr = SceneMgr.mgrMesh;

	m_nObjects = 2;
	m_ppObjects = new CGameObject*[m_nObjects];

	char ManagerNames[2][50] = { { "scene_skull" },{ "scene_warrok" } };//, { "scene_man_death" }, { "scene_player_0" }, { "scene_skull_0" }};
																		//int Pos[5][2] = { {1085, 220}, {1085, 260}, {} };
	CGameObject *pObject = nullptr;

	float fHeight = 0;
	//fHeight = pHeightMapTerrain->GetHeight(1085, 220, true);
	//m_ppObjects[0] = new CGameObject(1);
	//m_ppObjects[0]->AddRef();
	//m_ppObjects[0]->SetPosition(1085, fHeight + 5, 220);//(1105, 200, 250);

	fHeight = pHeightMapTerrain->GetHeight(1085, 260, true);
	CMonster * pAnimatedObject = new CSkeleton(1);
	pAnimatedObject->SetAnimationCycleTime(0, 2.0f);
	pAnimatedObject->SetMesh(meshmgr.GetObjects("scene_skull_0"));
	cout << "Skeleton Size : " << pAnimatedObject->GetSize() << endl;
	pAnimatedObject->SetSize(10);

	m_ppObjects[0] = pAnimatedObject;
	m_ppObjects[0]->SetPosition(1085, fHeight, 260);
	m_ppObjects[0]->AddRef();

	char AnimNames[6][50] = { "scene_warrok_idle", "scene_warrok_run", "scene_warrok_roar", "scene_warrok_punch", "scene_warrok_swiping", "scene_warrok_death" };

	fHeight = pHeightMapTerrain->GetHeight(1115, 275, false);
	pAnimatedObject = new CWarrock(CWarrock::eANI_WARROCK_ANIM_NUM);

	for (int i = 0; i < 6; ++i)
		pAnimatedObject->SetMesh(meshmgr.GetObjects(AnimNames[i]), i);

	cout << "Size : " << pAnimatedObject->GetSize() << endl;

	pAnimatedObject->InitializeAnimCycleTime();
	//static_cast<CWarrock*>(pAnimatedObject)->BuildObject(pAnimatedObject);

	m_ppObjects[1] = pAnimatedObject;
	m_ppObjects[1]->SetPosition(1115, fHeight, 275);
	m_ppObjects[1]->Rotate(0, 90.f, 0);
	m_ppObjects[1]->AddRef();

	//fHeight = pHeightMapTerrain->GetHeight(1140, 255, false);
	//pAnimatedObject = new CAnimatedObject(1);
	//pAnimatedObject->SetAnimationCycleTime(0, 2.0f);
	//m_ppObjects[3] = pAnimatedObject;
	//m_ppObjects[3]->SetPosition(1140, fHeight + 5, 255);
	//m_ppObjects[3]->AddRef();
	////m_ppObjects[3]->SetPosition(1100, 170, 255);
	//
	//fHeight = pHeightMapTerrain->GetHeight(1180, 255, false);
	//pAnimatedObject = new CAnimatedObject(1);
	//pAnimatedObject->SetAnimationCycleTime(0, 2.0f);
	//m_ppObjects[4] = pAnimatedObject;
	//m_ppObjects[4]->SetPosition(1180, fHeight, 255);
	//m_ppObjects[4]->AddRef();


	for (int i = 0; i < m_nObjects; ++i)
	{
		m_ppObjects[i]->SetTexture(txmgr.GetObjects(ManagerNames[i]));

		CCharacter * pAnimObject = nullptr;
		if (pAnimObject = dynamic_cast<CCharacter*>(m_ppObjects[i]))
		{
			pAnimObject->UpdateFramePerTime();
			pAnimObject->SetUpdatedContext(pHeightMapTerrain);
		}
	}
	/// 이상 스테틱 객체들
	EntityAllDynamicObjects();
}


void CCharacterShader::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	ID3D11ShaderResourceView * pd3dNullSRV[] = { nullptr, nullptr };
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);

	for (int j = 0; j < m_nObjects; j++)
	{
		//카메라의 절두체에 포함되는 객체들만을 렌더링한다.
		//m_ppObjects[j]->SetActive(true);
		if (m_ppObjects[j]->IsVisible())
		{
			//cout << j << " BB : " << m_ppObjects[j]->m_bcMeshBoundingCube << endl;
			pd3dDeviceContext->PSSetShaderResources(2, 2, pd3dNullSRV);
			m_ppObjects[j]->Render(pd3dDeviceContext, uRenderState, pCamera);
		}
	}
}

void CCharacterShader::GetGameMessage(CShader * byObj, eMessage eMSG, void * extra)
{
	if (eMSG == eMessage::MSG_PASS_PLAYERPTR)
	{
		for (int i = 0; i < m_nObjects; ++i)
			static_cast<CMonster*>(m_ppObjects[i])->BuildObject(static_cast<CCharacter*>(extra));
	}
}


#pragma region PointInstanceShader
CPointInstanceShader::CPointInstanceShader() : CShader(), CInstanceShader()
{
	m_pMaterial = nullptr;
	m_pTexture = nullptr;
	m_pd3dCubeInstanceBuffer = nullptr;

	m_nCubes = 0;
	//m_pd3dVertexBuffer = nullptr;
}

CPointInstanceShader::~CPointInstanceShader()
{
	if (m_pd3dCubeInstanceBuffer) m_pd3dCubeInstanceBuffer->Release();
	if (m_pTexture) m_pTexture->Release();
	if (m_pMaterial) m_pMaterial->Release();
}

void CPointInstanceShader::CreateShader(ID3D11Device *pd3dDevice)
{
#ifdef DRAW_GS_SPHERE
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "INFO",		 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/BillBoard.fx", "VSPointSphereInstance", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/BillBoard.fx", "PSPointInstance", "ps_5_0", &m_pd3dPixelShader);
	CreateGeometryShaderFromFile(pd3dDevice, L"fx/BillBoard.fx", "GSPointSphereInstance", "gs_5_0", &m_pd3dGeometryShader);
#else
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTANCE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);
	CreateVertexShaderFromFile(pd3dDevice,   L"fx/BillBoard.fx", "VSBillboard", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice,    L"fx/BillBoard.fx", "PSBillboardColor", "ps_5_0", &m_pd3dPixelShader);
	CreateGeometryShaderFromFile(pd3dDevice, L"fx/BillBoard.fx", "GSBillboardColor", "gs_5_0", &m_pd3dGeometryShader);
#endif
}

void CPointInstanceShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial * pMaterial)
{
	CMaterial * pMat              = new CMaterial();
	pMat->m_Material.m_xcAmbient  = XMFLOAT4(100, 100, 100, 10);
	pMat->m_Material.m_xcDiffuse  = XMFLOAT4(100, 100, 100, 10);
	pMat->m_Material.m_xcSpecular = XMFLOAT4(100, 100, 100, 10);
	pMat->m_Material.m_xcEmissive = XMFLOAT4(100, 100, 100, 10);

	m_pMaterial = pMat;//pMaterial;
	pMat->AddRef();//if (pMaterial) pMaterial->AddRef();
	m_nObjects = m_nCubes = (ELEMENT_NUM * 100);

//	XMFLOAT3 xmf3Pos;
	XMFLOAT2 xmf2Size(6, 6);

	//m_nInstanceBufferStride = sizeof(VS_VB_WORLD_POSITION);
	m_nInstanceBufferStride = sizeof(XMFLOAT4);
	m_nInstanceBufferOffset = 0;

	m_ppObjects = new CGameObject*[m_nObjects];

	CHeightMapTerrain * pTerrain = pHeightMapTerrain;
	int cxTerrain = pTerrain->GetWidth();
	int czTerrain = pTerrain->GetLength();
#ifdef DRAW_GS_SPHERE
	CPointSphereMesh * pPointMesh = new CPointSphereMesh(pd3dDevice, 20, 5);
#else
	CBillBoardVertex * pPointMesh = new CBillBoardVertex(pd3dDevice, 6, 6);
#endif
	CAbsorbMarble *pObject = nullptr;
	for (int i = 0; i < m_nObjects; i++)
	{
		float fx = rand() % cxTerrain;
		float fz = rand() % czTerrain;
		float fy = pTerrain->GetHeight(fx, fz) + 10;
#ifdef DRAW_GS_SPHERE
		pObject = new CGameObject(1);//CBillboardObject(XMFLOAT3(fx, fy, fz), 0, XMFLOAT2(5, 5) );
#else
		pObject = new CAbsorbMarble(XMFLOAT3(fx, fy, fz), (i % ELEMENT_NUM), xmf2Size);
#endif
		pObject->SetMesh(pPointMesh);
		pObject->Initialize();
		//pObject->SetMaterial(pMat);	
		//pObject->SetPosition(fx, fy, fz);
		pObject->AddRef();
		m_ppObjects[i] = pObject;

		//QUADMgr.EntityStaticObject(pObject);
	}
	m_pd3dCubeInstanceBuffer = CreateInstanceBuffer(pd3dDevice, m_nCubes, m_nInstanceBufferStride, nullptr);
	pPointMesh->AssembleToVertexBuffer(1, &m_pd3dCubeInstanceBuffer, &m_nInstanceBufferStride, &m_nInstanceBufferOffset);

	m_pTexture = new CTexture(1, 1, 0, 0);
	ID3D11ShaderResourceView * pd3dsrvArray;
	//ID3D11ShaderResourceView * pd3dsrvArray = m_pTexture->CreateTexture2DArraySRV(pd3dDevice, _T("../Assets/Image/Objects/bill"), 1);
	ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Objects/lightsphere1.png"), nullptr, nullptr, &pd3dsrvArray, nullptr));

	m_pTexture->SetTexture(0, pd3dsrvArray);
	m_pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));

	pd3dsrvArray->Release();

	EntityAllStaticObjects();
}

void CPointInstanceShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
	if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);

	int nCubeInstance = 0;
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dCubeInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	XMFLOAT4 *pnTreeInstance = (XMFLOAT4 *)d3dMappedResource.pData;

	XMFLOAT3 xmf3Pos;

#ifdef DRAW_GS_SPHERE
	CBillboardObject * pTree = nullptr;
	for (int j = 0; j < m_nCubes; j++)
	{
#ifdef _QUAD_TREE
		if (m_ppObjects[j]->IsVisible(nullptr))// if (m_ppObjects[j]->IsVisible(pCamera))//			{
#else
		if (m_ppObjects[j]->IsVisible(pCamera))
#endif
		{
			pTree = (CBillboardObject*)m_ppObjects[j];
			pnTreeInstance[nCubeInstance] = pTree->GetInstanceData();
			//printf("%0.2f %0.2f %0.2f \n", xmf3Pos.x, xmf3Pos.y, xmf3Pos.z);
			nCubeInstance++;
		}
		m_ppObjects[j]->SetActive(false);
	}
#else
	CBillboardObject * pTree = nullptr;
	for (int j = 0; j < m_nCubes; ++j)
	{
		pTree = (CBillboardObject*)m_ppObjects[j];

#ifdef _QUAD_TREE
		if (pTree->IsVisible(nullptr))// if (m_ppObjects[j]->IsVisible(pCamera))//			{
#else
		if (pTree->IsVisible(pCamera))
#endif
		{
			pnTreeInstance[nCubeInstance] = pTree->GetInstanceData();
			nCubeInstance++;
		}
		m_ppObjects[j]->SetActive(false);
	}
#endif
	pd3dDeviceContext->Unmap(m_pd3dCubeInstanceBuffer, 0);

//	cout << nCubeInstance << "개 그렸습니다." << endl;

	CMesh *pCubeMesh = m_ppObjects[0]->GetMesh();
	pCubeMesh->RenderInstanced(pd3dDeviceContext, uRenderState, nCubeInstance, 0);
}

void CPointInstanceShader::AnimateObjects(float fTimeElapsed)
{
	/*	
	if (GetAsyncKeyState('O') & 0x0001)
	{
		m_ppObjects[0]->SetPosition(XMFLOAT3(8, 0, 0));
	}
	else if (GetAsyncKeyState('P') & 0x0001)
	{
		m_ppObjects[0]->SetPosition(XMFLOAT3(1098, 190, 350));
	}
	*/
	//CBillboardObject * pTree = nullptr;
	for (int i = 0; i < m_nCubes; ++i)
	{
		m_ppObjects[i]->Animate(fTimeElapsed);
	}
}
#pragma endregion

CNormalShader::CNormalShader()
{
	m_pMaterial = nullptr;
	m_pTexture = nullptr;
}

CNormalShader::~CNormalShader()
{
}

void CNormalShader::CreateShader(ID3D11Device *pd3dDevice)
{
	CNormalMapShader::CreateShader(pd3dDevice);

	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	//CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSNormalMap", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	//CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSNormalMap", "ps_5_0", &m_pd3dPixelShader);

	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSBump", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreateHullShaderFromFile(pd3dDevice, L"fx/Effect.fx", "HSBump", "hs_5_0", &m_pd3dHullShader);
	CreateDomainShaderFromFile(pd3dDevice, L"fx/Effect.fx", "DSBump", "ds_5_0", &m_pd3dDomainShader);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSNormalMap", "ps_5_0", &m_pd3dPixelShader);
}

void CNormalShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial * pMaterial)
{
	m_Bump.m_fBumpMax = 30.0f;
	m_Bump.m_xv3BumpScale = XMFLOAT3(30.0f, 30.f, 30.0f);

	CHeightMapTerrain * pTerrain = pHeightMapTerrain;
	int cxTerrain = pTerrain->GetWidth();
	int czTerrain = pTerrain->GetLength();

	m_nObjects = 32 + 1;

	m_ppObjects = new CGameObject*[m_nObjects];

	CNormalCube * pPointMesh = new CNormalCube(pd3dDevice, 256, 256);
	CGameObject *pObject = nullptr;
	float xPitch = 256.0f, zPitch = 256.0f;
	float fHalf = 2056 * 0.5;

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

	CTexture * pTexture = new CTexture(2, 2, 0, 0, (SET_SHADER_VS | SET_SHADER_DS | SET_SHADER_PS));
	ID3D11ShaderResourceView * pd3dsrvArray;
	ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/stones_nmap.png"), nullptr, nullptr, &pd3dsrvArray, nullptr));
	pTexture->SetTexture(0, pd3dsrvArray);
	pTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvArray->Release();

	ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/stone.png"), nullptr, nullptr, &pd3dsrvArray, nullptr));
	pTexture->SetTexture(1, pd3dsrvArray);
	pTexture->SetSampler(1, pd3dSamplerState);
	pd3dsrvArray->Release();
	pd3dSamplerState->Release();

	m_pTexture = pTexture;
	m_pMaterial = pMaterial;
	for (int j = 0; j < 4; ++j) {
		bool bIsEven = (j % 2) == 0;

		for (int i = 0; i < 8; i++)
		{
			pObject = new CGameObject(1);
			float fx = bIsEven == false ? (j - 1) * fHalf + 30 : (i*xPitch) + 128;
			float fz = bIsEven == true ? j * fHalf + 30 : (i*xPitch) + 128;

			float fy = 150;
			pObject->SetMesh(pPointMesh);
			pObject->SetPosition(fx, fy, fz);
			pObject->Rotate(0, (j - 2) * 90, 0);
			pObject->AddRef();
			m_ppObjects[(j * 8) + i] = pObject;
		}
	}
	CNormalCube * pSmallMesh = new CNormalCube(pd3dDevice, 56, 56);
	m_ppObjects[32] = new CGameObject(1);
	m_ppObjects[32]->SetPosition(XMFLOAT3(1085, 180, 280));
	m_ppObjects[32]->SetMesh(pSmallMesh);
	//m_ppObjects[32]->Rotate(0, 0, 0);
	//m_ppObjects[0]->Rotate(0, 180, 0);
	//m_ppObjects[1]->SetPosition(XMFLOAT3(1085, 180, 260));
	//m_ppObjects[2]->SetPosition(XMFLOAT3(1115, 180, 265));
	//m_ppObjects[3]->SetPosition(XMFLOAT3(1100, 180, 255));
	//m_ppObjects[4]->SetPosition(XMFLOAT3(1140, 180, 265));
}

void CNormalShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	CNormalMapShader::OnPrepareRender(pd3dDeviceContext, uRenderState);

	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
	if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);

	for (int j = 0; j < m_nObjects - 1; j++)
	{
		if (m_ppObjects[j])
		{
			//카메라의 절두체에 포함되는 객체들만을 렌더링한다.
			if (m_ppObjects[j]->IsVisible(pCamera)) {
				m_ppObjects[j]->Render(pd3dDeviceContext, uRenderState, pCamera);
			}
		}
	}
	m_Bump.m_xv3BumpScale.y = 4;
	CNormalMapShader::UpdateBumpInfo(pd3dDeviceContext);
	m_ppObjects[m_nObjects - 1]->Render(pd3dDeviceContext, uRenderState, pCamera);
	m_Bump.m_xv3BumpScale.y = 20;
}
////////////////////////////////////////////////////////////////////////
CTextureAniShader::CTextureAniShader()
{
	m_ppEffctsObjects  = nullptr;
	m_pd3dBlendState   = nullptr;
	m_pd3dSamplerState = nullptr;
}

CTextureAniShader::~CTextureAniShader()
{
	if (m_ppEffctsObjects)
	{
		for (int i = 0; i < m_nObjects; ++i)
			delete m_ppEffctsObjects[i];
	}
	delete[] m_ppEffctsObjects;

	if (m_pd3dBlendState) m_pd3dBlendState->Release();
	if (m_pd3dSamplerState) m_pd3dSamplerState->Release();
}

void CTextureAniShader::CreateShader(ID3D11Device * pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);

	CreateVertexShaderFromFile(pd3dDevice, L"fx/TextureAni.fx", "VSTextureAnimate", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/TextureAni.fx", "PSTextureAnimate", "ps_5_0", &m_pd3dPixelShader);
	CreateGeometryShaderFromFile(pd3dDevice, L"fx/TextureAni.fx", "GSTextureAnimate", "gs_5_0", &m_pd3dGeometryShader);
}

void CTextureAniShader::CreateStates(ID3D11Device * pd3dDevice)
{
	//D3D11_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	//ZeroMemory(&d3dDepthStencilDesc, sizeof(d3dDepthStencilDesc));
	//d3dDepthStencilDesc.DepthEnable = false;
	//d3dDepthStencilDesc.StencilEnable = false;
	//d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//pd3dDevice->CreateDepthStencilState(&d3dDepthStencilDesc, &m_pd3dSODepthStencilState);

	//d3dDepthStencilDesc.DepthEnable = true;
	//d3dDepthStencilDesc.StencilEnable = false;
	//d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; //D3D11_DEPTH_WRITE_MASK_ZERO;
	//pd3dDevice->CreateDepthStencilState(&d3dDepthStencilDesc, &m_pd3dDepthStencilState);

	D3D11_BLEND_DESC d3dBlendStateDesc;
	ZeroMemory(&d3dBlendStateDesc, sizeof(D3D11_BLEND_DESC));
	d3dBlendStateDesc.IndependentBlendEnable = false;
	int index = 0;
	ZeroMemory(&d3dBlendStateDesc.RenderTarget[index], sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
	d3dBlendStateDesc.AlphaToCoverageEnable                     = true;
	d3dBlendStateDesc.RenderTarget[index].BlendEnable           = false;
	d3dBlendStateDesc.RenderTarget[index].SrcBlend              = D3D11_BLEND_SRC_ALPHA;// D3D11_BLEND_ONE;
	d3dBlendStateDesc.RenderTarget[index].DestBlend             = D3D11_BLEND_SRC_ALPHA;//D3D11_BLEND_SRC_ALPHA
	d3dBlendStateDesc.RenderTarget[index].BlendOp               = D3D11_BLEND_OP_SUBTRACT;//ADD
	d3dBlendStateDesc.RenderTarget[index].SrcBlendAlpha         = D3D11_BLEND_ZERO;
	d3dBlendStateDesc.RenderTarget[index].DestBlendAlpha        = D3D11_BLEND_ZERO;
	d3dBlendStateDesc.RenderTarget[index].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	d3dBlendStateDesc.RenderTarget[index].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;//D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_ALPHA;
	pd3dDevice->CreateBlendState(&d3dBlendStateDesc, &m_pd3dBlendState);

	m_pd3dSamplerState = TXMgr.GetSamplerState("ss_linear_wrap");
	m_pd3dSamplerState->AddRef();
}

void CTextureAniShader::BuildObjects(ID3D11Device * pd3dDevice, CHeightMapTerrain * pHeightMapTerrain, CMaterial * pMaterial)
{
	m_nObjects = 3;

	m_ppEffctsObjects = new CTxAnimationObject*[m_nObjects];

	m_ppEffctsObjects[0] = new CCircleMagic();
	m_ppEffctsObjects[1] = new CElectricBolt();//CIceBolt();
	m_ppEffctsObjects[2] = new CIceSpear();
	for (int i = 0; i < m_nObjects; ++i)
	{
		m_ppEffctsObjects[i]->Initialize(pd3dDevice);
	}

	CreateStates(pd3dDevice);
}

void CTextureAniShader::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	pd3dDeviceContext->PSSetSamplers(0, 1, &m_pd3dSamplerState);
	pd3dDeviceContext->OMSetBlendState(m_pd3dBlendState, nullptr, 0xffffffff);

	//pd3dDeviceContext->PSSetSamplers(0, 1, &TXMgr.GetSamplerState("ss_linear_wrap"));
	for (int i = 0; i < m_nObjects; ++i)
	{
		if (m_ppEffctsObjects[i]->IsAble())
			m_ppEffctsObjects[i]->Render(pd3dDeviceContext, uRenderState, pCamera);
	}

	pd3dDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void CTextureAniShader::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nObjects; ++i)
	{
		if (m_ppEffctsObjects[i]->IsAble())
			m_ppEffctsObjects[i]->Animate(fTimeElapsed);
	}
}

CParticleShader::CParticleShader() : CShader()
{
	m_pd3dSODepthStencilState = nullptr;
	m_pd3dDepthStencilState   = nullptr;
	m_pd3dBlendState          = nullptr;

	m_nObjects                = 0;

	m_ppParticle              = nullptr;
	m_pd3dcbGameInfo          = nullptr;

	m_pd3dRandomSRV           = nullptr;
	m_pd3dSamplerState        = nullptr;

	m_pd3dStreamRain		  = nullptr;
	m_pd3dVSRainDraw		  = nullptr;
	m_pd3dGSRainDraw		  = nullptr;
	m_pd3dPSRainDraw		  = nullptr;

	m_pRainParticle			  = nullptr;
}

CParticleShader::~CParticleShader()
{
	m_vcAbleParticleArray.clear();
	m_vcUsingParticleArray.clear();

	if (m_ppParticle)
	{
		for (int i = 0; i < m_nObjects; ++i)
			delete m_ppParticle[i];
		delete[] m_ppParticle;
	}

	if (m_pd3dcbGameInfo) m_pd3dcbGameInfo->Release();
	if (m_pd3dSODepthStencilState) m_pd3dSODepthStencilState->Release();
	if (m_pd3dDepthStencilState) m_pd3dDepthStencilState->Release();
	if (m_pd3dBlendState) m_pd3dBlendState->Release();
	if (m_pd3dSamplerState) m_pd3dSamplerState->Release();

	if (m_pd3dRandomSRV) m_pd3dRandomSRV->Release();

	if (m_pd3dVSSO) m_pd3dVSSO->Release();
	if (m_pd3dGSSO) m_pd3dGSSO->Release();

	if (m_pd3dStreamRain) m_pd3dStreamRain->Release();
	if (m_pd3dVSRainDraw) m_pd3dVSRainDraw->Release();
	if (m_pd3dGSRainDraw) m_pd3dGSRainDraw->Release();
	if (m_pd3dPSRainDraw) m_pd3dPSRainDraw->Release();

	if (m_pRainParticle) delete m_pRainParticle;
}

void CParticleShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE",     0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "AGE",      0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TYPE",     0, DXGI_FORMAT_R32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);

	CreateVertexShaderFromFile(pd3dDevice, L"fx/Particle.fx", "VSParticleDraw", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreateGeometryShaderFromFile(pd3dDevice, L"fx/Particle.fx", "GSParticleDraw", "gs_5_0", &m_pd3dGeometryShader);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Particle.fx", "PSParticleDraw", "ps_5_0", &m_pd3dPixelShader);
	m_pd3dVertexLayout->Release();

	D3D11_SO_DECLARATION_ENTRY SODeclaration[] =
	{  // 스트림 번호(인덱스)/ 시멘틱이름/ 출력원소 인덱스(같은이름 시멘틱)/ 출력 시작요소/ 출력 요소(0~3:w)/ 연결된 스트림 출력버퍼(0~3)
		{ 0, "POSITION", 0, 0, 3, 0 },
		{ 0, "VELOCITY", 0, 0, 3, 0 },
		{ 0, "SIZE", 0, 0, 2, 0 },
		{ 0, "AGE", 0, 0, 1, 0 },
		{ 0, "TYPE", 0, 0, 1, 0 }
	};
	UINT pBufferStrides[1] = { sizeof(SODeclaration) };

	CreateVertexShaderFromFile(pd3dDevice, L"fx/Particle.fx", "VSParticleSO", "vs_5_0", &m_pd3dVSSO, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreateGeometryStreamOutShaderFromFile(pd3dDevice, L"fx/Particle.fx", "GSParticleSO", "gs_5_0", &m_pd3dGSSO, SODeclaration, 5, pBufferStrides, 1, 0);
	m_pd3dVertexLayout->Release();

	CreateGeometryStreamOutShaderFromFile(pd3dDevice, L"fx/Particle.fx", "GSRainSO", "gs_5_0", &m_pd3dStreamRain, SODeclaration, 5, pBufferStrides, 1, 0);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Particle.fx", "VSRainDraw", "vs_5_0", &m_pd3dVSRainDraw, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreateGeometryShaderFromFile(pd3dDevice, L"fx/Particle.fx", "GSRainDraw", "gs_5_0", &m_pd3dGSRainDraw);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Particle.fx", "PSRainDraw", "ps_5_0", &m_pd3dPSRainDraw);
	//m_pd3dVertexLayout->Release();
}

void CParticleShader::CreateStates(ID3D11Device * pd3dDevice)
{
	D3D11_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	ZeroMemory(&d3dDepthStencilDesc, sizeof(d3dDepthStencilDesc));
	d3dDepthStencilDesc.DepthEnable    = false;
	d3dDepthStencilDesc.StencilEnable  = false;
	d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	pd3dDevice->CreateDepthStencilState(&d3dDepthStencilDesc, &m_pd3dSODepthStencilState);

	d3dDepthStencilDesc.DepthEnable    = true;
	d3dDepthStencilDesc.StencilEnable  = false;
	d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; //D3D11_DEPTH_WRITE_MASK_ZERO;
	pd3dDevice->CreateDepthStencilState(&d3dDepthStencilDesc, &m_pd3dDepthStencilState);

	D3D11_BLEND_DESC d3dBlendStateDesc;
	ZeroMemory(&d3dBlendStateDesc, sizeof(D3D11_BLEND_DESC));
	d3dBlendStateDesc.IndependentBlendEnable = false;
	int index = 0;
	ZeroMemory(&d3dBlendStateDesc.RenderTarget[index], sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
	d3dBlendStateDesc.AlphaToCoverageEnable                     = true;
	d3dBlendStateDesc.RenderTarget[index].BlendEnable           = false;
	d3dBlendStateDesc.RenderTarget[index].SrcBlend              = D3D11_BLEND_SRC_ALPHA;// D3D11_BLEND_ONE;
	d3dBlendStateDesc.RenderTarget[index].DestBlend				= D3D11_BLEND_SRC_COLOR;//D3D11_BLEND_SRC_ALPHA; //D3D11_BLEND_ONE;  
	d3dBlendStateDesc.RenderTarget[index].BlendOp				= D3D11_BLEND_OP_ADD;
	d3dBlendStateDesc.RenderTarget[index].SrcBlendAlpha         = D3D11_BLEND_ZERO;
	d3dBlendStateDesc.RenderTarget[index].DestBlendAlpha        = D3D11_BLEND_ZERO;
	d3dBlendStateDesc.RenderTarget[index].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	d3dBlendStateDesc.RenderTarget[index].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;//D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_ALPHA;
	pd3dDevice->CreateBlendState(&d3dBlendStateDesc, &m_pd3dBlendState);

	m_pd3dSamplerState = TXMgr.GetSamplerState("ss_linear_wrap");
	m_pd3dSamplerState->AddRef();
}

void CParticleShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial * pMaterial)
{
	CreateStates(pd3dDevice);
	CreateShaderVariables(pd3dDevice);

	m_nObjects = 5;
	m_ppObjects = nullptr;
	m_ppParticle = new CParticle*[m_nObjects];

	m_ppParticle[0] = new CSmokeBoomParticle();
	m_ppParticle[0]->Initialize(pd3dDevice);//(pd3dDevice, cbParticle, 20.0, 800);

	m_ppParticle[1] = new CSmokeBoomParticle();
	m_ppParticle[1]->Initialize(pd3dDevice);

	m_ppParticle[2] = new CSmokeBoomParticle();
	m_ppParticle[2]->Initialize(pd3dDevice);

	m_ppParticle[3] = new CSmokeBoomParticle();
	m_ppParticle[3]->Initialize(pd3dDevice);

	m_ppParticle[4] = new CFireBallParticle();
	m_ppParticle[4]->Initialize(pd3dDevice);

	m_pRainParticle = new CRainParticle();
	m_pRainParticle->Initialize(pd3dDevice);
	m_pRainParticle->Enable();

	m_pd3dRandomSRV = ViewMgr.GetSRV("srv_random1d");// CShader::CreateRandomTexture1DSRV(pd3dDevice);
	m_pd3dRandomSRV->AddRef();

	m_vcAbleParticleArray.reserve(m_nObjects);
	m_vcUsingParticleArray.reserve(m_nObjects);
}

void CParticleShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	static ID3D11Buffer * nullBuffers[1] = { nullptr };
	UINT index = 0;

	pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	pd3dDeviceContext->IASetInputLayout(m_pd3dVertexLayout);

	CParticleShader::SOSetState(pd3dDeviceContext);

	pd3dDeviceContext->OMSetDepthStencilState(m_pd3dSODepthStencilState, 0);

	for (auto it = m_vcUsingParticleArray.begin(); it != m_vcUsingParticleArray.end(); ++it)
	{
		it->second->StreamOut(pd3dDeviceContext);
	}
	if (m_pRainParticle && m_pRainParticle->IsAble())
	{
		pd3dDeviceContext->GSSetShader(m_pd3dStreamRain, nullptr, 0);
		m_pRainParticle->StreamOut(pd3dDeviceContext);
	}
	pd3dDeviceContext->SOSetTargets(1, nullBuffers, 0);

	pd3dDeviceContext->VSSetShader(m_pd3dVertexShader, nullptr, 0);
	pd3dDeviceContext->GSSetShader(m_pd3dGeometryShader, nullptr, 0);
	pd3dDeviceContext->PSSetShader(m_pd3dPixelShader, nullptr, 0);

	pd3dDeviceContext->PSSetSamplers(0, 1, &m_pd3dSamplerState);

	pd3dDeviceContext->OMSetDepthStencilState(m_pd3dDepthStencilState, 0);
	pd3dDeviceContext->OMSetBlendState(m_pd3dBlendState, nullptr, 0xffffffff);
	for (auto it = m_vcUsingParticleArray.begin(); it != m_vcUsingParticleArray.end(); ++it)
	{
		if (it->second->IsActvie())
			it->second->Render(pd3dDeviceContext, uRenderState, pCamera);
	}

	pd3dDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	if (m_pRainParticle && m_pRainParticle->IsAble())
	{
		RainDrawShader(pd3dDeviceContext);
		m_pRainParticle->Render(pd3dDeviceContext, uRenderState, pCamera);
	}
}

void CParticleShader::AnimateObjects(float fTimeElapsed)
{
	m_vcAbleParticleArray.clear();
	m_vcUsingParticleArray.clear();

	for (int i = 0; i < m_nObjects; ++i)
	{
		if (false == m_ppParticle[i]->IsAble())
		{
			if(i != 4)
				m_vcAbleParticleArray.push_back(m_ppParticle[i]);
		}
		else
		{
			m_ppParticle[i]->Update(fTimeElapsed);
			m_vcUsingParticleArray.push_back(ParticleInfo(i, m_ppParticle[i]));
		}
	}

	if (m_pRainParticle && m_pRainParticle->IsAble()) 
		m_pRainParticle->Update(fTimeElapsed);
}

void CParticleShader::SOSetState(ID3D11DeviceContext * pd3dDeviceContext)
{
	pd3dDeviceContext->VSSetShader(m_pd3dVSSO, nullptr, 0);
	pd3dDeviceContext->GSSetShader(m_pd3dGSSO, nullptr, 0);
	pd3dDeviceContext->PSSetShader(nullptr, nullptr, 0);
	pd3dDeviceContext->OMSetDepthStencilState(m_pd3dSODepthStencilState, 0);

	pd3dDeviceContext->GSSetShaderResources(TX_SLOT_RANDOM1D, 1, &m_pd3dRandomSRV);
	pd3dDeviceContext->GSSetSamplers(8, 1, &m_pd3dSamplerState);

	pd3dDeviceContext->RSSetState(nullptr);
}

void CParticleShader::RainDrawShader(ID3D11DeviceContext * pd3dDeviceContext)
{
	pd3dDeviceContext->VSSetShader(m_pd3dVSRainDraw, nullptr, 0);
	pd3dDeviceContext->GSSetShader(m_pd3dGSRainDraw, nullptr, 0);
	pd3dDeviceContext->PSSetShader(m_pd3dPSRainDraw, nullptr, 0);
}
