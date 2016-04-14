#pragma once

#ifndef __SYSTEM_H
#define __SYSTEM_H
#define TOTAL_PLAYER 8

struct PLAYER_DATA_INFO
{
	UINT m_iPlayerNum   : 4;
	UINT m_iPlayerPoint : 12;
	UINT m_nDeathCount  : 6;
	UINT m_nKillCount   : 6;
};

class CGameObject;
class CSystemManager
{
private:
	UINT m_iMapInfo				: 4;
	UINT m_iRoundNumber         : 4;
	UINT m_iTotalRound          : 4;
	UINT m_nPlayers             : 4;
	UINT m_iOccupationPlayerNum : 4;

	PLAYER_DATA_INFO mPlayerInfo[TOTAL_PLAYER];
	//UINT m_i;
	float m_fRoundTime;

//public:
	const float mfLIMIT_ROUND_TIME = 3600.f;
	const float mfDEATH_MATCH_TIME = 1800.f;

	float m_fWaterHeight;

private:
	CGameObject * m_pPortalGate;
	CGameObject * m_pMinimMap;

public:
	CGameObject* GetPortalGate() { return m_pPortalGate; }
	CGameObject* GetMiniMap()    { return m_pMinimMap; }

private:
	CSystemManager();
	~CSystemManager();

public:
	static CSystemManager & GetInstance()
	{
		static CSystemManager instance;
		return instance;
	}
	void Build() {}

};

#endif