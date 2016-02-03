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
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSInstancedTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSInstancedTexturedColor", "ps_5_0", &m_pd3dPixelShader);
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
		if (m_ppObjects[j])
		{
			if (m_ppObjects[j]->IsVisible(pCamera))
			{
				Chae::XMFloat4x4Transpose(&pnCubeInstances[nCubeInstances].m_d3dxTransform, &m_ppObjects[j]->m_xmf44World); // XMFLOAT4X4Transpose(&pnSphereInstances[nSphereInstances++].m_d3dxTransform, &m_ppObjects[j]->m_xmf44World);
				nCubeInstances++;
			}
		}
	}
	pd3dDeviceContext->Unmap(m_pd3dCubeInstanceBuffer, 0);

	CMesh *pSphereMesh = m_ppObjects[0]->GetMesh();
	pSphereMesh->RenderInstanced(pd3dDeviceContext, nCubeInstances, 0);

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
	//m_pd3dVertexBuffer = nullptr;
}

CBillboardShader::~CBillboardShader()
{
	//if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
	if (m_pTexture) m_pTexture->Release();
}

void CBillboardShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSBillboard", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSBillboard", "ps_5_0", &m_pd3dPixelShader);
	CreateGeometryShaderFromFile(pd3dDevice, L"Effect.fx", "GSBillboard", "gs_5_0", &m_pd3dGeometryShader);
}

void CBillboardShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain)
{
	//m_nObjects = 0;
	m_nObjects = m_nTrees = 100;

	XMFLOAT3 xmf3Pos;
	XMFLOAT2 xmf2Size = XMFLOAT2(20, 50);

	m_nInstanceBufferStride = sizeof(VS_VB_WORLD_POSITION);

	CTrees * pTree = nullptr;
	m_ppObjects = new CGameObject*[m_nObjects];

	CHeightMapTerrain * pTerrain = pHeightMapTerrain;
	int cxTerrain = pTerrain->GetHeightMapWidth();
	int czTerrain = pTerrain->GetHeightMapLength();


	CTreeVertex * pTreeMesh = new CTreeVertex(pd3dDevice, xmf2Size.x, xmf2Size.y);

	for (int i = 0; i < m_nTrees; ++i){
		float fxTerrain = xmf3Pos.x = rand() % cxTerrain;
		float fzTerrain = xmf3Pos.z = rand() % czTerrain;
		xmf3Pos.y = pTerrain->GetHeight(fxTerrain, fzTerrain, false);
		//xmf3Pos = XMFLOAT3(1006, 200, 308);
		pTree = new CTrees(xmf3Pos, xmf2Size);
		pTree->SetMesh(pTreeMesh);
		m_ppObjects[i] = pTree;
	}

	m_pd3dTreeInstanceBuffer = CreateInstanceBuffer(pd3dDevice, m_nTrees, m_nInstanceBufferStride, nullptr);
	pTreeMesh->AssembleToVertexBuffer(1, &m_pd3dTreeInstanceBuffer, &m_nInstanceBufferStride, &m_nInstanceBufferOffset);



	m_pTexture = new CTexture(1, 1, TX_SLOT_TEXTURE_ARRAY, 0);
	// 크기 동일
	ID3D11ShaderResourceView * pd3dsrvArray = m_pTexture->CreateTexture2DArraySRV(pd3dDevice, _T("../Assets/Image/Objects/bill"), 1);
	//HRESULT hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Objects/Tree01.png"), nullptr, nullptr, &pd3dsrvArray, nullptr);
	//if (FAILED(hr)) printf("오류 \n");

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

	m_pTexture->SetTexture(0, pd3dsrvArray);
	m_pTexture->SetSampler(0, pd3dSamplerState);

	pd3dsrvArray->Release();
	pd3dSamplerState->Release();



}

void CBillboardShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
	//	pCamera->UpdateCameraPositionCBBuffer(pd3dDeviceContext);

	//m_ppObjects[0]->Render(pd3dDeviceContext, nullptr);
	int nTreeObject = m_nTrees;

	int nTreeInstance = 0;
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dTreeInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	VS_VB_WORLD_POSITION *pnTreeInstances = (VS_VB_WORLD_POSITION *)d3dMappedResource.pData;

	CTrees * pTree = nullptr;
	XMFLOAT3 xmf3Pos;
	for (int j = 0; j < nTreeObject; j++)
	{
		if (m_ppObjects[j])
		{
			pTree = (CTrees*)m_ppObjects[j];

			if (pTree->IsVisible(pCamera))
			{
				xmf3Pos = pTree->m_xv3Pos;
				pnTreeInstances[nTreeInstance].m_xv3Position = XMFLOAT4(xmf3Pos.x, xmf3Pos.y, xmf3Pos.z, 1.0f);
				printf("%0.2f %0.2f %0.2f \n", pnTreeInstances[nTreeInstance].m_xv3Position.x, pnTreeInstances[nTreeInstance].m_xv3Position.y, pnTreeInstances[nTreeInstance].m_xv3Position.z);
				nTreeInstance++;
			}
		}
	}
	pd3dDeviceContext->Unmap(m_pd3dTreeInstanceBuffer, 0);

	CMesh *pTreeMesh = m_ppObjects[0]->GetMesh();
	pTreeMesh->RenderInstanced(pd3dDeviceContext, nTreeInstance, 0);

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
#pragma endregion

CStaticShader::CStaticShader() : CShader()
{
	//m_pTexture = nullptr;
	//m_pMaterial = nullptr;

}

CStaticShader::~CStaticShader()
{
	//if (m_pTexture) m_pTexture->Release();
	//if (m_pMaterial) m_pMaterial->Release();

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

	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSNormalAndSF", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSNormalAndSF", "ps_5_0", &m_pd3dPixelShader);

	//D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	//{
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	//};
	//UINT nElements = ARRAYSIZE(d3dInputElements);
	//CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	//CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader);
	//CreateShaderVariables(pd3dDevice);
}

void CStaticShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial * pMaterial)
{
	//m_pMaterial = pMaterial;
	//if (pMaterial) pMaterial->AddRef();

	//m_pTexture = pTexture;
	//if (pTexture) pTexture->AddRef();

	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;

	CTexture *pSwordTexture = new CTexture(3, 1, 0, 0);
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Objects/medicbag_d.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	ASSERT(SUCCEEDED(hr));
	pSwordTexture->SetTexture(0, pd3dsrvTexture);
	pd3dsrvTexture->Release();

	hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Objects/medicbag_n.png"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	ASSERT(SUCCEEDED(hr));
	pSwordTexture->SetTexture(1, pd3dsrvTexture);
	pd3dsrvTexture->Release();

	hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Objects/medicbag_s.png"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	ASSERT(SUCCEEDED(hr));
	pSwordTexture->SetTexture(2, pd3dsrvTexture);
	pd3dsrvTexture->Release();

	pSwordTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));

	CLoadMeshByChae *pCubeMesh = new CLoadMeshByChae(pd3dDevice, ("../Assets/Image/Objects/Medicbag.chae"), 10.0f);//new CCubeMeshTexturedIlluminated(pd3dDevice, 12.0f, 12.0f, 12.0f);
	//CCubeMeshTexturedIlluminated *pCubeMesh = new CCubeMeshTexturedIlluminated(pd3dDevice);

	m_nObjects = 1;
	m_ppObjects = new CGameObject*[m_nObjects];

	//CMaterial * pMat = new CMaterial();
	//pMat->m_Material.m_xcAmbient = { 1.0f, 1.0f, 1.0f, 1.0f };
	//pMat->m_Material.m_xcDiffuse = { 0.64f, 0.64f, 0.64f, 0.64f };
	//pMat->m_Material.m_xcSpecular = { 0.5f, 0.5f, 0.5f, 0.5f };
	//pMat->m_Material.m_xcEmissive = { 0.0f, 0.0f, 0.0f, 0.0 };

	CGameObject *pObject = nullptr;
	for (int i = 0; i < m_nObjects; i++)
	{
		pObject = new CGameObject(1);
		pObject->SetMesh(pCubeMesh);
	//	pObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	//	pObject->SetRotationSpeed(36.0f * (i % 10) + 36.0f);
		pObject->SetTexture(pSwordTexture);
		pObject->SetMaterial(pMaterial);
		m_ppObjects[i] = pObject;
	}

	m_ppObjects[0]->SetPosition(1085, 230, 220);//(1105, 200, 250);
	//m_ppObjects[1]->SetPosition(1085, 170, 260);
	//m_ppObjects[2]->SetPosition(1115, 170, 265);
	//m_ppObjects[3]->SetPosition(1100, 170, 255);
	//m_ppObjects[4]->SetPosition(1140, 170, 265);

	/// 이상 스테틱 객체들
}

void CStaticShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	//pd3dDeviceContext->PSSetShaderResources(0, 0, nullptr);
	//pd3dDeviceContext->PSSetSamplers(0, 0, nullptr);

	OnPrepareRender(pd3dDeviceContext, uRenderState);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			//카메라의 절두체에 포함되는 객체들만을 렌더링한다. 
			if (m_ppObjects[j]->IsVisible(pCamera))
			{
				m_ppObjects[j]->Render(pd3dDeviceContext, uRenderState, pCamera);
			}
		}
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
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "INFO",		 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	//D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	//{
	//	{ "SIZE", 0, DXGI_FORMAT_R32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	//};
	UINT nElements = ARRAYSIZE(d3dInputLayout);
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSPointSphereInstance", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSPointInstance", "ps_5_0", &m_pd3dPixelShader);
	CreateGeometryShaderFromFile(pd3dDevice, L"Effect.fx", "GSPointSphereInstance", "gs_5_0", &m_pd3dGeometryShader);
}

void CPointInstanceShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial * pMaterial)
{
	CMaterial * pMat = new CMaterial();
	pMat->m_Material.m_xcAmbient = XMFLOAT4(100, 100, 100, 10);
	pMat->m_Material.m_xcDiffuse = XMFLOAT4(100, 100, 100, 10);
	pMat->m_Material.m_xcSpecular = XMFLOAT4(100, 100, 100, 10);
	pMat->m_Material.m_xcEmissive = XMFLOAT4(100, 100, 100, 10);


	m_pMaterial = pMat;//pMaterial;
	pMat->AddRef();//if (pMaterial) pMaterial->AddRef();
	m_nObjects = m_nCubes = 1000;

	XMFLOAT3 xmf3Pos;
	XMFLOAT2 xmf2Size = XMFLOAT2(20, 50);

	//m_nInstanceBufferStride = sizeof(VS_VB_WORLD_POSITION);
	m_nInstanceBufferStride = sizeof(XMFLOAT4);
	m_nInstanceBufferOffset = 0;

	m_ppObjects = new CGameObject*[m_nObjects];

	CHeightMapTerrain * pTerrain = pHeightMapTerrain;
	int cxTerrain = pTerrain->GetWidth();
	int czTerrain = pTerrain->GetLength();

	CPointSphereMesh * pPointMesh = new CPointSphereMesh(pd3dDevice, 10, 10);
	CGameObject *pObject = nullptr;
	for (int i = 0; i < m_nObjects; i++)
	{
		pObject = new CGameObject(1);
		float fx = rand() % cxTerrain;
		float fz = rand() % czTerrain;
		float fy = pTerrain->GetHeight(fx, fz) + 10;
		pObject->SetMesh(pPointMesh);
		//pObject->SetMaterial(pMat);
		pObject->SetPosition(fx, fy, fz);
		m_ppObjects[i] = pObject;

		QUADMgr.EntityStaticObject(pObject);
	}

	m_ppObjects[0]->SetPosition(XMFLOAT3(1098, 190, 350)); 
	//m_ppObjects[1]->SetPosition(XMFLOAT3(1085, 180, 260));
	//m_ppObjects[2]->SetPosition(XMFLOAT3(1115, 180, 265));
	//m_ppObjects[3]->SetPosition(XMFLOAT3(1100, 180, 255)); 
	//m_ppObjects[4]->SetPosition(XMFLOAT3(1140, 180, 265));

	m_pd3dCubeInstanceBuffer = CreateInstanceBuffer(pd3dDevice, m_nCubes, m_nInstanceBufferStride, nullptr);
	pPointMesh->AssembleToVertexBuffer(1, &m_pd3dCubeInstanceBuffer, &m_nInstanceBufferStride, &m_nInstanceBufferOffset);



	m_pTexture = new CTexture(1, 1, 0, 0);
	ID3D11ShaderResourceView * pd3dsrvArray;
	//ID3D11ShaderResourceView * pd3dsrvArray = m_pTexture->CreateTexture2DArraySRV(pd3dDevice, _T("../Assets/Image/Objects/bill"), 1);
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/Marble01.jpg"), nullptr, nullptr, &pd3dsrvArray, nullptr);
	ASSERT(SUCCEEDED(hr));

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

	m_pTexture->SetTexture(0, pd3dsrvArray);
	m_pTexture->SetSampler(0, pd3dSamplerState);

	pd3dsrvArray->Release();
	pd3dSamplerState->Release();

}

void CPointInstanceShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
	if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);

	//m_ppObjects[0]->Render(pd3dDeviceContext, nullptr);
	//int nTreeObject = m_nCubes;

	int nCubeInstance = 0;
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dCubeInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	VS_VB_WORLD_POSITION *pnTreeInstance = (VS_VB_WORLD_POSITION *)d3dMappedResource.pData;

	XMFLOAT3 xmf3Pos;
	for (int j = 0; j < m_nCubes; j++)
	{
		if (m_ppObjects[j])
		{
			if (m_ppObjects[j]->IsVisible(nullptr))// if (m_ppObjects[j]->IsVisible(pCamera))//			{
			{
				xmf3Pos = m_ppObjects[j]->GetPosition();
				pnTreeInstance[nCubeInstance].m_xv3Position = XMFLOAT4(xmf3Pos.x, xmf3Pos.y, xmf3Pos.z, 1.0f);
				//printf("%0.2f %0.2f %0.2f \n", xmf3Pos.x, xmf3Pos.y, xmf3Pos.z);
				nCubeInstance++;
			}
			m_ppObjects[j]->SetActive(false);
		}
	}
	pd3dDeviceContext->Unmap(m_pd3dCubeInstanceBuffer, 0);

	cout << nCubeInstance << "개 그렸습니다." << endl;

	CMesh *pCubeMesh = m_ppObjects[0]->GetMesh();
	pCubeMesh->RenderInstanced(pd3dDeviceContext, uRenderState, nCubeInstance, 0);

}


