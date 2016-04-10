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
	UINT	m_uSize : 14;
	bool	m_bActive : 1;
	bool	m_bUseCollide : 1;

public:
	CEntity();
	virtual ~CEntity(){}

public:
	AABB         m_bcMeshBoundingCube;

	void SetActive(const bool bActive = false) { m_bActive = bActive; }
	bool IsActvie() { return m_bActive; }

	void SetCollide(const bool bCollide) { m_bUseCollide = bCollide; }
	bool CanCollide(CEntity * pObj) const
	{
		if (false == m_bUseCollide) return false;
		if (this == pObj) return false;
		return true;
	}

	UINT GetSize() const { return m_uSize; }
	void SetSize(UINT uSize) { m_uSize = uSize; }

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
	UINT	m_nReferences : 16;

	CGameObject * m_pChild;
	CGameObject * m_pSibling;

public:
	void	 AddRef();
	void	 Release();

	XMFLOAT4X4	 m_xmf44World;
	//��ü�� ������ �޽��鿡 ���� �����Ϳ� �� �����̴�.
	CMesh **     m_ppMeshes;
	int          m_nMeshes;
	//AABB         m_bcMeshBoundingCube;

	//���� ��ü�� �ϳ��� ������ ���� �� �ִ�.
	CMaterial  * m_pMaterial;
	void SetMaterial(CMaterial *pMaterial);

public:
	void ChangeChild(CGameObject * pObject);
	void SetChild(CGameObject* pObject);
	void SetSibling(CGameObject * pObject);

	CGameObject * GetChildObject() { return m_pChild; }
	CGameObject * GetSiblingObject() { return m_pSibling; }

	// �θ� ���� �ڽ��� �� �������
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
	// child, sibling ��ü�� �׸���, ���� ��ǥ�踦 �� �Ѵ�.
	void UpdateSubResources(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld = nullptr);

	virtual void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& xv3Position);
	void SetPosition(const XMVECTOR* xv3Position);

	virtual XMFLOAT3 GetPosition() const;

public:
	virtual bool IsVisible(CCamera *pCamera = nullptr);

	//���� x-��, y-��, z-�� �������� �̵��Ѵ�.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	//���� x-��, y-��, z-�� �������� ȸ���Ѵ�.
	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	virtual void Rotate(XMFLOAT3 *pxv3Axis, float fAngle);

	//��ü�� ��ġ, ���� x-��, y-��, z-�� ���� ���͸� ��ȯ�Ѵ�.
	XMFLOAT3 GetLookAt() const;
	XMFLOAT3 GetUp() const;
	XMFLOAT3 GetRight() const;

	//��ü�� �������ϱ� ���� ȣ��Ǵ� �Լ��̴�.
	virtual void OnPrepareRender() { }

#ifdef PICKING
	//���� ��ǥ���� ��ŷ ������ �����Ѵ�.
	void GenerateRayForPicking(XMFLOAT3 *pxv3PickPosition, XMFLOAT4X4 *pxmtxWorld, XMFLOAT4X4 *pxmtxView, XMFLOAT3 *pxv3PickRayPosition, XMFLOAT3 *pxv3PickRayDirection);
	//���� ��ǥ���� ��ŷ ������ �����Ѵ�.
	int PickObjectByRayIntersection(XMFLOAT3 *pxv3PickPosition, XMFLOAT4X4 *pxmtxView, MESHINTERSECTINFO *pd3dxIntersectInfo);
#endif
};

// ������ �������� ���ϴ� ������ ��ü
class CStaticObject : public CGameObject
{
protected:

public:
	CStaticObject(int nMeshes);
	virtual ~CStaticObject();
};

// ������ ������ �� �ִ� �������� ��ü
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
	//���� �ӵ��� ȸ���� ���͸� ��Ÿ���� ��� ������ �����Ѵ�.
	float m_fRotationSpeed;
	XMFLOAT3 m_xv3RotationAxis;

public:
	//���� �ӵ��� ȸ���� ���͸� �����ϴ� �Լ��̴�.
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
	// ���� ȸ����� ȸ�� �ӷ��� ��Ÿ����.
	XMFLOAT3 m_xv3RevolutionAxis;
	float m_fRevolutionSpeed;

public:
	// ���� �ӷ��� �����Ѵ�.
	void SetRevolutionSpeed(float fRevolutionSpeed) { m_fRevolutionSpeed = fRevolutionSpeed; }
	// ������ ���� ȸ������ �����Ѵ�.
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