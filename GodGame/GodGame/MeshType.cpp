#include "stdafx.h"
#include "MeshType.h"
#include "MyInline.h"

CMesh::CMesh(ID3D11Device *pd3dDevice)
{
	m_nBuffers = 0;
	m_pd3dPositionBuffer = nullptr;
	//m_pd3dColorBuffer = nullptr;
	m_ppd3dVertexBuffers = nullptr;

	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_nSlot = 0;
	m_nStartVertex = 0;

	m_pd3dIndexBuffer = nullptr;
	m_nIndices = 0;
	m_nStartIndex = 0;
	m_nBaseVertex = 0;
	m_nIndexOffset = 0;
	m_dxgiIndexFormat = DXGI_FORMAT_R32_UINT;

	m_pd3dRasterizerState = nullptr;

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	m_nReferences = 0;

	m_pxv3Positions = nullptr;
	m_pnIndices = nullptr;
}


CMesh::~CMesh()
{
	if (m_pd3dRasterizerState) m_pd3dRasterizerState->Release();
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();
	//if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();

	if (m_ppd3dVertexBuffers) delete[] m_ppd3dVertexBuffers;
	if (m_pnVertexStrides) delete[] m_pnVertexStrides;
	if (m_pnVertexOffsets) delete[] m_pnVertexOffsets;

	if (m_pxv3Positions) delete[] m_pxv3Positions;
	if (m_pnIndices) delete[] m_pnIndices;
}

void CMesh::AssembleToVertexBuffer(int nBuffers, ID3D11Buffer **ppd3dBuffers, UINT *pnBufferStrides, UINT *pnBufferOffsets)
{
	ID3D11Buffer **ppd3dNewVertexBuffers = new ID3D11Buffer*[m_nBuffers + nBuffers];
	UINT *pnNewVertexStrides = new UINT[m_nBuffers + nBuffers];
	UINT *pnNewVertexOffsets = new UINT[m_nBuffers + nBuffers];

	if (m_nBuffers > 0)
	{
		for (int i = 0; i < m_nBuffers; i++)
		{
			ppd3dNewVertexBuffers[i] = m_ppd3dVertexBuffers[i];
			pnNewVertexStrides[i] = m_pnVertexStrides[i];
			pnNewVertexOffsets[i] = m_pnVertexOffsets[i];
		}
		if (m_ppd3dVertexBuffers) delete[] m_ppd3dVertexBuffers;
		if (m_pnVertexStrides) delete[] m_pnVertexStrides;
		if (m_pnVertexOffsets) delete[] m_pnVertexOffsets;
	}

	for (int i = 0; i < nBuffers; i++)
	{
		ppd3dNewVertexBuffers[m_nBuffers + i] = ppd3dBuffers[i];
		pnNewVertexStrides[m_nBuffers + i] = pnBufferStrides[i];
		pnNewVertexOffsets[m_nBuffers + i] = pnBufferOffsets[i];
	}

	m_nBuffers += nBuffers;
	m_ppd3dVertexBuffers = ppd3dNewVertexBuffers;
	m_pnVertexStrides = pnNewVertexStrides;
	m_pnVertexOffsets = pnNewVertexOffsets;
}

void CMesh::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState)
{
	//�޽��� ������ ���� ���� ���� ���۷� ǥ���ȴ�.
	pd3dDeviceContext->IASetVertexBuffers(m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets);
	pd3dDeviceContext->IASetIndexBuffer(m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset);
	pd3dDeviceContext->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	if (!(uRenderState & RS_SHADOWMAP)) pd3dDeviceContext->RSSetState(m_pd3dRasterizerState);

	if (m_pd3dIndexBuffer)
	{
		pd3dDeviceContext->DrawIndexed(m_nIndices, m_nStartIndex, m_nBaseVertex);
	}
	else
		pd3dDeviceContext->Draw(m_nVertices, m_nStartVertex);

}

void CMesh::RenderInstanced(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, int nInstances, int nStartInstance)
{
	//�ν��Ͻ��� ��� �Է� �����⿡ �޽��� ���� ���ۿ� �ν��Ͻ� ���� ���۰� ����ȴ�.
	pd3dDeviceContext->IASetVertexBuffers(m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets);
	pd3dDeviceContext->IASetIndexBuffer(m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset);
	pd3dDeviceContext->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	if(uRenderState == 0 )
		pd3dDeviceContext->RSSetState(m_pd3dRasterizerState);

	//��ü���� �ν��Ͻ����� �������Ѵ�. 
	if (m_pd3dIndexBuffer)
		pd3dDeviceContext->DrawIndexedInstanced(m_nIndices, nInstances, m_nStartIndex, m_nBaseVertex, nStartInstance);
	else
		pd3dDeviceContext->DrawInstanced(m_nVertices, nInstances, m_nStartVertex, nStartInstance);

}


void CMesh::CreateRasterizerState(ID3D11Device *pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	//�����Ͷ����� �ܰ迡�� �ø�(���� ����)�� ���� �ʵ��� �����Ͷ����� ���¸� �����Ѵ�.
	d3dRasterizerDesc.CullMode = D3D11_CULL_NONE;
	d3dRasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	d3dRasterizerDesc.DepthClipEnable = true;
	pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}

