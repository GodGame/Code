#pragma once

#ifndef __EVENT_MGR
#define __EVENT_MGR

#include "stdafx.h"

enum eMessage : BYTE
{
//	MSG_NONE = -1,
	MSG_NORMAL = 0,
	MSG_COLLIDE,
	MSG_COLLIDED,

	MSG_DAMAGED,
	MSG_GETPOINT,

	MSG_BUFF_ON,
	MSG_BUFF_OFF,
	
	MSG_DEBUFF_ON,
	MSG_DEBUFF_OFF,

	MSG_CULL_OUT,
	MSG_CULL_IN,

	MSG_EFFECT_GLARE_ON,
	MSG_EFFECT_GLARE_OFF,
	MSG_EFFECT_RADIAL_ON,
	MSG_EFFECT_RADIAL_OFF
};

//template <class T>
class cMessage
{
protected:
	float		 m_fLastTime;
	eMessage	 m_eMessage;
	void*		 m_pExtra;

public:
	float GetLastTime() const { return m_fLastTime; }

	bool operator<(const cMessage & emsg)     { return m_fLastTime < emsg.m_fLastTime;  }
	bool operator<(float ftime)               { return m_fLastTime < ftime;             }
	bool operator>(const cMessage & emsg)     { return m_fLastTime > emsg.m_fLastTime;  }
	bool operator>(float ftime)               { return m_fLastTime > ftime;             }
	bool operator==(const cMessage & emsg)    { return m_fLastTime == emsg.m_fLastTime; }
	bool operator==(float ftime)              { return m_fLastTime == ftime;            }

	bool IsTerminal(float fTime) const        { return m_fLastTime < fTime; }

	virtual bool MessageUpdate(float fTime) const { return false; }
};

template<class T>
class cMessageSystem : public cMessage
{
protected:
	T*		 m_pToObj;
	T*		 m_pByObj;

public:
	cMessageSystem<T>(float fLastTime, eMessage eMsg, T * pToObj, T * pByObj, void * extra)
	{
		m_fLastTime = fLastTime;
		m_eMessage  = eMsg;
		m_pToObj    = pToObj;
		m_pByObj    = pByObj;
		m_pExtra    = extra;
	}

	virtual bool MessageUpdate(float fTime) const
	{
		if (IsTerminal(fTime)) 
		{
			if (m_pByObj) m_pByObj->SendGameMessage(m_pToObj, m_eMessage);
			else		  m_pToObj->GetGameMessage(nullptr, m_eMessage);
			return true;
		}
		return false;
	}
};

bool operator<(const cMessage & left, const cMessage & right);

class CScene;
class CShader;
class CGameObject;
class CGameEventMgr
{
private:
	float	m_fCurrentTime;
	set<cMessage*>	   m_mpMessageList;

private:
	CGameEventMgr();
	~CGameEventMgr();
	CGameEventMgr& operator=(const CGameEventMgr&);

public:
	enum MSGType : BYTE
	{
		MSG_TYPE_NONE = 0,
		MSG_TYPE_SCENE,
		MSG_TYPE_SHADER,
		MSG_TYPE_OBJECT
	};

	static CGameEventMgr& GetInstance();
	// ByObj�� ä�������� SendGameMessage�� ȣ���ϸ�, ä������ ������ GetGameMessage�� ȣ���Ѵ�.
	void InsertDelayMessage(float fDelayeTime, eMessage eMsg, MSGType eType, void * pToObj, void * pByObj = nullptr, void * extra = nullptr);

	void Initialize();
	void Update(float fFrameTime);
};

#define EVENTMgr CGameEventMgr::GetInstance()
#endif