void CPointInstanceShader::AnimateObjects(float fTimeElapsed)
{
	if (GetAsyncKeyState('O') & 0x0001)
	{
		m_ppObjects[0]->SetPosition(XMFLOAT3(8, 0, 0));
	}
	else if (GetAsyncKeyState('P') & 0x0001)
	{
		m_ppObjects[0]->SetPosition(XMFLOAT3(1098, 190, 350));
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
	//CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSNormalMap", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	//CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSNormalMap", "ps_5_0", &m_pd3dPixelShader);
	
	CreateVertexShaderFromFile(pd3dDevice, L"Effect.fx", "VSBump", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreateHullShaderFromFile(pd3dDevice, L"Effect.fx", "HSBump", "hs_5_0", &m_pd3dHullShader);
	CreateDomainShaderFromFile(pd3dDevice, L"Effect.fx", "DSBump", "ds_5_0", &m_pd3dDomainShader);
	CreatePixelShaderFromFile(pd3dDevice, L"Effect.fx", "PSNormalMap", "ps_5_0", &m_pd3dPixelShader);

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
	d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = 0;
	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);

	CTexture * pTexture = new CTexture(2, 2, 0, 0, (SET_SHADER_VS | SET_SHADER_DS | SET_SHADER_PS));
	ID3D11ShaderResourceView * pd3dsrvArray;
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/stones_nmap.png"), nullptr, nullptr, &pd3dsrvArray, nullptr);
	pTexture->SetTexture(0, pd3dsrvArray);
	pTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvArray->Release();

	hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/stone.png"), nullptr, nullptr, &pd3dsrvArray, nullptr);
	pTexture->SetTexture(1, pd3dsrvArray);
	pTexture->SetSampler(1, pd3dSamplerState);
	pd3dsrvArray->Release();
	pd3dSamplerState->Release();	

	m_pTexture = pTexture;
	m_pMaterial = pMaterial;
	for (int j = 0; j < 4; ++j){
		bool bIsEven = (j % 2) == 0;

		for (int i = 0; i < 8; i++)
		{
			pObject = new CGameObject(1);
			float fx = bIsEven == false ? (j-1) * fHalf + 30 : (i*xPitch) + 128;
			float fz = bIsEven == true ? j * fHalf + 30 : (i*xPitch) + 128;

			float fy = 150;
			pObject->SetMesh(pPointMesh);
			pObject->SetPosition(fx, fy, fz);
			pObject->Rotate(0, (j-2) * 90, 0);
			m_ppObjects[ (j * 8) + i] = pObject;
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

CParticleShader::CParticleShader() : CShader()
{
	m_pd3dSODepthStencilState = nullptr;
	m_pd3dDepthStencilState = nullptr;
	m_pd3dBlendState = nullptr;

	m_nObjects = 0;

	m_ppParticle = nullptr;
	m_pd3dcbGameInfo = nullptr;

	m_pd3dRandomSRV = nullptr;
	m_pd3dSamplerState = nullptr;
	m_ppd3dParticleImageSRV = nullptr;
	m_nImages = 0;
}

CParticleShader::~CParticleShader()
{
	for (int i = 0; i < m_nObjects; ++i)
		delete m_ppParticle[i];

	if (m_pd3dcbGameInfo) m_pd3dcbGameInfo->Release();
	if (m_pd3dSODepthStencilState) m_pd3dSODepthStencilState->Release();
	if (m_pd3dDepthStencilState) m_pd3dDepthStencilState->Release();
	if (m_pd3dBlendState) m_pd3dBlendState->Release();

	if (m_pd3dRandomSRV) m_pd3dRandomSRV->Release();
	if (m_pd3dSamplerState) m_pd3dSamplerState->Release();

	if (m_ppd3dParticleImageSRV)
	{
		for (int i = 0; i < m_nImages; ++i)
			m_ppd3dParticleImageSRV[i]->Release();
		delete[] m_ppd3dParticleImageSRV;
	}

}

void CParticleShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);

	CreateVertexShaderFromFile(pd3dDevice, L"Particle.fx", "VSParticleDraw", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreateGeometryShaderFromFile(pd3dDevice, L"Particle.fx", "GSParticleDraw", "gs_5_0", &m_pd3dGeometryShader);
	CreatePixelShaderFromFile(pd3dDevice, L"Particle.fx", "PSParticleDraw", "ps_5_0", &m_pd3dPixelShader);

	D3D11_SO_DECLARATION_ENTRY SODeclaration[] = 
	{  // 스트림 번호(인덱스)/ 시멘틱이름/ 출력원소 인덱스(같은이름 시멘틱)/ 출력 시작요소/ 출력 요소(0~3:w)/ 연결된 스트림 출력버퍼(0~3)
		{ 0, "POSITION", 0, 0, 3, 0 },
		{ 0, "VELOCITY", 0, 0, 3, 0 },
		{ 0, "SIZE", 0, 0, 2, 0 },
		{ 0, "AGE", 0, 0, 1, 0 },
		{ 0, "TYPE", 0, 0, 1, 0 }
	};
	UINT pBufferStrides[1] = { sizeof(SODeclaration) };

	CreateVertexShaderFromFile(pd3dDevice, L"Particle.fx", "VSParticleSO", "vs_5_0", &m_pd3dVSSO, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreateGeometryStreamOutShaderFromFile(pd3dDevice, L"Particle.fx", "GSParticleSO", "gs_5_0", &m_pd3dGSSO, 
		SODeclaration, 5, pBufferStrides, 1, 0);
}


void CParticleShader::CreateStates(ID3D11Device * pd3dDevice)
{
	D3D11_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	ZeroMemory(&d3dDepthStencilDesc, sizeof(d3dDepthStencilDesc));
	d3dDepthStencilDesc.DepthEnable = false;
	d3dDepthStencilDesc.StencilEnable = false;
	d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	pd3dDevice->CreateDepthStencilState(&d3dDepthStencilDesc, &m_pd3dSODepthStencilState);

	d3dDepthStencilDesc.DepthEnable = true;
	d3dDepthStencilDesc.StencilEnable = false;
	d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; //D3D11_DEPTH_WRITE_MASK_ZERO;
	pd3dDevice->CreateDepthStencilState(&d3dDepthStencilDesc, &m_pd3dDepthStencilState);

	D3D11_BLEND_DESC d3dBlendStateDesc;
	ZeroMemory(&d3dBlendStateDesc, sizeof(D3D11_BLEND_DESC));
	d3dBlendStateDesc.IndependentBlendEnable = false;
	int index = 0;
	ZeroMemory(&d3dBlendStateDesc.RenderTarget[index], sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
	d3dBlendStateDesc.AlphaToCoverageEnable = true;
	d3dBlendStateDesc.RenderTarget[index].BlendEnable = true;
	d3dBlendStateDesc.RenderTarget[index].SrcBlend = D3D11_BLEND_SRC_ALPHA;// D3D11_BLEND_ONE;
	d3dBlendStateDesc.RenderTarget[index].DestBlend = D3D11_BLEND_ONE;
	d3dBlendStateDesc.RenderTarget[index].BlendOp = D3D11_BLEND_OP_ADD;
	d3dBlendStateDesc.RenderTarget[index].SrcBlendAlpha = D3D11_BLEND_ZERO;
	d3dBlendStateDesc.RenderTarget[index].DestBlendAlpha = D3D11_BLEND_ZERO;
	d3dBlendStateDesc.RenderTarget[index].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	d3dBlendStateDesc.RenderTarget[index].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;//D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_ALPHA;
	pd3dDevice->CreateBlendState(&d3dBlendStateDesc, &m_pd3dBlendState);

	D3D11_SAMPLER_DESC d3dSamplerDesc;
	ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = 0;
	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &m_pd3dSamplerState);
}


void CParticleShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial * pMaterial)
{

	
	CreateStates(pd3dDevice);
	CreateShaderVariables(pd3dDevice);

	m_nObjects = 2;
	m_ppParticle = new CParticle*[m_nObjects];
	
	CB_PARTICLE cbParticle;
	cbParticle.m_fLifeTime = 1.0f;
	cbParticle.m_vAccel = XMFLOAT3(0, 10, 0);
	cbParticle.m_vParticleEmitPos = XMFLOAT3(1105, 180, 250);
	cbParticle.m_vParticleVelocity = XMFLOAT3(15, 15, 15);
	cbParticle.m_fNewTime = 0.005f;

	m_ppParticle[0] = new CParticle();
	m_ppParticle[0]->Initialize(pd3dDevice, cbParticle, 20.0, 400);

	m_ppParticle[1] = new CParticle();
	cbParticle.m_fLifeTime = 1.0f;
	cbParticle.m_vAccel = XMFLOAT3(0, 20, 0);
	cbParticle.m_vParticleEmitPos = XMFLOAT3(1105, 180, 250);
	cbParticle.m_vParticleVelocity = XMFLOAT3(15, 15, 15);
	cbParticle.m_fNewTime = 0.005f;
	m_ppParticle[1]->Initialize(pd3dDevice, cbParticle, 1.0f, 400);

	m_pd3dRandomSRV = CShader::CreateRandomTexture1DSRV(pd3dDevice);

	m_nImages = 1;
	m_ppd3dParticleImageSRV = new ID3D11ShaderResourceView*[m_nImages];

	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, 
		_T("../Assets/Image/Resource/particle-cloud.png"), nullptr, nullptr, &m_ppd3dParticleImageSRV[0], nullptr);
	if (FAILED(hr)) printf("오류");
}

void CParticleShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	pd3dDeviceContext->IASetInputLayout(m_pd3dVertexLayout);

	CParticleShader::SOSetState(pd3dDeviceContext);

	pd3dDeviceContext->OMSetDepthStencilState(m_pd3dSODepthStencilState, 0);
	for (int i = 0; i < m_nObjects; ++i)
	{
		if (m_ppParticle[i]->IsAble() == false) continue;
		UpdateShaderVariable(pd3dDeviceContext, m_ppParticle[i]);
		m_ppParticle[i]->StreamOut(pd3dDeviceContext);
	}

	pd3dDeviceContext->VSSetShader(m_pd3dVertexShader, nullptr, 0);
	pd3dDeviceContext->GSSetShader(m_pd3dGeometryShader, nullptr, 0);
	pd3dDeviceContext->PSSetShader(m_pd3dPixelShader, nullptr, 0);
	
	pd3dDeviceContext->PSSetShaderResources(0, m_nImages, m_ppd3dParticleImageSRV);
	pd3dDeviceContext->PSSetSamplers(0, 1, &m_pd3dSamplerState);
	
	pd3dDeviceContext->OMSetDepthStencilState(m_pd3dDepthStencilState, 0);
	pd3dDeviceContext->OMSetBlendState(m_pd3dBlendState, nullptr, 0xffffffff);
	for (int i = 0; i < m_nObjects; ++i)
	{
		if (m_ppParticle[i]->IsAble() == false) continue;
		m_ppParticle[i]->Render(pd3dDeviceContext, uRenderState, pCamera);
	}
	pd3dDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void CParticleShader::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nObjects; ++i)
	{
		if (m_ppParticle[i]->IsAble() == false) continue;
		m_ppParticle[i]->Update(fTimeElapsed);
	}
}


