#pragma once
#include "Object.h"
//#include "Camera.h"

class CPlayer : public CGameObject
{
protected:
	//�÷��̾��� ��ġ ����, x-��(Right), y-��(Up), z-��(Look) �����̴�.
	XMFLOAT3 m_xv3Position;
	XMFLOAT3 m_xv3Right;
	XMFLOAT3 m_xv3Up;
	XMFLOAT3 m_xv3Look;

	//�÷��̾ ���� x-��(Right), y-��(Up), z-��(Look)���� �󸶸�ŭ ȸ���ߴ°��� ��Ÿ����.
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	//�÷��̾��� �̵� �ӵ��� ��Ÿ���� �����̴�.
	XMFLOAT3 m_xv3Velocity;
	//�÷��̾ �ۿ��ϴ� �߷��� ��Ÿ���� �����̴�.
	XMFLOAT3 m_xv3Gravity;
	//xz-��鿡�� (�� ������ ����) �÷��̾��� �̵� �ӷ��� �ִ밪�� ��Ÿ����.
	float m_fMaxVelocityXZ;
	//y-�� �������� (�� ������ ����) �÷��̾��� �̵� �ӷ��� �ִ밪�� ��Ÿ����.
	float m_fMaxVelocityY;
	//�÷��̾ �ۿ��ϴ� �������� ��Ÿ����.
	float m_fFriction;

	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnPlayerUpdated() �Լ����� ����ϴ� �������̴�.
	LPVOID m_pPlayerUpdatedContext;
	//ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnCameraUpdated() �Լ����� ����ϴ� �������̴�.
	LPVOID m_pCameraUpdatedContext;

	//�÷��̾ ���� ������ ī�޶��̴�.
	CCamera *m_pCamera;

public:
	CPlayer(int nMeshes = 1);
	virtual ~CPlayer();

	//�÷��̾��� ���� ī�޶� �����ϰ� ��ȯ�ϴ� ��� �Լ��� �����Ѵ�.
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }
	CCamera *GetCamera() { return(m_pCamera); }

	//�÷��̾��� ��� ���۸� �����ϰ� �����ϴ� ��� �Լ��� �����Ѵ�.
	void CreateShaderVariables(ID3D11Device *pd3dDevice);
	void UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext);


	XMFLOAT3 GetPosition() { return(m_xv3Position); }
	XMFLOAT3 GetLookVector() { return(m_xv3Look); }
	XMFLOAT3 GetUpVector() { return(m_xv3Up); }
	XMFLOAT3 GetRightVector() { return(m_xv3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xv3Gravity) { m_xv3Gravity = xv3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xv3Velocity) { m_xv3Velocity = xv3Velocity; }

	/*�÷��̾��� ��ġ�� xv3Position ��ġ�� �����Ѵ�. xv3Position ���Ϳ��� ���� �÷��̾��� ��ġ ���͸� ���� ���� �÷��̾��� ��ġ���� xv3Position ���������� ���� ���Ͱ� �ȴ�. ���� �÷��̾��� ��ġ���� �� ���� ���� ��ŭ �̵��Ѵ�.*/
	void InitPosition(XMFLOAT3 xv3Position);
	void SetPosition(XMFLOAT3& xv3Position);

	const XMFLOAT3& GetVelocity() const { return(m_xv3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	//�÷��̾ �̵��ϴ� �Լ��̴�.
	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(XMFLOAT3& xv3Shift, bool bVelocity = false);
	void Move(XMVECTOR& xvShift) { XMFLOAT3 xv3shift; XMStoreFloat3(&xv3shift, xvShift); Move(xv3shift); }
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	//�÷��̾ ȸ���ϴ� �Լ��̴�.
	void Rotate(float x, float y, float z);

	//�÷��̾��� ��ġ�� ȸ�� ������ ��� �ð��� ���� �����ϴ� �Լ��̴�.
	void Update(float fTimeElapsed);

	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�.
	virtual void OnPlayerUpdated(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

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