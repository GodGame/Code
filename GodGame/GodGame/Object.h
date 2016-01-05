#pragma once

#define DIR_FORWARD	0x01
#define DIR_BACKWARD	0x02
#define DIR_LEFT	0x04
#define DIR_RIGHT	0x08
#define DIR_UP		0x10
#define DIR_DOWN	0x20


#include "Mesh.h"
#include "Camera.h"
#include "TextureMgr.h"

#define PARTICLE_TYPE_EMITTER	0
#define PARTICLE_TYPE_FLARE		1

struct PATICLE_INFO
{
	XMFLOAT3 m_xmf3Pos;
	XMFLOAT3 m_xmf3Velocity;
	XMFLOAT2 m_xmf2Size;
	float	 m_fAge;
	unsigned int	 m_uType;
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
	XMFLOAT2 m_NewSize;
	float m_bEnable;

};

class CParticle
{
	int m_nVertices;
	UINT m_nStartVertex;
	UINT m_nVertexStrides;
	UINT m_nVertexOffsets;
private:
	PATICLE_INFO m_cParticle;
	CB_PARTICLE m_cbParticle;
	ID3D11Query * m_pd3dQuery;

	bool m_bInitilize;
	bool m_bEnable;

	float m_fDurability;

	ID3D11Buffer * m_pd3dInitialVertexBuffer;
	ID3D11Buffer * m_pd3dStreamOutVertexBuffer;
	ID3D11Buffer * m_pd3dDrawVertexBuffer;

public:
	CParticle();
	~CParticle();

	CB_PARTICLE * GetCBParticle() { return &m_cbParticle; }
	void Update(float fTimeElapsed);
	void Initialize(ID3D11Device *pd3dDevice, CB_PARTICLE & info, float fDurability, int iMaxParticle);

	void SetDurabilityTime(float fTime) { m_fDurability = fTime; }
	void SetLifeTime(float fLifeTime) { m_cbParticle.m_fLifeTime = fLifeTime; }
	void SetEmitPosition(XMFLOAT3 & pos) { m_cbParticle.m_vParticleEmitPos = pos; }
	void SetEmitDirection(XMFLOAT3 & dir) { m_cbParticle.m_vParticleVelocity = dir; }
	void SetAccelation(XMFLOAT3 & accel) { m_cbParticle.m_vAccel = accel; }

	bool IsAble() { return m_bEnable; }
	void Enable();
	void Disable();

	void StreamOut(ID3D11DeviceContext *pd3dDeviceContext);
	void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
};


//재질의 색상을 나타내는 구조체이다.
struct MATERIAL
{
	XMFLOAT4 m_xcAmbient;
	XMFLOAT4 m_xcDiffuse;
	XMFLOAT4 m_xcSpecular; //(r,g,b,a=power)
	XMFLOAT4 m_xcEmissive;
};

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

private:
	SETSHADER	m_uMaterialSet;
	int m_nReferences;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	MATERIAL m_Material;
};


class CGameObject
{
public:
	CGameObject(int nMeshes = 0);
	virtual ~CGameObject();

	//게임 객체는 하나의 재질을 가질 수 있다.
	CMaterial *m_pMaterial;
	void SetMaterial(CMaterial *pMaterial);
	XMFLOAT4	m_xmfPlane;

private:
	int		m_nReferences;
	bool	m_bActive;

public:
	void	 AddRef();
	void	 Release();

	XMFLOAT4X4	 m_xmf44World;
	//객체가 가지는 메쉬들에 대한 포인터와 그 개수이다.
	CMesh **m_ppMeshes;
	int m_nMeshes;
	AABB m_bcMeshBoundingCube;

	void SetMesh(CMesh *pMesh, int nIndex = 0);
	
	CMesh *GetMesh(int nIndex = 0) { return(m_ppMeshes[nIndex]); }
	//게임 객체는 텍스쳐 가질 수 있다.
	CTexture *m_pTexture;
	void SetTexture(CTexture *pTexture, bool beforeRelease = true);
	void SetActive(bool bActive = false) { m_bActive = bActive; }

	virtual void Animate(float fTimeElapsed);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
	virtual void RenderReflected(ID3D11DeviceContext *pd3dDeviceContext, XMMATRIX *xmtxReflect, CCamera *pCamera);

	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3 xv3Position);
	virtual void SetPosition(XMVECTOR * xv3Position);

	XMFLOAT3 GetPosition();

public:
	bool IsVisible(CCamera *pCamera = nullptr);

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

#ifdef PICKING
	//월드 좌표계의 픽킹 광선을 생성한다.
	void GenerateRayForPicking(XMFLOAT3 *pxv3PickPosition, XMFLOAT4X4 *pxmtxWorld, XMFLOAT4X4 *pxmtxView, XMFLOAT3 *pxv3PickRayPosition, XMFLOAT3 *pxv3PickRayDirection);
	//월드 좌표계의 픽킹 광선을 생성한다.
	int PickObjectByRayIntersection(XMFLOAT3 *pxv3PickPosition, XMFLOAT4X4 *pxmtxView, MESHINTERSECTINFO *pd3dxIntersectInfo);
#endif
};

class CMirrorObject : public CGameObject
{
public :
	CMirrorObject();
	virtual ~CMirrorObject();
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
	//virtual void Animate(float fTimeElapsed);
	virtual void SetPosition(float x, float y, float z);
	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
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
	CHeightMap(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xv3Scale);
	~CHeightMap(void);

	//높이 맵 이미지에서 (x, z) 위치의 픽셀 값에 기반한 지형의 높이를 반환한다.
	float GetHeight(float x, float z, bool bReverseQuad = false);
	//높이 맵 이미지에서 (x, z) 위치의 법선 벡터를 반환한다.
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return(m_xv3Scale); }

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
	XMFLOAT3 GetNormal(float x, float z) { return(m_pHeightMap->GetHeightMapNormal(int(x / m_xv3Scale.x), int(z / m_xv3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMap->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMap->GetHeightMapLength()); }

	XMFLOAT3 GetScale() { return(m_xv3Scale); }
	//지형의 실제 크기(가로/세로)를 반환한다. 높이 맵의 크기에 스케일을 곱한 값이다.
	float GetWidth() { return(m_nWidth * m_xv3Scale.x); }
	float GetLength() { return(m_nLength * m_xv3Scale.z); }

	float GetPeakHeight() { return(m_bcMeshBoundingCube.m_xv3Maximum.y); }
};

class CSkyBox : public CGameObject
{

public:
	CSkyBox(ID3D11Device *pd3dDevice);
	virtual ~CSkyBox();

	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
};

class CTrees : public CGameObject
{
	AABB m_BoundingBox;

public:
	XMFLOAT3 m_xv3Pos;
	XMFLOAT2 m_xv2Size;

	CTrees(){}
	CTrees(XMFLOAT3 xmf3Pos, XMFLOAT2 xmf2Size);
	~CTrees() {}
	bool IsVisible(CCamera *pCamera = nullptr);
};

