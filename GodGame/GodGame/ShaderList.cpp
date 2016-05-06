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

void CInstancingShader::BuildObjects(ID3D11Device *pd3dDevice, CMaterial *pMaterial, CTexture *pTexture, int k)
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

	CMapManager * pHeightMapTerrain = &MAPMgr;
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

#pragma region StaticInstanceShader
CStaticInstaningShader::CStaticInstaningShader() : CShader(), CInstanceShader()
{
	for (auto & bufferPtr : m_pd3dRockInstanceBuffer) bufferPtr = nullptr;
	for (auto & nNum : m_nRocks) nNum = 0;
	for (auto & tx : m_pRockTexture) tx = nullptr;

	for (auto & bufferPtr : m_pd3dStoneInstanceBuffer) bufferPtr = nullptr;
	for (auto & nNum : m_nStones) nNum = 0;
	for (auto & tx : m_pStoneTexture) tx = nullptr;

	m_pMaterial = nullptr;
}

CStaticInstaningShader::~CStaticInstaningShader()
{
	for (auto bufferPtr : m_pd3dRockInstanceBuffer) if (bufferPtr) bufferPtr->Release();
	for (auto tx : m_pRockTexture) if (tx) tx->Release();

	for (auto bufferPtr : m_pd3dStoneInstanceBuffer) if (bufferPtr) bufferPtr->Release();
	for (auto tx : m_pStoneTexture) if (tx) tx->Release();

	if (m_pMaterial) m_pMaterial->Release();
}

void CStaticInstaningShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTANCE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSNormStaticInstancing", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice,  L"fx/Effect.fx", "PSNormStaticInstancing", "ps_5_0", &m_pd3dPixelShader);
}

