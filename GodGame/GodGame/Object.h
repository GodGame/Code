#pragma once

#define DIR_FORWARD	0x01
#define DIR_BACKWARD	0x02
#define DIR_LEFT	0x04
#define DIR_RIGHT	0x08
#define DIR_UP		0x10
#define DIR_DOWN	0x20

#include "Camera.h"
#include "MgrList.h"

#define PARTICLE_TYPE_EMITTER	0
#define PARTICLE_TYPE_FLARE		1

struct PARTICLE_INFO
{
	XMFLOAT3 m_xmf3Pos;
	XMFLOAT3 m_xmf3Velocity;
	XMFLOAT2 m_xmf2Size;
	float	 m_fAge;
	float	 m_uType;
};

struct CB_PARTICLE
{
	XMFLOAT3 m_vParticleEmitPos;
	float m_fGameTime;
	XMFLOAT3 m_vParticleVelocity;
	float m_fLifeTime;
	XMFLOAT3 m_vAccel;
	float m_fTimeStep;
	float m_fNewTime;
	XMFLOAT2 m_MaxSize;
	float m_bEnable;
};

class CParticle
{
	//int m_nVertices;
	UINT m_nStartVertex;
	UINT m_nVertexStrides;
	UINT m_nVertexOffsets;

private:
	PARTICLE_INFO m_cParticle;
	CB_PARTICLE m_cbParticle;
	//ID3D11Query * m_pd3dQuery;

	bool m_bInitilize : 1;
	bool m_bEnable    : 1;
	bool m_bMove      : 1;
	bool m_bUseAccel  : 1;
	//UINT m_bExtra     : 28;

	MoveVelocity m_velocity;
	float m_fDurability;

	ID3D11Buffer * m_pd3dInitialVertexBuffer;
	ID3D11Buffer * m_pd3dStreamOutVertexBuffer;
	ID3D11Buffer * m_pd3dDrawVertexBuffer;

public:
	CParticle();
	~CParticle();

	CB_PARTICLE * GetCBParticle() { return &m_cbParticle; }
	void Update(float fTimeElapsed);
	void Initialize(ID3D11Device *pd3dDevice, CB_PARTICLE & info, MoveVelocity & Velocity, float fDurability, int iMaxParticle);

	void SetDurabilityTime(float fTime)   { m_fDurability = fTime; }
	void SetLifeTime(float fLifeTime)     { m_cbParticle.m_fLifeTime = fLifeTime; }
	void SetEmitPosition(XMFLOAT3 & pos)  { m_cbParticle.m_vParticleEmitPos = pos; }
	void SetEmitDirection(XMFLOAT3 & dir) { m_cbParticle.m_vParticleVelocity = dir; }
	void SetAccelation(XMFLOAT3 & accel)  { m_cbParticle.m_vAccel = accel; }

	bool IsAble() { return m_bEnable; }
	bool Enable(XMFLOAT3 * pos = nullptr);
	bool Disable();

	void StreamOut(ID3D11DeviceContext *pd3dDeviceContext);
	void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
};

class CGameObject
{
public:
	CGameObject(int nMeshes = 0);
	virtual ~CGameObject();

	//게임 객체는 하나의 재질을 가질 수 있다.
	CMaterial  * m_pMaterial;
	void SetMaterial(CMaterial *pMaterial);

protected:
	UINT	m_uSize : 16;
	UINT	m_nReferences : 15;
	bool	m_bActive : 1;

public:
	void	 AddRef();
	void	 Release();

	XMFLOAT4X4	 m_xmf44World;
	//객체가 가지는 메쉬들에 대한 포인터와 그 개수이다.
	CMesh **     m_ppMeshes;
	int          m_nMeshes;
	AABB         m_bcMeshBoundingCube;

public:
	CMesh *GetMesh(int nIndex = 0) { return(m_ppMeshes[nIndex]); }
	//게임 객체는 텍스쳐 가질 수 있다.
	CTexture *m_pTexture;
	void SetTexture(CTexture *pTexture, bool beforeRelease = true);
	void SetActive(bool bActive = false) { m_bActive = bActive; }
	virtual void UpdateBoundingBox();

