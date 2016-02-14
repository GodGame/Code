#include "stdafx.h"
#include "EventMgr.h"

#include "Scene.h"

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

void CGameEventMgr::SceneDelayMessage(float fDelayeTime, eMessage eMsg, CScene * pToScene, CScene * pByScene, void * extra)
{
	cMessage<CScene> msg{ m_fCurrentTime + fDelayeTime, eMsg, pToScene, pByScene, extra };
	m_mgrScene.InsertDelayMessage(msg);
}

void CGameEventMgr::ShaderDelayMessage(float fDelayeTime, eMessage eMsg, CShader * pToShader, CShader * pByShader, void * extra)
{
	cMessage<CShader> msg{ m_fCurrentTime + fDelayeTime, eMsg, pToShader, pByShader, extra };
	m_mgrShader.InsertDelayMessage(msg);
}

void CGameEventMgr::ObjectDelayMessage(float fDelayeTime, eMessage eMsg, CGameObject * pToObj, CGameObject * pByObject, void * extra)
{
	cMessage<CGameObject> msg{ m_fCurrentTime + fDelayeTime, eMsg, pToObj, pByObject, extra };
	m_mgrObject.InsertDelayMessage(msg);
}

void CGameEventMgr::Initialize()
{
	m_fCurrentTime = 0.0f;
	m_mgrScene.Initialize();
	m_mgrShader.Initialize();
	m_mgrObject.Initialize();
}

void CGameEventMgr::Update(float fFrameTime)
{
	m_fCurrentTime += fFrameTime;
	m_mgrScene.Update(m_fCurrentTime);
	m_mgrShader.Update(m_fCurrentTime);
	m_mgrObject.Update(m_fCurrentTime);
}