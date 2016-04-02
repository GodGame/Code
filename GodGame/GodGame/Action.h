#pragma once

#ifndef __ACTION_H
#define __ACTION_H

class CActionNode
{
private:
	//CActionNode * m_pNextAction;

public:
	CActionNode();
	virtual ~CActionNode();
	virtual void Execute(float fFrameTime, float fEvalValue);

public:
	//void SetNextNode(CActionNode * pNextNode) { m_pNextAction = pNextNode; }

};

class CFrontAttack
{
	float m_fSize;

public:
	CFrontAttack();
	virtual ~CFrontAttack();
	virtual void Execute(float fFrameTime, float fEvalValue);

};

class CFrontSwipping
{
public:
	CFrontSwipping();
	virtual ~CFrontSwipping();
	virtual void Execute(float fFrameTime, float fEvalValue);

};

class CChangeAnimation
{
public:
	CChangeAnimation();
	virtual ~CChangeAnimation();
	virtual void Execute(float fFrameTime, float fEvalValue);

};

#endif