	void SetMesh(CMesh *pMesh, int nIndex = 0);

	virtual void Animate(float fTimeElapsed);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
	//virtual void RenderReflected(ID3D11DeviceContext *pd3dDeviceContext, XMMATRIX *xmtxReflect, CCamera *pCamera);

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xv3Position);
	void SetPosition(XMVECTOR* xv3Position);

	XMFLOAT3 GetPosition();
	UINT GetSize() { return m_uSize; }

public:
	virtual bool IsVisible(CCamera *pCamera = nullptr);

	//로컬 x-축, y-축, z-축 방향으로 이동한다.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	//로컬 x-축, y-축, z-축 방향으로 회전한다.
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxv3Axis, float fAngle);

	//객체의 위치, 로컬 x-축, y-축, z-축 방향 벡터를 반환한다.
	XMFLOAT3 GetLookAt();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	//객체를 렌더링하기 전에 호출되는 함수이다.
	virtual void OnPrepareRender() { }
	virtual void GetGameMessage(CGameObject * byObj, eMessage eMSG, void * extra = nullptr);
	virtual void SendGameMessage(CGameObject * toObj, eMessage eMSG, void * extra = nullptr);
	static void MessageObjToObj(CGameObject * byObj, CGameObject * toObj, eMessage eMSG, void * extra = nullptr);
#ifdef PICKING
	//월드 좌표계의 픽킹 광선을 생성한다.
	void GenerateRayForPicking(XMFLOAT3 *pxv3PickPosition, XMFLOAT4X4 *pxmtxWorld, XMFLOAT4X4 *pxmtxView, XMFLOAT3 *pxv3PickRayPosition, XMFLOAT3 *pxv3PickRayDirection);
	//월드 좌표계의 픽킹 광선을 생성한다.
	int PickObjectByRayIntersection(XMFLOAT3 *pxv3PickPosition, XMFLOAT4X4 *pxmtxView, MESHINTERSECTINFO *pd3dxIntersectInfo);
#endif
};
// 스스로 움직이지 못하는 고정된 객체
class CStaticObject : public CGameObject
{
protected:

public:
	CStaticObject(int nMeshes);
	virtual ~CStaticObject();
};
// 스스로 움직일 수 있는 유동적인 객체
class CDynamicObject : public CGameObject
{
public:
	CDynamicObject(int nMeshes);
	virtual ~CDynamicObject();

protected:
	XMFLOAT3 m_xv3Velocity;

public:
	void SetVelocity(const XMFLOAT3& xv3Velocity) { m_xv3Velocity = xv3Velocity; }
	const XMFLOAT3& GetVelocity() const { return(m_xv3Velocity); }
};

class CRotatingObject : public CGameObject
{
protected:
	//자전 속도와 회전축 벡터를 나타내는 멤버 변수를 선언한다.
	float m_fRotationSpeed;
	XMFLOAT3 m_xv3RotationAxis;

public:
	//자전 속도와 회전축 벡터를 설정하는 함수이다.
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xv3RotationAxis) { m_xv3RotationAxis = xv3RotationAxis; }

	CRotatingObject(int nMeshes = 1);
	virtual ~CRotatingObject();

	virtual void Animate(float fTimeElapsed);
};

class CRevolvingObject : public CGameObject
{
public:
	CRevolvingObject(int nMeshes = 1);
	virtual ~CRevolvingObject();

	virtual void Animate(float fTimeElapsed);

private:
	// 공전 회전축과 회전 속력을 나타낸다.
	XMFLOAT3 m_xv3RevolutionAxis;
	float m_fRevolutionSpeed;

public:
	// 공전 속력을 설정한다.
	void SetRevolutionSpeed(float fRevolutionSpeed) { m_fRevolutionSpeed = fRevolutionSpeed; }
	// 공전을 위한 회전축을 설정한다.
	void SetRevolutionAxis(XMFLOAT3 xv3RevolutionAxis) { m_xv3RevolutionAxis = xv3RevolutionAxis; }
};

