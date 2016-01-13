#pragma once
#include "CollisionMgr.h"
#define RANDOM_COLOR ((rand() * 0xFFFFFF) / RAND_MAX)

class CTexture;
class AABB;
struct POSANDSIZE{
	XMFLOAT3 m_xmf3Pos;
	float		 m_fSize;
};

struct MESHINTERSECTINFO {
	DWORD m_dwFaceIndex;
	float m_fU;
	float m_fV;
	float m_fDistance;
};

struct CTexture2Vertex
{
	XMFLOAT3 m_xv3Position;
	XMFLOAT2 m_xv2Tex;
};

struct CNormalMapVertex
{
	XMFLOAT3 m_xv3Position;
	XMFLOAT3 m_xv3Normal;
	XMFLOAT3 m_xv3Tangent;
	XMFLOAT2 m_xv2Tex;
};

class CVertex
{
	XMFLOAT3 m_xv3Position;		//������ ��ġ ����(3���� ����)�� �����ϱ� ���� ��� ������ �����Ѵ�. 
public:
	CVertex() { m_xv3Position = XMFLOAT3(0, 0, 0); }
	CVertex(XMFLOAT3 xv3Position) { m_xv3Position = xv3Position; }
	~CVertex() { }
};

class CDiffusedVertex
{
	XMFLOAT3 m_xv3Position;
	//������ ������ ��Ÿ���� ��� ����(XMFLOAT4 ����ü)�� �����Ѵ�. 
	XMFLOAT4 m_xcDiffuse;
public:
	//�����ڿ� �Ҹ��ڸ� �����Ѵ�.
	CDiffusedVertex(float x, float y, float z, XMFLOAT4 xcDiffuse) { m_xv3Position = XMFLOAT3(x, y, z); m_xcDiffuse = xcDiffuse; }
	CDiffusedVertex(XMFLOAT3 xv3Position, XMFLOAT4 xcDiffuse) { m_xv3Position = xv3Position; m_xcDiffuse = xcDiffuse; }
	CDiffusedVertex() { m_xv3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xcDiffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); }
	~CDiffusedVertex() { }
};

class CMesh
{
private:
	int m_nReferences;

public:
	CMesh(ID3D11Device *pd3dDevice);
	virtual ~CMesh();	//CMesh Ŭ���� ��ü�� ����(Reference)�� ���õ� ��� ������ �Լ��� �����Ѵ�.

	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	/*�� ������ ��ġ ���͸� ��ŷ�� ���Ͽ� �����Ѵ�(���� ���۸� DYNAMIC���� �����ϰ� Map()�� ���� �ʾƵ� �ǵ���).*/
	XMFLOAT3 *m_pxv3Positions;
	/*�޽��� �ε����� �����Ѵ�(�ε��� ���۸� DYNAMIC���� �����ϰ� Map()�� ���� �ʾƵ� �ǵ���).*/
	UINT *m_pnIndices;

protected:
	//������ �����̴�. 
	int m_nVertices;
	UINT m_nStartVertex;
	//������ ��ҵ��� ��Ÿ���� ���۵��� ������ ����Ʈ ���� ��Ÿ���� �迭�̴�. 
	UINT *m_pnVertexStrides;
	//������ ��ҵ��� ��Ÿ���� ���۵��� ���� ��ġ(����Ʈ ��)�� ��Ÿ���� �迭�̴�. 
	UINT *m_pnVertexOffsets;

	//���� �����Ͱ� � ������Ƽ�긦 ǥ���ϰ� �ִ� ���� ��Ÿ���� ��� ������ �����Ѵ�.
	D3D11_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology;
	//������ ��ġ ���Ϳ� ������ �����ϱ� ���� ���ۿ� ���� �������̽� �������̴�. 
	ID3D11Buffer *m_pd3dPositionBuffer;
	ID3D11Buffer *m_pd3dColorBuffer;

	UINT m_nSlot;	//���۵��� �Է������⿡ �����ϱ� ���� ���� ���� ��ȣ�̴�. 
protected:
	/*�ν��Ͻ��� ���� ���� ���۴� �޽��� ���� �����Ϳ� �ν��Ͻ� ������(��ü�� ��ġ�� ����)�� ���´�. �׷��Ƿ� �ν��Ͻ��� ���� ���� ���۴� �ϳ��� �ƴ϶� ���۵��� �迭�̴�. ������ ��ҵ��� ��Ÿ���� ���۵��� �Է������⿡ �����ϱ� ���� �����̴�.*/
	ID3D11Buffer **m_ppd3dVertexBuffers;
	int m_nBuffers;

