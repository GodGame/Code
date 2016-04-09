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

class CEffect : public CEntity
{
protected:
	MoveVelocity m_velocity;
	float m_fDurability;

	bool m_bReserveDelete : 1;
	bool m_bEnable      : 1;
	bool m_bTerminal    : 1;
	bool m_bSubordinate : 1;
	bool m_bMove        : 1;
	bool m_bUseAccel    : 1;

public:
	void SetDurabilityTime(float fTime)  { m_fDurability = fTime; }
	float GetDurabilityTime()	const	 { return m_fDurability; }

	void SetMoveVelocity(const XMFLOAT3 & vel) { m_velocity.xmf3Velocity = vel; }
	void SetMoveAccel(const XMFLOAT3 & acc)    { m_velocity.xmf3Accelate = acc; }

	bool IsAble() { return m_bEnable; }
	bool IsSubordinative() { return m_bSubordinate; }

	void MoveUpdate(const float & fGameTime, const float & fTimeElapsed, XMFLOAT3 & xmf3Pos);
	void SetMoveVelocity(MoveVelocity & move, XMFLOAT3 * InitPos);

protected:
	UINT m_nStartVertex;
	UINT m_nVertexStrides;
	UINT m_nVertexOffsets;

	ID3D11ShaderResourceView * m_pd3dSRVImagesArrays;
	ID3D11Buffer * m_pd3dDrawVertexBuffer;

public:
	CEffect();
	virtual ~CEffect();

	void SetShaderResourceView(ID3D11ShaderResourceView * pSRV) 
	{
		if (m_pd3dSRVImagesArrays) m_pd3dSRVImagesArrays->Release();
		m_pd3dSRVImagesArrays = pSRV;
		m_pd3dSRVImagesArrays->AddRef();
	}
	inline void UpdateShaderResourceArray(ID3D11DeviceContext * pd3dDeviceContext)
	{
		pd3dDeviceContext->PSSetShaderResources(TX_SLOT_TEXTURE_ARRAY, 1, &m_pd3dSRVImagesArrays);
	}
	inline void UpdateShaderResource(ID3D11DeviceContext * pd3dDeviceContext)
	{
		pd3dDeviceContext->PSSetShaderResources(0, 1, &m_pd3dSRVImagesArrays);
	}
	virtual void UpdateBoundingBox() = 0;
	virtual void NextEffectOn(){}

	virtual void GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra = nullptr);
	virtual void SendGameMessage(CEntity * toEntity, eMessage eMSG, void * extra = nullptr);

	virtual bool Enable(XMFLOAT3 * pos = nullptr, int fColorNum = COLOR_NONE) { return false; }
	virtual bool Disable() { return false; }
	virtual void Collide();
};
/////////////////////////////////////////////////////////////////////////////////////////////////////
struct TX_ANIMATION_VERTEX
{
	XMFLOAT2	xmf2FrameRatePercent;
	XMFLOAT2	xmf2FrameTotalSize;
};

class CTxAnimationObject : public CEffect
{
protected:
	CB_TX_ANIMATION_INFO m_cbInfo;
	CTxAnimationObject * m_pNextEffect;
	ID3D11Buffer * m_pd3dCSBuffer;
	bool	 m_bUseAnimation;
//	float	 m_fTimeStartUseAnimation;

public:
	CTxAnimationObject();
	virtual ~CTxAnimationObject();

	virtual void Initialize(ID3D11Device *pd3dDevice);
	void CreateBuffers(ID3D11Device * pd3dDevice, XMFLOAT2 & xmf2ObjSize, XMFLOAT2 & xmf2FrameTotalSize, XMFLOAT2 & xmf2FrameSize, UINT dwFrameNum, float dwFramePerTime);
	/* 1 : info ����ü, 2 : �� ������ �ʺ�, 3 : �� ������ ����, 4 : ������ �� ����, 5 : ������ �� �ð�  */
	void CalculateCSInfoTime(TX_ANIMATION_VERTEX & vertex, XMFLOAT2 & xmf2ObjSize, XMFLOAT2 & xmf2FrameSize, UINT dwFrameNum, float dwFramePerTime);
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext);

