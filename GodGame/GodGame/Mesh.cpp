#include "stdafx.h"
#include "MyInline.h"
#include "Mesh.h"
#include "Object.h"

/////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
CHeightMapGridMesh::CHeightMapGridMesh(ID3D11Device *pd3dDevice, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xv3Scale, void *pContext) 
#ifdef DETAIL_TERRAIN
: CMeshDetailTexturedIlluminated(pd3dDevice)
#else
#ifdef SPLAT_TERRAIN
: CMeshSplatTexturedIlluminated(pd3dDevice)
#endif
#endif
{
	m_nVertices = nWidth * nLength;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	XMFLOAT3 *pxv3Normals = new XMFLOAT3[m_nVertices];
	XMFLOAT2 *pxv2TexCoords = new XMFLOAT2[m_nVertices];
	XMFLOAT2 *pxv2AlphaTexCoords = new XMFLOAT2[m_nVertices];

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xv3Scale = xv3Scale;

	CHeightMap *pHeightMap = (CHeightMap *)pContext;
	int cxHeightMap = pHeightMap->GetHeightMapWidth();
	int czHeightMap = pHeightMap->GetHeightMapLength();
	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	/*xStart와 zStart는 격자의 시작 위치(x-좌표와 z-좌표)를 나타낸다. 지형을 격자들의 이차원 배열로 만들 것이기 때문에 지형에서 각 격자의 시작 위치를 나타내는 정보가 필요하다. <그림 18>은 격자의 교점(정점)을 나열하는 순서를 보여준다.*/
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			//정점의 높이와 색상을 높이 맵으로부터 구한다.
			fHeight = OnGetHeight(x, z, pContext) ;
			m_pxv3Positions[i] = XMFLOAT3((x*m_xv3Scale.x), fHeight, (z*m_xv3Scale.z));
			pxv3Normals[i] = pHeightMap->GetHeightMapNormal(x, z);
			pxv2TexCoords[i] = XMFLOAT2(float(x) / float(m_xv3Scale.x*0.5f), float(z) / float(m_xv3Scale.z*0.5f));
			pxv2AlphaTexCoords[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
#ifdef DETAIL_TERRAIN
			pxv3DetailTexCoords[i] = XMFLOAT2(float(x) / float(m_xv3Scale.x*0.5f), float(z) / float(m_xv3Scale.z*0.5f));
#endif
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);
	//법선벡터
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferData.pSysMem = pxv3Normals;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer);
	//텍스쳐
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);
#ifdef DETAIL_TERRAIN
	//디테일텍스쳐
	d3dBufferData.pSysMem = pxv3DetailTexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dDetailTexCoordBuffer);

	ID3D11Buffer *pd3dBuffers[4] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer, m_pd3dDetailTexCoordBuffer };
	UINT pnBufferStrides[4] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT2), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[4] = { 0, 0, 0, 0 };
	AssembleToVertexBuffer(4, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	if (pxv3Normals) delete[] pxv3Normals;
	if (pxv2TexCoords) delete[] pxv2TexCoords;
	if (pxv3DetailTexCoords) delete[] pxv3DetailTexCoords;
#else
#ifdef SPLAT_TERRAIN
	d3dBufferData.pSysMem = pxv2AlphaTexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dAlphaTexCoordBuffer);

	ID3D11Buffer *pd3dBuffers[4] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer, m_pd3dAlphaTexCoordBuffer };
	UINT pnBufferStrides[4] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT2), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[4] = { 0, 0, 0, 0 };
	AssembleToVertexBuffer(4, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	if (pxv3Normals) delete[] pxv3Normals;
	if (pxv2TexCoords) delete[] pxv2TexCoords;
	if (pxv2AlphaTexCoords) delete[] pxv2AlphaTexCoords;
	//if (pxv3DetailTexCoords) delete[] pxv3DetailTexCoords;
	// 와인딩 순서 주의
#endif
#endif

	m_nIndices = ((nWidth * 2)*(nLength - 1)) + ((nLength - 1) - 1);
	m_pnIndices = new UINT[m_nIndices];
	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			//홀수 번째 줄이므로(z = 0, 2, 4, ...) 인덱스의 나열 순서는 왼쪽에서 오른쪽 방향이다.
			for (int x = 0; x < nWidth; x++)
			{
				//첫 번째 줄을 제외하고 줄이 바뀔 때마다(x == 0) 첫 번째 인덱스를 추가한다.
				if ((x == 0) && (z > 0)) m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				//아래, 위의 순서로 인덱스를 추가한다.
				m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				m_pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			//짝수 번째 줄이므로(z = 1, 3, 5, ...) 인덱스의 나열 순서는 오른쪽에서 왼쪽 방향이다.
			for (int x = nWidth - 1; x >= 0; x--)
			{
				//줄이 바뀔 때마다(x == (nWidth-1)) 첫 번째 인덱스를 추가한다.
				if (x == (nWidth - 1)) m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				//아래, 위의 순서로 인덱스를 추가한다.
				m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				m_pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}

	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pnIndices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(xStart*m_xv3Scale.x, fMinHeight, zStart*m_xv3Scale.z);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3((xStart + nWidth)*m_xv3Scale.x, fMaxHeight, (zStart + nLength)*m_xv3Scale.z);
	//CreateRasterizerState(pd3dDevice);
}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
}

