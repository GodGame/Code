#include "stdafx.h"
#include "Character.h"

void CWarrock::InitializeAnimCycleTime()
{
	SetAnimationCycleTime(CWarrock::eANI_WARROCK_IDLE, mfIdleAnim);
	SetAnimationCycleTime(CWarrock::eANI_WARROCK_RUN, mfRunAnim);
	SetAnimationCycleTime(CWarrock::eANI_WARROCK_ROAR, mfRoarAnim);
	SetAnimationCycleTime(CWarrock::eANI_WARROCK_PUNCH, mfPunchAnim);
	SetAnimationCycleTime(CWarrock::eANI_WARROCK_SWIPING, mfSwipingAnim);
	SetAnimationCycleTime(CWarrock::eANI_WARROCK_DEATH, mfDeathAnim);
}
