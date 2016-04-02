#pragma once

#ifndef __AI_STATE_H
#define __AI_STATE_H

#include "Evaluator.h"
#include "Action.h"

class CGameObject;

template <class obj>
class CAIState
{
private:

public:
	CAIState() {}
	virtual ~CAIState() {};

	virtual void Enter(obj*) = 0;
	virtual void Execute(obj*, float) = 0;
	virtual void Exit(obj*) = 0;
};


template <class entity_type>
class CStateMachine
{
private:

	entity_type*          m_pOwner;

	CAIState<entity_type>*   m_pCurrentState;
	CAIState<entity_type>*   m_pPreviousState;
	CAIState<entity_type>*   m_pGlobalState;

public:
	CStateMachine(entity_type* owner) :m_pOwner(owner),
		m_pCurrentState(nullptr),
		m_pPreviousState(nullptr),
		m_pGlobalState(nullptr)
	{}

	virtual ~CStateMachine() {}

	//use these methods to initialize the FSM
	void SetCurrentState(CAIState<entity_type>* s) { m_pCurrentState = s; m_pCurrentState->Enter(m_pOwner);}
	void SetGlobalState(CAIState<entity_type>* s)   { m_pGlobalState = s; m_pGlobalState->Enter(m_pOwner);}
	void SetPreviousState(CAIState<entity_type>* s) { m_pPreviousState = s; m_pPreviousState->Enter(m_pOwner);}

	//call this to update the FSM
	void  Update(float fFrameTime) const
	{
		if (m_pGlobalState)  m_pGlobalState->Execute(m_pOwner, fFrameTime);
		if (m_pCurrentState) m_pCurrentState->Execute(m_pOwner, fFrameTime);
	}

	//change to a new state
	void  ChangeState(CAIState<entity_type>* pNewState)
	{
		assert(pNewState && "<StateMachine::ChangeState>:trying to assign null state to current");

		m_pPreviousState = m_pCurrentState;
		m_pCurrentState->Exit(m_pOwner);
		m_pCurrentState = pNewState;
		m_pCurrentState->Enter(m_pOwner);
	}

	void  RevertToPreviousState()
	{
		ChangeState(m_pPreviousState);
	}

	bool  isInState(const CAIState<entity_type>& st)const
	{
		if (typeid(*m_pCurrentState) == typeid(st)) return true;
		return false;
	}

	CAIState<entity_type>*  CurrentState()  const { return m_pCurrentState; }
	CAIState<entity_type>*  GlobalState()   const { return m_pGlobalState; }
	CAIState<entity_type>*  PreviousState() const { return m_pPreviousState; }
};



#endif