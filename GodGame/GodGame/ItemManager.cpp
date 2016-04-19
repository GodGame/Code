#include "stdafx.h"
#include "ItemManager.h"
#include "Weapon.h"

CItemManager::CItemManager()
{
}

CItemManager::~CItemManager()
{
	for (auto info : m_mpList)
	{
		info.second.mpItem->Release();
	}
}

CItemManager & CItemManager::GetInstance()
{
	static CItemManager instance;
	return instance;
}

void CItemManager::Build()
{
	// 파일입출력
	static const int iCostLight[] = { 8, 16, 24 };
	static const int iCostFire[]  = { 10, 20, 30 };
	static const int iCostWind[]  = { 10, 20, 30 };
	static const int iCostIce[]   = { 10, 20, 30 };
	static const int iCostDark[]  = { 12, 24, 36 };
	static const int iCostElectric[]  = { 10, 20, 30 };
	static const char name[6][12] = { { "light" },{ "fire" },{ "wind" },{ "ice" },{ "dark" },{ "electirc" } };

	const int* CostArray[] = { iCostLight , iCostFire, iCostWind, iCostIce, iCostDark, iCostElectric };
		
	CStaff * pItem = nullptr;
	char temp[24];
	for (int i = 0; i < ELEMENT_NUM; ++i)
	{
		for (int lv = 0; lv < WEAPON_LEVEL_MAX; ++lv)
		{
			pItem = new CStaff(1);
			pItem->BuildObject(ELEMENT_LIGHT, CostArray[i][lv], lv);
			sprintf(temp, "staff_%s_%d", name[i], lv);
			
			StaffNameArray[i][lv] = temp;
			//cout << lv << "레벨의 Cost " << CostArray[i][lv] << "인 무기 " << StaffNameArray[i][lv] << endl;
			InsertObject((ItemInfo{ 0, pItem }), temp);
		}
	}
}