public:
	void UseAnimation() { m_bUseAnimation = true; }
	virtual XMFLOAT3 GetPosition() const { return m_cbInfo.m_xmf3Pos; }
	virtual void UpdateBoundingBox()
	{
		m_bcMeshBoundingCube.Update(m_cbInfo.m_xmf3Pos, m_uSize);
	}

	virtual bool Enable(XMFLOAT3 * pos = nullptr, int fColorNum = COLOR_NONE);
	virtual bool Disable();
	virtual void NextEffectOn();
	bool IsTermainal() { return (m_pNextEffect) ? m_pNextEffect->IsTermainal() : !m_bEnable; }

public:
	void OnPrepare(ID3D11DeviceContext * pd3dDeviceContext) { UpdateShaderResource(pd3dDeviceContext); }
	void Animate(float fTimeElapsed);
	void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld = nullptr);
};

class CCircleMagic : public CTxAnimationObject
{
public:
	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CIceSpear : public CTxAnimationObject
{
public:
	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CIceBolt : public CTxAnimationObject
{
public:
	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CElectricBolt : public CTxAnimationObject
{
public:
	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CElementSpike : public CTxAnimationObject
{
public:
	virtual void Initialize(ID3D11Device *pd3dDevice);
};
///////////////////////////////////////////////////////////////////////////////////////////////////////
struct PARTICLE_INFO
{
	XMFLOAT3 m_xmf3Pos;
	XMFLOAT3 m_xmf3Velocity;
	XMFLOAT2 m_xmf2Size;
	float	 m_fAge;
	float	 m_uType;
};

struct PARTILCE_ON_INFO
{
	XMFLOAT3 m_xmf3Pos;
	XMFLOAT3 m_xmf3Velocity;
	XMFLOAT3 m_xmfAccelate;
	float fColor;
	int iNum;
};

class CParticle : public CEffect
{
protected:
	CB_PARTICLE m_cbParticle;

	bool m_bInitilize   : 1;
	//UINT m_bExtra     : 28;
	ID3D11Buffer * m_pd3dInitialVertexBuffer;
	ID3D11Buffer * m_pd3dStreamOutVertexBuffer;

	ID3D11Buffer * m_pd3dCSParticleBuffer;

protected:
	CParticle   * m_pcNextParticle;

public:
	CParticle();
	virtual ~CParticle();

	CB_PARTICLE * GetCBParticle() { return &m_cbParticle; }
	void Update(float fTimeElapsed);
	void LifeUpdate(const float & fGameTime, const float & fTimeElapsed);

	virtual void Initialize(ID3D11Device *pd3dDevice);
	void SetParticle(CB_PARTICLE & info, MoveVelocity & Velocity, float fDurability, UINT uMaxParticle);
	void CreateParticleBuffer(ID3D11Device *pd3dDevice, PARTICLE_INFO & pcInfo, UINT nMaxNum);

	void StreamOut(ID3D11DeviceContext *pd3dDeviceContext);
	void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld = nullptr);
	void OnPrepare(ID3D11DeviceContext * pd3dDeviceContext) { UpdateShaderResourceArray(pd3dDeviceContext); }
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext);

public:
	// virtual void GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra = nullptr);
	//virtual void SendGameMessage(CEntity * toEntity, eMessage eMSG, void * extra = nullptr);
	virtual XMFLOAT3 GetPosition() const { return m_cbParticle.m_vParticleEmitPos; }
	virtual void UpdateBoundingBox()
	{
		m_bcMeshBoundingCube.Update(m_cbParticle.m_vParticleEmitPos, m_uSize);
		//cout << "BB : " << m_bcMeshBoundingCube << endl;
	}

public:
	void SetLifeTime(float fLifeTime)     { m_cbParticle.m_fLifeTime = fLifeTime; }
	void SetEmitPosition(XMFLOAT3 & pos)  { m_cbParticle.m_vParticleEmitPos = pos; }
	void SetEmitDirection(XMFLOAT3 & dir) { m_cbParticle.m_vParticleVelocity = dir; }
	void SetAccelation(XMFLOAT3 & accel)  { m_cbParticle.m_vAccel = accel; }
	void SetParticleSize(float fSize)     { m_cbParticle.m_fMaxSize = fSize; }

	virtual bool Enable(XMFLOAT3 * pos = nullptr, int fColorNum = COLOR_NONE);
	virtual bool Disable();
	virtual void NextEffectOn();
	//void NextParticleOn();

	bool IsTermainal() { return (m_pcNextParticle) ? m_pcNextParticle->IsTermainal() : !m_bEnable; }
};

class CSmokeBoomParticle : public CParticle
{
	static const UINT m_nMaxParticlenum = 200;

public:
	CSmokeBoomParticle(){}
	virtual ~CSmokeBoomParticle(){}

	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CFireBallParticle : public CParticle
{
	static const UINT m_nMaxParticlenum = 500;

public:
	CFireBallParticle() {}
	virtual ~CFireBallParticle() {}

	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CRainParticle : public CParticle
{
	static const UINT m_nMaxParticlenum = 4000;

public:
	CRainParticle(){}
	virtual ~CRainParticle(){}
	
	//void StreamOut(ID3D11DeviceContext *pd3dDeviceContext);
	virtual void Initialize(ID3D11Device *pd3dDevice);
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

class CHeightMap
{
private:
	//���� �� �̹��� �ȼ�(8-��Ʈ)���� ������ �迭�̴�. �� �ȼ��� 0~255�� ���� ���´�.
	BYTE* m_pHeightMapImage;
	//���� �� �̹����� ���ο� ���� ũ���̴�.
	int m_nWidth;
	int m_nLength;
	//���� ���� ������ �� �� Ȯ���Ͽ� ����� ���ΰ��� ��Ÿ���� ������ �����̴�.
	XMFLOAT3 m_xv3Scale;

public:
	CHeightMap(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3& xv3Scale);
	~CHeightMap(void);

	//���� �� �̹������� (x, z) ��ġ�� �ȼ� ���� ����� ������ ���̸� ��ȯ�Ѵ�.
	float GetHeight(float x, float z, bool bReverseQuad = false) const;
	//���� �� �̹������� (x, z) ��ġ�� ���� ���͸� ��ȯ�Ѵ�.
	XMFLOAT3& GetHeightMapNormal(int x, int z);
	XMFLOAT3& GetScale() { return(m_xv3Scale); }

	BYTE* GetHeightMapImage() const { return(m_pHeightMapImage); }
	int GetHeightMapWidth()   const { return(m_nWidth); }
	int GetHeightMapLength()  const { return(m_nLength); }
};

class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(ID3D11Device *pd3dDevice, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xv3Scale);
	virtual ~CHeightMapTerrain();

private:
	//������ ���� ������ ����� �̹����̴�.
	CHeightMap *m_pHeightMap;

	//������ ���ο� ���� ũ���̴�.
	int m_nWidth;
	int m_nLength;

	//������ ������ �� �� Ȯ���� ���ΰ��� ��Ÿ���� ������ �����̴�.
	XMFLOAT3 m_xv3Scale;
public:
	//������ ���� ���̸� ��ȯ�Ѵ�. ���� ���� ���̿� �������� ���� ���̴�.
	float GetHeight(float x, float z, bool bReverseQuad = false) { return(m_pHeightMap->GetHeight(x, z, bReverseQuad) * m_xv3Scale.y); }
	XMFLOAT3& GetNormal(float x, float z) { return(m_pHeightMap->GetHeightMapNormal(int(x / m_xv3Scale.x), int(z / m_xv3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMap->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMap->GetHeightMapLength()); }

	XMFLOAT3& GetScale() { return(m_xv3Scale); }
	//������ ���� ũ��(����/����)�� ��ȯ�Ѵ�. ���� ���� ũ�⿡ �������� ���� ���̴�.
	float GetWidth() { return(m_nWidth * m_xv3Scale.x); }
	float GetLength() { return(m_nLength * m_xv3Scale.z); }

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
	void SetTarget(CGameObject * pGameObject);

	virtual bool IsVisible(CCamera *pCamera = nullptr);
	virtual void Animate(float fTimeElapsed);
	virtual void GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra = nullptr);
};