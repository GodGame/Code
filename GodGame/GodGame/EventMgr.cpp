#include "stdafx.h"
#include "EventMgr.h"

#include "Scene.h"
#include "AI.h"

bool operator<(const cMessage & left, const cMessage & right)
{
	return left.GetLastTime() < right.GetLastTime();
}

CGameEventMgr::CGameEventMgr()
{
	m_fCurrentTime = 0.0f;
}

CGameEventMgr::~CGameEventMgr()
{
}

CGameEventMgr & CGameEventMgr::GetInstance()
{
	static CGameEventMgr instance;
	return instance;
}

void CGameEventMgr::InsertDelayMessage(float fDelayeTime, eMessage eMsg, MSGType eType, void* pToObj, void * pByObj, void * extra)
{
	switch (eType)
	{

	case MSGType::MSG_TYPE_OBJECT:
		m_mpMessageList.insert(new cMessageSystem<CGameObject>
			(m_fCurrentTime + fDelayeTime, eMsg, (CGameObject*)pToObj, (CGameObject*)pByObj, extra));
		return;

	case MSGType::MSG_TYPE_SHADER:
		m_mpMessageList.insert(new cMessageSystem<CShader>
			(m_fCurrentTime + fDelayeTime, eMsg, (CShader*)pToObj, (CShader*)pByObj, extra));
		return;

	case MSGType::MSG_TYPE_SCENE:
		m_mpMessageList.insert(new cMessageSystem<CScene>
			(m_fCurrentTime + fDelayeTime, eMsg, (CScene*)pToObj, (CScene*)pByObj, extra));
		return;

	//case MSGType::MSG_TYPE_FSM:
	//	m_mpMessageList.insert(new cMessageSystem<CAIState<CWarrock>>
	//		(m_fCurrentTime + fDelayeTime, eMsg, (CAIState<CWarrock>*)pToObj, (CAIState<CWarrock>*)pByObj, extra));
	//	return;

	case MSGType::MSG_TYPE_NONE:
		return;

	default:
		ASSERT(SUCCEEDED(1));
	}
}

void CGameEventMgr::Initialize()
{
	m_fCurrentTime = 0.0f;

	for (auto it = m_mpMessageList.begin(); it != m_mpMessageList.end(); ++it)
		delete (*it);

	m_mpMessageList.clear();
}

void CGameEventMgr::Update(float fFrameTime)
{
	m_fCurrentTime += fFrameTime;

	if (!m_mpMessageList.empty())
	{
		auto it = m_mpMessageList.begin();

		if ((*it)->IsTerminal(m_fCurrentTime))
		{
			cMessage * pMsg = *it;
			m_mpMessageList.erase(it);
			pMsg->MessageExecute();
		}
			//m_mpMessageList.dequeue();
	}
}

UIRectMgr::UIRectMgr()
{
}

UIRectMgr::~UIRectMgr()
{
}

UIRectMgr & UIRectMgr::GetInstance()
{
	static UIRectMgr instance;
	return instance;
}

void UIRectMgr::BuildResources()
{
	UIInfo info;
	// left, top, right, bottom
	info.m_rect = { 210, 540, 250, 500 };
	info.m_msgUI = UIMessage::MSG_UI_TITLE_INSERT_INGAME;
	InsertObject(info, "ui_title_start");

}

bool UIRectMgr::CollisionCheck(XMFLOAT3 & pos, string name)
{
	POINT pt{ pos.x, pos.y };
	return CollisionCheck(pt, name);
}

bool UIRectMgr::CollisionCheck(POINT & pt, string name)
{
	RECT & rt = GetObjects(name).m_rect;

	if (pt.x > rt.right)  return false;
	if (pt.x < rt.left)   return false;
	if (pt.y > rt.top)    return false;
	if (pt.y < rt.bottom) return false;

	return true;
}
