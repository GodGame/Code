#pragma once
#include "CollisionMgr.h"
#define RANDOM_COLOR ((rand() * 0xFFFFFF) / RAND_MAX)


enum ANIMATION_STATE
{
	eANI_IDLE = 0,
	eANI_RUN_FORWARD,
	eANI_WALK_BACK,
	eANI_WALK_LEFT,
	eANI_WALK_RIGHT,
	eANI_1H_CAST,
	eANI_1H_MAGIC_ATTACK,
	eANI_1H_MAGIC_AREA,
	eANI_TOTAL_NUM
};

class CTexture;
class AABB;
struct POSANDSIZE {
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
struct V3T2N3
{
	XMFLOAT3 xmf3Pos;
	XMFLOAT2 xmf2Tex;
	XMFLOAT3 xmf3Normal;
};

struct NormalMapVertex
{
	XMFLOAT3 xmf3Pos;
	XMFLOAT2 xmf2Tex;
	XMFLOAT3 xmf3Normal;
	XMFLOAT3 xmf3Tangent;

	void operator=(V3T2N3 & vertex)
	{
		xmf3Pos = vertex.xmf3Pos;
		xmf2Tex = vertex.xmf2Tex;
		xmf3Normal = vertex.xmf3Normal;
	}
};

struct MeshBuffer
{	
	int nVertexes;
	int nStrides;
	int nOffsets;
	int nStartSlot;
	ID3D11Buffer * pd3dBuffer;
	AABB bb;
};

class CVertex
{
	XMFLOAT3 m_xv3Position;		//정점의 위치 정보(3차원 벡터)를 저장하기 위한 멤버 변수를 선언한다.
public:
	CVertex() { m_xv3Position = XMFLOAT3(0, 0, 0); }
	CVertex(XMFLOAT3 xv3Position) { m_xv3Position = xv3Position; }
	~CVertex() { }
};

class CDiffusedVertex
{
	XMFLOAT3 m_xv3Position;
	//정점의 색상을 나타내는 멤버 변수(XMFLOAT4 구조체)를 선언한다.
	XMFLOAT4 m_xcDiffuse;
public:
	//생성자와 소멸자를 선언한다.
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
	virtual ~CMesh();	//CMesh 클래스 객체의 참조(Reference)와 관련된 멤버 변수와 함수를 선언한다.

	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	/*각 정점의 위치 벡터를 픽킹을 위하여 저장한다(정점 버퍼를 DYNAMIC으로 생성하고 Map()을 하지 않아도 되도록).*/
	XMFLOAT3 *m_pxv3Positions;
	/*메쉬의 인덱스를 저장한다(인덱스 버퍼를 DYNAMIC으로 생성하고 Map()을 하지 않아도 되도록).*/
	UINT *m_pnIndices;

protected:
	//정점의 개수이다.
	UINT m_nVertices;
	UINT m_nStartVertex;
	//정점의 요소들을 나타내는 버퍼들의 원소의 바이트 수를 나타내는 배열이다.
	UINT *m_pnVertexStrides;
	//정점의 요소들을 나타내는 버퍼들의 시작 위치(바이트 수)를 나타내는 배열이다.
	UINT *m_pnVertexOffsets;

	//정점 데이터가 어떤 프리미티브를 표현하고 있는 가를 나타내는 멤버 변수를 선언한다.
	D3D11_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology;
	//정점의 위치 벡터와 색상을 저장하기 위한 버퍼에 대한 인터페이스 포인터이다.
	ID3D11Buffer *m_pd3dPositionBuffer;
	ID3D11Buffer *m_pd3dColorBuffer;

	UINT m_nSlot;	//버퍼들을 입력조립기에 연결하기 위한 시작 슬롯 번호이다.
protected:
	/*인스턴싱을 위한 정점 버퍼는 메쉬의 정점 데이터와 인스턴싱 데이터(객체의 위치와 방향)를 갖는다. 그러므로 인스턴싱을 위한 정점 버퍼는 하나가 아니라 버퍼들의 배열이다. 정점의 요소들을 나타내는 버퍼들을 입력조립기에 전달하기 위한 버퍼이다.*/
	ID3D11Buffer **m_ppd3dVertexBuffers;
	int m_nBuffers;

