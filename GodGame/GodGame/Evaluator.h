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

	float Evaluate(CGameObject * pThis, CGameObject * pTarget) { return EVAL_NULL; }
};

class CDistanceEvaluator : public CEvaluator
{
	float m_fWantDistance;

public:
	CDistanceEvaluator();
	virtual ~CDistanceEvaluator();

	virtual float Evaluate(CGameObject * pThis, CGameObject * pTarget);

public:
	void SetEvaluate(float fWantDistance);
	float GetWantDistance()   { return m_fWantDistance; }
};

class CTargetDotEvaluator : public CEvaluator
{
	float m_fMinValue;
	float m_fMaxValue;

public:
	CTargetDotEvaluator();
	virtual ~CTargetDotEvaluator(){}

	void SetEvaluate(float fMin = 0.0f, float fMax = 0.0f);
	virtual float Evaluate(CGameObject * pThis, CGameObject * pTarget);
};

class CTargetDotAndDistEvaluator : public CEvaluator
{
	float m_fMinValue;
	float m_fMaxValue;
	float m_fDist;

public:
	CTargetDotAndDistEvaluator();
	virtual ~CTargetDotAndDistEvaluator() {}

	void SetEvaluate(float fDist, float fMin = 0.0f, float fMax = 0.0f);
	virtual float Evaluate(CGameObject * pThis, CGameObject * pTarget);
};

#endif