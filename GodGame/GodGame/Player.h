#pragma once

#include "Character.h"
//#include "Camera.h"
//#include "GameInfo.h"

class CPlayer : public CCharacter //public CAnimatedObject
{
protected:
	//플레이어가 로컬 x-축(Right), y-축(Up), z-축(Look)으로 얼마만큼 회전했는가를 나타낸다.
	float    m_fPitch;
	float    m_fYaw;
	float    m_fRoll;
	
	//카메라의 위치가 바뀔 때마다 호출되는 OnCameraUpdated() 함수에서 사용하는 데이터이다.
	LPVOID   m_pCameraUpdatedContext;
	//플레이어에 현재 설정된 카메라이다.
	CCamera *m_pCamera;

	CScene * m_pScene;

public:
	CPlayer(int nMeshes = 1);
	virtual ~CPlayer();
	virtual void BuildObject() {}
	virtual void InitializeAnimCycleTime(){}

	//플레이어의 현재 카메라를 설정하고 반환하는 멤버 함수를 선언한다.
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }
	CCamera *GetCamera() { return(m_pCamera); }
	void SetScene(CScene* pScene) { m_pScene = pScene; }
	//플레이어의 상수 버퍼를 생성하고 갱신하는 멤버 함수를 선언한다.
	void CreateShaderVariables(ID3D11Device *pd3dDevice);
	void UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext);

	/*플레이어의 위치를 xv3Position 위치로 설정한다. xv3Position 벡터에서 현재 플레이어의 위치 벡터를 빼면 현재 플레이어의 위치에서 xv3Position 방향으로의 방향 벡터가 된다. 현재 플레이어의 위치에서 이 방향 벡터 만큼 이동한다.*/
	void SetPosition(XMFLOAT3& xv3Position) { CCharacter::SetPosition(xv3Position); }
	void InitPosition(XMFLOAT3 xv3Position);

	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	//플레이어를 이동하는 함수이다.
	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(XMFLOAT3& xv3Shift, bool bVelocity = false);
	void Move(XMVECTOR& xvShift) { XMFLOAT3 xv3shift; XMStoreFloat3(&xv3shift, xvShift); Move(xv3shift); }
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	//플레이어를 회전하는 함수이다.
	virtual void Rotate(float x, float y, float z);
	virtual void Rotate(XMFLOAT3 & xmf3RotAxis, float fAngle);
	//플레이어의 위치와 회전 정보를 경과 시간에 따라 갱신하는 함수이다.
	virtual void Update(float fTimeElapsed);

	//플레이어의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnPlayerUpdated(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pUpdatedContext = pContext; }

	//카메라의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnCameraUpdated(float fTimeElapsed);
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	//카메라를 변경할 때 호출되는 함수이다.
	CCamera *OnChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual void ChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed);
	//플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수이다.
	virtual void OnPrepareRender();
	//플레이어의 카메라가 3인칭 카메라일 때 플레이어 메쉬를 렌더링한다.
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