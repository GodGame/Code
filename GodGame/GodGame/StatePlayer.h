#pragma once

#ifndef __PLAYER_STATE_H
#define __PLAYER_STATE_H

#include "AI.h"

class CInGamePlayer;

class CPlayerIdleState : public CAIState<CInGamePlayer>
{
private:
	//const float mfATTACK_RAGNE = 50.0f;

	//CPlayerIdleState mEvaluator;

private:
	CPlayerIdleState() : CAIState<CInGamePlayer>() {}
	virtual ~CPlayerIdleState() {}
	CPlayerIdleState& operator=(const CPlayerIdleState&);

public:
	static CPlayerIdleState & GetInstance();
	virtual void Enter(CInGamePlayer * pPlayer);
	virtual void Execute(CInGamePlayer * pPlayer, float fFrameTime);
	virtual void Exit(CInGamePlayer * pPlayer);
};

class CPlayerCastState : public CAIState<CInGamePlayer>
{
private:
	//const float mfATTACK_RAGNE = 50.0f;

	//CPlayerIdleState mEvaluator;

private:
	CPlayerCastState() : CAIState<CInGamePlayer>() {}
	virtual ~CPlayerCastState() {}
	CPlayerCastState& operator=(const CPlayerCastState&);

public:
	static CPlayerCastState & GetInstance();
	virtual void Enter(CInGamePlayer * pPlayer);
	virtual void Execute(CInGamePlayer * pPlayer, float fFrameTime);
	virtual void Exit(CInGamePlayer * pPlayer);
};

class CPlayerUseMagicState : public CAIState<CInGamePlayer>
{
private:
	//const float mfATTACK_RAGNE = 50.0f;

	//CPlayerIdleState mEvaluator;

private:
	CPlayerUseMagicState() : CAIState<CInGamePlayer>() {}
	virtual ~CPlayerUseMagicState() {}
	CPlayerUseMagicState& operator=(const CPlayerUseMagicState&);

public:
	static CPlayerUseMagicState & GetInstance();
	virtual void Enter(CInGamePlayer * pPlayer);
	virtual void Execute(CInGamePlayer * pPlayer, float fFrameTime);
	virtual void Exit(CInGamePlayer * pPlayer);
};

class CPlayerDamagedState : public CAIState<CInGamePlayer>
{
private:

private:
	CPlayerDamagedState() : CAIState<CInGamePlayer>() {}
	virtual ~CPlayerDamagedState() {}
	CPlayerDamagedState& operator=(const CPlayerDamagedState&);

public:
	static CPlayerDamagedState & GetInstance();
	virtual void Enter(CInGamePlayer * pPlayer);
	virtual void Execute(CInGamePlayer * pPlayer, float fFrameTime);
	virtual void Exit(CInGamePlayer * pPlayer);
};

class CPlayerKnockbackState : public CAIState<CInGamePlayer>
{
private:
	const float mfKnockBackLength = 100.0f;

private:
	CPlayerKnockbackState() : CAIState<CInGamePlayer>() {}
	virtual ~CPlayerKnockbackState() {}
	CPlayerKnockbackState& operator=(const CPlayerKnockbackState&);

public:
	static CPlayerKnockbackState & GetInstance();
	virtual void Enter(CInGamePlayer * pPlayer);
	virtual void Execute(CInGamePlayer * pPlayer, float fFrameTime);
	virtual void Exit(CInGamePlayer * pPlayer);
};

class CPlayerDeathState : public CAIState<CInGamePlayer>
{
private:
	//const float mfATTACK_RAGNE = 50.0f;

	//CPlayerIdleState mEvaluator;

private:
	CPlayerDeathState() : CAIState<CInGamePlayer>() { m_bCanChangeState = false; }
	virtual ~CPlayerDeathState() {}
	CPlayerDeathState& operator=(const CPlayerDamagedState&);

public:
	static CPlayerDeathState & GetInstance();
	virtual void Enter(CInGamePlayer * pPlayer);
	virtual void Execute(CInGamePlayer * pPlayer, float fFrameTime);
	virtual void Exit(CInGamePlayer * pPlayer);
};
#endif