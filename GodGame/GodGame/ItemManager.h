#pragma once

#ifndef __ITEM_MGR
#define __ITEM_MGR
#include "MgrType.h"

class CItem;
struct ItemInfo
{
	UINT mItemNo;
	CItem * mpItem;

	ItemInfo() : mpItem(nullptr) { mItemNo = 0; }
	ItemInfo(UINT no, CItem * item) : mItemNo(no), mpItem(item) {}
	~ItemInfo() { }
};

class CItemManager : public CMgrCase<ItemInfo>
{
	CItemManager();
	virtual ~CItemManager();

	//CMgrCase<int> mItemNoManager;

	const int WEAPON_START_INDEX = 10000;

public:
	string StaffNameArray[6][3];
	//static string StaffName;
	
	static CItemManager& GetInstance();

public:
	void Build(ID3D11Device * pd3dDevice);

//	int GetItemNo(string name) { return mItemNoManager.GetObjects(name); }
//	string& GetItemName(int itemNo) { return mItemNoManager.}

};
#define ITEMMgr CItemManager::GetInstance()
#endif