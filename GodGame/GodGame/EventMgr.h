#pragma once

#ifndef __EVENT_MGR
#define __EVENT_MGR

#include "stdafx.h"

enum eMessage
{
	MSG_NONE = -1,
	MSG_NORMAL = 0,
	MSG_COLLIDE,
	MSG_COLLIDED,
	MSG_DAMAGED,
	MSG_GETPOINT,

	MSG_CULL_OUT,
	MSG_CULL_IN,

	MSG_EFFECT_GLARE_ON,
	MSG_EFFECT_GLARE_OFF,
	MSG_EFFECT_RADIAL_ON,
	MSG_EFFECT_RADIAL_OFF
};

template <class T>
struct cMessage
{
public:
	float		 m_fLastTime;
	eMessage	 m_eMessage;
	T*			 m_pToObj;
	T*			 m_pByObj;
	void*		 m_pExtra;

public:
	bool operator<(const cMessage<T> & emsg)  { return m_flasttime < emsg.m_flasttime;  };
	bool operator<(float ftime)               { return m_flasttime < ftime;             };
	bool operator>(const cMessage<T> & emsg)  { return m_flasttime > emsg.m_flasttime;  };
	bool operator>(float ftime)               { return m_flasttime > ftime;             };
	bool operator==(const cMessage<T> & emsg) { return m_flasttime == emsg.m_flasttime; };
	bool operator==(float ftime)              { return m_flasttime == ftime;            };

	bool IsTerminal(float fTime) const        { return m_fLastTime < fTime; }
};

template <class T>
bool operator<(const cMessage<T> & left, const cMessage<T> & right)
{
	return left.m_fLastTime < right.m_fLastTime;
}

template <class T>
class cMessageMgr
{
	set<cMessage<T>>	   m_mpMessageList;

public:
	cMessageMgr() { }
	void Initialize() { m_mpMessageList.clear(); }

	void InsertDelayMessage(cMessage<T> & cMsg)
	{
		cMessage<T> msg = cMsg;
		m_mpMessageList.insert(msg);
	}
	void Update(float fCurrentTime)
	{
		if (m_mpMessageList.size() > 0)
		{
			auto it = m_mpMessageList.begin();
			if (it->IsTerminal(fCurrentTime))
			{
				if (it->m_pByObj)
					(it->m_pByObj)->SendGameMessage(it->m_pToObj, it->m_eMessage);
				else
					(it->m_pToObj)->GetGameMessage(nullptr, it->m_eMessage);

				m_mpMessageList.erase(it);
			}
		}
	}
};

class CScene;
class CShader;
class CGameObject;
class CGameEventMgr
{
private:
	float	m_fCurrentTime;

	cMessageMgr<CScene>         m_mgrScene;
	cMessageMgr<CShader>        m_mgrShader;
	cMessageMgr<CGameObject>    m_mgrObject;

private:
	CGameEventMgr();
	~CGameEventMgr();

public:
	static CGameEventMgr& GetInstance();

	void SceneDelayMessage(float fDelayeTime, eMessage eMsg, CScene * pToScene, CScene * pByScene = nullptr, void * extra = nullptr);
	void ShaderDelayMessage(float fDelayeTime, eMessage eMsg, CShader* pToShader, CShader * pByShader = nullptr, void * extra = nullptr);
	void ObjectDelayMessage(float fDelayeTime, eMessage eMsg, CGameObject* pToObj, CGameObject * pByObject = nullptr, void * extra = nullptr);

	void Initialize();
	void Update(float fFrameTime);
};

#define EVENTMgr CGameEventMgr::GetInstance()
#endif