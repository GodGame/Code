#pragma once

#ifndef __EVALUATOR_H
#define __EVALUATOR_H

class CGameObject;

class CEvaluator
{
protected:
	float EVAL_NULL = -1.f;
	float EVAL_MAX  = 1.0f;

public:
	CEvaluator();
	virtual ~CEvaluator();

	virtual float Evaluate() { return EVAL_NULL; }
};

class CTargetEvaluator : public CEvaluator
{
protected:
	CGameObject * m_pTarget;
	CGameObject * m_pThis;

public:
	CTargetEvaluator();
	virtual ~CTargetEvaluator();

	virtual float Evaluate() { return EVAL_NULL; }

public:
	void SetEvaluate(CGameObject * pTarget) { m_pTarget = pTarget; }

	CGameObject * GetTarget() { return m_pTarget; }
	CGameObject * GetThis() { return m_pThis; }
};

class CDistanceEvaluator : public CTargetEvaluator
{
	float m_fWantDistance;

public:
	CDistanceEvaluator();
	virtual ~CDistanceEvaluator();

	virtual float Evaluate();

public:
	void SetEvaluate(CGameObject * pTarget) { m_pTarget = pTarget; }
	void SetEvaluate(CGameObject * pTarget, CGameObject * pThis, float fWantDistance);
	float GetWantDistance()   { return m_fWantDistance; }
};

class CTargetDotEvaluator : public CTargetEvaluator
{
	float m_fMinValue;
	float m_fMaxValue;

public:
	CTargetDotEvaluator();
	virtual ~CTargetDotEvaluator(){}

	void SetEvaluate(CGameObject * pTarget) { m_pTarget = pTarget; }
	void SetEvaluate(CGameObject * pTarget, CGameObject * pThis, float fMin = 0.0f, float fMax = 0.0f);
	virtual float Evaluate();
};

#endif