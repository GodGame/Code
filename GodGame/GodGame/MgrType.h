#pragma once

#ifndef __MGR_TYPE
#define __MGR_TYPE

template<class Target>
class CMgr
{
protected:	
	typedef map<string, Target*> MgrList;
	MgrList m_mpList;
	
public:
	CMgr(){}
	virtual ~CMgr()
	{
		for (MgrList::iterator it = m_mpList.begin(); it != m_mpList.end(); ++it)
		{
			it->second->Release();
		}
	}

public:
	virtual void InsertObject(Target* pObject, string name) 
	{ 
		if (m_mpList[name]) m_mpList[name]->Release();
		m_mpList[name] = pObject;
		pObject->AddRef();
	}
	virtual void BuildResources(ID3D11Device *pd3dDevice)      {};
	inline virtual Target * GetObjects(string name)		       { return m_mpList[name]; }
	inline virtual void EraseObjects(string name)              { m_mpList[name]->Release(); m_mpList.erase(name); }
};

template<class Target>
class CMgrCasePointer
{
protected:
	typedef map<string, Target*> MgrList;
	MgrList m_mpList;

public:
	CMgrCasePointer() {}
	virtual ~CMgrCasePointer() {}

public:
	virtual void InsertObject(Target* pObject, string name)
	{
		m_mpList[name] = pObject;
	}
	virtual void BuildResources(ID3D11Device *pd3dDevice) {};
	inline virtual Target * GetObjects(string name) { return m_mpList[name]; }
	inline virtual void EraseObjects(string name) { m_mpList.erase(name); }
};

template<class Target>
class CMgrCase
{
protected:
	typedef map<string, Target> MgrList;
	MgrList m_mpList;

public:
	CMgrCase() {}
	virtual ~CMgrCase() {}

public:
	virtual void InsertObject(Target & pObject, string name)
	{
		m_mpList[name] = pObject;
	}
	virtual void BuildResources(ID3D11Device *pd3dDevice) {};
	inline virtual Target & GetObjects(string name) { return m_mpList[name]; }
	inline virtual void EraseObjects(string name) { m_mpList.erase(name); }
};

#endif