void CStaticInstaningShader::BuildObjects(ID3D11Device *pd3dDevice, CShader::BUILD_RESOURCES_MGR & mgrScene)
{
	m_pMaterial = MaterialMgr.GetObjects("White");
	m_pMaterial->AddRef();

	m_nObjects = 0;
#ifdef _LOAD_DATA
	vector<UINT> mObjs;
	mObjs.reserve(10);

	vector<vector<XMFLOAT3>> mVertexes;
	mVertexes.reserve(10);

	FILE * file;
	file = fopen("../Assets/Data/Map01/StaticRocks.data", "rb");
	{
		int index = 0;
		while (true) {
			UINT nobj;
			int read = fread(&nobj, sizeof(UINT), 1, file);
			if (read == 0) break;
			mObjs.push_back(nobj);

			XMFLOAT3 pos;
			mVertexes.emplace_back();
			mVertexes[index].reserve(nobj);
			for (int i = 0; i < nobj; ++i)
			{
				fread(&pos, sizeof(XMFLOAT3), 1, file);
				mVertexes[index].push_back(pos);
			}
			++index;
		}
	}
	fclose(file);
#endif

#ifndef _LOAD_DATA
	for (int & nRocks : m_nRocks){
		nRocks = 80;
		m_nObjects += nRocks;
	}

	for (int & nStones : m_nStones){
		nStones = 20;
		m_nObjects += nStones;
	}
#endif

	XMFLOAT3 xmf3Pos;
	XMFLOAT2 xmf2Size = XMFLOAT2(80, 60);

	m_nInstanceBufferStride = sizeof(XMFLOAT4);
	m_nInstanceBufferOffset = 0;

	CGameObject * pObj = nullptr;
	CMapManager * pTerrain = &MAPMgr;
	int cxTerrain = pTerrain->GetWidth() - 200;
	int czTerrain = pTerrain->GetLength() - 200;
#ifndef _LOAD_DATA
#ifdef _SAVE_DATA
	FILE * save;
	save = fopen("../Assets/Data/Map01/StaticRocks.data", "wb");
#endif
#endif

	CMesh * pRockMesh[NUM_ROCK]; 
	char name[24];
	int in = 1;
	for (auto & pMesh : pRockMesh)
	{
		sprintf(name, "scene_rock%d", in);
		pMesh = mgrScene.mgrMesh.GetObjects(name);
		m_pRockTexture[in - 1] = mgrScene.mgrTexture.GetObjects(name);
		m_pRockTexture[in - 1]->AddRef();
		++in;
	}

	in = 1;
	CMesh * pStoneMesh[NUM_STONE];
	for (auto & pMesh : pStoneMesh)
	{
		sprintf(name, "scene_stone%d", in);
		pMesh = mgrScene.mgrMesh.GetObjects(name);
		m_pStoneTexture[in - 1] = mgrScene.mgrTexture.GetObjects(name);
		m_pStoneTexture[in - 1]->AddRef();
		++in;
	}

	XMFLOAT3 pos;
	int index = -1;

	float fxTerrain = 0.f, fzTerrain = 0.f;
	CQuadTreeManager & mgr = QUADMgr;
	for (int i = 0; i < NUM_ROCK; ++i)
	{
#ifdef _LOAD_DATA
		m_nRocks[i] = mObjs[++index];
		m_ppRockObjects[i] = new CGameObject*[m_nRocks[i]];
#else
		m_ppRockObjects[i] = new CGameObject*[m_nRocks[i]];
#ifdef _SAVE_DATA
		fwrite(&m_nRocks[i], sizeof(UINT), 1, save);
#endif
#endif
		for (int j = 0; j < m_nRocks[i]; ++j)
		{
			pObj = new CGameObject(1);
#ifdef _LOAD_DATA
			pObj->SetPosition(mVertexes[index][j]);
#else
			pObj->SetPosition(MAPMgr.GetRandPos());// xmf3Pos);
#ifdef _SAVE_DATA
			pos = pObj->GetPosition();
			fwrite(&pos, sizeof(XMFLOAT3), 1, save);
#endif
#endif
			pObj->SetMesh(pRockMesh[i]);
			pObj->AddRef();
			pObj->SetCollide(true);
			pObj->SetObstacle(true);
			pObj->UpdateBoundingBox();
			mgr.InsertStaticEntity(pObj);

			m_ppRockObjects[i][j] = pObj;
		}
		m_pd3dRockInstanceBuffer[i] = CreateInstanceBuffer(pd3dDevice, m_nRocks[i], m_nInstanceBufferStride, nullptr);
		pRockMesh[i]->AssembleToVertexBuffer(1, &m_pd3dRockInstanceBuffer[i], &m_nInstanceBufferStride, &m_nInstanceBufferOffset);
 	}

	for (int i = 0; i < NUM_STONE; ++i)
	{
#ifdef _LOAD_DATA
		m_nStones[i] = mObjs[++index];
		m_ppStoneObjects[i] = new CGameObject*[m_nStones[i]];
#else
		m_ppStoneObjects[i] = new CGameObject*[m_nStones[i]];
#ifdef _SAVE_DATA
		fwrite(&m_nStones[i], sizeof(UINT), 1, save);
#endif
#endif
		for (int j = 0; j < m_nStones[i]; ++j)
		{
			pObj = new CGameObject(1);
#ifdef _LOAD_DATA
			pObj->SetPosition(mVertexes[index][j]);
#else
			pObj->SetPosition(MAPMgr.GetRandPos());//xmf3Pos);
#ifdef _SAVE_DATA
			pos = pObj->GetPosition();
			fwrite(&pos, sizeof(XMFLOAT3), 1, save);
#endif
#endif
			pObj->SetMesh(pStoneMesh[i]);
			pObj->AddRef();
			pObj->SetCollide(true);
			pObj->SetObstacle(true);
			pObj->UpdateBoundingBox();
			mgr.InsertStaticEntity(pObj);

			m_ppStoneObjects[i][j] = pObj;
		}
		m_pd3dStoneInstanceBuffer[i] = CreateInstanceBuffer(pd3dDevice, m_nStones[i], m_nInstanceBufferStride, nullptr);
		pStoneMesh[i]->AssembleToVertexBuffer(1, &m_pd3dStoneInstanceBuffer[i], &m_nInstanceBufferStride, &m_nInstanceBufferOffset);
	}

	//EntityAllStaticObjects();

#ifndef _LOAD_DATA
#ifdef _SAVE_DATA
	fclose(save);
#endif
#endif
}

void CStaticInstaningShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);
	if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);

	for (int i = 0; i < NUM_ROCK; ++i)
	{
		m_pRockTexture[i]->UpdateShaderVariable(pd3dDeviceContext);

		int nRockInstance = 0;
		D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
		pd3dDeviceContext->Map(m_pd3dRockInstanceBuffer[i], 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
		XMFLOAT4 *pnRockInstances = (XMFLOAT4 *)d3dMappedResource.pData;

		CGameObject * pObj = nullptr;
		XMFLOAT3 pos;
		for (int j = 0; j < m_nRocks[i]; ++j)
		{
			pObj = m_ppRockObjects[i][j];
			if (pObj->IsVisible())
			{
				pos = pObj->GetPosition();
				pnRockInstances[nRockInstance++] = move(XMFLOAT4(pos.x, pos.y, pos.z, 1.f));
				pObj->SetVisible(false);
			}
		}
		pd3dDeviceContext->Unmap(m_pd3dRockInstanceBuffer[i], 0);

		pObj->GetMesh()->RenderInstanced(pd3dDeviceContext, uRenderState, nRockInstance, 0);
	}

	for (int i = 0; i < NUM_STONE; ++i)
	{
		m_pStoneTexture[i]->UpdateShaderVariable(pd3dDeviceContext);

		int nStoneInstance = 0;
		D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
		pd3dDeviceContext->Map(m_pd3dStoneInstanceBuffer[i], 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
		XMFLOAT4 *pnStoneInstances = (XMFLOAT4 *)d3dMappedResource.pData;

		CGameObject * pObj = nullptr;
		XMFLOAT3 pos;
		for (int j = 0; j < m_nStones[i]; ++j)
		{
			pObj = m_ppStoneObjects[i][j];
			if (pObj->IsVisible())
			{
				pos = pObj->GetPosition();
				pnStoneInstances[nStoneInstance++] = move(XMFLOAT4(pos.x, pos.y, pos.z, 1.f));
				pObj->SetVisible(false);
			}
		}
		pd3dDeviceContext->Unmap(m_pd3dStoneInstanceBuffer[i], 0);

		pObj->GetMesh()->RenderInstanced(pd3dDeviceContext, uRenderState, nStoneInstance, 0);
	}
}

void CStaticInstaningShader::AnimateObjects(float fTimeElapsed)
{
}

void CStaticInstaningShader::AllRender(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera )
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);
	if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);

	for (int i = 0; i < NUM_ROCK; ++i)
	{
		m_pRockTexture[i]->UpdateShaderVariable(pd3dDeviceContext);

		int nRockInstance = 0;
		D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
		pd3dDeviceContext->Map(m_pd3dRockInstanceBuffer[i], 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
		XMFLOAT4 *pnRockInstances = (XMFLOAT4 *)d3dMappedResource.pData;

		CGameObject * pObj = nullptr;
		XMFLOAT3 pos;
		for (int j = 0; j < m_nRocks[i]; ++j)
		{
			pObj = m_ppRockObjects[i][j];
			pos = pObj->GetPosition();
			pnRockInstances[nRockInstance++] = move(XMFLOAT4(pos.x, pos.y, pos.z, 1.f));
		}
		pd3dDeviceContext->Unmap(m_pd3dRockInstanceBuffer[i], 0);

		pObj->GetMesh()->RenderInstanced(pd3dDeviceContext, uRenderState, nRockInstance, 0);
	}
	for (int i = 0; i < NUM_STONE; ++i)
	{
		m_pStoneTexture[i]->UpdateShaderVariable(pd3dDeviceContext);

		int nStoneInstance = 0;
		D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
		pd3dDeviceContext->Map(m_pd3dStoneInstanceBuffer[i], 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
		XMFLOAT4 *pnStoneInstances = (XMFLOAT4 *)d3dMappedResource.pData;

		CGameObject * pObj = nullptr;
		XMFLOAT3 pos;
		for (int j = 0; j < m_nStones[i]; ++j)
		{
			pObj = m_ppStoneObjects[i][j];
			if (pObj->IsVisible())
			{
				pos = pObj->GetPosition();
				pnStoneInstances[nStoneInstance++] = move(XMFLOAT4(pos.x, pos.y, pos.z, 1.f));
				pObj->SetVisible(false);
			}
		}
		pd3dDeviceContext->Unmap(m_pd3dStoneInstanceBuffer[i], 0);

		pObj->GetMesh()->RenderInstanced(pd3dDeviceContext, uRenderState, nStoneInstance, 0);
	}
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

void CBillboardShader::BuildObjects(ID3D11Device *pd3dDevice)
{
	m_nObjects = m_nTrees = 200;

	XMFLOAT3 xmf3Pos;
	XMFLOAT2 xmf2Size = XMFLOAT2(80, 60);

	m_nInstanceBufferStride = sizeof(VS_VB_WORLD_POSITION);
	m_nInstanceBufferOffset = 0;

	CBillboardObject * pTree = nullptr;
	m_ppObjects = new CGameObject*[m_nObjects];

	CMapManager * pTerrain = &MAPMgr;
	int cxTerrain = pTerrain->GetWidth();
	int czTerrain = pTerrain->GetLength();

	float fxTerrain = 0.f, fzTerrain = 0.f;
	const XMINT2 posRange[4] { { 50, czTerrain - 100 }, { cxTerrain - 100 , 50 },
	{ 50 , czTerrain - 100 }, { cxTerrain - 100 , 50 } };
	const XMINT2 rangePlus[4] { { 50, 50 }, { 50 , 50 },
	{ cxTerrain - 100 , 50 }, { 50 , czTerrain - 100 } };

	CBillBoardVertex * pTreeMesh = new CBillBoardVertex(pd3dDevice, xmf2Size.x, xmf2Size.y);
	for (int i = 0; i < m_nTrees; ++i) 
	{
		int index = i % 4;

		fxTerrain = xmf3Pos.x = rand() % posRange[index].x + rangePlus[index].x;
		fzTerrain = xmf3Pos.z = rand() % posRange[index].y + rangePlus[index].y;
		xmf3Pos.y = pTerrain->GetHeight(fxTerrain, fzTerrain, !(int(fzTerrain) % 2)) + 22;
		pTree = new CBillboardObject(xmf3Pos, i * rand() % 4, xmf2Size);
		pTree->SetMesh(pTreeMesh);
		pTree->SetSize(20.0f);
		//pTree->SetCollide(true);
		pTree->AddRef();
		m_ppObjects[i] = pTree;
	}

	m_pd3dTreeInstanceBuffer = CreateInstanceBuffer(pd3dDevice, m_nTrees, m_nInstanceBufferStride, nullptr);
	pTreeMesh->AssembleToVertexBuffer(1, &m_pd3dTreeInstanceBuffer, &m_nInstanceBufferStride, &m_nInstanceBufferOffset);

	m_pTexture = new CTexture(1, 1, TX_SLOT_TEXTURE_ARRAY, 0, SET_SHADER_PS);
	// 크기 동일'
	//ID3D11ShaderResourceView * pd3dsrvArray = nullptr;
	ID3D11ShaderResourceView * pd3dsrvArray = CTexture::CreateTexture2DArraySRV(pd3dDevice, _T("../Assets/Image/Objects/trees/Bills"), _T("png"), 4);

	m_pTexture->SetTexture(0, pd3dsrvArray);
	m_pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));

	pd3dsrvArray->Release();

	EntityAllStaticObjects(nullptr);
}

void CBillboardShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);
	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);

	//if (uRenderState & RS_SHADOWMAP)
	//{
	//	AllRender(pd3dDeviceContext, uRenderState, pCamera);
	//	return;
	//}
	//	pCamera->UpdateCameraPositionCBBuffer(pd3dDeviceContext);

	int nTreeInstance = 0;
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dTreeInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	XMFLOAT4 *pnTreeInstances = (XMFLOAT4 *)d3dMappedResource.pData;

	CBillboardObject * pTree = nullptr;
	for (int j = 0; j < m_nTrees; ++j)
	{
		pTree = (CBillboardObject*)m_ppObjects[j];
		if (pTree->IsVisible())
		{
			pnTreeInstances[nTreeInstance] = pTree->GetInstanceData();
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
	m_pMaterial = nullptr;
}

CStaticShader::~CStaticShader()
{
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

void CStaticShader::BuildObjects(ID3D11Device *pd3dDevice, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr)
{
	m_pMaterial = pMaterial;
	if (pMaterial) pMaterial->AddRef();

	TEXTURE_MGR & txmgr = SceneMgr.mgrTexture;
	MESH_MGR & meshmgr = SceneMgr.mgrMesh;

	m_nObjects = 5;
	m_ppObjects = new CGameObject*[m_nObjects];

	string names[] = { "scene_portal", "scene_ruin1", "scene_ruin2", "scene_colum1", "scene_colum2" };

	CMapManager * pHeightMapTerrain = &MAPMgr;
	float fHeight = pHeightMapTerrain->GetHeight(1015, 1574, false);
	m_ppObjects[0] = SYSTEMMgr.GetPortalGate();
	m_ppObjects[0]->SetPosition(1015, fHeight, 1574);
	m_ppObjects[0]->AddRef();


	m_ppObjects[1] = new CGameObject(1);
	fHeight = pHeightMapTerrain->GetHeight(1065, 1544, false);
	m_ppObjects[1]->SetPosition(1065, fHeight, 1544);
	m_ppObjects[1]->AddRef();

	m_ppObjects[2] = new CGameObject(1);
	fHeight = pHeightMapTerrain->GetHeight(955, 1596, false);
	m_ppObjects[2]->SetPosition(955, fHeight, 1596);
	m_ppObjects[2]->AddRef();

	m_ppObjects[3] = new CGameObject(1);
	fHeight = pHeightMapTerrain->GetHeight(955, 1506, false);
	m_ppObjects[3]->SetPosition(955, fHeight, 1506);
	m_ppObjects[3]->AddRef();

	m_ppObjects[4] = new CGameObject(1);
	fHeight = pHeightMapTerrain->GetHeight(1065, 1506, false);
	m_ppObjects[4]->SetPosition(1065, fHeight, 1506);
	m_ppObjects[4]->AddRef();


	for (int i = 0; i < m_nObjects; ++i)
	{
		m_ppObjects[i]->SetMesh(meshmgr.GetObjects(names[i]));
		if ( 1 < i < 3) m_ppObjects[i]->SetSize(m_ppObjects[i]->GetSize() * 1.5);
		m_ppObjects[i]->SetTexture(txmgr.GetObjects(names[i]));
		m_ppObjects[i]->SetCollide(true);
		m_ppObjects[i]->SetDetailCollide(true);
		//m_ppObjects[i]->UpdateBoundingBox();
	}

	EntityAllStaticObjects("Static");
	/// 이상 스테틱 객체들
}

void CStaticShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	ID3D11ShaderResourceView * pd3dNullSRV[] = { nullptr, nullptr };
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);

	for (int j = 0; j < m_nObjects; j++)
	{
		//카메라의 절두체에 포함되는 객체들만을 렌더링한다.
		//m_ppObjects[j]->SetActive(true);
		if (m_ppObjects[j]->IsVisible())
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

void CCharacterShader::BuildObjects(ID3D11Device * pd3dDevice, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr)
{
	m_pMaterial = pMaterial;
	if (pMaterial) pMaterial->AddRef();

	TEXTURE_MGR & txmgr = SceneMgr.mgrTexture;
	MESH_MGR & meshmgr = SceneMgr.mgrMesh;

	m_nObjects = 5;
	m_ppObjects = new CGameObject*[m_nObjects];

	//char ManagerNames[2][50] = { { "scene_skull" },{ "scene_warrok" } };//, { "scene_man_death" }, { "scene_player_0" }, { "scene_skull_0" }};
																		//int Pos[5][2] = { {1085, 220}, {1085, 260}, {} };
	CGameObject *pObject = nullptr;
	CMonster * pAnimatedObject = nullptr;//new CSkeleton(1);

		//pAnimatedObject->SetMesh(meshmgr.GetObjects("scene_skull_0"));
		//pAnimatedObject->SetAnimationCycleTime(0, 2.0f);
	char AnimNames[6][50] = { "scene_warrok_idle", "scene_warrok_run", "scene_warrok_roar", "scene_warrok_punch", "scene_warrok_swiping", "scene_warrok_death" };
	for (int j = 0; j < m_nObjects; ++j)
	{
		pAnimatedObject = new CWarrock(CWarrock::eANI_WARROCK_ANIM_NUM);

		for (int i = 0; i < CWarrock::eANI_WARROCK_ANIM_NUM; ++i)
			pAnimatedObject->SetMesh(meshmgr.GetObjects(AnimNames[i]), i);

		pAnimatedObject->InitializeAnimCycleTime();
		pAnimatedObject->SetSize(10);

		m_ppObjects[j] = pAnimatedObject;
		m_ppObjects[j]->SetPosition(MAPMgr.GetRandPos());
		m_ppObjects[j]->AddRef();
	}


	for (int i = 0; i < m_nObjects; ++i)
	{
		m_ppObjects[i]->SetTexture(txmgr.GetObjects("scene_warrok"));//ManagerNames[i]));

		CCharacter * pAnimObject = nullptr;
		if (pAnimObject = dynamic_cast<CCharacter*>(m_ppObjects[i]))
		{
			pAnimObject->UpdateFramePerTime();
			pAnimObject->SetUpdatedContext(&MAPMgr);
		}
	}
	EntityAllDynamicObjects("Monster");
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
			pd3dDeviceContext->PSSetShaderResources(2, 2, pd3dNullSRV);
			m_ppObjects[j]->Render(pd3dDeviceContext, uRenderState, pCamera);
		}
	}
}