void CParticleShader::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(CB_PARTICLE);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = pd3dDevice->CreateBuffer(&bd, nullptr, &m_pd3dcbGameInfo);
	if (FAILED(hr)) 
		printf("오류입니다!!");
}

void CParticleShader::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, CParticle * pParticle)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbGameInfo, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	CB_PARTICLE *pcbParticle = (CB_PARTICLE *)d3dMappedResource.pData;
	memcpy(pcbParticle, pParticle->GetCBParticle(), sizeof(CB_PARTICLE));
	//*pcbParticle = *pParticle->GetCBParticle();
	//cout << pcbParticle->m_fTime << "// " << pcbParticle->m_fTimeStep << "// " << pcbParticle->m_vParticleEmitPos.x << ", " << pcbParticle->m_vParticleEmitPos.y << ", "<< pcbParticle->m_vParticleEmitPos.z << endl;
	pd3dDeviceContext->Unmap(m_pd3dcbGameInfo, 0);

	//상수 버퍼를 디바이스의 슬롯(CB_SLOT_WORLD_MATRIX)에 연결한다.
	pd3dDeviceContext->VSSetConstantBuffers(0x04, 1, &m_pd3dcbGameInfo);
	pd3dDeviceContext->GSSetConstantBuffers(0x04, 1, &m_pd3dcbGameInfo);
}

void CParticleShader::SOSetState(ID3D11DeviceContext * pd3dDeviceContext)
{
	pd3dDeviceContext->HSSetShader(nullptr, nullptr, 0);
	pd3dDeviceContext->DSSetShader(nullptr, nullptr, 0);
	pd3dDeviceContext->VSSetShader(m_pd3dVSSO, nullptr, 0);
	pd3dDeviceContext->GSSetShader(m_pd3dGSSO, nullptr, 0);
	pd3dDeviceContext->PSSetShader(nullptr, nullptr, 0);
	pd3dDeviceContext->OMSetDepthStencilState(m_pd3dSODepthStencilState, 0);

	pd3dDeviceContext->GSSetShaderResources(9, 1, &m_pd3dRandomSRV);
	pd3dDeviceContext->GSSetSamplers(8, 1, &m_pd3dSamplerState);

	pd3dDeviceContext->RSSetState(nullptr);
}
