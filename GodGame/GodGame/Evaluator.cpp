#include "stdafx.h"
#include "Evaluator.h"
#include "Character.h"

CEvaluator::CEvaluator()
{
}

CEvaluator::~CEvaluator()
{
}

CTargetEvaluator::CTargetEvaluator()
{
	m_pTarget = nullptr;
	m_pThis = nullptr;
}

CTargetEvaluator::~CTargetEvaluator()
{
}


CDistanceEvaluator::CDistanceEvaluator() : CTargetEvaluator()
{
	m_fWantDistance = 0.0f;
}

CDistanceEvaluator::~CDistanceEvaluator()
{
}

void CDistanceEvaluator::SetEvaluate(CGameObject * pTarget, CGameObject * pThis, float fWantDistance)
{
	m_pTarget = pTarget;
	m_pThis = pThis;
	m_fWantDistance = fWantDistance;
}

float CDistanceEvaluator::Evaluate()
{
	XMVECTOR xmvTargetPos = XMLoadFloat3(&m_pTarget->GetPosition());
	XMVECTOR xmvThisPos   = XMLoadFloat3(&m_pThis->GetPosition());
	float fDistanceSq = EVAL_NULL;
	float fWantDistSq = m_fWantDistance * m_fWantDistance;

	XMStoreFloat(&fDistanceSq, XMVector3LengthSq(xmvTargetPos - xmvThisPos));

	return fDistanceSq / fWantDistSq;
}

CTargetDotEvaluator::CTargetDotEvaluator()
{
	m_fMinValue = 0.0f;
	m_fMaxValue = 0.0f;
}

void CTargetDotEvaluator::SetEvaluate(CGameObject * pTarget, CGameObject * pThis, float fMin, float fMax)
{
	m_pTarget = pTarget;
	m_pThis = pThis;
	m_fMaxValue = fMax;
	m_fMinValue = fMin;
}


float CTargetDotEvaluator::Evaluate()
{
	CGameObject * pTarget = GetTarget();
	CGameObject * pThis   = GetThis();

	XMVECTOR xmvToVector  = XMLoadFloat3(&pTarget->GetPosition()) - XMLoadFloat3(&pThis->GetPosition());
	XMVECTOR xmvLook = XMLoadFloat3(
		&static_cast<CCharacter*>(pTarget)->GetLookVector());

	float fResult = 0.0f;
	XMStoreFloat(&fResult, XMVector3Dot(xmvLook, XMVector3Normalize(xmvToVector)));

	return fResult;
}

CTargetDotAndDistEvaluator::CTargetDotAndDistEvaluator()
{
	m_fMinValue = 0.0f;
	m_fMaxValue = 0.0f;
	m_fDist = 0.0f;
}

void CTargetDotAndDistEvaluator::SetEvaluate(CGameObject * pTarget, CGameObject * pThis, float fDist, float fMin, float fMax)
{
	m_pTarget = pTarget;
	m_pThis = pThis;
	m_fMaxValue = fMax;
	m_fMinValue = fMin;
	m_fDist = fDist;
}

float CTargetDotAndDistEvaluator::Evaluate()
{
	CGameObject * pTarget = GetTarget();
	CGameObject * pThis   = GetThis();

	XMVECTOR xmvToVector = XMLoadFloat3(&pTarget->GetPosition()) 
		- XMLoadFloat3(&pThis->GetPosition());
	
	float fDistanceSq = EVAL_NULL;
	float fWantDistSq = m_fDist * m_fDist;

	XMStoreFloat(&fDistanceSq, XMVector3LengthSq(xmvToVector));
	float fResult = EVAL_NULL;

	if (fDistanceSq - fWantDistSq < 0.0f)
	{
		XMVECTOR xmvLook = XMLoadFloat3(&static_cast<CCharacter*>(pTarget)->GetLookVector());
		
		XMStoreFloat(&fResult, XMVector3Dot(xmvLook, XMVector3Normalize(xmvToVector)));
	}
	return fResult;
}