#ifdef PICKING
int CMesh::CheckRayIntersection(XMFLOAT3 *pxv3RayPosition, XMFLOAT3 *pxv3RayDirection, MESHINTERSECTINFO *pd3dxIntersectInfo)
{
	//�� ��ǥ���� ������ ������(pxv3RayPosition)�� ������ �־��� �� �޽����� �浹 �˻縦 �Ѵ�.
	int nIntersections = 0;
	BYTE *pbPositions = (BYTE *)m_pxv3Positions + m_pnVertexOffsets[0];

	int nOffset = (m_d3dPrimitiveTopology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? 3 : 1;
	//�޽��� ������Ƽ��(�ﰢ��)���� �����̴�. �ﰢ�� ����Ʈ�� ��� (������ ���� / 3) �Ǵ� (�ε����� ���� / 3), �ﰢ�� ��Ʈ���� ��� (������ ���� - 2) �Ǵ� (�ε����� ���� - 2)�̴�.
	int nPrimitives = (m_d3dPrimitiveTopology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? (m_nVertices / 3) : (m_nVertices - 2);
	if (m_nIndices > 0) nPrimitives = (m_d3dPrimitiveTopology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? (m_nIndices / 3) : (m_nIndices - 2);

	XMFLOAT3 v0, v1, v2;
	float fuHitBaryCentric, fvHitBaryCentric, fHitDistance, fNearHitDistance = FLT_MAX;
	/*�޽��� ��� ������Ƽ��(�ﰢ��)�鿡 ���Ͽ� ��ŷ �������� �浹�� �˻��Ѵ�. �浹�ϴ� ��� �ﰢ���� ã�� ������ ������(�����δ� ī�޶� ��ǥ���� ����)�� ���� ����� �ﰢ���� ã�´�.*/
	for (int i = 0; i < nPrimitives; i++)
	{
		v0 = *(XMFLOAT3 *)(pbPositions + ((m_pnIndices) ? (m_pnIndices[(i*nOffset) + 0]) : ((i*nOffset) + 0)) * m_pnVertexStrides[0]);
		v1 = *(XMFLOAT3 *)(pbPositions + ((m_pnIndices) ? (m_pnIndices[(i*nOffset) + 1]) : ((i*nOffset) + 1)) * m_pnVertexStrides[0]);
		v2 = *(XMFLOAT3 *)(pbPositions + ((m_pnIndices) ? (m_pnIndices[(i*nOffset) + 2]) : ((i*nOffset) + 2)) * m_pnVertexStrides[0]);
		//XMV

		if (D3DXIntersectTri(&v0, &v1, &v2, pxv3RayPosition, pxv3RayDirection, &fuHitBaryCentric, &fvHitBaryCentric, &fHitDistance))
		{
			if (fHitDistance < fNearHitDistance)
			{
				fNearHitDistance = fHitDistance;
				if (pd3dxIntersectInfo)
				{
					pd3dxIntersectInfo->m_dwFaceIndex = i;
					pd3dxIntersectInfo->m_fU = fuHitBaryCentric;
					pd3dxIntersectInfo->m_fV = fvHitBaryCentric;
					pd3dxIntersectInfo->m_fDistance = fHitDistance;
				}
			}
			nIntersections++;
		}
	}
	return(nIntersections);
}
#endif


void AABB::Union(XMFLOAT3& xv3Minimum, XMFLOAT3& xv3Maximum)
{
	if (xv3Minimum.x < m_xv3Minimum.x) m_xv3Minimum.x = xv3Minimum.x;
	if (xv3Minimum.y < m_xv3Minimum.y) m_xv3Minimum.y = xv3Minimum.y;
	if (xv3Minimum.z < m_xv3Minimum.z) m_xv3Minimum.z = xv3Minimum.z;
	if (xv3Maximum.x > m_xv3Maximum.x) m_xv3Maximum.x = xv3Maximum.x;
	if (xv3Maximum.y > m_xv3Maximum.y) m_xv3Maximum.y = xv3Maximum.y;
	if (xv3Maximum.z > m_xv3Maximum.z) m_xv3Maximum.z = xv3Maximum.z;
}

void AABB::Union(AABB *pAABB)
{
	Union(pAABB->m_xv3Minimum, pAABB->m_xv3Maximum);
}

void AABB::Update(XMFLOAT4X4 *pmtxTransform)
{
	XMVECTOR vVertices[8];
	vVertices[0] = XMVectorSet(m_xv3Minimum.x, m_xv3Minimum.y, m_xv3Minimum.z, 1);
	vVertices[1] = XMVectorSet(m_xv3Minimum.x, m_xv3Minimum.y, m_xv3Maximum.z, 1);
	vVertices[2] = XMVectorSet(m_xv3Maximum.x, m_xv3Minimum.y, m_xv3Maximum.z, 1);
	vVertices[3] = XMVectorSet(m_xv3Maximum.x, m_xv3Minimum.y, m_xv3Minimum.z, 1);
	vVertices[4] = XMVectorSet(m_xv3Minimum.x, m_xv3Maximum.y, m_xv3Minimum.z, 1);
	vVertices[5] = XMVectorSet(m_xv3Minimum.x, m_xv3Maximum.y, m_xv3Maximum.z, 1);
	vVertices[6] = XMVectorSet(m_xv3Maximum.x, m_xv3Maximum.y, m_xv3Maximum.z, 1);
	vVertices[7] = XMVectorSet(m_xv3Maximum.x, m_xv3Maximum.y, m_xv3Minimum.z, 1);
	m_xv3Minimum = XMFLOAT3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	m_xv3Maximum = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	//8���� �������� x, y, z ��ǥ�� �ּҰ��� �ִ밪�� ���Ѵ�.
	XMMATRIX mtxTransform;
	XMFLOAT4 xmvVertcies;
	for (int i = 0; i < 8; i++)
	{
		//������ ��ȯ�Ѵ�.
		mtxTransform = XMLoadFloat4x4(pmtxTransform);
		vVertices[i] = XMVector3TransformCoord(vVertices[i], mtxTransform);
		XMStoreFloat4(&xmvVertcies, vVertices[i]);
		//xv3ec3TransformCoord(&vVertices[i], &vVertices[i], pmtxTransform);
		if (xmvVertcies.x < m_xv3Minimum.x) m_xv3Minimum.x = xmvVertcies.x;
		if (xmvVertcies.y < m_xv3Minimum.y) m_xv3Minimum.y = xmvVertcies.y;
		if (xmvVertcies.z < m_xv3Minimum.z) m_xv3Minimum.z = xmvVertcies.z;
		if (xmvVertcies.x > m_xv3Maximum.x) m_xv3Maximum.x = xmvVertcies.x;
		if (xmvVertcies.y > m_xv3Maximum.y) m_xv3Maximum.y = xmvVertcies.y;
		if (xmvVertcies.z > m_xv3Maximum.z) m_xv3Maximum.z = xmvVertcies.z;
	}
}
#pragma region BasicMesh

CMeshDiffused::CMeshDiffused(ID3D11Device *pd3dDevice) : CMesh(pd3dDevice)
{
	m_pd3dColorBuffer = nullptr;
}

CMeshDiffused::~CMeshDiffused()
{
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
}

CCubeMeshDiffused::CCubeMeshDiffused(ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth, XMFLOAT4 xmcolor) : CMeshDiffused(pd3dDevice)
{
	m_nVertices = 8;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	//������ü �޽��� 2���� ���� ����(��ġ ���� ���ۿ� ���� ����)�� �����ȴ�.
	//������ü �޽��� ���� ����(��ġ ���� ����)�� �����Ѵ�.

	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	m_pxv3Positions[0] = XMFLOAT3(-fx, +fy, -fz);
	m_pxv3Positions[1] = XMFLOAT3(+fx, +fy, -fz);
	m_pxv3Positions[2] = XMFLOAT3(+fx, +fy, +fz);
	m_pxv3Positions[3] = XMFLOAT3(-fx, +fy, +fz);
	m_pxv3Positions[4] = XMFLOAT3(-fx, -fy, -fz);
	m_pxv3Positions[5] = XMFLOAT3(+fx, -fy, -fz);
	m_pxv3Positions[6] = XMFLOAT3(+fx, -fy, +fz);
	m_pxv3Positions[7] = XMFLOAT3(-fx, -fy, +fz);

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

	//������ü �޽��� ���� ����(���� ����)�� �����Ѵ�.
	XMFLOAT4 pXMFLOAT4s[8];
	XMVECTOR xmvColor = XMLoadFloat4(&xmcolor);
	for (int i = 0; i < 8; i++)
		XMStoreFloat4(&pXMFLOAT4s[i], xmvColor + XMVectorSet(RANDOM_COLOR, RANDOM_COLOR, RANDOM_COLOR, RANDOM_COLOR));

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT4)* m_nVertices;
	d3dBufferData.pSysMem = pXMFLOAT4s;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dColorBuffer);

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dColorBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT4) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	/*�ε��� ���۴� ������ü�� 6���� ��(�簢��)�� ���� ���� ������ ���´�. �ﰢ�� ����Ʈ�� ������ü�� ǥ���� ���̹Ƿ� �� ���� 2���� �ﰢ���� ������ �� �ﰢ���� 3���� ������ �ʿ��ϴ�. ��, �ε��� ���۴� ��ü 36(=6*2*3)���� �ε����� ������.*/
	m_nIndices = 36;

	m_pnIndices = new UINT[m_nIndices];
	//�� �ո�(Front) �簢���� ���� �ﰢ��
	m_pnIndices[0] = 3; m_pnIndices[1] = 1; m_pnIndices[2] = 0;
	//�� �ո�(Front) �簢���� �Ʒ��� �ﰢ��
	m_pnIndices[3] = 2; m_pnIndices[4] = 1; m_pnIndices[5] = 3;
	//�� ����(Top) �簢���� ���� �ﰢ��
	m_pnIndices[6] = 0; m_pnIndices[7] = 5; m_pnIndices[8] = 4;
	//�� ����(Top) �簢���� �Ʒ��� �ﰢ��
	m_pnIndices[9] = 1; m_pnIndices[10] = 5; m_pnIndices[11] = 0;
	//�� �޸�(Back) �簢���� ���� �ﰢ��
	m_pnIndices[12] = 3; m_pnIndices[13] = 4; m_pnIndices[14] = 7;
	//�� �޸�(Back) �簢���� �Ʒ��� �ﰢ��
	m_pnIndices[15] = 0; m_pnIndices[16] = 4; m_pnIndices[17] = 3;
	//�� �Ʒ���(Bottom) �簢���� ���� �ﰢ��
	m_pnIndices[18] = 1; m_pnIndices[19] = 6; m_pnIndices[20] = 5;
	//�� �Ʒ���(Bottom) �簢���� �Ʒ��� �ﰢ��
	m_pnIndices[21] = 2; m_pnIndices[22] = 6; m_pnIndices[23] = 1;
	//�� ����(Left) �簢���� ���� �ﰢ��
	m_pnIndices[24] = 2; m_pnIndices[25] = 7; m_pnIndices[26] = 6;
	//�� ����(Left) �簢���� �Ʒ��� �ﰢ��
	m_pnIndices[27] = 3; m_pnIndices[28] = 7; m_pnIndices[29] = 2;
	//�� ����(Right) �簢���� ���� �ﰢ��
	m_pnIndices[30] = 6; m_pnIndices[31] = 4; m_pnIndices[32] = 5;
	//�� ����(Right) �簢���� �Ʒ��� �ﰢ��
	m_pnIndices[33] = 7; m_pnIndices[34] = 4; m_pnIndices[35] = 6;

	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pnIndices;
	//�ε��� ���۸� �����Ѵ�.
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	CreateRasterizerState(pd3dDevice);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, -fz);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, +fz);
}

