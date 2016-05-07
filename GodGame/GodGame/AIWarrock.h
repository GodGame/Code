#pragma once

#ifndef __WARROCK_STATE_H
#define __WARROCK_STATE_H

#include "AI.h"

class CWarrock;

class CWarrockIdleState : public CAIState<CWarrock>
{
private:
	const float mfATTACK_RAGNE = 60.0f;

	CDistanceEvaluator mEvaluator;

private:
	CWarrockIdleState() : CAIState<CWarrock>() {}
	virtual ~CWarrockIdleState(){}
	CWarrockIdleState& operator=(const CWarrockIdleState&);

public:
	static CWarrockIdleState & GetInstance();
	virtual void Enter(CWarrock * pWarrock);
	virtual void Execute(CWarrock * pWarrock, float fFrameTime);
	virtual void Exit(CWarrock * pWarrock);
};

class CWarrockPunchState : public CAIState<CWarrock>
{
private:
	const float m_fDelay = 0.8f;

	const float mfMAX_RAGNE = 35.0f;
	CTargetDotEvaluator mEvaluator;


private:
	CWarrockPunchState() : CAIState<CWarrock>(){}
	virtual ~CWarrockPunchState() {}
	CWarrockPunchState& operator=(const CWarrockPunchState&);

public:
	static CWarrockPunchState & GetInstance();
	virtual void Enter(CWarrock * pWarrock);
	virtual void Execute(CWarrock * pWarrock, float fFrameTime);
	virtual void Exit(CWarrock * pWarrock);
};

class CWarrockSwipingState : public CAIState<CWarrock>
{
private:
	const float m_fDelay = 1.2f;

	const float mfMAX_RAGNE = 50.0f;
	CTargetDotAndDistEvaluator mEvaluator;

private:
	CWarrockSwipingState() : CAIState<CWarrock>() {}
	virtual ~CWarrockSwipingState() {}
	CWarrockSwipingState& operator=(const CWarrockSwipingState&);

public:
	static CWarrockSwipingState & GetInstance();
	virtual void Enter(CWarrock * pWarrock);
	virtual void Execute(CWarrock * pWarrock, float fFrameTime);
	virtual void Exit(CWarrock * pWarrock);
};

class CWarrockRoarState : public CAIState<CWarrock>
{
private:
	//CWarrockRoarState mEvaluator;

private:
	CWarrockRoarState() : CAIState<CWarrock>() {}
	virtual ~CWarrockRoarState() {}
	CWarrockRoarState& operator=(const CWarrockRoarState&);

public:
	static CWarrockRoarState & GetInstance();
	virtual void Enter(CWarrock * pWarrock);
	virtual void Execute(CWarrock * pWarrock, float fFrameTime);
	virtual void Exit(CWarrock * pWarrock);
};


class CWarrockRotateState : public CAIState<CWarrock>
{
private:
	const float mfRotateUnitPerSecond = 20.f;
	CTargetDotEvaluator mEvaluator;


private:
	CWarrockRotateState() : CAIState<CWarrock>() {}
	virtual ~CWarrockRotateState() {}
	CWarrockRotateState& operator=(const CWarrockPunchState&);

public:
	static CWarrockRotateState & GetInstance();
	virtual void Enter(CWarrock * pWarrock);
	virtual void Execute(CWarrock * pWarrock, float fFrameTime);
	virtual void Exit(CWarrock * pWarrock);
};


class CWarrockChaseTargetState : public CAIState<CWarrock>
{
private:
	const float mfCHASE_RAGNE = 100.0f;

	CDistanceEvaluator mEvaluator;

private:
	CWarrockChaseTargetState() : CAIState<CWarrock>() {}
	virtual ~CWarrockChaseTargetState() {}
	CWarrockChaseTargetState& operator=(const CWarrockChaseTargetState&);

public:
	static CWarrockChaseTargetState & GetInstance();
	virtual void Enter(CWarrock * pWarrock);
	virtual void Execute(CWarrock * pWarrock, float fFrameTime);
	virtual void Exit(CWarrock * pWarrock);
};

class CWarrockDelayState : public CAIState<CWarrock>
{
private:
	const float mfCHASE_RAGNE = 100.0f;

	CDistanceEvaluator mEvaluator;

private:
	CWarrockDelayState() : CAIState<CWarrock>() {}
	virtual ~CWarrockDelayState() {}
	CWarrockDelayState& operator=(const CWarrockDelayState&);

public:
	static CWarrockDelayState & GetInstance();
	virtual void Enter(CWarrock * pWarrock);
	virtual void Execute(CWarrock * pWarrock, float fFrameTime);
	virtual void Exit(CWarrock * pWarrock);
};

class CWarrockDeathState : public CAIState<CWarrock>
{
private:
	//const float mfCHASE_RAGNE = 100.0f;

	//CDistanceEvaluator mEvaluator;

private:
	CWarrockDeathState() : CAIState<CWarrock>() { m_bCanChangeState = false; }
	virtual ~CWarrockDeathState() {}
	CWarrockDeathState& operator=(const CWarrockDeathState&);

public:
	static CWarrockDeathState & GetInstance();
	virtual void Enter(CWarrock * pWarrock);
	virtual void Execute(CWarrock * pWarrock, float fFrameTime);
	virtual void Exit(CWarrock * pWarrock);
};

#endif