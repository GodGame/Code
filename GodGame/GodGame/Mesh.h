#pragma once
#include "MeshType.h"

class CMirrorMesh : public CMeshTexturedIlluminated
{
public:
	CMirrorMesh(ID3D11Device *pd3dDevice, float fWidth = 2.f, float fHeight = 2.f);
	virtual ~CMirrorMesh();
};
#define SPLAT_TERRAIN
/*지형 메쉬의 각 정점은 두 개의 텍스쳐 좌표를 갖는다.*/
#ifdef DETAIL_TERRAIN
class CHeightMapGridMesh : public CMeshDetailTexturedIlluminated
#else
#ifdef SPLAT_TERRAIN
class CHeightMapGridMesh : public CMeshSplatTexturedIlluminated
#endif
#endif
{
protected:
	//격자의 크기(가로: x-방향, 세로: z-방향)이다.
	int m_nWidth;
	int m_nLength;
	/*격자의 스케일(가로: x-방향, 세로: z-방향, 높이: y-방향) 벡터이다. 실제 격자 메쉬의 각 정점의 x-좌표, y-좌표, z-좌표는 스케일 벡터의 x-좌표, y-좌표, z-좌표로 곱한 값을 갖는다. 즉, 실제 격자의 x-축 방향의 간격은 1이 아니라 스케일 벡터의 x-좌표가 된다. 이렇게 하면 작은 격자를 사용하더라도 큰 격자를 생성할 수 있다.*/
	XMFLOAT3 m_xv3Scale;

public:
	CHeightMapGridMesh(ID3D11Device *pd3dDevice, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xv3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), void *pContext = nullptr);
	virtual ~CHeightMapGridMesh();

	XMFLOAT3 GetScale() { return(m_xv3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }

	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
	//virtual void RenderReflected(ID3D11DeviceContext *pd3dDeviceContext);
	//격자의 교점(정점)의 높이를 설정한다.
	virtual float OnGetHeight(int x, int z, void *pContext);
	//격자의 교점(정점)의 색상을 설정한다.
	virtual XMFLOAT4 OnGetColor(int x, int z, void *pContext);
};



class CTerrainPartMesh : public CMesh
{
protected:
	/*격자의 스케일(가로: x-방향, 세로: z-방향, 높이: y-방향) 벡터이다. 실제 격자 메쉬의 각 정점의 x-좌표, y-좌표, z-좌표는 스케일 벡터의 x-좌표, y-좌표, z-좌표로 곱한 값을 갖는다. 즉, 실제 격자의 x-축 방향의 간격은 1이 아니라 스케일 벡터의 x-좌표가 된다. 이렇게 하면 작은 격자를 사용하더라도 큰 격자를 생성할 수 있다.*/
	XMFLOAT3 m_xv3Scale;

public:
	CTerrainPartMesh(ID3D11Device *pd3dDevice, float xStart, float zStart, int nWidth, int nLength, XMFLOAT3 xv3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), void *pContext = nullptr);
	virtual ~CTerrainPartMesh();

	XMFLOAT3 GetScale() { return(m_xv3Scale); }

	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
	//virtual void RenderReflected(ID3D11DeviceContext *pd3dDeviceContext);
	//격자의 교점(정점)의 높이를 설정한다.
	virtual float OnGetHeight(int x, int z, void *pContext);
	//격자의 교점(정점)의 색상을 설정한다.
	virtual void CreateRasterizerState(ID3D11Device *pd3dDevice);
};

class CSkyBoxMesh : public CMeshTextured
{
protected:
	ID3D11DepthStencilState *m_pd3dDepthStencilState;

	CTexture *m_pSkyboxTexture;

public:
	CSkyBoxMesh(ID3D11Device *pd3dDevice, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f);
	virtual ~CSkyBoxMesh();

	void OnChangeSkyBoxTextures(ID3D11Device *pd3dDevice, int nIndex = 0);

	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
};

class CTreeVertex : public CMesh
{
public:
	CTreeVertex(ID3D11Device *pd3dDevice, float fWSize, float fHSize);
	~CTreeVertex();
};

class CPointCubeMesh : public CMesh
{
protected:
	ID3D11Buffer * m_pd3dSizeBuffer;
public:
	CPointCubeMesh(ID3D11Device *pd3dDevice, float fSize);
	~CPointCubeMesh();
};


class CPointSphereMesh : public CMesh
{
protected:
	ID3D11Buffer * m_pd3dSizeBuffer;
public:
	CPointSphereMesh(ID3D11Device *pd3dDevice, float nSlices, float fSize);
	~CPointSphereMesh();
};

class CBezierMesh : public CMesh
{
public:
	CBezierMesh(ID3D11Device *pd3dDevice);
	~CBezierMesh();
};

class CNormalCube : public CMesh
{
public:
	CNormalCube(ID3D11Device *pd3dDevice, float fWidth, float fHeight);
	~CNormalCube();
	virtual void CreateRasterizerState(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
};