CCubeMeshDiffused::~CCubeMeshDiffused()
{
}

/////////////////////////////////////////

CSphereMeshDiffused::CSphereMeshDiffused(ID3D11Device *pd3dDevice, float fRadius, int nSlices, int nStacks, XMFLOAT4 xmcolor) : CMeshDiffused(pd3dDevice)
{
	m_nVertices = (nSlices * nStacks) * 3 * 2;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	//�� �޽��� 2���� ���� ����(��ġ ���� ���ۿ� ���� ����)�� �����ȴ�.
	//�� �޽��� ���� ����(��ġ ���� ����)�� �����Ѵ�.
	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	float theta_i, theta_ii, phi_j, phi_jj, fRadius_j, fRadius_jj, y_j, y_jj;
	for (int j = 0, k = 0; j < nStacks; j++)
	{
		phi_j = float(PI / nStacks) * j;
		phi_jj = float(PI / nStacks) * (j + 1);
		fRadius_j = fRadius * sinf(phi_j);
		fRadius_jj = fRadius * sinf(phi_jj);
		y_j = fRadius*cosf(phi_j);
		y_jj = fRadius*cosf(phi_jj);
		for (int i = 0; i < nSlices; i++)
		{
			theta_i = float(2 * PI / nSlices) * i;
			theta_ii = float(2 * PI / nSlices) * (i + 1);
			m_pxv3Positions[k++] = XMFLOAT3(fRadius_j*cosf(theta_i), y_j, fRadius_j*sinf(theta_i));
			m_pxv3Positions[k++] = XMFLOAT3(fRadius_jj*cosf(theta_i), y_jj, fRadius_jj*sinf(theta_i));
			m_pxv3Positions[k++] = XMFLOAT3(fRadius_j*cosf(theta_ii), y_j, fRadius_j*sinf(theta_ii));
			m_pxv3Positions[k++] = XMFLOAT3(fRadius_jj*cosf(theta_i), y_jj, fRadius_jj*sinf(theta_i));
			m_pxv3Positions[k++] = XMFLOAT3(fRadius_jj*cosf(theta_ii), y_jj, fRadius_jj*sinf(theta_ii));
			m_pxv3Positions[k++] = XMFLOAT3(fRadius_j*cosf(theta_ii), y_j, fRadius_j*sinf(theta_ii));
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

	//�� �޽��� ���� ����(���� ����)�� �����Ѵ�.
	XMFLOAT4 pXMFLOAT4s[8];
	XMVECTOR xmvColor = XMLoadFloat4(&xmcolor);
	for (int i = 0; i < m_nVertices; i++)
		XMStoreFloat4(&pXMFLOAT4s[i], xmvColor + XMVectorSet(RANDOM_COLOR, RANDOM_COLOR, RANDOM_COLOR, RANDOM_COLOR));

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT4)* m_nVertices;
	d3dBufferData.pSysMem = pXMFLOAT4s;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dColorBuffer);

	delete[] pXMFLOAT4s;

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dColorBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT4) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fRadius, -fRadius, -fRadius);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fRadius, +fRadius, +fRadius);
}