	//�ε��� ����(�ε����� �迭)�� ���� �������̽� �������̴�. 
	ID3D11Buffer *m_pd3dIndexBuffer;
	//�ε��� ���۰� �����ϴ� �ε����� �����̴�. 
	UINT m_nIndices;
	//�ε��� ���ۿ��� �޽��� ǥ���ϱ� ���� ���Ǵ� ���� �ε����̴�. 
	UINT m_nStartIndex;
	//�� �ε����� ������ �ε����̴�. 
	int m_nBaseVertex;
	UINT m_nIndexOffset;
	//�� �ε����� ����(DXGI_FORMAT_R32_UINT �Ǵ� DXGI_FORMAT_R16_UINT)�̴�. 
	DXGI_FORMAT	m_dxgiIndexFormat;

	/*�����Ͷ����� ���� ��ü�� ���� �������̽� �����͸� �����Ѵ�. �����Ͷ����� ���� ��ü�� ��CGameObject" Ŭ������ ����� �߰��Ͽ��� �ȴ�. �����Ͷ����� ���� ��ü�� �޽��� ����� ���� ���� ��ü�� ����� ���� �ǹ̻��� ���̸� �����غ��⸦ �ٶ���.*/
	ID3D11RasterizerState	*m_pd3dRasterizerState;

	AABB m_bcBoundingCube;

public:
	AABB GetBoundingCube() { return(m_bcBoundingCube); }
#ifdef PICKING
	int CheckRayIntersection(XMFLOAT3 *pxv3RayPosition, XMFLOAT3 *pxv3RayDirection, MESHINTERSECTINFO *pd3dxIntersectInfo);
#endif
	//�޽��� ���� ���۵��� �迭�� �����Ѵ�. 
	void AssembleToVertexBuffer(int nBuffers = 0, ID3D11Buffer **m_pd3dBuffers = nullptr, UINT *pnBufferStrides = nullptr, UINT *pnBufferOffsets = nullptr);

	virtual void CreateRasterizerState(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
	//�ν��Ͻ��� ����Ͽ� �������Ѵ�. 
	virtual void RenderInstanced(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState,int nInstances = 0, int nStartInstance = 0);
};

class CMeshDiffused : public CMesh
{
public:
	CMeshDiffused(ID3D11Device *pd3dDevice);
	virtual ~CMeshDiffused();

protected:
	ID3D11Buffer *m_pd3dColorBuffer;
};



/*CCubeMesh�� CCubeMeshDiffused�� �����ϰ� ���̽� Ŭ������ CMeshDiffused�� �����Ѵ�.*/
class CCubeMeshDiffused : public CMeshDiffused
{
public:
	CCubeMeshDiffused(ID3D11Device *pd3dDevice, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f, XMFLOAT4 XMFLOAT4 = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f));
	virtual ~CCubeMeshDiffused();
};

/*CSphereMesh�� CSphereMeshDiffused�� �����ϰ� ���̽� Ŭ������ CMeshDiffused�� �����Ѵ�.*/
class CSphereMeshDiffused : public CMeshDiffused
{
public:
	CSphereMeshDiffused(ID3D11Device *pd3dDevice, float fRadius = 2.0f, int nSlices = 20, int nStacks = 20, XMFLOAT4 XMFLOAT4 = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f));
	virtual ~CSphereMeshDiffused();
};

class CMeshIlluminated : public CMesh
{
public:
	CMeshIlluminated(ID3D11Device *pd3dDevice);
	virtual ~CMeshIlluminated();

protected:
	//������ ������ ����ϱ� ���Ͽ� �������Ͱ� �ʿ��ϴ�.
	ID3D11Buffer *m_pd3dNormalBuffer;

public:
	//������ ���Ե� �ﰢ���� �������͸� ����ϴ� �Լ��̴�.
	XMFLOAT3 CalculateTriAngleNormal(UINT nIndex0, UINT nIndex1, UINT nIndex2);
	void SetTriAngleListVertexNormal(XMFLOAT3 *pxv3Normals);

	//������ ���������� ����� ����ϴ� �Լ��̴�.
	void SetAverageVertexNormal(XMFLOAT3 *pxv3Normals, int nPrimitives, int nOffset, bool bStrip);
	void CalculateVertexNormal(XMFLOAT3 *pxv3Normals);
};


class CMeshTextured : public CMesh
{
public:
	CMeshTextured(ID3D11Device *pd3dDevice);
	virtual ~CMeshTextured();

protected:
	//�ؽ��� ������ �ϱ� ���Ͽ� �ؽ��� ��ǥ�� �ʿ��ϴ�.
	ID3D11Buffer *m_pd3dTexCoordBuffer;
};


