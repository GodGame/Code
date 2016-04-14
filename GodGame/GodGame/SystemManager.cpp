#include "stdafx.h"
#include "SystemManager.h"


CSystemManager::CSystemManager()
{
	m_iMapInfo             = 0;
	m_iRoundNumber         = 0;
	m_iTotalRound          = 0;
	m_nPlayers             = 0;
	m_iOccupationPlayerNum = 0;

	ZeroMemory(&mPlayerInfo, sizeof(PLAYER_DATA_INFO) * TOTAL_PLAYER);

	m_pPortalGate = nullptr;
	m_pMinimMap   = nullptr;

	m_fWaterHeight = 0.f;
}


CSystemManager::~CSystemManager()
{
}
