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
	const float fGoalTime = m_fCurrentTime + fDelayeTime;
	switch (eType)
	{
#
	case MSGType::MSG_TYPE_ENTITY:
		m_mpMessageQueue.push(new cMessageSystem<CEntity>
			(fGoalTime, eMsg, (CEntity*)pToObj, (CEntity*)pByObj, extra));
		return;

	case MSGType::MSG_TYPE_SHADER:
		m_mpMessageQueue.push(new cMessageSystem<CShader>
			(fGoalTime, eMsg, (CShader*)pToObj, (CShader*)pByObj, extra));
		return;

	case MSGType::MSG_TYPE_SCENE:
		m_mpMessageQueue.push(new cMessageSystem<CScene>
			(fGoalTime, eMsg, (CScene*)pToObj, (CScene*)pByObj, extra));
		return;

	case MSGType::MSG_TYPE_NONE:
		return;

	default:
		ASSERT(SUCCEEDED(1));
	}
}

void CGameEventMgr::Initialize()
{
	m_fCurrentTime = 0.0f;

	for (int i = 0; i < m_mpMessageQueue.size(); ++i)
	{
		auto msg = m_mpMessageQueue.top();
		m_mpMessageQueue.pop();
		delete msg;
	}
#ifdef _NOT_USE_PRIORTY
		for (auto it = m_mpMessageQueue.begin(); it != m_mpMessageQueue.end(); ++it)
			delete (*it);

		m_mpMessageList.clear();
#endif
}

void CGameEventMgr::Update(float fFrameTime)
{
	m_fCurrentTime += fFrameTime;

	if (!m_mpMessageQueue.empty())
	{
		auto msg = m_mpMessageQueue.top();

		if (msg->IsTerminal(m_fCurrentTime))
		{
			m_mpMessageQueue.pop();
			msg->MessageExecute();
			delete msg;
		}
	}
#ifdef _NOT_USE_PRIORTY
	if (!m_mpMessageList.empty())
	{
		auto it = m_mpMessageList.begin();

		if ((*it)->IsTerminal(m_fCurrentTime))
		{
   			//cout << "Event Terminal : " << m_fCurrentTime <<" , Goal : " << (*it)->GetLastTime() << endl;
			cMessage * pMsg = *it;
			m_mpMessageList.erase(it);
			pMsg->MessageExecute();
			delete pMsg;
		}
			//m_mpMessageList.dequeue();
	}
#endif
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
