#pragma once

#include "Character.h"
//#include "Camera.h"
//#include "GameInfo.h"

class CPlayer : public CCharacter //public CAnimatedObject
{
protected:
	//�÷��̾ ���� x-��(Right), y-��(Up), z-��(Look)���� �󸶸�ŭ ȸ���ߴ°��� ��Ÿ����.
	float    m_fPitch;
	float    m_fYaw;
	float    m_fRoll;
	
	//ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnCameraUpdated() �Լ����� ����ϴ� �������̴�.
	LPVOID   m_pCameraUpdatedContext;
	//�÷��̾ ���� ������ ī�޶��̴�.
	CCamera *m_pCamera;

	CScene * m_pScene;

public:
	CPlayer(int nMeshes = 1);
	virtual ~CPlayer();
	virtual void BuildObject() {}
	virtual void InitializeAnimCycleTime(){}

	//�÷��̾��� ���� ī�޶� �����ϰ� ��ȯ�ϴ� ��� �Լ��� �����Ѵ�.
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }
	CCamera *GetCamera() { return(m_pCamera); }
	void SetScene(CScene* pScene) { m_pScene = pScene; }
	//�÷��̾��� ��� ���۸� �����ϰ� �����ϴ� ��� �Լ��� �����Ѵ�.
	void CreateShaderVariables(ID3D11Device *pd3dDevice);
	void UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext);

	/*�÷��̾��� ��ġ�� xv3Position ��ġ�� �����Ѵ�. xv3Position ���Ϳ��� ���� �÷��̾��� ��ġ ���͸� ���� ���� �÷��̾��� ��ġ���� xv3Position ���������� ���� ���Ͱ� �ȴ�. ���� �÷��̾��� ��ġ���� �� ���� ���� ��ŭ �̵��Ѵ�.*/
	void SetPosition(XMFLOAT3& xv3Position) { CCharacter::SetPosition(xv3Position); }
	void InitPosition(XMFLOAT3 xv3Position);

	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	//�÷��̾ �̵��ϴ� �Լ��̴�.
	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(XMFLOAT3& xv3Shift, bool bVelocity = false);
	void Move(XMVECTOR& xvShift) { XMFLOAT3 xv3shift; XMStoreFloat3(&xv3shift, xvShift); Move(xv3shift); }
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	//�÷��̾ ȸ���ϴ� �Լ��̴�.
	virtual void Rotate(float x, float y, float z);
	virtual void Rotate(XMFLOAT3 & xmf3RotAxis, float fAngle);
	//�÷��̾��� ��ġ�� ȸ�� ������ ��� �ð��� ���� �����ϴ� �Լ��̴�.
	virtual void Update(float fTimeElapsed);

	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�.
	virtual void OnPlayerUpdated(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pUpdatedContext = pContext; }

	//ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�.
	virtual void OnCameraUpdated(float fTimeElapsed);
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	//ī�޶� ������ �� ȣ��Ǵ� �Լ��̴�.
	CCamera *OnChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual void ChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed);
	//�÷��̾��� ��ġ�� ȸ�������κ��� ���� ��ȯ ����� �����ϴ� �Լ��̴�.
	virtual void OnPrepareRender();
	//�÷��̾��� ī�޶� 3��Ī ī�޶��� �� �÷��̾� �޽��� �������Ѵ�.
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
	virtual void Animate(float fTimeElapsed);
};

class CTerrainPlayer : public CPlayer
{
public:
	CTerrainPlayer(int nMeshes = 1);
	virtual ~CTerrainPlayer();

	virtual void ChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed);

	virtual void OnPlayerUpdated(float fTimeElapsed);
	virtual void OnCameraUpdated(float fTimeElapsed);
};

class CInGamePlayer : public CTerrainPlayer
{
public:
	WORD mwd1HMagicShot[2] = {eANI_1H_CAST, eANI_1H_MAGIC_ATTACK};
private:
	const short mMAX_HEALTH             = 50;

	const float mfIdleAnim              = 2.0f;
	const float mfRunForwardAnim        = 0.8f;

	const float mf1HCastAnimTime        = 1.0f;
	const float mf1HMagicAttackAnimTime = 1.4f;
	const float mf1HMagicAreaAnimTime   = 2.5f;

	const float mfDamagedAnimTime01     = 0.8f;
	const float mfDamagedAnimTime02     = 1.3f;
	const float mfDeathAnimTime         = 3.0f;

private:
	CStateMachine<CInGamePlayer>* m_pStateMachine;

	ElementEnergy	m_nElemental;

public:
	CInGamePlayer(int m_nMeshes);
	virtual ~CInGamePlayer();
	virtual void BuildObject(CMesh ** ppMeshList, int nMeshes, CTexture * pTexture, CMaterial * pMaterial, CHeightMapTerrain * pTerrain);

	virtual void GetGameMessage(CGameObject * byObj, eMessage eMSG, void * extra);
	virtual void SendGameMessage(CGameObject * toObj, eMessage eMSG, void * extra);

	virtual void InitializeAnimCycleTime();
	virtual void Update(float fTimeElapsed);

public:
	virtual void Attack(CCharacter * pToChar, short stDamage);
	virtual void AttackSuccess(CCharacter * pToChar, short stDamage);
	virtual void Damaged(CCharacter * pByChar, short stDamage);
	virtual void Reset();
	virtual void Revive();
public:
	void PlayerKeyEventOn(WORD key, void * extra);
	CStateMachine<CInGamePlayer>* GetFSM() { return m_pStateMachine; }

public:
	XMFLOAT3 GetCenterPosition() { return XMFLOAT3(m_xv3Position.x, m_xv3Position.y + 9.0f, m_xv3Position.z); }

	BYTE & GetEnergyNum(UINT index) { return m_nElemental.m_nEnergies[index]; }
	BYTE & GetEnergyNum() { return m_nElemental.m_nSum; }
	
	void AddEnergy(UINT index, UINT num = 0);

	UINT UseEnergy(UINT index, BYTE energyNum, bool bForced = false);
	UINT UseEnergy(UINT energyNum, bool bForced = false);
	UINT UseAllEnergy(UINT energyNum, bool bForced = false);

	void InitEnergy() { ZeroMemory(&m_nElemental, sizeof(m_nElemental)); }

public:
	PARTILCE_ON_INFO Get1HAnimShotParticleOnInfo();
};