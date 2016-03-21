#pragma once
#include "Object.h"
//#include "Camera.h"
#include "GameInfo.h"

class CPlayer : public CAnimatedObject
{
protected:
	//플레이어의 위치 벡터, x-축(Right), y-축(Up), z-축(Look) 벡터이다.
	XMFLOAT3 m_xv3Position;
	XMFLOAT3 m_xv3Right;
	XMFLOAT3 m_xv3Up;
	XMFLOAT3 m_xv3Look;

	//플레이어가 로컬 x-축(Right), y-축(Up), z-축(Look)으로 얼마만큼 회전했는가를 나타낸다.
	float    m_fPitch;
	float    m_fYaw;
	float    m_fRoll;

	//플레이어에 작용하는 중력을 나타내는 벡터이다.
	XMFLOAT3 m_xv3Gravity;
	//xz-평면에서 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다.
	float    m_fMaxVelocityXZ;
	//y-축 방향으로 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다.
	float    m_fMaxVelocityY;
	//플레이어에 작용하는 마찰력을 나타낸다.
	float    m_fFriction;

	//플레이어의 위치가 바뀔 때마다 호출되는 OnPlayerUpdated() 함수에서 사용하는 데이터이다.
	LPVOID   m_pPlayerUpdatedContext;
	//카메라의 위치가 바뀔 때마다 호출되는 OnCameraUpdated() 함수에서 사용하는 데이터이다.
	LPVOID   m_pCameraUpdatedContext;
	//플레이어에 현재 설정된 카메라이다.
	CCamera *m_pCamera;

	CScene * m_pScene;

public:
	CPlayer(int nMeshes = 1);
	virtual ~CPlayer();
	virtual void BuildObject() {}

	//플레이어의 현재 카메라를 설정하고 반환하는 멤버 함수를 선언한다.
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }
	CCamera *GetCamera() { return(m_pCamera); }
	void SetScene(CScene* pScene) { m_pScene = pScene; }
	//플레이어의 상수 버퍼를 생성하고 갱신하는 멤버 함수를 선언한다.
	void CreateShaderVariables(ID3D11Device *pd3dDevice);
	void UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext);

	XMFLOAT3 & GetPosition() { return(m_xv3Position); }
	XMFLOAT3 & GetLookVector() { return(m_xv3Look); }
	XMFLOAT3 & GetUpVector() { return(m_xv3Up); }
	XMFLOAT3 & GetRightVector() { return(m_xv3Right); }

	XMFLOAT3 GetLookVectorInverse() { return XMFLOAT3(-m_xv3Look.x, -m_xv3Look.y, -m_xv3Look.z); }
	XMFLOAT3 GetUpVectorInverse() { return XMFLOAT3(-m_xv3Up.x, -m_xv3Up.y, -m_xv3Up.z); }
	XMFLOAT3 GetRightVectorInverse() { return XMFLOAT3(-m_xv3Right.x, -m_xv3Right.y, -m_xv3Right.z); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xv3Gravity) { m_xv3Gravity = xv3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }

	/*플레이어의 위치를 xv3Position 위치로 설정한다. xv3Position 벡터에서 현재 플레이어의 위치 벡터를 빼면 현재 플레이어의 위치에서 xv3Position 방향으로의 방향 벡터가 된다. 현재 플레이어의 위치에서 이 방향 벡터 만큼 이동한다.*/
	void InitPosition(XMFLOAT3 xv3Position);
	void SetPosition(XMFLOAT3& xv3Position);

	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	//플레이어를 이동하는 함수이다.
	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(XMFLOAT3& xv3Shift, bool bVelocity = false);
	void Move(XMVECTOR& xvShift) { XMFLOAT3 xv3shift; XMStoreFloat3(&xv3shift, xvShift); Move(xv3shift); }
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	//플레이어를 회전하는 함수이다.
	void Rotate(float x, float y, float z);
	void Rotate(XMFLOAT3 & xmf3RotAxis, float fAngle);
	//플레이어의 위치와 회전 정보를 경과 시간에 따라 갱신하는 함수이다.
	void Update(float fTimeElapsed);

	//플레이어의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnPlayerUpdated(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

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
	ElementEnergy	m_nElemental;
	StatusInfo		m_Status;

	CBuff		  * m_pBuff;
	CDeBuff       * m_pDebuff;

public:
	CInGamePlayer(int m_nMeshes);
	virtual ~CInGamePlayer();
	virtual void BuildObject();

	virtual void GetGameMessage(CGameObject * byObj, eMessage eMSG, void * extra);
	virtual void SendGameMessage(CGameObject * toObj, eMessage eMSG, void * extra);

public:
	BYTE & GetEnergyNum(UINT index) { return m_nElemental.m_nEnergies[index]; }
	BYTE & GetEnergyNum() { return m_nElemental.m_nSum; }
	
	void AddEnergy(UINT index, UINT num = 0);

	UINT UseEnergy(UINT index, BYTE energyNum, bool bForced = false);
	UINT UseEnergy(UINT energyNum, bool bForced = false);
	UINT UseAllEnergy(UINT energyNum, bool bForced = false);

	void InitEnergy() { ZeroMemory(&m_nElemental, sizeof(m_nElemental)); }
};