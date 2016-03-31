#pragma once

#ifndef __CHARACTER
#define __CHARACTER

#include "Object.h"

class CCharacter : public CAnimatedObject
{
public:
	CCharacter(int nMeshes) : CAnimatedObject(nMeshes) {}
	virtual ~CCharacter(){}

	virtual void InitializeAnimCycleTime(){}
};

class CMonster : public CCharacter
{
public:
	CMonster(int nMeshes) : CCharacter(nMeshes){}
	virtual ~CMonster() {}
};

class CSkeleton : public CCharacter
{
public:
	CSkeleton(int nMeshes) : CCharacter(nMeshes) {}
	virtual ~CSkeleton() {}
};

class CWarrock : public CCharacter
{
private:
	const float mfIdleAnim = 1.0f;
	const float mfRunAnim = 1.0f;
	const float mfRoarAnim = 3.0f;
	const float mfPunchAnim = 1.0f;
	const float mfSwipingAnim = 2.0f;
	const float mfDeathAnim = 2.0f;

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

public:
	CWarrock(int nMeshes) : CCharacter(nMeshes) {}
	virtual ~CWarrock() {}

	virtual void InitializeAnimCycleTime();
};


#endif