void CCharacterShader::Reset()
{
	for (int i = 0; i < m_nObjects; ++i)
	{
		static_cast<CMonster*>(m_ppObjects[i])->Reset();
		m_ppObjects[i]->SetPosition(MAPMgr.GetRandPos());
	}

	XMFLOAT3 pos = SYSTEMMgr.GetPortalGate()->GetPosition();
	//pos.x += rand() % 100 - 50;
	pos.z -= 50;
	pos.y = MAPMgr.GetHeight(pos);
	m_ppObjects[0]->SetPosition(pos.x, pos.y, pos.z);

	float fHeight = 0;

	fHeight = MAPMgr.GetHeight(1165, 275, false);
	m_ppObjects[1]->SetPosition(1165, fHeight, 275);

	fHeight = MAPMgr.GetHeight(715, 475, false);
	m_ppObjects[2]->SetPosition(715, fHeight, 475);

	fHeight = MAPMgr.GetHeight(1425, 1025, false);
	m_ppObjects[3]->SetPosition(1425, fHeight, 1025);

	fHeight = MAPMgr.GetHeight(605, 805, false);
	m_ppObjects[4]->SetPosition(605, fHeight, 805);

	for (int i = 0; i < m_nObjects; ++i)
	{
		m_ppObjects[i]->UpdateBoundingBox();
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
///////////////////////////////////////////////////////////////////////////////////////////
CItemShader::CItemShader() : CShader(), CInstanceShader()
{
	m_pMaterial = nullptr;

	m_vcItemList.reserve(10);
}

CItemShader::~CItemShader()
{
	if (m_pMaterial) m_pMaterial->Release();

	for (auto ItemList : m_vcItemList)
	{
		for (auto it : ItemList)
			delete it;
		ItemList.clear();
	}
	m_vcItemList.clear();
}

void CItemShader::CreateShader(ID3D11Device * pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);

	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VS_VNT", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PS_VNT", "ps_5_0", &m_pd3dPixelShader);
}

