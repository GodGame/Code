#pragma once

#ifndef __EVENT_MGR
#define __EVENT_MGR

#include "stdafx.h"
#include "MgrType.h"

enum eMessage : BYTE
{
//	MSG_NONE = -1,
	MSG_NORMAL = 0,
	//Collide
	MSG_COLLIDE,
	MSG_COLLIDED,
	MSG_COLLIDE_LOCATION,
	MSG_COLLIDE_UI,
	// Mouse
	MSG_MOUSE_DOWN,
	MSG_MOUSE_DOWN_OVER,
	MSG_MOUSE_UP,
	MSG_MOUSE_UP_OVER,
	// Quad-Tree
	MSG_QUAD_DELETE,
	MSG_CULL_OUT,
	MSG_CULL_IN,
	// Scene
	MSG_SCENE_CHANGE,
	// Round
	MSG_GAME_START,
	MSG_ROUND_ENTER,
	MSG_ROUND_START,
	MSG_ROUND_END,
	MSG_ROUND_CLEAR,
	MSG_GAME_END,
	// Data-Pass
	MSG_PASS_PLAYERPTR,
	// Object
	MSG_OBJECT_RENEW,
	MSG_OBJECT_ANIM_CHANGE,
	MSG_OBJECT_STATE_CHANGE,
	MSG_OBJECT_DAMAGED,
	// Player
	MSG_PLAYER_USE_MAGIC,
	MSG_PLAYER_GETPOINT,
	MSG_PLAYER_SOUL,
	MSG_PLAYER_DOMIATE_END,
	MSG_PLAYER_COOLTIME,
	// Magic
	MSG_MAGIC,
	MSG_MAGIC_SHOT,
	MSG_MAGIC_CAST,
	MSG_MAGIC_AREA,
	// Item
	MSG_ITEM_STAFF_CHANGE,
	// Particle
	MSG_PARTICLE_ON,

	MSG_BUFF_ON,
	MSG_BUFF_OFF,	
	MSG_DEBUFF_ON,
	MSG_DEBUFF_OFF,

	MSG_EFFECT,
	MSG_EFFECT_DOMINATE_SUCCESS,
	MSG_EFFECT_GLARE_ON,
	MSG_EFFECT_GLARE_OFF,
	MSG_EFFECT_RADIAL_ON,
	MSG_EFFECT_RADIAL_OFF,
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

	virtual void MessageExecute() const {}
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

	virtual void MessageExecute() const
	{
		//if (m_pByObj)		m_pByObj->SendGameMessage(m_pToObj, m_eMessage, m_pExtra);
		if (m_pToObj) m_pToObj->GetGameMessage(nullptr, m_eMessage, m_pExtra);
	}

	virtual bool MessageUpdate(float fTime) const
	{
		if (IsTerminal(fTime)) 
		{
			MessageExecute();
			return true;
		}
		return false;
	}
};

bool operator<(const cMessage & left, const cMessage & right);
//bool operator<(const cMessage * left, const cMessage * right);

class cMessageLessTime
{
public:
	bool operator()(cMessage * p1, cMessage * p2)
	{
		return p1->GetLastTime() < p2->GetLastTime();
	}
};

class cMessageGreaterTime
{
public:
	bool operator()(cMessage * p1, cMessage * p2)
	{
		return p1->GetLastTime() > p2->GetLastTime();
	}
};

class CScene;
class CShader;
class CGameObject;
class CGameEventMgr
{
private:
	float	m_fCurrentTime;
	set<cMessage*, cMessageLessTime>	   m_mpMessageList;
	//MyPriorityPointerQueue<cMessage>	   m_mpMessageList;

private:
	CGameEventMgr();
	~CGameEventMgr();
	CGameEventMgr& operator=(const CGameEventMgr&);

public:
	enum MSGType : UCHAR
	{
		MSG_TYPE_NONE = 0,
		MSG_TYPE_SCENE,
		MSG_TYPE_SHADER,
		MSG_TYPE_ENTITY,
		MSG_TYPE_FSM
	};

	static CGameEventMgr& GetInstance();
	// ByObj를 채워넣으면 SendGameMessage를 호출하며, 채워넣지 않으면 GetGameMessage만 호출한다.
	void InsertDelayMessage(float fDelayeTime, eMessage eMsg, MSGType eType, void * pToObj, void * pByObj = nullptr, void * extra = nullptr);

	void Initialize();
	void Update(float fFrameTime);
};

#define EVENTMgr CGameEventMgr::GetInstance()

enum UIMessage
{
	MSG_UI_NONE = 0,
	MSG_UI_TITLE_INSERT_INGAME
};

struct UIInfo
{
	RECT m_rect;
	UIMessage m_msgUI;

	UIInfo()
	{
		m_rect  = { 0, 0, 0, 0 };
		m_msgUI = MSG_UI_NONE;
	}
};

class UIRectMgr : public CMgrCase<UIInfo>
{
private:
	UIRectMgr();
	~UIRectMgr();
	UIRectMgr& operator=(const UIRectMgr&);

public:
	static UIRectMgr& GetInstance();

	void BuildResources();
	bool CollisionCheck(XMFLOAT3 & pos, string name);
	bool CollisionCheck(POINT & pt, string name);
	inline RECT& GetCollisionBox(string name) { return GetObjects(name).m_rect;}
};
#define UIMgr UIRectMgr::GetInstance()
#endif