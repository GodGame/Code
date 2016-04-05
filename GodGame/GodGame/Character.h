#pragma once

#ifndef __CHARACTER
#define __CHARACTER

#include "Object.h"
#include "GameInfo.h"

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
	float m_fGravity;
	XMFLOAT3 m_xv3ExternalPower;
	//xz-평면에서 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다.
	float    m_fMaxVelocityXZ;
	//y-축 방향으로 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다.
	float    m_fMaxVelocityY;
	//플레이어에 작용하는 마찰력을 나타낸다.
	float    m_fFriction;
	//플레이어의 위치가 바뀔 때마다 호출되는 OnPlayerUpdated() 함수에서 사용하는 데이터이다.
	LPVOID   m_pUpdatedContext;

	StatusInfo		m_Status;

public:
	void SetFriction(float fFriction)           { m_fFriction = fFriction; }
	void SetGravity(const float fGravity)		{ m_fGravity = fGravity; }
	void SetMaxVelocityXZ(float fMaxVelocity)   { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity)    { m_fMaxVelocityY = fMaxVelocity; }
	void SetExternalPower(XMFLOAT3 & xmf3Power);

	StatusInfo& GetStatus() {return m_Status;}
	void CalculateFriction(float fTimeElapsed);

public:
	CCharacter(int nMeshes);
	virtual ~CCharacter();

	virtual void BuildObject() {}
	virtual void InitializeAnimCycleTime(){}

	virtual void OnPrepareRender();
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
	virtual void Animate(float fTimeElapsed);

public:
	virtual void Attack(CCharacter * pToChar, short stDamage);
	virtual void AttackSuccess(CCharacter * pToChar, short stDamage);
	virtual void Damaged(CCharacter * pByChar, short stDamage);
	virtual void Reset() { m_Status.ResetStatus(); }
	virtual void Revive() { Reset(); }
public:
	virtual void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xv3Position);

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(XMFLOAT3& xv3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);

	virtual void Rotate(float x, float y, float z);
	virtual void Rotate(XMFLOAT3 & xmf3RotAxis, float fAngle);

	void LookToTarget(CGameObject * pTarget);
	//위치와 회전 정보를 경과 시간에 따라 갱신하는 함수이다.
	void Update(float fTimeElapsed);

	//플레이어의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnContextUpdated(float fTimeElapsed);
	void SetUpdatedContext(LPVOID pContext) { m_pUpdatedContext = pContext; }
};

class CMonster : public CCharacter
{
protected:
	CCharacter * m_pTarget;

public:
	CMonster(int nMeshes); 
	virtual ~CMonster();
	virtual void BuildObject(CCharacter * pTarget) {SetTarget(pTarget);}

public:
	void SetTarget(CCharacter * pTarget) { m_pTarget = pTarget; }
	CCharacter* GetTarget() { return m_pTarget; }
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

private:
	const float mfMAX_HEALTH     = 50.0f;
	const float mfSWIPING_DAMAGE = 10.0f;
	const float mfPUNCH_DAMAGE   = 5.0f;

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

	virtual void BuildObject(CCharacter * pTarget);
	virtual void InitializeAnimCycleTime();

	//virtual void OnPrepareRender();
	//virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
	virtual void Animate(float fTimeElapsed);
	virtual void GetGameMessage(CGameObject * byObj, eMessage eMSG, void * extra = nullptr);

public:
	virtual void Attack(CCharacter * pToChar, short stDamage);
	virtual void AttackSuccess(CCharacter * pToChar, short stDamage);
	virtual void Damaged(CCharacter * pByChar, short stDamage);
	virtual void Reset() {m_Status.ResetStatus(); m_Status.ChangeHP(mfMAX_HEALTH); }

public:
	CStateMachine<CWarrock>* GetFSM() { return m_pStateMachine; }

	float GetPunchDamage()   { return mfPUNCH_DAMAGE; }
	float GetSwipingDamage() { return mfSWIPING_DAMAGE; }
};


#endif