class CPlaneMesh : public CMeshTextured
{
public:
	CPlaneMesh(ID3D11Device * pd3dDevice, int fx, int fy);
	virtual ~CPlaneMesh();
};


class CMeshDetailTextured : public CMeshTextured
{
public:
	CMeshDetailTextured(ID3D11Device *pd3dDevice);
	virtual ~CMeshDetailTextured();

protected:
	ID3D11Buffer *m_pd3dDetailTexCoordBuffer;
};

class CMeshTexturedIlluminated : public CMeshIlluminated
{
public:
	CMeshTexturedIlluminated(ID3D11Device *pd3dDevice);
	virtual ~CMeshTexturedIlluminated();

protected:
	ID3D11Buffer *m_pd3dTexCoordBuffer;
};


class CMeshSplatTexturedIlluminated : public CMeshIlluminated
{
public:
	CMeshSplatTexturedIlluminated(ID3D11Device *pd3dDevice);
	virtual ~CMeshSplatTexturedIlluminated();

protected:
	ID3D11Buffer *m_pd3dTexCoordBuffer;
	ID3D11Buffer *m_pd3dAlphaTexCoordBuffer;
};


class CCubeMeshIlluminated : public CMeshIlluminated
{
public:
	CCubeMeshIlluminated(ID3D11Device *pd3dDevice, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshIlluminated();
};
/*�ؽ��� ������ ����Ͽ� ������ �����ϱ� ���Ͽ� ������ �ؽ��� ��ǥ�� ���� ������ü �޽� Ŭ�����̴�.*/
class CCubeMeshTextured : public CMeshTextured
{
public:
	CCubeMeshTextured(ID3D11Device *pd3dDevice, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshTextured();
};

/*�ؽ��� ������ ����Ͽ� ������ �����ϱ� ���Ͽ� ������ �ؽ��� ��ǥ�� ���� �� �޽� Ŭ�����̴�.*/
class CSphereMeshTextured : public CMeshTextured
{
public:
	CSphereMeshTextured(ID3D11Device *pd3dDevice, float fRadius = 2.0f, int nSlices = 20, int nStacks = 20);
	virtual ~CSphereMeshTextured();
};

class CCubeMeshTexturedIlluminated : public CMeshTexturedIlluminated
{
public:
	CCubeMeshTexturedIlluminated(ID3D11Device *pd3dDevice, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshTexturedIlluminated();
};

class CSphereMeshTexturedIlluminated : public CMeshTexturedIlluminated
{
public:
	CSphereMeshTexturedIlluminated(ID3D11Device *pd3dDevice, float fRadius = 2.0f, int nSlices = 20, int nStacks = 20);
	virtual ~CSphereMeshTexturedIlluminated();
};

class CMeshDetailTexturedIlluminated : public CMeshIlluminated
{
public:
	CMeshDetailTexturedIlluminated(ID3D11Device *pd3dDevice);
	virtual ~CMeshDetailTexturedIlluminated();

protected:
	ID3D11Buffer *m_pd3dTexCoordBuffer;
	ID3D11Buffer *m_pd3dDetailTexCoordBuffer;
};

struct V3T2
{
	XMFLOAT3 xmf3Pos;
	XMFLOAT2 xmf2Tex;
};

struct V3N3T2
{
	XMFLOAT3 xmf3Pos;
	XMFLOAT3 xmf3Normal;
	XMFLOAT2 xmf2Tex;
};

class CLoadMeshByChae : public CMeshTexturedIlluminated
{
protected:
	XMFLOAT3 * m_xmf3Normal;
	XMFLOAT2 * m_xmf2TexCoords;
public:
	CLoadMeshByChae(ID3D11Device *pd3dDevice, char * tMeshName, float xScale = 1.0f, float yScale = 1.0f, float zScale = 1.0f);
	virtual ~CLoadMeshByChae();
	virtual void CreateRasterizerState(ID3D11Device *pd3dDevice);
};

class CLoadMesh : public CMeshTexturedIlluminated
{
protected:
	XMFLOAT3 * m_xmf3Normal;
	XMFLOAT2 * m_xmf2TexCoords;
public:
	CLoadMesh(ID3D11Device *pd3dDevice, wchar_t * tMeshName);
	virtual ~CLoadMesh();

};

class CLoadMeshCommon : public CLoadMesh
{
public:
	CLoadMeshCommon(ID3D11Device *pd3dDevice, wchar_t * tMeshName, float xScale = 1.0f, float yScale = 1.0f, float zScale = 1.0f);
	virtual ~CLoadMeshCommon();
};
