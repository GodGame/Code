#pragma once
#ifndef __GAME_INFO
#define __GAME_INFO

#define ELEMENT_NULL			-1
#define ELEMENT_LIGHT			COLOR_WHITE
#define ELEMENT_FIRE			COLOR_RED
#define ELEMENT_WIND			COLOR_GREEN
#define ELEMENT_ICE				COLOR_BLUE
#define ELEMENT_DARK			COLOR_BLACK
#define ELEMENT_ELECTRIC		COLOR_YELLOW
#define ELEMENT_NUM				6


class CCharacter;
class CDeBuff
{
	enum eDeBuff : BYTE
	{
		DEBUFF_NORMAL      = 0,
		DEBUFF_SLOW        = (1 << 0),
		DEBUFF_FREEZE      = (1 << 1),
		DEBUFF_STUN        = (1 << 2),
		DEBUFF_GLARED      = (1 << 3),
		DEBUFF_DOT_DAMAGED = (1 << 4),	// Damage Over Time
		DEBUFF_ADD_DAMAGED = (1 << 5), // 추가 데미지
		DEBUFF_DOWN_DAMAGE = (1 << 6)
	};

	BYTE	 m_eDebuff;

private:
	list<eDeBuff> m_DebuffMessageList;

	float	 m_fPlusDebuffTime;
	CCharacter * m_pMaster;

	const float m_fTimeSlow   = 1.5f;
	const float m_fTimeFreeze = 1.5f;
	const float m_fTimeStun   = 1.5f;


public:
	CDeBuff();
	~CDeBuff();
	void Reset();
	void SetPlayer(CCharacter* pPlayer) { m_pMaster = pPlayer; }

	void PlayerDelayMessage(eDeBuff eType, float fDebuffTime);
	void DebuffPop();

public:
	void OnSlow()     { m_eDebuff |= DEBUFF_SLOW; PlayerDelayMessage(DEBUFF_SLOW, m_fTimeSlow); }
	void DownSlow()   { m_eDebuff &= ~DEBUFF_SLOW; }
	bool IsSlow()	  { return DEBUFF_SLOW & m_eDebuff; }
	void OnFreeze()   { m_eDebuff |= DEBUFF_FREEZE;  PlayerDelayMessage(DEBUFF_FREEZE, m_fTimeFreeze);}
	void DownFreeze() { m_eDebuff &= ~DEBUFF_FREEZE; }
	bool IsFreeze()   { return DEBUFF_FREEZE & m_eDebuff; }
};

class CBuff
{
	enum eBuff : BYTE
	{
		BUFF_NORMAL      = 0,
		BUFF_UNBEATABLE  = (1 << 0),
		BUFF_PLUS_DAMAGE = (1 << 1),
		BUFF_CAST        = (1 << 2),
		BUFF_MOVE        = (1 << 3),
		BUFF_CRITICAL    = (1 << 4)
	};

	BYTE	 m_eBuff;

private:
	list<eBuff> m_BuffMessageList;

	float	 m_fPlusDebuffTime;
	CCharacter * m_pMaster;

	const float m_fTimePlusDamage = 1.5f;
	const float m_fTimeCast       = 1.5f;
	const float m_fTimeMove       = 1.5f;
	const float m_fTimeCritical   = 1.5f;

public:
	CBuff();
	~CBuff();
	void Reset();
	void SetMaster(CCharacter* pMaster) { m_pMaster = pMaster; }

	void PlayerDelayMessage(eBuff eType, float fDebuffTime);
	void BuffPop();

public:
	void OnPlusDamage()   { m_eBuff |= BUFF_PLUS_DAMAGE; PlayerDelayMessage(BUFF_PLUS_DAMAGE, m_fTimePlusDamage); }
	void DownPlusDamage() { m_eBuff &= ~BUFF_PLUS_DAMAGE; }
	bool IsPlusDamage()   { return BUFF_PLUS_DAMAGE & m_eBuff; }
	void OnCastUp()       { m_eBuff |= BUFF_CAST; PlayerDelayMessage(BUFF_CAST, m_fTimeCast); }
	void DownCastUp()     { m_eBuff &= ~BUFF_CAST; }
	bool IsCastUp()       { return BUFF_PLUS_DAMAGE & m_eBuff; }
};

class StatusInfo
{
private:
	CDeBuff	 m_Debuff;
	CBuff	 m_Buff;

public:
	CDeBuff & GetDebuffMgr() { return m_Debuff; }
	CBuff   & GetBuffMgr()   { return m_Buff; }

private:
	UINT	m_uGamePoint    : 18;
	UINT	m_uGold         : 18;

	UINT	m_uAttackDamage : 10;
	UINT	m_uStamina	    : 6;
	UINT	m_nShields		: 6;

	UINT    m_bRange		: 1;
	UINT    m_bUnbeatable   : 1;
	UINT    m_bCanJump      : 1;
	UINT    m_bCanMove      : 1;
	UINT	m_bAlive        : 1;

	short	m_sHP;
	float	m_fAttackSpeed;
	float	m_fAttackRange;
	//float   m_fAttackDamage;

public:
	StatusInfo();
	~StatusInfo();

	void ResetStatus();
	void RoundReady();
	void RoundStart();

	WORD GetGold()  { return m_uGold; }
	WORD GetPoint() { return m_uGamePoint; }

	short GetHP()	{ return m_sHP; }
	void SetHP(short hp) { m_sHP = hp; }
	void ChangeHP(short hpValue) { m_sHP += hpValue; }
	void Damaged(short damage) { ChangeHP(-damage); m_bCanMove = false; }

	float GetAttackSpeed()          { return m_fAttackSpeed; }
	void SetAttackSpeed(float fVal) { m_fAttackSpeed = fVal; }
	float GetAttackRange()          { return m_fAttackRange; }
	void SetAttackRange(float fVal) { m_fAttackRange = fVal; }
//	float GetAttackDamage()			{ return m_fAttackDamage; }


	void SetUnbeatable(bool bVal) { m_bUnbeatable = bVal; }
	bool IsUnbeatable() { return m_bUnbeatable; }

	bool IsCanMove()          { return m_bCanMove; }
	void SetCanMove(bool val) { m_bCanMove = val; }

	bool IsAlive()            { return m_bAlive; }
	void SetAlive(bool val)  { m_bAlive = val; }

	bool IsCanDamaged() { return (m_bAlive && !m_bUnbeatable); }
};

typedef BYTE ELEMENT;
struct ElementEnergy
{
	BYTE m_nSum;
	BYTE m_nMinNum;
	ELEMENT m_nEnergies[ELEMENT_NUM];
};

ostream& operator<<(ostream& os, ElementEnergy & element);

#endif