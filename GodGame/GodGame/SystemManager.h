#pragma once

#ifndef __SYSTEM_H
#define __SYSTEM_H

#define TOTAL_PLAYER 6

#define PLAYER_00_COLOR "WhiteLight"
#define PLAYER_01_COLOR "RedLight"
#define PLAYER_02_COLOR "GreenLight"
#define PLAYER_03_COLOR "BlueLight"
//#define PLAYER_04_COLOR "WhiteLight"
//#define PLAYER_05_COLOR "WhiteLight"

class CFontUIObject
{
public:
	virtual void DrawFont() = 0;
};

class CGlobalFontUI : public CFontUIObject
{
public:
	virtual void DrawFont();
};

class CGameStartFontUI : public CFontUIObject
{
public:
	virtual void DrawFont();
};

class CRoundEnterFontUI: public CFontUIObject
{
public:
	virtual void DrawFont();
};

class CRoundStartFontUI : public CFontUIObject
{
public:
	virtual void DrawFont();
};

class CRoundDominateFontUI : public CFontUIObject
{
public:
	virtual void DrawFont();
};

class CRoundDeathMatchFontUI : public CFontUIObject
{
public:
	virtual void DrawFont();
};

class CRoundEndFontUI : public CFontUIObject
{
public:
	virtual void DrawFont();
};

class CRoundClearFontUI : public CFontUIObject
{
public:
	virtual void DrawFont();
};

class CGameEndFontUI : public CFontUIObject
{
public:
	virtual void DrawFont();
};

struct PLAYER_DATA_INFO
{
	UINT m_iPlayerNum   : 4;
	UINT m_iPlayerPoint : 12;
	UINT m_nDeathCount  : 6;
	UINT m_nKillCount   : 6;
};

class CScene;
class CGameObject;
class CPortalGate;
class CInGamePlayer;
class CMaterial;
class CSystemManager
{
public:
	enum ROUND_STATE : CHAR
	{
		eROUND_NONE = -1,
		eGAME_START,
		eROUND_ENTER,
		eROUND_START,
		eROUND_DOMINATE,
		eROUND_DEATH_MATCH,
		eROUND_END,
		eROUND_CLEAR,
		eGAME_END,
		eROUND_STATE_NUM
	};

private:
	//void(*m_pFontFunc) ();
	CFontUIObject * m_pGlobalFont;
	CFontUIObject * m_pFont[eROUND_STATE_NUM];
	ROUND_STATE mRoundState;

	UINT m_iMapInfo				: 4;
	UINT m_iRoundNumber         : 4;
	UINT m_iTotalRound          : 4;
	UINT m_nPlayers             : 4;
	UINT m_iThisPlayer			: 4;
	UINT m_iDominatingPlayerNum : 4;

	PLAYER_DATA_INFO mPlayerInfo[TOTAL_PLAYER];
	float mPlayersDominateTryTime[TOTAL_PLAYER];
	float mPlayersDominatingTime[TOTAL_PLAYER];
	
	float m_fRoundTime;
	UINT m_nRoundMinute : 8;
	UINT m_nRoundSecond : 8;

	float m_fEnterTime;
	const float mfENTER_TIME = 5.5f;
	float m_fEndTime;
	const float mfEND_TIME = 5.f;

//public:
	const float mfCAN_DOMINATE_LENGTH = 80.f;
	const float mfDOMINATE_SPEND_TIME = 2.f;

	float m_fWaterHeight;

private:
	CScene * m_pNowScene;
	CPortalGate* m_pPortalGate;
	CGameObject* m_pMinimMap;
	
	vector<CMaterial*> m_vcPlayerColorMaterial;

	XMFLOAT3 m_xv3PortalZonePos;

	void _CreateFontUiArray();

public:
	ROUND_STATE GetRoundState() { return mRoundState; }

	void SetInitialPlayerInfo(int nPlayers, int nPlayerNum) { m_nPlayers = nPlayers; m_iThisPlayer = nPlayerNum; }
	void SetPlayerNum(int iPlayerNum) { m_iThisPlayer = iPlayerNum; }

	int GetTotalPlayerNum() { return m_nPlayers; }
	int GetPlayerNum() { return m_iThisPlayer; }

	void SetScene(CScene * pScene) { m_pNowScene = pScene; }
	void ReleaseScene(void);

	CPortalGate* GetPortalGate() { return m_pPortalGate; }
	CGameObject* GetPortalZoneObject();
	CGameObject* GetMiniMap()    { return m_pMinimMap; }

	XMFLOAT3 & GetPortalZonePos(); 
	float GetDominateSpendTime() { return mfDOMINATE_SPEND_TIME;}
	float GetRoundTime()  { return m_fRoundTime; }
	UINT GetRoundNumber() { return m_iRoundNumber; }
	UINT GetRoundMinute() { return m_nRoundMinute; }
	UINT GetRoundSecond() { return m_nRoundSecond; }

private:
	CSystemManager();
	~CSystemManager();

public:
	const float mfLIMIT_ROUND_TIME = 40.f;
	const float mfDEATH_MATCH_TIME = 1800.f;
	const int mfGOAL_ROUND = 2;

	static CSystemManager & GetInstance()
	{
		static CSystemManager instance;
		return instance;
	}

	bool CheckCanDominateRange(CInGamePlayer * pPlayer);
	bool CheckCanDomianteSuccess(CInGamePlayer * pPlayer);

	bool IsWinPlayer(CInGamePlayer * pPlayer = nullptr);

	UINT GetDominatePlayerNum() {return m_iDominatingPlayerNum;}
	void DominatePortalGate(int iPlayerNum);

public:
	PLAYER_DATA_INFO * GetPlayerInfo() { return mPlayerInfo; }

	void DrawSystemFont() { m_pGlobalFont->DrawFont(); m_pFont[mRoundState]->DrawFont(); /*m_pFontFunc(); */ }
	void Build(ID3D11Device * pd3dDevice);
	void Update(float fFrameTime);

	void GameStart();
	void RoundEnter();
	void RoundStart();
	void RoundEnd();
	void RoundClear();
	void GameEnd();
};
#define SYSTEMMgr CSystemManager::GetInstance()

#endif