float CHeightMapGridMesh::OnGetHeight(int x, int z, void *pContext)
{
	//높이 맵 객체의 높이 맵 이미지의 픽셀 값을 지형의 높이로 반환한다. 
	CHeightMap *pHeightMap = (CHeightMap *)pContext;
	BYTE *pHeightMapImage = pHeightMap->GetHeightMapImage();
	//printf("HEIGHT : %d \t", *pHeightMapImage);
	XMFLOAT3 xv3Scale = pHeightMap->GetScale();
	int cxTerrain = pHeightMap->GetHeightMapWidth();
	float fHeight = pHeightMapImage[x + (z*cxTerrain)] * xv3Scale.y;
	return(fHeight);
}

XMFLOAT4 CHeightMapGridMesh::OnGetColor(int x, int z, void *pContext)
{
	//조명의 방향 벡터(정점에서 조명까지의 벡터)이다.
	//XMVECTOR xv3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	XMVECTOR xv3LightDirection = XMVectorSet(-1.0f, 1.0f, 1.0f, 0.0f);
	xv3LightDirection = XMVector3Normalize(xv3LightDirection);

	XMFLOAT3 xmf3LightDirecton;
	XMStoreFloat3(&xmf3LightDirecton, xv3LightDirection);

	CHeightMap *pHeightMap = (CHeightMap *)pContext;
	XMFLOAT3 xv3Scale = pHeightMap->GetScale();

	/*정점 (x, z)에서 조명이 반사되는 양은 정점 (x, z)의 법선 벡터와 조명의 방향 벡터의 내적(cos)과 인접한 3개의 점 (x+1, z), (x, z+1), (x+1, z+1)의 법선 벡터와 조명의 방향 벡터의 내적을 평균하여 구한다. 정점 (x, z)의 색상은 조명 색상(세기)과 반사되는 양을 곱한 값이다.*/

	float fScale = Chae::XMFloat3Dot(&pHeightMap->GetHeightMapNormal(x, z), &xmf3LightDirecton);
	fScale += Chae::XMFloat3Dot(&pHeightMap->GetHeightMapNormal(x + 1, z), &xmf3LightDirecton);
	fScale += Chae::XMFloat3Dot(&pHeightMap->GetHeightMapNormal(x + 1, z + 1), &xmf3LightDirecton);
	fScale += Chae::XMFloat3Dot(&pHeightMap->GetHeightMapNormal(x, z + 1), &xmf3LightDirecton);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;

	//조명의 색상(세기)이다. 
	XMVECTOR vIncidentLight = XMVectorSet(0.9f, 0.8f, 0.4f, 1.0f);

	XMFLOAT4 xmf4Light;
	XMStoreFloat4(&xmf4Light, fScale * vIncidentLight);
	return(xmf4Light);
}

void CHeightMapGridMesh::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState)
{
	CMesh::Render(pd3dDeviceContext, uRenderState);
}



