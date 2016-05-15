#include "stdafx.h"
#include "SystemManager.h"
#include "SystemObject.h"
//#include "SceneInGame.h"
#include "GameFramework.h"
#include "Protocol.h"

CSystemManager::CSystemManager()
{
	_CreateFontUiArray();

	m_ppPlayerArray = nullptr;

	mRoundState			   = eROUND_NONE;
	m_iMapInfo             = 0;
	m_iRoundNumber         = 0;
	m_iTotalRound          = 0;
	m_nPlayers             = 0;
	m_iThisPlayer		   = 0;
	m_iDominatingPlayerNum = -1;

	m_nRoundMinute = 0;
	m_nRoundSecond = 0;

	ZeroMemory(&mPlayerInfo, sizeof(PLAYER_DATA_INFO)  * TOTAL_PLAYER);
	ZeroMemory(&mPlayersDominateTryTime, sizeof(float) * TOTAL_PLAYER);
	ZeroMemory(&mPlayersDominatingTime, sizeof(float)  * TOTAL_PLAYER);

	m_pPortalGate = nullptr;
	m_pMinimMap   = nullptr;

	m_fWaterHeight = 0.f;
	m_iRoundNumber = 0;

	m_vcPlayerColorMaterial.reserve(TOTAL_PLAYER);
}

CSystemManager::~CSystemManager()
{
	if (m_pPortalGate) m_pPortalGate->Release();
	if (m_pMinimMap) m_pMinimMap->Release();

	for (auto pFont : m_pFont)
		delete pFont;
}

void CSystemManager::_CreateFontUiArray()
{
	m_pGlobalFont = new CGlobalFontUI();
	m_pFont[ROUND_STATE::eGAME_READY]        = new CGameReadyFontUI();
	m_pFont[ROUND_STATE::eGAME_START]        = new CGameStartFontUI();
	m_pFont[ROUND_STATE::eROUND_ENTER]       = new CRoundEnterFontUI();
	m_pFont[ROUND_STATE::eROUND_START]       = new CRoundStartFontUI();
	m_pFont[ROUND_STATE::eROUND_DOMINATE]    = new CRoundDominateFontUI();
	m_pFont[ROUND_STATE::eROUND_DEATH_MATCH] = new CRoundDeathMatchFontUI();
	m_pFont[ROUND_STATE::eROUND_END]         = new CRoundEndFontUI();
	m_pFont[ROUND_STATE::eROUND_CLEAR]       = new CRoundClearFontUI();
	m_pFont[ROUND_STATE::eGAME_END]          = new CGameEndFontUI();
}

void CSystemManager::ReleaseScene()
{
	m_pNowScene = nullptr;
	m_pPortalGate->SetMesh(nullptr);
}

CGameObject * CSystemManager::GetPortalZoneObject()
{
	return m_pPortalGate->GetChildObject();
}

XMFLOAT3 & CSystemManager::GetPortalZonePos()
{
	return m_xv3PortalZonePos = GetPortalZoneObject()->GetPosition();
}

void CSystemManager::Build(ID3D11Device * pd3dDevice)
{
	m_vcPlayerColorMaterial.push_back(MaterialMgr.GetObjects(PLAYER_00_COLOR));
	m_vcPlayerColorMaterial.push_back(MaterialMgr.GetObjects(PLAYER_01_COLOR));
	m_vcPlayerColorMaterial.push_back(MaterialMgr.GetObjects(PLAYER_02_COLOR));
	m_vcPlayerColorMaterial.push_back(MaterialMgr.GetObjects(PLAYER_03_COLOR));

	m_pPortalGate = new CPortalGate(1);
	m_pPortalGate->SetInheritAutoRender(false);
	m_pPortalGate->AddRef();

	CTexture * pTexture = new CTexture(1, 1);
	pTexture->SetTexture(0, TXMgr.GetShaderResourceView("srv_portal_zone_01"));
	pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));

	CGameObject * pObject = new CGameObject(1);
	pObject->SetMesh(new CDoublePlaneMesh(pd3dDevice, 28, 28));
	//pObject->SetMaterial(MaterialMgr.GetObjects("WhiteLight")); 
	pObject->SetTexture(pTexture);
	pObject->SetInheritAutoRender(false);
	pObject->SetActive(false);
	pObject->AddRef();

	m_pPortalGate->SetChild(pObject);
}

