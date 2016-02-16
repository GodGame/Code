#pragma once
#ifndef __GAME_INFO
#define __GAME_INFO

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
	bool IsSlow()	  { return m_eDebuff & DEBUFF_SLOW;}
	void OnFreeze()   { m_eDebuff |= DEBUFF_FREEZE;  PlayerDelayMessage(DEBUFF_FREEZE, m_fTimeFreeze);}
	void DownFreeze() { m_eDebuff &= ~DEBUFF_FREEZE; }
	bool IsFreeze()   { return m_eDebuff & DEBUFF_FREEZE; }
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
	void OnPlusDamage() { m_eBuff |= BUFF_PLUS_DAMAGE; PlayerDelayMessage(BUFF_PLUS_DAMAGE, m_fTimePlusDamage); }
	void DownPlusDamage() { m_eBuff &= ~BUFF_PLUS_DAMAGE; }
	bool IsPlusDamage() { return m_eBuff & BUFF_PLUS_DAMAGE; }
	void OnCastUp() { m_eBuff |= BUFF_CAST; PlayerDelayMessage(BUFF_CAST, m_fTimeCast); }
	void DownCastUp() { m_eBuff &= ~BUFF_CAST; }
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

	UINT	m_uHP           : 17;
	UINT	m_uAttackDamage : 17;
	UINT    m_bCanJump      : 1;
	UINT	m_bAlive        : 1;

public:
	StatusInfo();
	~StatusInfo();

	WORD GetGold() { return m_uGold; }
	WORD GetPoint() { return m_uGamePoint; }
};

struct ElementEnergy
{
	BYTE m_nDarkness;
	BYTE m_nLight;
	BYTE m_nFire;
	BYTE m_nWind;
	BYTE m_nIce;
	BYTE m_nElectricity;
};

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