CTerrainPartMesh::CTerrainPartMesh(ID3D11Device *pd3dDevice, float xStart, float zStart, int nWidth, int nLength, XMFLOAT3 xv3Scale, void *pContext) : CMesh(pd3dDevice)
{
	m_nVertices = 4;//nWidth * nLength;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	CTexture2Vertex * pxvInfo = new CTexture2Vertex[m_nVertices];

	m_xv3Scale = xv3Scale;

	CHeightMap *pHeightMap = (CHeightMap *)pContext;
	int cxHeightMap = pHeightMap->GetHeightMapWidth();
	int czHeightMap = pHeightMap->GetHeightMapLength();
	float fHeight = 0.0f, fMinHeight = 0, fMaxHeight = 256 * xv3Scale.y;

	int index = 0;
	float xSize = xv3Scale.x, zSize = xv3Scale.z;
	float xPos, zPos;
	
	for (float z = 0.0; z < 2.0; ++z){
		for (float x = 0.0; x < 2.0; ++x){
			xPos = nWidth * (xStart + x), zPos = nLength * (zStart + z);
			m_pxv3Positions[index] = XMFLOAT3(xPos, 100, zPos);
			pxvInfo[index++] = { m_pxv3Positions[index], XMFLOAT2((xStart + x) / xSize, (zSize - z - zStart) / zSize)};
			printf("%f %f \n", xStart / xSize, (zSize - zStart) / zSize);
		}
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	d3dBufferDesc.ByteWidth = sizeof(CTexture2Vertex)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pxvInfo; //m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	if (pxvInfo) delete[] pxvInfo;

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dPositionBuffer };
	UINT pnBufferStrides[1] = { sizeof(CTexture2Vertex) };
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);


	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(m_pxv3Positions[0].x, 0, m_pxv3Positions[0].z);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(m_pxv3Positions[3].x, fMaxHeight, m_pxv3Positions[3].z);
	CreateRasterizerState(pd3dDevice);
}

CTerrainPartMesh::~CTerrainPartMesh()
{
}

float CTerrainPartMesh::OnGetHeight(int x, int z, void *pContext)
{
	//높이 맵 객체의 높이 맵 이미지의 픽셀 값을 지형의 높이로 반환한다. 
	CHeightMap *pHeightMap = (CHeightMap *)pContext;
	BYTE *pHeightMapImage = pHeightMap->GetHeightMapImage();
	//printf("HEIGHT : %d \t", *pHeightMapImage);
	XMFLOAT3 xv3Scale = pHeightMap->GetScale();
	int cxTerrain = pHeightMap->GetHeightMapWidth();
	float fHeight = pHeightMapImage[x + (z*cxTerrain)] * xv3Scale.y;
	return(fHeight);
}

void CTerrainPartMesh::CreateRasterizerState(ID3D11Device *pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	//래스터라이저 단계에서 컬링(은면 제거)을 하지 않도록 래스터라이저 상태를 생성한다.
	d3dRasterizerDesc.CullMode = D3D11_CULL_NONE;
	d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;//WIREFRAME;
	d3dRasterizerDesc.DepthClipEnable =false;
	pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}

void CTerrainPartMesh::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState)
{
	CMesh::Render(pd3dDeviceContext, uRenderState);
}