CSphereMeshDiffused::~CSphereMeshDiffused()
{
}

CMeshTextured::CMeshTextured(ID3D11Device *pd3dDevice) : CMesh(pd3dDevice)
{
	m_pd3dTexCoordBuffer = nullptr;
}

CMeshTextured::~CMeshTextured()
{
	if (m_pd3dTexCoordBuffer) m_pd3dTexCoordBuffer->Release();
}

CMeshDetailTextured::CMeshDetailTextured(ID3D11Device *pd3dDevice) : CMeshTextured(pd3dDevice)
{
	m_pd3dDetailTexCoordBuffer = nullptr;
}

CMeshDetailTextured::~CMeshDetailTextured()
{
	if (m_pd3dDetailTexCoordBuffer) m_pd3dDetailTexCoordBuffer->Release();
}

////////////////////////////
CMeshTexturedIlluminated::CMeshTexturedIlluminated(ID3D11Device *pd3dDevice) : CMeshIlluminated(pd3dDevice)
{
	m_pd3dTexCoordBuffer = nullptr;
}

CMeshTexturedIlluminated::~CMeshTexturedIlluminated()
{
	if (m_pd3dTexCoordBuffer) m_pd3dTexCoordBuffer->Release();
}

CMeshDetailTexturedIlluminated::CMeshDetailTexturedIlluminated(ID3D11Device *pd3dDevice) : CMeshIlluminated(pd3dDevice)
{
	m_pd3dTexCoordBuffer = nullptr;
	m_pd3dDetailTexCoordBuffer = nullptr;
}

