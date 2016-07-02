#pragma once

#define DIR_FORWARD	0x01
#define DIR_BACKWARD	0x02
#define DIR_LEFT	0x04
#define DIR_RIGHT	0x08
#define DIR_UP		0x10
#define DIR_DOWN	0x20

#include "Camera.h"
#include "MgrList.h"
#include "AI.h"

#define PARTICLE_TYPE_EMITTER	0
#define PARTICLE_TYPE_FLARE		1

class CEntity
{	
protected:
	UINT	m_uSize			: 14;
	bool    m_bActive		: 1;
	bool	m_bVisible		: 1;
	bool	m_bUseCollide	: 1;
	bool    m_bObstacle		: 1;
	bool	m_bDetailCollide : 1;

protected:
	void _ResetVisible(UINT uRenderState)
	{
		m_bVisible = uRenderState & DRAW_AND_ACTIVE;
	}

public:
	CEntity();
	virtual ~CEntity(){}

public:
	AABB         m_bcMeshBoundingCube;

	void SetVisible(const bool bVisible = false) { m_bVisible = bVisible; }
	bool IsVisible() { return m_bVisible; }
	void SetActive(const bool bActive) { m_bActive = bActive; }
	bool IsActive()  { return m_bActive; }

	bool IsObstacle() { return m_bObstacle; }
	void SetObstacle(bool bVal) { m_bObstacle = bVal; }
	bool IsDetailCollide() { return m_bDetailCollide; }
	void SetDetailCollide(bool set) { m_bDetailCollide = set; }

	void SetCollide(const bool bCollide) { m_bUseCollide = bCollide; }
	bool CanCollide(CEntity * pObj) const
	{
		return m_bActive && m_bUseCollide && this != pObj;
#if 0
		if (false == m_bActive) return false;
		if (false == m_bUseCollide) return false;
		if (this == pObj) return false;
		return true;
#endif
	}

	UINT GetSize() const { return m_uSize; }
	void SetSize(UINT uSize) { m_uSize = uSize; }

public:
	virtual XMFLOAT3 GetPosition() const { return XMFLOAT3(0, 0, 0); }
	virtual void UpdateBoundingBox() = 0;

	virtual void GetGameMessage (CEntity * byEntity, eMessage eMSG, void * extra = nullptr);
	virtual void SendGameMessage(CEntity * toEntity, eMessage eMSG, void * extra = nullptr);
	static void MessageEntToEnt (CEntity * byObj, CEntity * toObj, eMessage eMSG, void * extra = nullptr);
};

class CGameObject : public CEntity
{
public:
	CGameObject(int nMeshes = 0);
	virtual ~CGameObject();

protected:
	UINT	m_nReferences : 15;
	bool    m_bUseInheritAutoRender : 1;

	CGameObject * m_pChild;
	CGameObject * m_pSibling;
	//CGameObject * m_pParent;

protected:
	void _SetMaterialAndTexture(ID3D11DeviceContext *pd3dDeviceContext);

public:
	void	 AddRef();
	void	 Release();

	XMFLOAT4X4	 m_xmf44World;
	//객체가 가지는 메쉬들에 대한 포인터와 그 개수이다.
	CMesh **     m_ppMeshes;
	int          m_nMeshes;
	//AABB         m_bcMeshBoundingCube;

	//게임 객체는 하나의 재질을 가질 수 있다.
	CMaterial  * m_pMaterial;
	void SetMaterial(CMaterial *pMaterial);

public:
	void SetInheritAutoRender(bool bUse) { m_bUseInheritAutoRender = bUse; }
	void ChangeChild(CGameObject * pObject);
	void SetChild(CGameObject* pObject);
	void SetSibling(CGameObject * pObject);
	//void SetParent(CGameObject * pObject) { m_pParent = pObject; }

	CGameObject * GetChildObject() { return m_pChild; }
	CGameObject * GetSiblingObject() { return m_pSibling; }
	//CGameObject * GetParentObject() { return m_pParent; }

	// 부모 형제 자식을 다 끊어버림
	void ReleaseRelationShip();

public:
	CMesh* GetMesh(const int nIndex = 0) { return(m_ppMeshes[nIndex]); }
	CTexture* m_pTexture;

	void SetTexture(CTexture* const pTexture, bool beforeRelease = true);
	CTexture* GetTexture() { return m_pTexture; }