void CItemShader::BuildObjects(ID3D11Device * pd3dDevice, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr)
{
	m_pMaterial = pMaterial;
	m_pMaterial->AddRef();

	m_nInstanceBufferStride = sizeof(XMFLOAT4X4);
	m_nInstanceBufferOffset = 0;

	string name;//char name[48];
	CMapManager * pTerrain = &MAPMgr;

	int lv = -1;
	int element = -1;

	for (int i = 0; i < 13; ++i)
	{
		m_vcItemList.emplace_back(ItemList());

		auto it = m_vcItemList.end() - 1;
		if (i < 12)
		{
			lv = (i / 6);
			element = (i % 6);
		}
		else
		{
			lv = 2;
			element = 0;
		}
		name = ITEMMgr.StaffNameArray[element][lv];
#if _DEBUG
		cout << "Staff " << element << ", " << lv << "::" << name << endl;
#endif
		XMFLOAT3 xmfPos;
		for (int j = 0; j < 10; ++j)
		{
			CStaff * pItem = new CStaff(1);
			pItem->AddRef();
			pItem->BuildObject(element, 0, lv);
			pItem->SetMesh(SceneMgr.mgrMesh.GetObjects(name));
			pItem->SetCollide(true);
			//pItem->SetSize(10);
			pItem->SetTexture(SceneMgr.mgrTexture.GetObjects(name));
			pItem->SetMaterial(m_pMaterial);

			pItem->AllocPositionAndEntityQuadTree();
			it->push_back(pItem);
		}
	}
}

void CItemShader::AnimateObjects(float fTimeElapsed)
{
	for (auto ItemList : m_vcItemList)
		for (auto it : ItemList)
		{
			it->Animate(fTimeElapsed);
		}
}

void CItemShader::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	ID3D11ShaderResourceView * pd3dNullSRV[] = { nullptr, nullptr };
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	// 각자 재질은 설정 되어있지만, 배치 처리한다.
	CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);

	for (auto ItemList : m_vcItemList)
	{
		(*ItemList.begin())->GetTexture()->UpdateShaderVariable(pd3dDeviceContext);

		for (auto it : ItemList)
		{
			if ((uRenderState & RS_SHADOWMAP) || it->IsVisible())
				it->Render(pd3dDeviceContext, uRenderState, pCamera);
		}
	}
}