CMeshDetailTexturedIlluminated::~CMeshDetailTexturedIlluminated()
{
	if (m_pd3dTexCoordBuffer) m_pd3dTexCoordBuffer->Release();
	if (m_pd3dDetailTexCoordBuffer) m_pd3dDetailTexCoordBuffer->Release();
}


CMeshSplatTexturedIlluminated::CMeshSplatTexturedIlluminated(ID3D11Device *pd3dDevice) : CMeshIlluminated(pd3dDevice)
{
	m_pd3dTexCoordBuffer = nullptr;
	m_pd3dAlphaTexCoordBuffer = nullptr;
}

CMeshSplatTexturedIlluminated::~CMeshSplatTexturedIlluminated()
{
	if (m_pd3dTexCoordBuffer) m_pd3dTexCoordBuffer->Release();
	if (m_pd3dAlphaTexCoordBuffer) m_pd3dAlphaTexCoordBuffer->Release();
}





CCubeMeshIlluminated::CCubeMeshIlluminated(ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth) : CMeshIlluminated(pd3dDevice)
{
	m_nVertices = 8;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	m_pxv3Positions[0] = XMFLOAT3(-fx, +fy, -fz);
	m_pxv3Positions[1] = XMFLOAT3(+fx, +fy, -fz);
	m_pxv3Positions[2] = XMFLOAT3(+fx, +fy, +fz);
	m_pxv3Positions[3] = XMFLOAT3(-fx, +fy, +fz);
	m_pxv3Positions[4] = XMFLOAT3(-fx, -fy, -fz);
	m_pxv3Positions[5] = XMFLOAT3(+fx, -fy, -fz);
	m_pxv3Positions[6] = XMFLOAT3(+fx, -fy, +fz);
	m_pxv3Positions[7] = XMFLOAT3(-fx, -fy, +fz);

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

	m_nIndices = 36;
	m_pnIndices = new UINT[m_nIndices];

	m_pnIndices[0] = 3; m_pnIndices[1] = 1; m_pnIndices[2] = 0;
	m_pnIndices[3] = 2; m_pnIndices[4] = 1; m_pnIndices[5] = 3;
	m_pnIndices[6] = 0; m_pnIndices[7] = 5; m_pnIndices[8] = 4;
	m_pnIndices[9] = 1; m_pnIndices[10] = 5; m_pnIndices[11] = 0;
	m_pnIndices[12] = 3; m_pnIndices[13] = 4; m_pnIndices[14] = 7;
	m_pnIndices[15] = 0; m_pnIndices[16] = 4; m_pnIndices[17] = 3;
	m_pnIndices[18] = 1; m_pnIndices[19] = 6; m_pnIndices[20] = 5;
	m_pnIndices[21] = 2; m_pnIndices[22] = 6; m_pnIndices[23] = 1;
	m_pnIndices[24] = 2; m_pnIndices[25] = 7; m_pnIndices[26] = 6;
	m_pnIndices[27] = 3; m_pnIndices[28] = 7; m_pnIndices[29] = 2;
	m_pnIndices[30] = 6; m_pnIndices[31] = 4; m_pnIndices[32] = 5;
	m_pnIndices[33] = 7; m_pnIndices[34] = 4; m_pnIndices[35] = 6;

	XMFLOAT3 pxv3Normals[8];
	CalculateVertexNormal(pxv3Normals); // ���

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferData.pSysMem = pxv3Normals;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer);

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pnIndices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, -fz);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, +fz);
}