class CHeightMap
{
private:
	//높이 맵 이미지 픽셀(8-비트)들의 이차원 배열이다. 각 픽셀은 0~255의 값을 갖는다.
	BYTE *m_pHeightMapImage;
	//높이 맵 이미지의 가로와 세로 크기이다.
	int m_nWidth;
	int m_nLength;
	//높이 맵을 실제로 몇 배 확대하여 사용할 것인가를 나타내는 스케일 벡터이다.
	XMFLOAT3 m_xv3Scale;

public:
	CHeightMap(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3& xv3Scale);
	~CHeightMap(void);

	//높이 맵 이미지에서 (x, z) 위치의 픽셀 값에 기반한 지형의 높이를 반환한다.
	float GetHeight(float x, float z, bool bReverseQuad = false);
	//높이 맵 이미지에서 (x, z) 위치의 법선 벡터를 반환한다.
	XMFLOAT3& GetHeightMapNormal(int x, int z);
	XMFLOAT3& GetScale() { return(m_xv3Scale); }

	BYTE *GetHeightMapImage() { return(m_pHeightMapImage); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};

class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(ID3D11Device *pd3dDevice, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xv3Scale);
	virtual ~CHeightMapTerrain();

private:
	//지형의 높이 맵으로 사용할 이미지이다.
	CHeightMap *m_pHeightMap;

	//지형의 가로와 세로 크기이다.
	int m_nWidth;
	int m_nLength;

	//지형을 실제로 몇 배 확대할 것인가를 나타내는 스케일 벡터이다.
	XMFLOAT3 m_xv3Scale;
public:
	//지형의 실제 높이를 반환한다. 높이 맵의 높이에 스케일을 곱한 값이다.
	float GetHeight(float x, float z, bool bReverseQuad = false) { return(m_pHeightMap->GetHeight(x, z, bReverseQuad) * m_xv3Scale.y); }
	XMFLOAT3& GetNormal(float x, float z) { return(m_pHeightMap->GetHeightMapNormal(int(x / m_xv3Scale.x), int(z / m_xv3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMap->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMap->GetHeightMapLength()); }

	XMFLOAT3& GetScale() { return(m_xv3Scale); }
	//지형의 실제 크기(가로/세로)를 반환한다. 높이 맵의 크기에 스케일을 곱한 값이다.
	float GetWidth() { return(m_nWidth * m_xv3Scale.x); }
	float GetLength() { return(m_nLength * m_xv3Scale.z); }

	float GetPeakHeight() { return(m_bcMeshBoundingCube.m_xv3Maximum.y); }
};

class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D11Device *pd3dDevice, UINT uImageNum);
	virtual ~CSkyBox();

	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
};

class CBillboardObject : public CGameObject
{
	XMFLOAT2 m_xv2Size;
	XMFLOAT4 m_xv4InstanceData;

public:
	CBillboardObject() : CGameObject(1) {};
	CBillboardObject(XMFLOAT3 pos, UINT fID, XMFLOAT2 xmf2Size);
	~CBillboardObject() {}

	void UpdateInstanceData();
	virtual bool IsVisible(CCamera *pCamera = nullptr);
	XMFLOAT4& GetInstanceData() { return m_xv4InstanceData; }
};

class CAbsorbMarble : public CBillboardObject
{
	bool	m_bAbsorb;
	float	m_fAbsorbTime;
	float	m_fSpeed;

	CGameObject * m_pTargetObject;
	XMFLOAT3 m_xvRandomVelocity;

public:
	CAbsorbMarble();
	CAbsorbMarble(XMFLOAT3 pos, UINT fID, XMFLOAT2 xmf2Size);
	~CAbsorbMarble();

	void Initialize();
	void SetTarget(CGameObject * pGameObject);

	virtual bool IsVisible(CCamera *pCamera = nullptr);
	virtual void Animate(float fTimeElapsed);
	virtual void GetGameMessage(CGameObject * byObj, eMessage eMSG, void * extra = nullptr);
};