void CSystemManager::Update(float fFrameTime)
{
	if (mRoundState == eGAME_READY) return;

	m_fRoundTime -= fFrameTime;

	//m_nRoundSecond = m_fRoundTime;
	m_nRoundSecond = CLIENT.GetRoundTime();
	m_nRoundMinute = m_nRoundSecond / 60;
	m_nRoundSecond = m_nRoundSecond % 60;
}

bool CSystemManager::CheckCanDominateRange(CInGamePlayer * pPlayer)
{
	XMVECTOR vecPlayerPos = XMLoadFloat3(&pPlayer->GetPosition());
	XMVECTOR vecGatePos   = XMLoadFloat3(&m_pPortalGate->GetPosition());
	const float fGoalDistance = mfCAN_DOMINATE_LENGTH * mfCAN_DOMINATE_LENGTH;

	float fLengthSq = 0.f; 
	XMStoreFloat(&fLengthSq, XMVector3LengthSq(vecGatePos - vecPlayerPos));
	//cout << "거리는 " << fLengthSq;
	if (fLengthSq <= fGoalDistance)
	{
		mPlayersDominateTryTime[pPlayer->GetPlayerNum()] = m_fRoundTime + 0.1f;
		return true;
	}
	return false;
}

bool CSystemManager::CheckCanDomianteSuccess(CInGamePlayer * pPlayer)
{
	const int iPlayerNum = pPlayer->GetPlayerNum();

	bool bResult = (m_fRoundTime < 
		(mPlayersDominateTryTime[iPlayerNum] + mfDOMINATE_SPEND_TIME));

	if (bResult)
	{
		cs_packet_dominate dominate_packet;
		dominate_packet.size = sizeof(cs_packet_dominate);
		dominate_packet.type = CS_DOMINATE;
		CLIENT.GetWSASendBuffer().buf = reinterpret_cast<CHAR*>(CLIENT.GetUSendBuffer());
		CLIENT.GetWSASendBuffer().len = sizeof(cs_packet_dominate);
		memcpy(CLIENT.GetUSendBuffer(), reinterpret_cast<UCHAR*>(&dominate_packet), sizeof(cs_packet_dominate));
		DWORD ioBytes;
		int ret = WSASend(CLIENT.GetClientSocket(), &CLIENT.GetWSASendBuffer(), 1, &ioBytes, 0, NULL, NULL);
		if (ret)
		{
			int error_code = WSAGetLastError();
			if (WSA_IO_PENDING != error_code)
			{
				CLIENT.error_display(__FUNCTION__ " SC_PUT_PLAYER:WSASend", error_code);
			}
		}
		//CLIENT.SendPacket(reinterpret_cast<unsigned char*>(&dominate_packet));
		DominatePortalGate(iPlayerNum);
	}
	return bResult;
}

bool CSystemManager::IsWinPlayer(CInGamePlayer * pPlayer)
{
	if (pPlayer)
		return pPlayer->GetPlayerNum() == m_iDominatingPlayerNum;
	return m_iThisPlayer == m_iDominatingPlayerNum;
}

void CSystemManager::DominatePortalGate(int iPlayerNum)
{
	//static int testid = 0;
	mRoundState = ROUND_STATE::eROUND_DOMINATE;

	m_iDominatingPlayerNum = (iPlayerNum);// +testid) % 4;

	//cout << "점령 ID : " << m_iDominatingPlayerNum << endl;
	CGameObject* pPortal = GetPortalZoneObject();
	pPortal->AddRef();
	pPortal->SetMaterial(m_vcPlayerColorMaterial[m_iDominatingPlayerNum]);
	pPortal->SetActive(true);

	if (iPlayerNum != m_iThisPlayer)
	{
		static_cast<CInGamePlayer*>(m_ppPlayerArray[iPlayerNum])->SucceessDominate();
	}
	//testid++;
}

void CSystemManager::GameReady()
{
	mRoundState = ROUND_STATE::eGAME_READY;
	m_iRoundNumber = CLIENT.GetRoundNum();
}

void CSystemManager::GameStart()
{
	mRoundState = ROUND_STATE::eGAME_START;

	for (auto & info : mPlayerInfo)
	{
		info.m_iPlayerPoint = 0;
		info.m_nDeathCount  = 0;
		info.m_nKillCount   = 0;
	}

//	m_iRoundNumber = 0;
	m_iRoundNumber = CLIENT.GetRoundNum();
	RoundEnter();
}

