#include "stdafx.h"
#include "GameInfo.h"
#include "Player.h"

CDeBuff::CDeBuff()
{
	m_eDebuff			 = 0;
	m_fPlusDebuffTime	 = 0.0f;

	m_pPlayer = nullptr;
}

CDeBuff::~CDeBuff()
{
}

void CDeBuff::PlayerDelayMessage(eDeBuff eType, float fDebuffTime)
{
	if (m_pPlayer) 
	{
		m_DebuffMessageList.push_back(eType);
		void * DebuffMsg = &(*(--m_DebuffMessageList.end()));

		EVENTMgr.InsertDelayMessage(fDebuffTime, eMessage::MSG_DEBUFF_OFF, CGameEventMgr::MSG_TYPE_OBJECT,
			(void*) this, nullptr, DebuffMsg);
	}
}

void CDeBuff::DebuffPop()
{
	//eDeBuff debuff = *((eDeBuff*)msg);
	auto it = m_DebuffMessageList.begin();
	m_DebuffMessageList.pop_front();
	m_eDebuff &= ~(*it);
}

///////////////////////////////////////////////////////////////////
CBuff::CBuff()
{
	m_eBuff           = 0;
	m_fPlusDebuffTime = 0.0f;

	m_pPlayer         = nullptr;
}

CBuff::~CBuff()
{
}

void CBuff::PlayerDelayMessage(eBuff eType, float fBuffTime)
{
	if (m_pPlayer)
	{
		m_BuffMessageList.push_back(eType);
		void * DebuffMsg = &(*(--m_BuffMessageList.end()));

		EVENTMgr.InsertDelayMessage(fBuffTime, eMessage::MSG_BUFF_OFF, CGameEventMgr::MSG_TYPE_OBJECT,
			(void*) this, nullptr, DebuffMsg);
	}
}

void CBuff::BuffPop()
{
	//eDeBuff debuff = *((eDeBuff*)msg);
	auto it = m_BuffMessageList.begin();
	m_BuffMessageList.pop_front();
	m_eBuff &= ~(*it);
}

//////////////////////////////////////////////
StatusInfo::StatusInfo()
{
	m_uGamePoint		 = 0;
	m_uGold				 = 0;

	m_uHP				 = 0;
	m_uStamina			 = 0;
	m_nShields			 = 0;
	m_uAttackDamage		 = 0;
	m_bRange		     = false;
	m_bUnbeatable        = false;
	m_bCanJump			 = false;
	m_bAlive			 = false;

	m_fAttackSpeed		 = 0.0f;
	m_fAttackRange		 = 0.0f;
}

StatusInfo::~StatusInfo()
{
}

///////////////////////////////////////////
Element::Element()
{
}

Element::~Element()
{
}