CSkyBoxMesh::CSkyBoxMesh(ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth) : CMeshTextured(pd3dDevice)
{
	//스카이 박스는 6개의 면(사각형), 사각형은 정점 4개, 그러므로 24개의 정점이 필요하다.
	m_nVertices = 24;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	XMFLOAT2 *pxv2TexCoords = new XMFLOAT2[m_nVertices];

	int i = 0;
	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;
	// Front Quad 
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	// Back Quad
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	// Left Quad
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	// Right Quad
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	// Top Quad
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	// Bottom Quad
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	D3D11_BUFFER_DESC d3dBufferDesc;
	::ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	::ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	delete[] pxv2TexCoords;

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dPositionBuffer };
	UINT pnBufferStrides[1] = { sizeof(XMFLOAT3)};
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	//삼각형 스트립으로 사각형 1개를 그리기 위해 인덱스는 4개가 필요하다.
	m_nIndices = 4;
	m_pnIndices = new UINT[m_nIndices];

	m_pnIndices[0] = 0;
	m_pnIndices[1] = 1;
	m_pnIndices[2] = 3;
	m_pnIndices[3] = 2;

	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pnIndices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	D3D11_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	//스카이 박스 사각형들은 깊이 버퍼 알고리즘을 적용하지 않고 깊이 버퍼를 변경하지 않는다.
	d3dDepthStencilDesc.DepthEnable = false;
	d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D11_COMPARISON_NEVER;
	d3dDepthStencilDesc.StencilEnable = false;
	d3dDepthStencilDesc.StencilReadMask = 0xFF;
	d3dDepthStencilDesc.StencilWriteMask = 0xFF;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	pd3dDevice->CreateDepthStencilState(&d3dDepthStencilDesc, &m_pd3dDepthStencilState);

	ID3D11SamplerState *pd3dSamplerState = nullptr;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = 0;
	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);

	m_pSkyboxTexture = new CTexture(1, 1, TX_SLOT_CUBE_TEXTURE, SS_SLOT_CUBE_SAMPLER_STATE);

	m_pSkyboxTexture->SetSampler(0, pd3dSamplerState);
	pd3dSamplerState->Release();
	m_pSkyboxTexture->AddRef();

	OnChangeSkyBoxTextures(pd3dDevice, 6);

	//CMesh::CreateRasterizerState(pd3dDevice);
}

CSkyBoxMesh::~CSkyBoxMesh()
{
	if (m_pd3dDepthStencilState) m_pd3dDepthStencilState->Release();
	if (m_pSkyboxTexture) m_pSkyboxTexture->Release();
}

void CSkyBoxMesh::OnChangeSkyBoxTextures(ID3D11Device *pd3dDevice, int nIndex)
{
	_TCHAR pstrTextureNames[128];
	_stprintf_s(pstrTextureNames, _T("../Assets/Image/SkyBox/SkyBox_%d.dds"), nIndex, 128);
	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, pstrTextureNames, nullptr, nullptr, &pd3dsrvTexture, nullptr);
	m_pSkyboxTexture->SetTexture(0, pd3dsrvTexture); 
	pd3dsrvTexture->Release();

}

void CSkyBoxMesh::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState)
{
	//CMesh::Render(pd3dDeviceContext);
	pd3dDeviceContext->IASetVertexBuffers(m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets);
	pd3dDeviceContext->IASetIndexBuffer(m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset);
	pd3dDeviceContext->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	//pd3dDeviceContext->RSSetState(m_pd3dRasterizerState);

	//스카이 박스를 그리기 위한 샘플러 상태 객체와 깊이 스텐실 상태 객체를 설정한다.
	m_pSkyboxTexture->UpdateShaderVariable(pd3dDeviceContext);

	//m_pSkyboxTexture->UpdateSamplerShaderVariable(pd3dDeviceContext, 0, 2);
	//m_pSkyboxTexture->UpdateTextureShaderVariable(pd3dDeviceContext, 0, 8);

	pd3dDeviceContext->OMSetDepthStencilState(m_pd3dDepthStencilState, 1);
	for (int i = 0; i < 6; i++)
	{
		pd3dDeviceContext->DrawIndexed(4, 0, i * 4);
	}

	pd3dDeviceContext->OMSetDepthStencilState(nullptr, 1);
}