void CSystemManager::RoundEnter()
{
	mRoundState = ROUND_STATE::eROUND_ENTER;

	m_iRoundNumber = CLIENT.GetRoundNum();

	GetPortalZoneObject()->SetActive(false);
	m_fRoundTime = 6.f;// mfLIMIT_ROUND_TIME;
	m_iDominatingPlayerNum = -1;

	//m_pNowScene->GetGameMessage(nullptr, eMessage::MSG_ROUND_ENTER);
	//EVENTMgr.InsertDelayMessage(mfENTER_TIME, eMessage::MSG_ROUND_START, CGameEventMgr::MSG_TYPE_SCENE, m_pNowScene);
}

void CSystemManager::RoundStart()
{
	mRoundState = ROUND_STATE::eROUND_START;
}

void CSystemManager::RoundEnd()
{
	mRoundState = ROUND_STATE::eROUND_END;

	if (0 <= m_iDominatingPlayerNum)
	{
		mPlayerInfo[m_iDominatingPlayerNum].m_iPlayerPoint += mROUND_WIN_POINT;
	}
	m_fEndTime = m_fRoundTime;
	m_fRoundTime = mfEND_TIME;
	//EVENTMgr.InsertDelayMessage(mfEND_TIME, eMessage::MSG_ROUND_CLEAR, CGameEventMgr::MSG_TYPE_SCENE, m_pNowScene);
}

void CSystemManager::RoundClear()
{
	mRoundState = ROUND_STATE::eROUND_CLEAR;

	if (m_iRoundNumber + 1 > mfGOAL_ROUND)
		GameEnd();
	else
		RoundEnter();
}

void CSystemManager::GameEnd()
{
	mRoundState = ROUND_STATE::eGAME_END;
	//EVENTMgr.InsertDelayMessage(0.f, eMessage::MSG_GAME_END, CGameEventMgr::MSG_TYPE_SCENE, m_pNowScene);
}
///////////////////////////////////////////// font ////////////////////////////////////////////////////////
void CGlobalFontUI::DrawFont()
{
	const static UINT playerColor[] = { 0xffffffff, 0xff0000ff, 0xff00ff00, 0xffff0000 };

	const static XMFLOAT2 RoundTimeLocation{ XMFLOAT2(FRAME_BUFFER_WIDTH * 0.5, -7) };
	const static XMFLOAT2 RoundTimeLocation2{ XMFLOAT2(FRAME_BUFFER_WIDTH * 0.5 + 5, -2) };

	static char screenFont[52];
	static wchar_t wscreenFont[30];
	static const int wssize = sizeof(wscreenFont);
	const int roundmax = SYSTEMMgr.mfGOAL_ROUND;

	swprintf_s(wscreenFont, wssize, L"Round(%d / %d)  %02d:%02d", SYSTEMMgr.GetRoundNumber(), roundmax, SYSTEMMgr.GetRoundMinute(), SYSTEMMgr.GetRoundSecond());
	FRAMEWORK.SetFont("Gabriola");
	FRAMEWORK.DrawFont(wscreenFont, 40, RoundTimeLocation, 0xff0099ff);
	FRAMEWORK.DrawFont(wscreenFont, 40, RoundTimeLocation2, 0x333333ff);
	
	FRAMEWORK.SetFont("Broadway");
	const static XMFLOAT2 PlayerList{ XMFLOAT2(60, 100) };
	const int allplayer = SYSTEMMgr.GetTotalPlayerNum();

	PLAYER_DATA_INFO * info = SYSTEMMgr.GetPlayerInfo();
	CGameObject ** ppPlayers = SYSTEMMgr.GetPlayerArray();
	int iPlayerNum = SYSTEMMgr.GetPlayerNum();
	for (int i = 0; i < allplayer; ++i)
	{
		swprintf_s(wscreenFont, wssize, L"%dP : %d pt [K:%d / D:%d]", i, info[i].m_iPlayerPoint,
			info[i].m_nKillCount, info[i].m_nDeathCount);
		FRAMEWORK.DrawFont(wscreenFont, 25, XMFLOAT2(10, 100 + 25 * i), playerColor[i], FW1_TEXT_FLAG::FW1_LEFT);
	}

	{
		CInGamePlayer * pPlayer = static_cast<CInGamePlayer*>(SYSTEMMgr.GetPlayer());
		swprintf_s(wscreenFont, wssize, L"%02d  %02d  %02d  %02d  %02d  %02d", pPlayer->GetEnergyNum(0), pPlayer->GetEnergyNum(1),
			pPlayer->GetEnergyNum(2), pPlayer->GetEnergyNum(3), pPlayer->GetEnergyNum(4), pPlayer->GetEnergyNum(5));

		const static XMFLOAT2 ElementalPos{ XMFLOAT2(165, 40) };
		FRAMEWORK.DrawFont(wscreenFont, 26, ElementalPos, 0xff3893a8);
	}
}

