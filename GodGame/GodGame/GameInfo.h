#pragma once
#ifndef __GAME_INFO
#define __GAME_INFO

#define ELEMENT_LIGHT			COLOR_WHITE
#define ELEMENT_FIRE			COLOR_RED
#define ELEMENT_WIND			COLOR_GREEN
#define ELEMENT_ICE				COLOR_BLUE
#define ELEMENT_DARK			COLOR_BLACK
#define ELEMENT_ELECTRIC		COLOR_YELLOW
#define ELEMENT_NUM				6

class CPlayer;
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
	CPlayer * m_pPlayer;

	const float m_fTimeSlow   = 1.5f;
	const float m_fTimeFreeze = 1.5f;
	const float m_fTimeStun   = 1.5f;

public:
	CDeBuff();
	~CDeBuff();
	void GetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }

	void PlayerDelayMessage(eDeBuff eType, float fDebuffTime);
	void DebuffPop();

public:
	void OnSlow()     { m_eDebuff |= DEBUFF_SLOW; PlayerDelayMessage(DEBUFF_SLOW, m_fTimeSlow); }
	void DownSlow()   { m_eDebuff &= ~DEBUFF_SLOW; }
	bool IsSlow()	  { return DEBUFF_SLOW & m_eDebuff;}
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
	CPlayer * m_pPlayer;

	const float m_fTimePlusDamage = 1.5f;
	const float m_fTimeCast       = 1.5f;
	const float m_fTimeMove       = 1.5f;
	const float m_fTimeCritical   = 1.5f;

public:
	CBuff();
	~CBuff();
	void GetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }

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
	UINT	m_uHP           : 10;
	UINT	m_uStamina	    : 6;
	UINT	m_nShields		: 6;
	UINT    m_bRange		: 1;
	UINT    m_bUnbeatable   : 1;
	UINT    m_bCanJump      : 1;
	UINT	m_bAlive        : 1;

	float	m_fAttackSpeed;
	float	m_fAttackRange;

public:
	StatusInfo();
	~StatusInfo();

	WORD GetGold()  { return m_uGold; }
	WORD GetPoint() { return m_uGamePoint; }
	WORD GetHP()	{ return m_uHP; }
};

struct ElementEnergy
{
	BYTE m_nSum;
	BYTE m_nMinNum;
	BYTE m_nEnergies[ELEMENT_NUM];
};

ostream& operator<<(ostream& os, ElementEnergy & element);

class Element
{
public:
	Element();
	virtual ~Element();
};

class EFire : public Element
{
public:
	EFire(){}
	virtual ~EFire(){}
};

class EIce : public Element
{
public:
	EIce() {}
	virtual ~EIce(){}
};

#endif