CCubeMeshIlluminated::~CCubeMeshIlluminated()
{
}
#pragma endregion BasicMesh



CMeshIlluminated::CMeshIlluminated(ID3D11Device *pd3dDevice) : CMesh(pd3dDevice)
{
	m_pd3dNormalBuffer = nullptr;
}

CMeshIlluminated::~CMeshIlluminated()
{
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
}

void CMeshIlluminated::CalculateVertexNormal(XMFLOAT3 *pxv3Normals)
{
	switch (m_d3dPrimitiveTopology)
	{
		/*������Ƽ�갡 �ﰢ�� ����Ʈ�� �� �ε��� ���۰� �ִ� ���� ���� ��츦 �����Ͽ� ������ ���� ���͸� ����Ѵ�. �ε��� ���۸� ������� �ʴ� ��� �� ������ ���� ���ʹ� �� ������ ���Ե� �ﰢ���� ���� ���ͷ� ����Ѵ�. �ε��� ���۸� ����ϴ� ��� �� ������ ���� ���ʹ� �� ������ ���Ե� �ﰢ������ ���� ������ �������(���Ͽ�) ����Ѵ�.*/
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		if (m_pnIndices)
			SetAverageVertexNormal(pxv3Normals, (m_nIndices / 3), 3, false);
		else
			SetTriAngleListVertexNormal(pxv3Normals);
		break;
		/*������Ƽ�갡 �ﰢ�� ��Ʈ���� �� �� ������ ���� ���ʹ� �� ������ ���Ե� �ﰢ������ ���� ������ �������(���Ͽ�) ����Ѵ�.*/
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		SetAverageVertexNormal(pxv3Normals, (m_nIndices) ? (m_nIndices - 2) : (m_nVertices - 2), 1, true);
		break;
	default:
		break;
	}
}
void CMeshIlluminated::SetTriAngleListVertexNormal(XMFLOAT3 *pxv3Normals)
{
	/*�ﰢ��(������Ƽ��)�� ������ ���ϰ� �� �ﰢ���� ���� ���͸� ����ϰ� �ﰢ���� �����ϴ� �� ������ ���� ���ͷ� �����Ѵ�.*/
	int nPrimitives = m_nVertices / 3;
	for (int i = 0; i < nPrimitives; i++)
	{
		XMFLOAT3 xv3Normal = CalculateTriAngleNormal((i * 3 + 0), (i * 3 + 1), (i * 3 + 2));
		pxv3Normals[i * 3 + 0] = pxv3Normals[i * 3 + 1] = pxv3Normals[i * 3 + 2] = xv3Normal;
	}
}

XMFLOAT3 CMeshIlluminated::CalculateTriAngleNormal(UINT nIndex0, UINT nIndex1, UINT nIndex2)
{
	XMVECTOR xv3Normal;
	XMVECTOR xv3P0 = XMLoadFloat3(&m_pxv3Positions[nIndex0]);
	XMVECTOR xv3P1 = XMLoadFloat3(&m_pxv3Positions[nIndex1]);
	XMVECTOR xv3P2 = XMLoadFloat3(&m_pxv3Positions[nIndex2]);
	XMVECTOR xv3Edge1 = xv3P1 - xv3P0;
	XMVECTOR xv3Edge2 = xv3P2 - xv3P0;

	xv3Normal = XMVector3Cross(xv3Edge1, xv3Edge2);
	xv3Normal = XMVector3Normalize(xv3Normal);

	XMFLOAT3 xmf3Normal;
	XMStoreFloat3(&xmf3Normal, xv3Normal);
	return(xmf3Normal);
}

void CMeshIlluminated::SetAverageVertexNormal(XMFLOAT3 *pxv3Normals, int nPrimitives, int nOffset, bool bStrip)
{
	for (int j = 0; j < m_nVertices; j++)
	{
		XMVECTOR xvSumOfNormal = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		for (int i = 0; i < nPrimitives; i++)
		{
			UINT nIndex0 = (bStrip) ? (((i % 2) == 0) ? (i*nOffset + 0) : (i*nOffset + 1)) : (i*nOffset + 0);
			if (m_pnIndices) nIndex0 = m_pnIndices[nIndex0];
			UINT nIndex1 = (bStrip) ? (((i % 2) == 0) ? (i*nOffset + 1) : (i*nOffset + 0)) : (i*nOffset + 1);
			if (m_pnIndices) nIndex1 = m_pnIndices[nIndex1];
			UINT nIndex2 = (m_pnIndices) ? m_pnIndices[i*nOffset + 2] : (i*nOffset + 2);
			if ((nIndex0 == j) || (nIndex1 == j) || (nIndex2 == j))
			{
				xvSumOfNormal += XMLoadFloat3(&CalculateTriAngleNormal(nIndex0, nIndex1, nIndex2));
			}
		}
		xvSumOfNormal = XMVector3Normalize(xvSumOfNormal);
		XMStoreFloat3(&pxv3Normals[j], xvSumOfNormal);
		//		pxv3Normals[j] = xv3SumOfNormal;
	}
}

