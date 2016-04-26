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
	//mItemNoManager.c
}

CItemManager & CItemManager::GetInstance()
{
	static CItemManager instance;
	return instance;
}

void CItemManager::Build(ID3D11Device * pd3dDevice)
{
	// 파일입출력
	int index = 0;

	static const int iCostLight[] = { 8, 16, 24 };
	static const int iCostFire[]  = { 10, 20, 30 };
	static const int iCostWind[]  = { 10, 20, 30 };
	static const int iCostIce[]   = { 10, 20, 30 };
	static const int iCostDark[]  = { 12, 24, 36 };
	static const int iCostElectric[]  = { 10, 20, 30 };
	static const char name[6][12] = { { "light" },{ "fire" },{ "wind" },{ "ice" },{ "dark" },{ "electric" } };

	const int* CostArray[] = { iCostLight , iCostFire, iCostWind, iCostIce, iCostDark, iCostElectric };
		
	CStaff * pItem = nullptr;
	CTexture * pUI = nullptr;
	ID3D11ShaderResourceView * pd3dsrvTexture = nullptr;
	char temp[24];
	wchar_t wtemp[128];
	for (int i = 0; i < ELEMENT_NUM; ++i)
	{
		for (int lv = 0; lv < WEAPON_LEVEL_MAX; ++lv)
		{
			pItem = new CStaff(1);
			pItem->BuildObject(ELEMENT_LIGHT, CostArray[i][lv], lv);
			sprintf(temp, "staff_%s_%d", name[i], lv);

			swprintf(wtemp, L"../Assets/Image/UI/staffImage/%hs.JPG", temp);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, wtemp, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pUI = new CTexture(1, 0, 0, 0);
			pUI->SetTexture(0, pd3dsrvTexture);
			TXMgr.InsertObject(pUI, temp);
			//pUI->Release();

			StaffNameArray[i][lv] = temp;
			//cout << lv << "레벨의 Cost " << CostArray[i][lv] << "인 무기 " << StaffNameArray[i][lv] << endl;
			InsertObject((ItemInfo{ 0, pItem }), temp);
		}
	}
}
