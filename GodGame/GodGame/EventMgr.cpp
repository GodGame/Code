#include "stdafx.h"
#include "EventMgr.h"

#include "Scene.h"

bool operator<(const cMessage & left, const cMessage & right)
{
	return left.GetLastTime() < right.GetLastTime();
}

CGameEventMgr::CGameEventMgr()
{
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

	if (m_mpMessageList.size() > 0)
	{
		auto it = m_mpMessageList.begin();

		if ((*it)->MessageUpdate(m_fCurrentTime))
			m_mpMessageList.erase(it);
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
	RECT rt;	// left, top, right, bottom
	rt = { 210, 540, 250, 500 };
	InsertObject(rt, "title_start");
   	cout << rt << endl;
}

bool UIRectMgr::CollisionCheck(XMFLOAT3 & pos, string name)
{
	POINT pt{ pos.x, pos.y };

	return CollisionBox(pt, name);
}

bool UIRectMgr::CollisionBox(POINT & pt, string name)
{
	RECT & rt = GetObjects(name);

	if (pt.x > rt.right) return false;
	if (pt.x < rt.left) return false;
	if (pt.y > rt.top) return false;
	if (pt.y < rt.bottom) return false;

	return true;
}