CCubeMeshTextured::CCubeMeshTextured(ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth) : CMeshTextured(pd3dDevice)
{
	m_nVertices = 36;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	XMFLOAT2 pxv2TexCoords[36];
	int i = 0;

	//������ü�� �� ��(�ﰢ�� 2��)�� �ϳ��� �ؽ��� �̹��� ��ü�� ���εǵ��� �ؽ��� ��ǥ�� �����Ѵ�.
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

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

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	/*������ü�� �� �鿡 �ؽ��ĸ� �����Ϸ��� �ε����� ����� �� �����Ƿ� �ε��� ���۴� �������� �ʴ´�.*/

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, -fz);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, +fz);
}

CCubeMeshTextured::~CCubeMeshTextured()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
CSphereMeshTextured::CSphereMeshTextured(ID3D11Device *pd3dDevice, float fRadius, int nSlices, int nStacks) : CMeshTextured(pd3dDevice)
{
	m_nVertices = (nSlices * nStacks) * 3 * 2;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	XMFLOAT2 *xmf2TexCoords = new XMFLOAT2[m_nVertices];

	float theta_i, theta_ii, phi_j, phi_jj, fRadius_j, fRadius_jj, y_j, y_jj;
	for (int j = 0, k = 0; j < nStacks; j++)
	{
		phi_j = float(PI / nStacks) * j;
		phi_jj = float(PI / nStacks) * (j + 1);
		fRadius_j = fRadius * sinf(phi_j);
		fRadius_jj = fRadius * sinf(phi_jj);
		y_j = fRadius * cosf(phi_j);
		y_jj = fRadius * cosf(phi_jj);
		for (int i = 0; i < nSlices; i++)
		{
			theta_i = float(2 * PI / nSlices) * i;
			theta_ii = float(2 * PI / nSlices) * (i + 1);
			m_pxv3Positions[k] = XMFLOAT3(fRadius_j*cosf(theta_i), y_j, fRadius_j*sinf(theta_i));
			xmf2TexCoords[k++] = XMFLOAT2(float(i) / float(nSlices), float(j) / float(nStacks));
			m_pxv3Positions[k] = XMFLOAT3(fRadius_jj*cosf(theta_i), y_jj, fRadius_jj*sinf(theta_i));
			xmf2TexCoords[k++] = XMFLOAT2(float(i) / float(nSlices), float(j + 1) / float(nStacks));
			m_pxv3Positions[k] = XMFLOAT3(fRadius_j*cosf(theta_ii), y_j, fRadius_j*sinf(theta_ii));
			xmf2TexCoords[k++] = XMFLOAT2(float(i + 1) / float(nSlices), float(j) / float(nStacks));
			m_pxv3Positions[k] = XMFLOAT3(fRadius_jj*cosf(theta_i), y_jj, fRadius_jj*sinf(theta_i));
			xmf2TexCoords[k++] = XMFLOAT2(float(i) / float(nSlices), float(j + 1) / float(nStacks));
			m_pxv3Positions[k] = XMFLOAT3(fRadius_jj*cosf(theta_ii), y_jj, fRadius_jj*sinf(theta_ii));
			xmf2TexCoords[k++] = XMFLOAT2(float(i + 1) / float(nSlices), float(j + 1) / float(nStacks));
			m_pxv3Positions[k] = XMFLOAT3(fRadius_j*cosf(theta_ii), y_j, fRadius_j*sinf(theta_ii));
			xmf2TexCoords[k++] = XMFLOAT2(float(i + 1) / float(nSlices), float(j) / float(nStacks));
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

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = xmf2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	delete[] xmf2TexCoords;

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fRadius, -fRadius, -fRadius);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fRadius, +fRadius, +fRadius);
}

CSphereMeshTextured::~CSphereMeshTextured()
{
}

/////////////////////////////////////////////////////////////////////////////////////////

CCubeMeshTexturedIlluminated::CCubeMeshTexturedIlluminated(ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth) : CMeshTexturedIlluminated(pd3dDevice)
{
	m_nVertices = 36;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	XMFLOAT2 pxv2TexCoords[36];
	int i = 0;

	//������ü�� �� ��(�ﰢ�� 2��)�� �ϳ��� �ؽ��� �̹��� ��ü�� ���εǵ��� �ؽ��� ��ǥ�� �����Ѵ�.
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	//�ո�

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	// �Ʒ���
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	// ���� ��
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

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

	//���� ���͸� �����ϱ� ���� ���� �ڵ带 �߰��Ѵ�.
	XMFLOAT3 pxv3Normals[36];
	CalculateVertexNormal(pxv3Normals);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferData.pSysMem = pxv3Normals;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	//������ ��ġ ����, ���� ����, �ؽ��� ��ǥ�� ���´�.
	ID3D11Buffer *pd3dBuffers[3] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[3] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[3] = { 0, 0, 0 };
	AssembleToVertexBuffer(3, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, -fz);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, +fz);
}

CCubeMeshTexturedIlluminated::~CCubeMeshTexturedIlluminated()
{
}