void CItemShader::GetGameMessage(CShader * byObj, eMessage eMSG, void * extra)
{
	//if (eMSG == eMessage::MSG_PASS_PLAYERPTR)
	//{
	//	for (int i = 0; i < m_nObjects; ++i)
	//		static_cast<CMonster*>(m_ppObjects[i])->BuildObject(static_cast<CCharacter*>(extra));
	//}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

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

void CPointInstanceShader::BuildObjects(ID3D11Device *pd3dDevice, CMaterial * pMaterial)
{
	CMaterial * pMat              = new CMaterial();
	pMat->m_Material.m_xcAmbient  = XMFLOAT4(100, 100, 100, 10);
	pMat->m_Material.m_xcDiffuse  = XMFLOAT4(100, 100, 100, 10);
	pMat->m_Material.m_xcSpecular = XMFLOAT4(100, 100, 100, 10);
	pMat->m_Material.m_xcEmissive = XMFLOAT4(100, 100, 100, 10);

	m_pMaterial = pMat;//pMaterial;
	pMat->AddRef();//if (pMaterial) pMaterial->AddRef();
	m_nObjects = m_nCubes = (ELEMENT_NUM * 180);

//	XMFLOAT3 xmf3Pos;
	XMFLOAT2 xmf2Size(6, 6);

	//m_nInstanceBufferStride = sizeof(VS_VB_WORLD_POSITION);
	m_nInstanceBufferStride = sizeof(XMFLOAT4);
	m_nInstanceBufferOffset = 0;

	m_ppObjects = new CGameObject*[m_nObjects];

	CMapManager * pTerrain = &MAPMgr;
	
	CBillBoardVertex * pPointMesh = new CBillBoardVertex(pd3dDevice, 6, 6);
	CAbsorbMarble *pObject = nullptr;
	XMFLOAT3 pos;
	for (int i = 0; i < m_nObjects; i++)
	{
		pos = pTerrain->GetRandPos();
		pos.y += 10.f;

		pObject = new CAbsorbMarble(pos, (i % ELEMENT_NUM), xmf2Size);
		pObject->SetMesh(pPointMesh);
		pObject->Initialize();
		//pObject->SetMaterial(pMat);	
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

	EntityAllStaticObjects(nullptr);
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
		m_ppObjects[j]->SetVisible(false);
	}
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
#pragma region BlackAlpha
CBlackAlphaShader::CBlackAlphaShader() : CShader()
{
}

CBlackAlphaShader::~CBlackAlphaShader()
{
}

void CBlackAlphaShader::CreateShader(ID3D11Device * pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSTexturedBlackAlpha", "ps_5_0", &m_pd3dPixelShader);
}

void CBlackAlphaShader::BuildObjects(ID3D11Device * pd3dDevice, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr)
{
	m_nObjects = 1;

	CGameObject * pObject = nullptr;
	m_ppObjects = new CGameObject*[m_nObjects];
	pObject = SYSTEMMgr.GetPortalZoneObject();
	pObject->AddRef();

	pObject->SetPosition(SYSTEMMgr.GetPortalGate()->GetZonePosition());
	pObject->UpdateBoundingBox();
	m_ppObjects[0] = pObject;

	EntityAllStaticObjects(nullptr);
}

void CBlackAlphaShader::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]->IsVisible())
		{
			m_ppObjects[j]->Render(pd3dDeviceContext, uRenderState, pCamera);
		}
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

void CNormalShader::BuildObjects(ID3D11Device *pd3dDevice, CMaterial * pMaterial)
{
	m_Bump.m_fBumpMax = 30.0f;
	m_Bump.m_xv3BumpScale = XMFLOAT3(30.0f, 30.f, 30.0f);

	CMapManager * pTerrain = &MAPMgr;
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

void CTextureAniShader::BuildObjects(ID3D11Device * pd3dDevice, CMaterial * pMaterial)
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

void CParticleShader::BuildObjects(ID3D11Device *pd3dDevice, CMaterial * pMaterial)
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
	m_pRainParticle->Enable(nullptr);

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
		if (it->second->IsVisible())
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
			m_vcUsingParticleArray.emplace_back(i, m_ppParticle[i]);
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
