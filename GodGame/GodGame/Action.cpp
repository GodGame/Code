#include "stdafx.h"
#include "Action.h"


CActionNode::CActionNode()
{
	//m_pNextAction = nullptr;
}

CActionNode::~CActionNode()
{
	//if (m_pNextAction) delete m_pNextAction;
}

void CActionNode::Execute(float fFrameTime, float fEvalValue)
{
	//if (m_pNextAction) m_pNextAction->Execute(fFrameTime, fEvalValue);
}

CFrontAttack::CFrontAttack()
{
}

CFrontAttack::~CFrontAttack()
{
}

void CFrontAttack::Execute(float fFrameTime, float fEvalValue)
{
}

CChangeAnimation::CChangeAnimation()
{
}

CChangeAnimation::~CChangeAnimation()
{
}

void CChangeAnimation::Execute(float fFrameTime, float fEvalValue)
{
	//if (fEvalValue < 0.3f)
	//	ChangeAnimationState(eANI_WARROCK_PUNCH, true, nullptr, 0);

	//else if (fEvalValue < 1.0f)
	//	ChangeAnimationState(eANI_WARROCK_SWIPING, true, nullptr, 0);

	//else if (fEvalValue < 1.5f)
	//	ChangeAnimationState(eANI_WARROCK_ROAR, true, nullptr, 0);
}

CFrontSwipping::CFrontSwipping()
{
}

CFrontSwipping::~CFrontSwipping()
{
}

void CFrontSwipping::Execute(float fFrameTime, float fEvalValue)
{
}