CMirrorMesh::CMirrorMesh(ID3D11Device *pd3dDevice, float fWidth, float fHeight) : CMeshTexturedIlluminated(pd3dDevice)
{
	m_nVertices = 6;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	float fx = fWidth*0.5f, fy = fHeight*0.5f;

	//직육면체 메쉬는 2개의 정점 버퍼(위치 벡터 버퍼와 색상 버퍼)로 구성된다.
	//직육면체 메쉬의 정점 버퍼(위치 벡터 버퍼)를 생성한다.
	XMFLOAT2 pxv2TexCoords[6];
	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	
	int i = 0;
	m_pxv3Positions[0] = XMFLOAT3(-fx, +fy, 0); pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[1] = XMFLOAT3(+fx, +fy, 0); pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[2] = XMFLOAT3(-fx, -fy, 0); pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	m_pxv3Positions[3] = XMFLOAT3(+fx, +fy, 0); pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[4] = XMFLOAT3(+fx, -fy, 0); pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[5] = XMFLOAT3(-fx, -fy, 0); pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	//법선 벡터를 생성하기 위한 다음 코드를 추가한다.
	XMFLOAT3 pxv3Normals[6];
	CalculateVertexNormal(pxv3Normals);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferData.pSysMem = pxv3Normals;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	//정점은 위치 벡터, 법선 벡터, 텍스쳐 좌표를 갖는다.
	ID3D11Buffer *pd3dBuffers[3] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[3] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[3] = { 0, 0, 0 };
	AssembleToVertexBuffer(3, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	/*인덱스 버퍼는 직육면체의 6개의 면(사각형)에 대한 기하 정보를 갖는다. 삼각형 리스트로 직육면체를 표현할 것이므로 각 면은 2개의 삼각형을 가지고 각 삼각형은 3개의 정점이 필요하다. 즉, 인덱스 버퍼는 전체 36(=6*2*3)개의 인덱스를 가진다.*/
	
	//CreateRasterizerState(pd3dDevice);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, 0);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, 0);
}

CMirrorMesh::~CMirrorMesh()
{
}

CTreeVertex::CTreeVertex(ID3D11Device *pd3dDevice, float fWSize, float fHSize) : CMesh(pd3dDevice)
{
	m_nVertices = 1;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

	//직육면체 메쉬는 2개의 정점 버퍼(위치 벡터 버퍼와 색상 버퍼)로 구성된다.
	//직육면체 메쉬의 정점 버퍼(위치 벡터 버퍼)를 생성한다.
	
	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	m_pxv3Positions[0] = XMFLOAT3(0, 0, 0);//XMFLOAT3(1006, 200, 308);;

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);
	
	//직육면체 메쉬의 정점 버퍼(색상 버퍼)를 생성한다.
	XMFLOAT2 * pxmf2Size = new XMFLOAT2[m_nVertices];
	
	pxmf2Size[0] = XMFLOAT2(fWSize, fHSize);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxmf2Size;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dColorBuffer);
	
	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dColorBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	if (pxmf2Size) delete[] pxmf2Size;

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fWSize, -fHSize, -1);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fWSize, +fHSize, +1);
}

CTreeVertex::~CTreeVertex()
{
}


CPointCubeMesh::CPointCubeMesh(ID3D11Device *pd3dDevice, float fSize) : CMesh(pd3dDevice)
{
	m_nVertices = 1;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

	float pSize[1] = { fSize };

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(float) *m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pSize;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dSizeBuffer);

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dSizeBuffer };
	UINT pnBufferStrides[1] = { sizeof(float) };
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fSize, -fSize, -1);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(fSize, fSize, +1);
}

CPointCubeMesh::~CPointCubeMesh()
{
}



CPointSphereMesh::CPointSphereMesh(ID3D11Device *pd3dDevice,float nSlices, float fSize) : CMesh(pd3dDevice)
{
	m_nVertices = nSlices * nSlices;

	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	
	XMFLOAT4 * pxmf4StackInfo= new XMFLOAT4[m_nVertices];
	//float *pSize = new float[m_nVertices];
	for (int i = 0, index = 0; i < nSlices; ++i)
	{
		for (int j = 0; j < nSlices; ++j)
		{
			//pSize[index] = fSize;
			pxmf4StackInfo[index].x = i;
			pxmf4StackInfo[index].y = j;
			pxmf4StackInfo[index].z = nSlices;
			pxmf4StackInfo[index++].w = fSize;
		}
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT4) * m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pxmf4StackInfo;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dSizeBuffer);

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dSizeBuffer };
	UINT pnBufferStrides[1] = { sizeof(XMFLOAT4) };
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);


	//d0elete[] pSize;
	delete[] pxmf4StackInfo;

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fSize, -fSize, -1);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(fSize, fSize, +1);
}

CPointSphereMesh::~CPointSphereMesh()
{
}