CSphereMeshTexturedIlluminated::CSphereMeshTexturedIlluminated(ID3D11Device *pd3dDevice, float fRadius, int nSlices, int nStacks) : CMeshTexturedIlluminated(pd3dDevice)
{
	m_nVertices = (nSlices * nStacks) * 3 * 2;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	XMFLOAT3 *pxv3Normals = new XMFLOAT3[m_nVertices];
	XMFLOAT2 *xmf2TexCoords = new XMFLOAT2[m_nVertices];

	float theta_i, theta_ii, phi_j, phi_jj, fRadius_j, fRadius_jj, y_j, y_jj;
	for (int j = 0, k = 0; j < nStacks; j++)
	{
		phi_j = float(PI / nStacks) * j;
		phi_jj = float(PI / nStacks) * (j + 1);
		fRadius_j = fRadius * sinf(phi_j);
		fRadius_jj = fRadius * sinf(phi_jj);
		y_j = fRadius*cosf(phi_j);
		y_jj = fRadius*cosf(phi_jj);
		for (int i = 0; i < nSlices; i++)
		{
			theta_i = float(2 * PI / nSlices) * i;
			theta_ii = float(2 * PI / nSlices) * (i + 1);
			m_pxv3Positions[k] = XMFLOAT3(fRadius_j*cosf(theta_i), y_j, fRadius_j*sinf(theta_i));
			xmf2TexCoords[k] = XMFLOAT2(float(i) / float(nSlices), float(j) / float(nStacks));
			Chae::XMFloat3Normalize(&pxv3Normals[k], &m_pxv3Positions[k]); k++;
			m_pxv3Positions[k] = XMFLOAT3(fRadius_jj*cosf(theta_i), y_jj, fRadius_jj*sinf(theta_i));
			xmf2TexCoords[k] = XMFLOAT2(float(i) / float(nSlices), float(j + 1) / float(nStacks));
			Chae::XMFloat3Normalize(&pxv3Normals[k], &m_pxv3Positions[k]); k++;
			m_pxv3Positions[k] = XMFLOAT3(fRadius_j*cosf(theta_ii), y_j, fRadius_j*sinf(theta_ii));
			xmf2TexCoords[k] = XMFLOAT2(float(i + 1) / float(nSlices), float(j) / float(nStacks));
			Chae::XMFloat3Normalize(&pxv3Normals[k], &m_pxv3Positions[k]); k++;
			m_pxv3Positions[k] = XMFLOAT3(fRadius_jj*cosf(theta_i), y_jj, fRadius_jj*sinf(theta_i));
			xmf2TexCoords[k] = XMFLOAT2(float(i) / float(nSlices), float(j + 1) / float(nStacks));
			Chae::XMFloat3Normalize(&pxv3Normals[k], &m_pxv3Positions[k]); k++;
			m_pxv3Positions[k] = XMFLOAT3(fRadius_jj*cosf(theta_ii), y_jj, fRadius_jj*sinf(theta_ii));
			xmf2TexCoords[k] = XMFLOAT2(float(i + 1) / float(nSlices), float(j + 1) / float(nStacks));
			Chae::XMFloat3Normalize(&pxv3Normals[k], &m_pxv3Positions[k]); k++;
			m_pxv3Positions[k] = XMFLOAT3(fRadius_j*cosf(theta_ii), y_j, fRadius_j*sinf(theta_ii));
			xmf2TexCoords[k] = XMFLOAT2(float(i + 1) / float(nSlices), float(j) / float(nStacks));
			Chae::XMFloat3Normalize(&pxv3Normals[k], &m_pxv3Positions[k]); k++;
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

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferData.pSysMem = pxv3Normals;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = xmf2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	delete[] pxv3Normals;
	delete[] xmf2TexCoords;

	//������ ��ġ ����, ���� ����, �ؽ��� ��ǥ�� ���´�.
	ID3D11Buffer *pd3dBuffers[3] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[3] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[3] = { 0, 0, 0 };
	AssembleToVertexBuffer(3, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fRadius, -fRadius, -fRadius);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fRadius, +fRadius, +fRadius);
}

CSphereMeshTexturedIlluminated::~CSphereMeshTexturedIlluminated()
{
}




CPlaneMesh::CPlaneMesh(ID3D11Device * pd3dDevice, int fx, int fy) : CMeshTextured(pd3dDevice)
{
	m_nVertices = 4;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	XMFLOAT2 pxv2TexCoords[4];
	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	int i = 0;
	m_pxv3Positions[0] = XMFLOAT3(-fx, +fy, 0); pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[1] = XMFLOAT3(+fx, +fy, 0); pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[2] = XMFLOAT3(-fx, -fy, 0); pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	//m_pxv3Positions[3] = XMFLOAT3(+fx, +fy, 0); pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[3] = XMFLOAT3(+fx, -fy, 0); pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	//m_pxv3Positions[5] = XMFLOAT3(-fx, -fy, 0); pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

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

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer,  m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);


	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-FLT_MIN, -FLT_MIN, 0);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+FLT_MAX, +FLT_MAX, 0);
}



CPlaneMesh::~CPlaneMesh()
{

}
