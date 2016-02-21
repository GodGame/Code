#pragma once

#ifndef __MGR_TYPE
#define __MGR_TYPE
#ifdef _DEBUG
#include <string>
#endif
template<class Target>
class CMgr
{
protected:	
	typedef map<string, Target*> MgrList;
	MgrList m_mpList;
	
public:
	CMgr(){}
	virtual ~CMgr(){}

public:

	virtual void InsertObject(Target* pObject, string name) 
	{ 
		if (m_mpList[name]) m_mpList[name]->Release();
		m_mpList[name] = pObject;
		pObject->AddRef();
	}
	virtual void BuildResources(ID3D11Device *pd3dDevice)      {};
	virtual void ReleaseObjects()
	{
		for (MgrList::iterator it = m_mpList.begin(); it != m_mpList.end(); ++it)
		{
#ifdef _DEBUG
			int num = it->second->Release();
			if (num > 0)
			{
				cout << it->first << "가 이상있습니다. 개수 : " << num << endl;
				while (num > 0)
				{
					num = it->second->Release();
				}
			}
#else
			it->second->Release();
#endif
		}
		m_mpList.clear();
	}
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