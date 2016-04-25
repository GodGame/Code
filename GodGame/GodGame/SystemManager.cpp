#include "stdafx.h"
#include "SystemManager.h"
#include "SystemObject.h"
//#include "SceneInGame.h"
#include "GameFramework.h"

CSystemManager::CSystemManager()
{
	_CreateFontUiArray();

	mRoundState			   = eROUND_NONE;
	m_iMapInfo             = 0;
	m_iRoundNumber         = 0;
	m_iTotalRound          = 0;
	m_nPlayers             = 0;
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
	m_fRoundTime += fFrameTime;

	m_nRoundSecond = m_fRoundTime;
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
	//cout << "�Ÿ��� " << fLengthSq;
	if (fLengthSq <= fGoalDistance)
	{
		mPlayersDominateTryTime[pPlayer->GetPlayerNum()] = m_fRoundTime - 0.1f;
		return true;
	}
	return false;
}

bool CSystemManager::CheckCanDomianteSuccess(CInGamePlayer * pPlayer)
{
	const int iPlayerNum = pPlayer->GetPlayerNum();

	bool bResult = (m_fRoundTime > 
		(mPlayersDominateTryTime[iPlayerNum] + mfDOMINATE_SPEND_TIME));

	if (bResult)
	{
		DominatePortalGate(iPlayerNum);
	}
	return bResult;
}

bool CSystemManager::IsWinPlayer(CInGamePlayer * pPlayer)
{
	return pPlayer->GetPlayerNum() == m_iDominatingPlayerNum;
}

void CSystemManager::DominatePortalGate(int iPlayerNum)
{
	static int testid = 0;
	mRoundState = ROUND_STATE::eROUND_DOMINATE;

	m_iDominatingPlayerNum = iPlayerNum + testid;

	cout << "���� ID : " << m_iDominatingPlayerNum << endl;

	CGameObject* pPortal = GetPortalZoneObject();
	pPortal->AddRef();
	pPortal->SetMaterial(m_vcPlayerColorMaterial[m_iDominatingPlayerNum]);
	pPortal->SetActive(true);

	testid = (testid + 1) % 4;
}

void CSystemManager::GameStart()
{
	mRoundState = ROUND_STATE::eROUND_START;

	m_iRoundNumber = 0;
	RoundEnter();
}

void CSystemManager::RoundEnter()
{
	mRoundState = ROUND_STATE::eROUND_ENTER;

	m_iRoundNumber++;
	GetPortalZoneObject()->SetActive(false);
	m_fRoundTime = 0.f;
	m_iDominatingPlayerNum = -1;

	m_pNowScene->GetGameMessage(nullptr, eMessage::MSG_ROUND_ENTER);
	EVENTMgr.InsertDelayMessage(mfENTER_TIME, eMessage::MSG_ROUND_START, CGameEventMgr::MSG_TYPE_SCENE, m_pNowScene);
}

void CSystemManager::RoundStart()
{
	mRoundState = ROUND_STATE::eROUND_START;
}

void CSystemManager::RoundEnd()
{
	mRoundState = ROUND_STATE::eROUND_END;

	m_fEndTime = m_fRoundTime;
	EVENTMgr.InsertDelayMessage(mfEND_TIME, eMessage::MSG_ROUND_CLEAR, CGameEventMgr::MSG_TYPE_SCENE, m_pNowScene);
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
	EVENTMgr.InsertDelayMessage(0.f, eMessage::MSG_GAME_END, CGameEventMgr::MSG_TYPE_SCENE, m_pNowScene);
}
///////////////////////////////////////////// font ////////////////////////////////////////////////////////
void CGameStartFontUI::DrawFont()
{
}

void CRoundEnterFontUI::DrawFont()
{	
	const static XMFLOAT2 StartInfoLocation{ XMFLOAT2(FRAME_BUFFER_WIDTH * 0.5, 60) };
	static wchar_t wscreenFont[26];
	static const int wssize = sizeof(wchar_t) * 26;
	static const int roundmax = SYSTEMMgr.mfGOAL_ROUND;
	const int second = SYSTEMMgr.GetRoundSecond();

	if (second < 5)
	{
		swprintf_s(wscreenFont, wssize, L"�غ� %d!", 5 - second);
	}
	else
	{
		swprintf_s(wscreenFont, wssize, L"���� ����!!!");
	}

	FRAMEWORK.SetFont("HY�߰��");
	FRAMEWORK.DrawFont(wscreenFont, 40, StartInfoLocation, 0xff23ff23);
}

void CRoundStartFontUI::DrawFont()
{
}

void CRoundDominateFontUI::DrawFont()
{
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
	const int second = SYSTEMMgr.GetRoundSecond();


	swprintf_s(wscreenFont, wssize, L"���� ����");

	FRAMEWORK.SetFont("HY�߰��");
	FRAMEWORK.DrawFont(wscreenFont, 40, StartInfoLocation, 0xff23ff23);
}

void CRoundClearFontUI::DrawFont()
{
}

void CGameEndFontUI::DrawFont()
{
}