CBezierMesh::CBezierMesh(ID3D11Device *pd3dDevice) : CMesh(pd3dDevice)
{
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
	//m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_nVertices = 16;
	XMFLOAT3 pQuadPatchVertices[16] =
	{
		XMFLOAT3(-10.0f, -20.0f, 15.0f), XMFLOAT3(-5.0f, 0.0f, 15.0f), XMFLOAT3(5.0f, 0.0f, 15.0f), XMFLOAT3(10.0f, 0.0f, 15.0f),
		XMFLOAT3(-15.0f, 0.0f, 5.0f), XMFLOAT3(-5.0f, 0.0f, 5.0f), XMFLOAT3(5.0f, 30.0f, 5.0f), XMFLOAT3(15.0f, 0.0f, 5.0f),
		XMFLOAT3(-15.0f, 0.0f, -5.0f), XMFLOAT3(-5.0f, 0.0f, -5.0f), XMFLOAT3(5.0f, 0.0f, -5.0f), XMFLOAT3(15.0f, 0.0f, -5.0f),
		XMFLOAT3(-10.0f, 20.0f, -15.0f), XMFLOAT3(-5.0f, 0.0f, -15.0f), XMFLOAT3(5.0f, 0.0f, -15.0f), XMFLOAT3(25.0f, 20.0f, -15.0f)
	};

	CTexture2Vertex pInfo[16];
	for (int i = 0; i < 16; ++i)
		pInfo[i] = { pQuadPatchVertices[i], XMFLOAT2(0, 0) };

	D3D11_BUFFER_DESC d3dBufferDesc;
	d3dBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	d3dBufferDesc.ByteWidth = sizeof(CTexture2Vertex)* m_nVertices;		// 1개를 의미
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	d3dBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	d3dBufferData.pSysMem = pInfo;// pQuadPatchVertices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dPositionBuffer };
	UINT pnBufferStrides[1] = { sizeof(CTexture2Vertex) };
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);


	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-15, -20, -15);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+25, +20, +15);

	CreateRasterizerState(pd3dDevice);
}

CBezierMesh::~CBezierMesh()
{

}

CNormalCube::CNormalCube(ID3D11Device *pd3dDevice, float fWidth, float fHeight) : CMesh(pd3dDevice)
{
	m_nVertices = 4;
	//m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; 
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = 1;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	NormalMapVertex * pNormalMap = new NormalMapVertex[m_nVertices];
	int i = 0;

	//직육면체의 각 면(삼각형 2개)에 하나의 텍스쳐 이미지 전체가 맵핑되도록 텍스쳐 좌표를 설정한다.
	pNormalMap[0] = { m_pxv3Positions[0] = XMFLOAT3(-fx, +fy, 0), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)};
	pNormalMap[1] = { m_pxv3Positions[1] = XMFLOAT3(+fx, +fy, 0), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)};
	pNormalMap[2] = { m_pxv3Positions[2] = XMFLOAT3(-fx, -fy, 0), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)};
	//pNormalMap[3] = { m_pxv3Positions[3] = XMFLOAT3(+fx, +fy, 0), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) };
	pNormalMap[3] = { m_pxv3Positions[3] = XMFLOAT3(+fx, -fy, 0), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)};
	//pNormalMap[5] = { m_pxv3Positions[5] = XMFLOAT3(-fx, -fy, 0), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) };

	D3D11_BUFFER_DESC d3dBufferDesc;
	d3dBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	d3dBufferDesc.ByteWidth = sizeof(NormalMapVertex)* m_nVertices;		// 1개를 의미
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	d3dBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	d3dBufferData.pSysMem = pNormalMap;// pQuadPatchVertices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dPositionBuffer };
	UINT pnBufferStrides[1] = { sizeof(NormalMapVertex) };
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	delete[] pNormalMap;

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, -1);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, +1);

	//m_nIndices = 4;
	//m_pnIndices = new UINT[m_nIndices];


	//ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	//d3dBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	//d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	//d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//d3dBufferDesc.CPUAccessFlags = 0;
	//ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	//d3dBufferData.pSysMem = m_pnIndices;
	//pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);


	CreateRasterizerState(pd3dDevice);
}

CNormalCube::~CNormalCube()
{

}


void CNormalCube::CreateRasterizerState(ID3D11Device *pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	//래스터라이저 단계에서 컬링(은면 제거)을 하지 않도록 래스터라이저 상태를 생성한다.
	d3dRasterizerDesc.CullMode = D3D11_CULL_NONE;
	d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	d3dRasterizerDesc.DepthClipEnable = true;

	//pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}

void CNormalCube::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState)
{
	CMesh::Render(pd3dDeviceContext, uRenderState);
}