	//인덱스 버퍼(인덱스의 배열)에 대한 인터페이스 포인터이다.
	ID3D11Buffer *m_pd3dIndexBuffer;
	//인덱스 버퍼가 포함하는 인덱스의 개수이다.
	UINT m_nIndices;
	//인덱스 버퍼에서 메쉬를 표현하기 위해 사용되는 시작 인덱스이다.
	UINT m_nStartIndex;
	//각 인덱스에 더해질 인덱스이다.
	int m_nBaseVertex;
	UINT m_nIndexOffset;
	//각 인덱스의 형식(DXGI_FORMAT_R32_UINT 또는 DXGI_FORMAT_R16_UINT)이다.
	DXGI_FORMAT	m_dxgiIndexFormat;

	/*래스터라이저 상태 객체에 대한 인터페이스 포인터를 선언한다. 래스터라이저 상태 객체는 “CGameObject" 클래스의 멤버로 추가하여도 된다. 래스터라이저 상태 객체가 메쉬의 멤버일 때와 게임 객체의 멤버일 때의 의미상의 차이를 생각해보기를 바란다.*/
	ID3D11RasterizerState	*m_pd3dRasterizerState;

	AABB m_bcBoundingCube;

public:
	AABB & GetBoundingCube() { return(m_bcBoundingCube); }
#ifdef PICKING
	int CheckRayIntersection(XMFLOAT3 *pxv3RayPosition, XMFLOAT3 *pxv3RayDirection, MESHINTERSECTINFO *pd3dxIntersectInfo);
#endif
	ID3D11Buffer ** GetVertexBuffer() { return m_ppd3dVertexBuffers; }
	ID3D11Buffer * GetPositionBuffer() { return m_pd3dPositionBuffer; }
	int GetVerticesSize() { return m_nVertices; }
	//메쉬의 정점 버퍼들을 배열로 조립한다.
	void AssembleToVertexBuffer(int nBuffers = 0, ID3D11Buffer **m_pd3dBuffers = nullptr, UINT *pnBufferStrides = nullptr, UINT *pnBufferOffsets = nullptr);

	virtual void CreateRasterizerState(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
	//인스턴싱을 사용하여 렌더링한다.
	virtual void RenderInstanced(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, int nInstances, int nStartInstance);

	static MeshBuffer GetNormalMapVertexBuffer(ID3D11Device * pd3dDevice, NormalMapVertex * pData, int nSize, float fScale = 1.0f);
	static MeshBuffer GetNormalMapVertexBufferAndCalculateTangent(ID3D11Device * pd3dDevice, NormalMapVertex * pData, int nSize, float fScale = 1.0f);
};

class CAnimatedMesh : public CMesh
{
protected:
	vector<MeshBuffer> m_pvcMeshBuffers;
	bool m_bTerminal;

	int m_iIndex;
	float m_fNowFrameTime;
	float m_fFramePerTime;

public:
	CAnimatedMesh(ID3D11Device * pd3dDevice); 
	virtual ~CAnimatedMesh();

	virtual void Animate(float fFrameTime);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
	virtual void CreateRasterizerState(ID3D11Device *pd3dDevice);
public:
	AABB & GetAABBMesh() { return m_pvcMeshBuffers[m_iIndex].bb; }

	float SetOneCycleTime(float fCycleTime);
	void SetFramePerTime(float fFramePerTime);

	void SetAnimationIndex(int iIndex);
	int GetAnimationAllIndex() { return m_pvcMeshBuffers.size(); }

