#pragma once

#ifndef __CHARACTER
#define __CHARACTER

#include "Object.h"

class CCharacter : public CAnimatedObject
{
protected:
	XMFLOAT3 m_xv3Position;
	XMFLOAT3 m_xv3Right;
	XMFLOAT3 m_xv3Up;
	XMFLOAT3 m_xv3Look;

public:
	XMFLOAT3 & GetPosition()         { return(m_xv3Position); }
	XMFLOAT3 & GetLookVector()       { return(m_xv3Look); }
	XMFLOAT3 & GetUpVector()         { return(m_xv3Up); }
	XMFLOAT3 & GetRightVector()      { return(m_xv3Right); }

	XMFLOAT3 GetLookVectorInverse()  { return XMFLOAT3(-m_xv3Look.x, -m_xv3Look.y, -m_xv3Look.z); }
	XMFLOAT3 GetUpVectorInverse()    { return XMFLOAT3(-m_xv3Up.x, -m_xv3Up.y, -m_xv3Up.z); }
	XMFLOAT3 GetRightVectorInverse() { return XMFLOAT3(-m_xv3Right.x, -m_xv3Right.y, -m_xv3Right.z); }

protected:
	//�÷��̾ �ۿ��ϴ� �߷��� ��Ÿ���� �����̴�.
	XMFLOAT3 m_xv3Gravity;
	//xz-��鿡�� (�� ������ ����) �÷��̾��� �̵� �ӷ��� �ִ밪�� ��Ÿ����.
	float    m_fMaxVelocityXZ;
	//y-�� �������� (�� ������ ����) �÷��̾��� �̵� �ӷ��� �ִ밪�� ��Ÿ����.
	float    m_fMaxVelocityY;
	//�÷��̾ �ۿ��ϴ� �������� ��Ÿ����.
	float    m_fFriction;
	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnPlayerUpdated() �Լ����� ����ϴ� �������̴�.
	LPVOID   m_pUpdatedContext;

public:
	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xv3Gravity) { m_xv3Gravity = xv3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }

public:
	CCharacter(int nMeshes);
	virtual ~CCharacter();

	virtual void BuildObject() {}
	virtual void InitializeAnimCycleTime(){}

	virtual void OnPrepareRender();
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
	virtual void Animate(float fTimeElapsed);

public:
	virtual void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xv3Position);

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(XMFLOAT3& xv3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);

	virtual void Rotate(float x, float y, float z);
	virtual void Rotate(XMFLOAT3 & xmf3RotAxis, float fAngle);

	void LookToTarget(CGameObject * pTarget);
	//��ġ�� ȸ�� ������ ��� �ð��� ���� �����ϴ� �Լ��̴�.
	void Update(float fTimeElapsed);

	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�.
	virtual void OnContextUpdated(float fTimeElapsed);
	void SetUpdatedContext(LPVOID pContext) { m_pUpdatedContext = pContext; }
};

class CMonster : public CCharacter
{
protected:
	CGameObject * m_pTarget;

public:
	CMonster(int nMeshes); 
	virtual ~CMonster();
	virtual void BuildObject(CGameObject * pTarget) {SetTarget(pTarget);}

public:
	void SetTarget(CGameObject * pTarget) { m_pTarget = pTarget; }
	CGameObject* GetTarget() { return m_pTarget; }
};

class CSkeleton : public CMonster
{
private:
	CStateMachine<CSkeleton>* m_pStateMachine;

public:
	CSkeleton(int nMeshes);
	virtual ~CSkeleton();

	CStateMachine<CSkeleton>* GetFSM() { return m_pStateMachine;}
};

class CDistanceEvaluator;


class CWarrock : public CMonster
{
private:
	const float mfIDLE_ANIM    = 1.0f;
	const float mfRUN_ANIM     = 1.2f;
	const float mfROAR_ANIM    = 3.0f;
	const float mfPUNCH_ANIM   = 1.0f;
	const float mfSWIP_ANIM    = 2.0f;
	const float mfDEATH_ANIM   = 2.0f;

public:
	enum eWarrockAnim : WORD
	{
		eANI_WARROCK_IDLE = 0,
		eANI_WARROCK_RUN,
		eANI_WARROCK_ROAR,
		eANI_WARROCK_PUNCH,
		eANI_WARROCK_SWIPING,
		eANI_WARROCK_DEATH,
		eANI_WARROCK_ANIM_NUM
	};

private:
	CStateMachine<CWarrock>* m_pStateMachine;

	CDistanceEvaluator mEvaluator;
	CTargetDotEvaluator mDotEvaluator;

public:
	CWarrock(int nMeshes);
	virtual ~CWarrock();

	virtual void BuildObject(CGameObject * pTarget);
	virtual void InitializeAnimCycleTime();

	//virtual void OnPrepareRender();
	//virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
	virtual void Animate(float fTimeElapsed);
	virtual void GetGameMessage(CGameObject * byObj, eMessage eMSG, void * extra = nullptr);

public:
	CStateMachine<CWarrock>* GetFSM() { return m_pStateMachine; }

};


#endif