void CGameReadyFontUI::DrawFont()
{
	const static XMFLOAT2 StartInfoLocation{ XMFLOAT2(FRAME_BUFFER_WIDTH * 0.5, 60) };
	static wchar_t wscreenFont[26];
	static const int wssize = sizeof(wscreenFont);
	
	swprintf_s(wscreenFont, wssize, L"다른 플레이어 입장 대기중");
	
	FRAMEWORK.SetFont("HY견고딕");
	FRAMEWORK.DrawFont(wscreenFont, 40, StartInfoLocation, 0xff23ff23);
}

void CGameStartFontUI::DrawFont()
{
}

void CRoundEnterFontUI::DrawFont()
{	
	const static XMFLOAT2 StartInfoLocation{ XMFLOAT2(FRAME_BUFFER_WIDTH * 0.5, 60) };
	static wchar_t wscreenFont[26];
	static const int wssize = sizeof(wchar_t) * 26;
	static const int roundmax = SYSTEMMgr.mfGOAL_ROUND;
	static const float start_time = LIMIT_ROUND_TIME;//SYSTEMMgr.mfLIMIT_ROUND_TIME;
	const int second = SYSTEMMgr.GetRoundTime() + 1;
	float percent = (second - SYSTEMMgr.GetRoundTime());
	const int time_count = start_time - second;

//	cout << "Round Enter : " << second << " , " << time_count << endl;
	if (second > 0)
	{
		if(second <6)
			swprintf_s(wscreenFont, wssize, L"준비 %d!",second);
	}
	else
	{
		swprintf_s(wscreenFont, wssize, L"게임 시작!!!");
		percent = 0.f;
	}

	FRAMEWORK.SetFont("HY견고딕");
	FRAMEWORK.DrawFont(wscreenFont, 40 - 40 * percent, StartInfoLocation, 0xff23ff23);
}

void CRoundStartFontUI::DrawFont()
{
}

void CRoundDominateFontUI::DrawFont()
{
	const static UINT playerColor[] = { 0xffffffff, 0xff0000ff, 0xff00ff00, 0xffff0000 };
	const static XMFLOAT2 StartInfoLocation{ XMFLOAT2(FRAME_BUFFER_WIDTH * 0.5, 60) };
	static wchar_t wscreenFont[26];
	static const int wssize = sizeof(wchar_t) * 26;
	const int dominatedNum = SYSTEMMgr.GetDominatePlayerNum();

	swprintf_s(wscreenFont, wssize, L"점령중인 플레이어 : %d", dominatedNum);

	FRAMEWORK.SetFont("휴먼모음T");
	FRAMEWORK.DrawFont(wscreenFont, 40, StartInfoLocation, playerColor[dominatedNum]);
}

void CRoundDeathMatchFontUI::DrawFont()
{
}

void CRoundEndFontUI::DrawFont()
{
	const static XMFLOAT2 StartInfoLocation{ XMFLOAT2(FRAME_BUFFER_WIDTH * 0.5, 60) };
	static wchar_t wscreenFont[26];
	static const int wssize = sizeof(wchar_t) * 26;
	static const int roundmax = SYSTEMMgr.mfGOAL_ROUND;
	//const int second = SYSTEMMgr.GetRoundSecond();

	swprintf_s(wscreenFont, wssize, L"라운드 종료");

	FRAMEWORK.SetFont("HY견고딕");
	FRAMEWORK.DrawFont(wscreenFont, 40, StartInfoLocation, 0xff23ff23);
}

void CRoundClearFontUI::DrawFont()
{
}

void CGameEndFontUI::DrawFont()
{
}