	bool IsEndAnimation() { return m_bTerminal; }
	void ResetIndex() { m_iIndex = 0; m_fNowFrameTime = 0.0f; m_bTerminal = false; }
};

class CMeshDiffused : public CMesh
{
public:
	CMeshDiffused(ID3D11Device *pd3dDevice);
	virtual ~CMeshDiffused();

protected:
	ID3D11Buffer *m_pd3dColorBuffer;
};

/*CCubeMesh를 CCubeMeshDiffused로 변경하고 베이스 클래스를 CMeshDiffused로 변경한다.*/
class CCubeMeshDiffused : public CMeshDiffused
{
public:
	CCubeMeshDiffused(ID3D11Device *pd3dDevice, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f, XMFLOAT4 xmf4 = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f));
	virtual ~CCubeMeshDiffused();
};

class CWireCube : public CCubeMeshDiffused
{
public:
	CWireCube(ID3D11Device *pd3dDevice, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f, XMFLOAT4 xmf4 = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f));
	virtual ~CWireCube();
	virtual void CreateRasterizerState(ID3D11Device *pd3dDevice);
};
/*CSphereMesh를 CSphereMeshDiffused로 변경하고 베이스 클래스를 CMeshDiffused로 변경한다.*/
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
	//조명의 영향을 계산하기 위하여 법선벡터가 필요하다.
	ID3D11Buffer *m_pd3dNormalBuffer;

public:
	//정점이 포함된 삼각형의 법선벡터를 계산하는 함수이다.
	XMFLOAT3 CalculateTriAngleNormal(UINT nIndex0, UINT nIndex1, UINT nIndex2);
	void SetTriAngleListVertexNormal(XMFLOAT3 *pxv3Normals);

	//정점의 법선벡터의 평균을 계산하는 함수이다.
	void SetAverageVertexNormal(XMFLOAT3 *pxv3Normals, int nPrimitives, int nOffset, bool bStrip);
	void CalculateVertexNormal(XMFLOAT3 *pxv3Normals);
};

class CMeshTextured : public CMesh
{
public:
	CMeshTextured(ID3D11Device *pd3dDevice);
	virtual ~CMeshTextured();

protected:
	//텍스쳐 매핑을 하기 위하여 텍스쳐 좌표가 필요하다.
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
/*텍스쳐 매핑을 사용하여 색상을 결정하기 위하여 정점이 텍스쳐 좌표를 갖는 직육면체 메쉬 클래스이다.*/
class CCubeMeshTextured : public CMeshTextured
{
public:
	CCubeMeshTextured(ID3D11Device *pd3dDevice, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshTextured();
};

/*텍스쳐 매핑을 사용하여 색상을 결정하기 위하여 정점이 텍스쳐 좌표를 갖는 구 메쉬 클래스이다.*/
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

class CLoadMeshByChae : public CMeshTexturedIlluminated
{
public:
	CLoadMeshByChae(ID3D11Device *pd3dDevice, char * tMeshName, float fScale = 1.0f);
	virtual ~CLoadMeshByChae();
	virtual void CreateRasterizerState(ID3D11Device *pd3dDevice);
};

class CLoadMeshByFbxcjh : public CMeshTexturedIlluminated
{
public:
	CLoadMeshByFbxcjh(ID3D11Device *pd3dDevice, char * tMeshName, float fScale, vector<wstring> & vcFileName);
	virtual ~CLoadMeshByFbxcjh();
	virtual void CreateRasterizerState(ID3D11Device *pd3dDevice);
};
//class LoadNormalMapVertexByFile : public CMesh
//{
//public:
//	LoadNormalMapVertexByFile(ID3D11Device * pd3dDevice, NormalMapVertex * pData, int nSize, float fScale = 1.0f);
//	virtual ~LoadNormalMapVertexByFile();
//};

class CLoadAnimatedMeshByADFile : public CAnimatedMesh
{
public:
	CLoadAnimatedMeshByADFile(ID3D11Device * pd3dDevice, char * fileName, float fScale, vector<wstring> & vcFileName);
	virtual ~CLoadAnimatedMeshByADFile();
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