	virtual void UpdateBoundingBox();
	virtual void SetMesh(CMesh* const pMesh, int nIndex = 0);
	virtual void Animate(float fTimeElapsed);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld = nullptr);
	// child, sibling 객체를 그리고, 월드 좌표계를 셋 한다.
	void UpdateSubResources(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld = nullptr);

	virtual void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& xv3Position);
	void SetPosition(const XMVECTOR* xv3Position);

	virtual XMFLOAT3 GetPosition() const;

public:
	virtual bool IsVisible(CCamera *pCamera = nullptr);

	//로컬 x-축, y-축, z-축 방향으로 이동한다.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	//로컬 x-축, y-축, z-축 방향으로 회전한다.
	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	virtual void Rotate(XMFLOAT3 *pxv3Axis, float fAngle);

	//객체의 위치, 로컬 x-축, y-축, z-축 방향 벡터를 반환한다.
	XMFLOAT3 GetLookAt() const;
	XMFLOAT3 GetUp() const;
	XMFLOAT3 GetRight() const;

	//객체를 렌더링하기 전에 호출되는 함수이다.
	virtual void OnPrepareRender() { }

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
	float m_fFriction;
	XMFLOAT3 m_xmf3ExternalPower;

public:
	CStaticObject(int nMeshes);
	virtual ~CStaticObject();

public:
	void SetExternalPower(XMFLOAT3 & xmfPower) { m_xmf3ExternalPower = xmfPower; }
};

// 스스로 움직일 수 있는 유동적인 객체
class CDynamicObject : public CGameObject
{
public:
	CDynamicObject(int nMeshes);
	virtual ~CDynamicObject(){}

protected:
	XMFLOAT3 m_xv3Velocity;

public:
	void SetVelocity(const XMFLOAT3& xv3Velocity) { m_xv3Velocity = xv3Velocity; }
	const XMFLOAT3& GetVelocity() const { return(m_xv3Velocity); }
};

class CAnimatedObject : public CDynamicObject
{
public:
	CAnimatedObject(int nMeshes);
	virtual ~CAnimatedObject();

protected:
	bool m_bReserveBackIdle;
	typedef CAnimatedMesh ANI_MESH;
	
	WORD m_wdAnimateState;

	vector<float> m_vcfAnimationCycleTime;
	vector<float> m_vcfFramePerTime;
	vector<WORD>  m_vcNextAnimState;

public:
	void ChangeAnimationState(WORD wd, bool bReserveIdle, WORD * pNextStateArray, int nNum);
	void SetAnimationCycleTime(WORD wdAnimNum, float fCycleTime); 
	WORD GetAnimationState() const { return m_wdAnimateState;}
	void UpdateFramePerTime();

	ANI_MESH* GetAniMesh() { return static_cast<ANI_MESH*>(m_ppMeshes[m_wdAnimateState]); }

	virtual void SetMesh(CMesh *pMesh, int nIndex = 0);
	virtual void UpdateBoundingBox();

	virtual void Animate(float fTimeElapsed);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld = nullptr);
};

class CUIObject : public CGameObject
{
	UIInfo m_info;

public:
	CUIObject(int nMeshes, UIInfo info = UIInfo());
	virtual ~CUIObject() {}

	bool CollisionCheck(XMFLOAT3 & pos);
	bool CollisionCheck(POINT & pt);

	UIMessage & GetUIMessage() { return m_info.m_msgUI; }
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

class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(ID3D11Device *pd3dDevice, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xv3Scale);
	virtual ~CHeightMapTerrain();

public:
	float GetPeakHeight() { return(m_bcMeshBoundingCube.m_xv3Maximum.y); }
};

class CWaterTerrain : public CGameObject
{
	float m_fDepth;
public:
	CWaterTerrain(ID3D11Device *pd3dDevice, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xv3Scale);
	virtual ~CWaterTerrain();

	float GetDepth() { return m_fDepth; }
	void SetDepth(float depth) { m_fDepth = depth; }
};

class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D11Device *pd3dDevice, UINT uImageNum);
	virtual ~CSkyBox();

	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld = nullptr);
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
	const int miAbsorbSize = 50;

	bool	m_bAbsorb;
	float	m_fAbsorbTime;
	float	m_fSpeed;

	CGameObject * m_pTargetObject;
	XMFLOAT3      m_xvRandomVelocity;

public:
	CAbsorbMarble();
	CAbsorbMarble(XMFLOAT3 pos, UINT fID, XMFLOAT2 xmf2Size);
	~CAbsorbMarble();

	void Initialize();
	void Revive();
	void SetTarget(CGameObject * pGameObject);

	virtual bool IsVisible(CCamera *pCamera = nullptr);
	virtual void Animate(float fTimeElapsed);
	virtual void GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra = nullptr);
};