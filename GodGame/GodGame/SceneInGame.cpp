#include "stdafx.h"
#include "MyInline.h"
#include "SceneInGame.h"
#include "SceneTitle.h"
#include "GameFramework.h"
#include "Protocol.h"
bool bIsKeyDown = false;
//bool bIsMouseDown = false;

CSceneInGame::CSceneInGame() : CScene()
{
	m_nEffectShaderNum = 0;
	ZeroMemory(m_recvBuffer, sizeof(m_recvBuffer));
	ZeroMemory(&m_fHeight, sizeof(m_fHeight));
}

CSceneInGame::~CSceneInGame()
{
}

void CSceneInGame::InitializeRecv()
{
#ifdef USE_SERVER
	if (false == CLIENT.Setting(FRAMEWORK.m_hWnd, SERVER_PORT))
	{
		cout << "Server와 접속이 되지 않았습니다." << endl;
	}
	PACKET_MGR.Recv(); // 초기값 받는다.
	PACKET_MGR.GetRecvBuffer()->buf; // 미리 준비해둔 패킷 버퍼를 이용함 ( Async 에서는 CLIENT.GetRecvBuffer() )
	// 플레이어 고유번호 설정, 맵 배치 등을 먼저 받고나서 빌드시킴
	CLIENT.SetAsyncSelect(); // Async Select를 시작한다.
#endif
	if (false == CLIENT.Setting(FRAMEWORK.m_hWnd, SERVER_PORT))
	{
		cout << "Server와 접속이 되지 않았습니다." << endl;
	}
	CLIENT.SetPlayerShader(m_pPlayerShader);
	CLIENT.ReadPacket();
	// 플레이어 고유번호 설정, 맵 배치 등을 먼저 받고나서 빌드시킴
	CLIENT.SetAsyncSelect(); // Async Select를 시작한다.
}

void CSceneInGame::Reset()
{
	for (auto & shaders : m_vcResetShaders)
		shaders->Reset();
	cout << "Reset" << endl;
//	EVENTMgr.InsertDelayMessage(SYSTEMMgr.mfLIMIT_ROUND_TIME,
//		eMessage::MSG_ROUND_END, CGameEventMgr::MSG_TYPE_SCENE, this, nullptr, m_pCamera->GetPlayer());
}

void CSceneInGame::BuildMeshes(ID3D11Device * pd3dDevice)
{
	wstring baseDir{ _T("../Assets/Image/Objects/") };
	vector<wstring> vcTxFileNames;

	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	CTexture * pTexture = nullptr;
	CMesh * pMesh = nullptr;
	char file[128];
	wchar_t result[128];

	for (int i = 1; i < 5; ++i)
	{
		// 바위
		pTexture = new CTexture(3, 1, 0, 0);
		{
			swprintf(result, _T("../Assets/Image/Objects/Rock/rock_0%d_diffuse.jpg"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(0, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			swprintf(result, _T("../Assets/Image/Objects/Rock/rock_0%d_normals.jpg"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(1, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			swprintf(result, _T("../Assets/Image/Objects/Rock/rock_0%d_specular.jpg"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(2, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));
		}
		sprintf(file, "../Assets/Image/Objects/Rock/rock_0%d.fbxcjh", i);
		pMesh = new CLoadMeshByFbxcjh(pd3dDevice, file, 1.f, vcTxFileNames);
		vcTxFileNames.clear();

		sprintf(file, "scene_rock%d", i);
		m_SceneResoucres.mgrTexture.InsertObject(pTexture, file);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, file);
	}
	for (int i = 1; i < 4; ++i)
	{
		// stone
		pTexture = new CTexture(3, 1, 0, 0);
		{
			swprintf(result, _T("../Assets/Image/Objects/Rock/stone_0%d_diffuse.jpg"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(0, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			swprintf(result, _T("../Assets/Image/Objects/Rock/stone_0%d_normals.jpg"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(1, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			swprintf(result, _T("../Assets/Image/Objects/Rock/stone_0%d_specular.jpg"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(2, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));
		}
		sprintf(file, "../Assets/Image/Objects/Rock/stone_0%d.fbxcjh", i);
		pMesh = new CLoadMeshByFbxcjh(pd3dDevice, file, 1.f, vcTxFileNames);
		vcTxFileNames.clear();

		sprintf(file, "scene_stone%d", i);
		m_SceneResoucres.mgrTexture.InsertObject(pTexture, file);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, file);
	}
	for (int i = 1; i < 4; ++i)
	{
		// 기둥
		pTexture = new CTexture(3, 1, 0, 0);
		{
			swprintf(result, _T("../Assets/Image/Objects/Colum/column_0%d_diffuse.jpg"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(0, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			swprintf(result, _T("../Assets/Image/Objects/Colum/column_0%d_normals.jpg"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(1, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			swprintf(result, _T("../Assets/Image/Objects/Colum/column_0%d_specular.jpg"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(2, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));
		}
		sprintf(file, "../Assets/Image/Objects/Colum/column_0%d.fbxcjh", i);
		pMesh = new CLoadMeshByFbxcjh(pd3dDevice, file, 0.4f, vcTxFileNames);
		vcTxFileNames.clear();

		sprintf(file, "scene_colum%d", i);
		m_SceneResoucres.mgrTexture.InsertObject(pTexture, file);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, file);
	}
	for (int i = 1; i < 3; ++i)
	{
		// ruin
		pTexture = new CTexture(3, 1, 0, 0);
		{
			swprintf(result, _T("../Assets/Image/Objects/Colum/ruin_0%d_diffuse.jpg"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(0, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			swprintf(result, _T("../Assets/Image/Objects/Colum/ruin_0%d_normal.jpg"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(1, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			swprintf(result, _T("../Assets/Image/Objects/Colum/ruin_0%d_specular.jpg"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(2, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));
		}
		sprintf(file, "../Assets/Image/Objects/Colum/ruin_0%d.fbxcjh", i);
		pMesh = new CLoadMeshByFbxcjh(pd3dDevice, file, 0.3f, vcTxFileNames);
		vcTxFileNames.clear();

		sprintf(file, "scene_ruin%d", i);
		m_SceneResoucres.mgrTexture.InsertObject(pTexture, file);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, file);
	}

	for (int i = 0; i < 6; ++i)
	{
		// 스태프 1
		pTexture = new CTexture(2, 1, 0, 0);
		{
			wsprintf(result, _T("../Assets/Image/Objects/staff/Staff0%d_Diff.png"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(0, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			wsprintf(result, _T("../Assets/Image/Objects/staff/Staff0%d_Diff_NRM.png"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(1, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));
		}
		sprintf(file, "../Assets/Image/Objects/staff/Staff0%d.fbxcjh", i);
		pMesh = new CLoadMeshByFbxcjh(pd3dDevice, file, 0.2f, vcTxFileNames);
		vcTxFileNames.clear();

		string ObjName = ITEMMgr.StaffNameArray[i][0];
		m_SceneResoucres.mgrTexture.InsertObject(pTexture, ObjName);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, ObjName);
	}

	// 스태프2
	for (int i = 0; i < 7; ++i)
	{
		pTexture = new CTexture(3, 1, 0, 0);
		{
			wsprintf(result, _T("../Assets/Image/Objects/staff2/Staff2_0%d_Diff.png"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(0, pd3dsrvTexture);
			pd3dsrvTexture->Release();
			pTexture->SetTexture(1, nullptr);

			wsprintf(result, _T("../Assets/Image/Objects/staff2/Staff2_0%d_Spec.png"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(2, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));
		}

		sprintf(file, "../Assets/Image/Objects/staff2/Staff2_0%d.fbxcjh", i);
		pMesh = new CLoadMeshByFbxcjh(pd3dDevice, file, 0.2f, vcTxFileNames);
		vcTxFileNames.clear();

		if (i != 6)
		{
			string ObjName = ITEMMgr.StaffNameArray[i][1];
			m_SceneResoucres.mgrTexture.InsertObject(pTexture, ObjName);
			m_SceneResoucres.mgrMesh.InsertObject(pMesh, ObjName);
		}
		else
		{
			string ObjName = ITEMMgr.StaffNameArray[0][2];
			m_SceneResoucres.mgrTexture.InsertObject(pTexture, ObjName);
			m_SceneResoucres.mgrMesh.InsertObject(pMesh, ObjName);
		}
	}
	// 포탈
	{
		pTexture = new CTexture(3, 1, 0, 0);

		ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Objects/Portal/portal_diffuse.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr));
		pTexture->SetTexture(0, pd3dsrvTexture);
		pd3dsrvTexture->Release();

		ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Objects/Portal/portal_normals.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr));
		pTexture->SetTexture(1, pd3dsrvTexture);
		pd3dsrvTexture->Release();

		ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Objects/Portal/portal_specular.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr));
		pTexture->SetTexture(2, pd3dsrvTexture);
		pd3dsrvTexture->Release();

		//ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Objects/Portal/portal_glow.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr));
		//pTexture->SetTexture(3, pd3dsrvTexture);
		//pd3dsrvTexture->Release();

		pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));

		pMesh = new CLoadMeshByFbxcjh(pd3dDevice, ("../Assets/Image/Objects/Portal/portal.fbxcjh"), 0.5f, vcTxFileNames);
		vcTxFileNames.clear();

		m_SceneResoucres.mgrTexture.InsertObject(pTexture, "scene_portal");
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_portal");
	}

	// 플레이어 캐릭터
	{
		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_idle_01.ad", 0.1f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_idle");
		pTexture = CTextureMgr::MakeFbxcjhTextures(pd3dDevice, baseDir + _T("Aure/"), vcTxFileNames, 0);
		vcTxFileNames.clear();
		m_SceneResoucres.mgrTexture.InsertObject(pTexture, "scene_aure");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_Run_Forward.ad", 0.1f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_run_forwad");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_Run_Back.ad", 0.1f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_walk_Back");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_Run_Right.ad", 0.1f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_walk_right");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_Run_Left.ad", 0.1f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_walk_left");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_1H_Magic_Attack01.ad", 0.1f, vcTxFileNames, 5);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_magic_attack01");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_1H_Cast_01.ad", 0.1f, vcTxFileNames, 0, 15);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_magic_cast01");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_1H_Magic_Area01.ad", 0.1f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_magic_area01");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_Block_Start.ad", 0.1f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_block_start");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_Block_Idle.ad", 0.1f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_block_idle");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_Block_End.ad", 0.1f, vcTxFileNames, 5);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_block_end");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_Damaged_Forward01.ad", 0.1f, vcTxFileNames, 14);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_damaged_f01");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_Damaged_Forward02.ad", 0.1f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_damaged_f02");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Aure/Aure_Death_Front.ad", 0.1f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_aure_death_f");

		vcTxFileNames.clear();
	}
	// 워록 몬스터
	{
  		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Warrok/Warrok_Idle.ad", 0.5f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_warrok_idle");
		
		pTexture = CTextureMgr::MakeFbxcjhTextures(pd3dDevice, baseDir + _T("Warrok/"), vcTxFileNames, 0);
		vcTxFileNames.clear();
		m_SceneResoucres.mgrTexture.InsertObject(pTexture, "scene_warrok");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Warrok/Warrok_Run.ad", 0.5f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_warrok_run");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Warrok/Warrok_Roar.ad", 0.5f, vcTxFileNames);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_warrok_roar");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Warrok/Warrok_Punch.ad", 0.5f, vcTxFileNames, 2);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_warrok_punch");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Warrok/Warrok_Swiping.ad", 0.5f, vcTxFileNames, 2);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_warrok_swiping");

		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Warrok/Warrok_Death.ad", 0.5f, vcTxFileNames, 5);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_warrok_death");
		vcTxFileNames.clear();
	}

	//{
	//	pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Man_Death_0.ad", 0.13f, vcTxFileNames);
	//	pTexture = CTextureMgr::MakeFbxcjhTextures(pd3dDevice, baseDir, vcTxFileNames, 0);
	//	vcTxFileNames.clear();

	//	m_SceneResoucres.mgrTexture.InsertObject(pTexture, "scene_man_death");
	//	m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_man_death");
	//}
	//{
	//	pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Player_0.ad", 0.13f, vcTxFileNames);
	//	pTexture = CTextureMgr::MakeFbxcjhTextures(pd3dDevice, baseDir, vcTxFileNames, 0);
	//	vcTxFileNames.clear();

	//	m_SceneResoucres.mgrTexture.InsertObject(pTexture, "scene_player_0");
	//	m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_player_0");
	//}
	{
		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Skull/skeleton_0.ad", 3.5f, vcTxFileNames);
		pTexture = new CTexture(1, 0, 0, 0);
		ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Objects/Skull/Skull.png"), nullptr, nullptr, &pd3dsrvTexture, nullptr));
		pTexture->SetTexture(0, pd3dsrvTexture);
		pd3dsrvTexture->Release();
		vcTxFileNames.clear();

		m_SceneResoucres.mgrTexture.InsertObject(pTexture, "scene_skull");
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_skull_0");
	}
}

void CSceneInGame::BuildObjects(ID3D11Device *pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext, ShaderBuildInfo * SceneInfo)
{
	SYSTEMMgr.SetScene(this);

	m_nMRT     = NUM_MRT;
	//재질을 생성한다.
	CMaterial *pRedMaterial   = MaterialMgr.GetObjects("Red");
	CMaterial *pGreenMaterial = MaterialMgr.GetObjects("Green");
	CMaterial *pBlueMaterial  = MaterialMgr.GetObjects("Blue");
	CMaterial *pWhiteMaterial = MaterialMgr.GetObjects("White");

	int iCharacterShaderNum = -1;
	//메시 빌드
	BuildMeshes(pd3dDevice);
	{
		UINT index = 0;
		m_nShaders = NUM_SHADER;
		m_nThread = m_nShaders;
		m_ppShaders = new CShader*[m_nShaders];

		//스카이박스와 터레인 셰이더
		m_ppShaders[index] = new CEnviromentShader();
		m_vcStaticShadowShaders.push_back(m_ppShaders[index]);
		m_ppShaders[index++]->BuildObjects(pd3dDevice);

		iCharacterShaderNum = index;
		CCharacterShader *pCharShader = new CCharacterShader();
		pCharShader->CreateShader(pd3dDevice);
		pCharShader->BuildObjects(pd3dDevice, pWhiteMaterial, m_SceneResoucres);
		m_ppShaders[index++] = pCharShader;
		
		m_vcResetShaders.push_back(pCharShader);
		m_vcDynamicShadowShaders.push_back(pCharShader);

		CStaticModelingShader *pStaticShader = new CStaticModelingShader();
		pStaticShader->CreateShader(pd3dDevice);
		pStaticShader->BuildObjects(pd3dDevice, pWhiteMaterial, m_SceneResoucres);
		m_ppShaders[index++] = pStaticShader;
		
		m_vcStaticShadowShaders.push_back(pStaticShader);

		CStaticInstancingParentShader * pStInst = new CStaticInstancingParentShader();
		pStInst->CreateShader(pd3dDevice);
		pStInst->BuildObjects(pd3dDevice, nullptr, m_SceneResoucres);
		m_ppShaders[index++] = pStInst;
		
		m_vcStaticShadowShaders.push_back(pStInst);

		CPointInstanceShader *pPointShader = new CPointInstanceShader();
		pPointShader->CreateShader(pd3dDevice);
		pPointShader->BuildObjects(pd3dDevice, pWhiteMaterial);
		m_ppShaders[index++] = pPointShader;
		
		m_vcResetShaders.push_back(pPointShader);

		m_nEffectShaderNum = index;
		CEffectShader * pTxAni = new CEffectShader();
		pTxAni->CreateShader(pd3dDevice);
		pTxAni->BuildObjects(pd3dDevice);
		m_nEffectShaderNum = index;
		m_ppShaders[index++] = pTxAni;

		CSceneShader * pSceneShader = new CSceneShader();
		pSceneShader->CreateShader(pd3dDevice);
		pSceneShader->BuildObjects(pd3dDevice, SceneInfo->ppMRTSRVArray, 0, SceneInfo->pd3dBackRTV);
		pSceneShader->CreateConstantBuffer(pd3dDevice, pd3dDeviceContext);
		m_pSceneShader = pSceneShader;
	}
	{
		m_pPlayerShader = new CPlayerShader();
		m_pPlayerShader->CreateShader(pd3dDevice);
		m_pPlayerShader->BuildObjects(pd3dDevice, m_SceneResoucres, this);

		m_vcResetShaders.push_back(m_pPlayerShader);
		m_vcDynamicShadowShaders.push_back(m_pPlayerShader);

		SetCamera(m_pPlayerShader->GetPlayer()->GetCamera());
		CLIENT.SetPlayerShader(m_pPlayerShader);
		////////////// 플레이어 변경은 이 Build 끝나기 전에 해라//////////////////////////////////////
		InitializeRecv();
	/*	cs_packet_vector my_packet2;
		my_packet2.size = sizeof(cs_packet_vector);
		my_packet2.type = CS_INPUT;
		my_packet2.LookVector = m_pPlayerShader->GetPlayer(CLIENT.GetClientID())->GetLookVector();
		my_packet2.RightVector = m_pPlayerShader->GetPlayer(CLIENT.GetClientID())->GetRightVector();
		CLIENT.SendPacket(reinterpret_cast<unsigned  char*>(&my_packet2));*/
		cout << "Current Client ID : " << CLIENT.GetClientID() << endl;
		ChangeGamePlayerID(CLIENT.GetClientID());
		////////////// 플레이어 변경은 이 Build 끝나기 전에 해라//////////////////////////////////////

		m_pCamera->SetViewport(pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->GenerateViewMatrix();

		m_ppShaders[iCharacterShaderNum]->GetGameMessage(nullptr, eMessage::MSG_PASS_PLAYERPTR, m_pCamera->GetPlayer());
	}
	{
		CInGameUIShader * pUIShader = new CInGameUIShader();
		pUIShader->CreateShader(pd3dDevice);
		pUIShader->BuildObjects(pd3dDevice, SceneInfo->pd3dBackRTV, this);
		m_pUIShader = pUIShader;
	}

	CreateShaderVariables(pd3dDevice);
	BuildStaticShadowMap(pd3dDeviceContext);

	SYSTEMMgr.GameReady();
	Reset();
	//SYSTEMMgr.GameStart();
}

void CSceneInGame::ReleaseObjects()
{
	CScene::ReleaseObjects();
	QUADMgr.ReleaseQuadTree();
	SYSTEMMgr.ReleaseScene();
}

void CSceneInGame::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));
	//게임 월드 전체를 비추는 주변조명을 설정한다.
	m_pLights->m_xcGlobalAmbient             = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);

	//3개의 조명(점 광원, 스팟 광원, 방향성 광원)을 설정한다.
#if 0
	m_pLights->m_pLights[0].m_bEnable        = 1.0f;
	m_pLights->m_pLights[0].m_nType          = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[0].m_xcAmbient      = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[0].m_xcDiffuse      = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights->m_pLights[0].m_xcSpecular     = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xv3Direction   = XMFLOAT3(-0.707f, -0.707f, 0.0f);
#endif
	m_pLights->m_pLights[0].m_bEnable        = 1.0f;
	m_pLights->m_pLights[0].m_nType          = SPOT_LIGHT;
	m_pLights->m_pLights[0].m_fRange         = 100.0f;
	m_pLights->m_pLights[0].m_xcAmbient      = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[0].m_xcDiffuse      = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[0].m_xcSpecular     = XMFLOAT4(0.3f, 0.3f, 0.3f, 5.0f);
	m_pLights->m_pLights[0].m_xv3Position    = XMFLOAT3(500.0f, 300.0f, 500.0f);
	m_pLights->m_pLights[0].m_xv3Direction   = XMFLOAT3(0.0f, -1.f, 0.0f);
	m_pLights->m_pLights[0].m_xv3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[0].m_fFalloff       = 8.0f;
	m_pLights->m_pLights[0].m_fPhi           = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[0].m_fTheta         = (float)cos(XMConvertToRadians(20.0f));

	XMFLOAT3 pos = XMFLOAT3(1024, 0, 320);
	pos.y = MAPMgr.GetHeight(pos) + 10;

	m_pLights->m_pLights[1].m_bEnable        = 1.0f;//1.0f;
	m_pLights->m_pLights[1].m_nType          = POINT_LIGHT;
	m_pLights->m_pLights[1].m_fRange         = 20.0f;
	m_pLights->m_pLights[1].m_xcAmbient      = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xcDiffuse      = XMFLOAT4(0.7f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xcSpecular     = XMFLOAT4(0.3f, 0.0f, 0.0f, 0.1f);
	m_pLights->m_pLights[1].m_xv3Position    = pos;
	m_pLights->m_pLights[1].m_xv3Direction   = XMFLOAT3(0.0f, -1.f, 0.0f);
	m_pLights->m_pLights[1].m_xv3Attenuation = XMFLOAT3(1.0f, 0.05f, 0.001f);

	pos = XMFLOAT3(1074, 0, 320);
	pos.y = MAPMgr.GetHeight(pos) + 10;

	m_pLights->m_pLights[2].m_bEnable        = 1.0f;//1.0f;
	m_pLights->m_pLights[2].m_nType          = POINT_LIGHT;
	m_pLights->m_pLights[2].m_fRange         = 20.0f;
	m_pLights->m_pLights[2].m_xcAmbient      = XMFLOAT4(0.0f, 0.0f, 0.4f, 1.0f);
	m_pLights->m_pLights[2].m_xcDiffuse      = XMFLOAT4(0.1f, 0.1f, 0.2f, 1.0f);
	m_pLights->m_pLights[2].m_xcSpecular     = XMFLOAT4(0.0f, 0.0f, 0.1f, 0.1f);
	m_pLights->m_pLights[2].m_xv3Position    = pos;
	m_pLights->m_pLights[2].m_xv3Direction   = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_pLights->m_pLights[2].m_xv3Attenuation = XMFLOAT3(1.0f, 0.05f, 0.001f);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(d3dBufferDesc));
	d3dBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth      = sizeof(LIGHTS);
	d3dBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem        = m_pLights;
	if (!m_pd3dcbLights) pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dcbLights);
}

void CSceneInGame::ReleaseShaderVariables()
{
	if (m_pLights)		delete m_pLights;
	if (m_pd3dcbLights) m_pd3dcbLights->Release();
}

void CSceneInGame::BuildStaticShadowMap(ID3D11DeviceContext * pd3dDeviceContext)
{
	// 방향성 광원은 위치 필요 없다.
	//LIGHT * pLight = m_pScene->GetLight(2);
	CMapManager * pTerrain = &MAPMgr;
	float fHalf = MAPMgr.GetWidth() * 0.5f;//pTerrain->GetWidth() * 0.5;

	CShadowMgr * pSdwMgr = &ShadowMgr;
	pSdwMgr->BuildShadowMap(pd3dDeviceContext, XMFLOAT3(fHalf, 0.0f, fHalf), XMFLOAT3(fHalf + 80.f, 80.f, fHalf), fHalf);

	const UINT uRenderState = (RS_SHADOWMAP);
	pSdwMgr->SetStaticShadowMap(pd3dDeviceContext, m_pCamera);

	for(auto & shader : m_vcStaticShadowShaders)
		shader->Render(pd3dDeviceContext, uRenderState, m_pCamera);

	pSdwMgr->ResetStaticShadowMap(pd3dDeviceContext, m_pCamera);
	pSdwMgr->UpdateStaticShadowResource(pd3dDeviceContext);
}

void CSceneInGame::PreProcessing(ID3D11DeviceContext * pd3dDeviceContext)
{
	UINT uRenderState = (NOT_PSUPDATE | RS_SHADOWMAP | DRAW_AND_ACTIVE);

	//CHeightMapTerrain * pTerrain = GetTerrain();
	float fHalf = 100.0f;//pTerrain->GetWidth() * 0.3;
	XMFLOAT3 xmfTarget = m_pCamera->GetPlayer()->GetPosition();
	XMFLOAT3 xmfLight = xmfTarget;
	
	xmfLight.x += 20.0f;
	xmfLight.y += 20.0f;

	CShadowMgr * pSdwMgr = &ShadowMgr;
	pSdwMgr->BuildShadowMap(pd3dDeviceContext, xmfTarget, xmfLight, fHalf);
	pSdwMgr->SetDynamicShadowMap(pd3dDeviceContext, m_pCamera);

	m_pPlayerShader->Render(pd3dDeviceContext, uRenderState, m_pCamera);
	m_ppShaders[1]->Render(pd3dDeviceContext, uRenderState, m_pCamera);
	m_ppShaders[2]->Render(pd3dDeviceContext, uRenderState, m_pCamera);

	pSdwMgr->ResetDynamicShadowMap(pd3dDeviceContext, m_pCamera);
	pSdwMgr->UpdateDynamicShadowResource(pd3dDeviceContext);

	uRenderState = NULL;
}

void CSceneInGame::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbLights, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	LIGHTS *pcbLight = (LIGHTS *)d3dMappedResource.pData;
	memcpy(pcbLight, pLights, sizeof(LIGHTS));
	pd3dDeviceContext->Unmap(m_pd3dcbLights, 0);
	pd3dDeviceContext->PSSetConstantBuffers(CB_PS_SLOT_LIGHT, 1, &m_pd3dcbLights);
}

bool CSceneInGame::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	CInGamePlayer * pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer());

	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'B' :
		case 'X':
		case 'C':
			((CEffectShader*)m_ppShaders[m_nEffectShaderNum])->ShaderKeyEventOn(m_pPlayerShader->GetPlayer(), wParam, nullptr);
			return(false);

		case 'P':
			//pPlayer->ChangeAnimationState(eANI_DEATH_FRONT, true, nullptr, 0);
			return false;

		//case 'Z':
		//	this->GetGameMessage(nullptr, eMessage::MSG_ROUND_END);
			//SYSTEMMgr.RoundEnd();
			return false;

		case 'G':
		case 'E':
		//case 'N':
		case 'M':
			pPlayer->PlayerKeyEventOn(wParam, this);
			return(false);
		}
		return(false);

	case WM_KEYUP:
		switch (wParam)
		{
#if 0
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
#endif

		case '0':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':

		case 'W' :
		case 'S' :
		case 'A' :
		case 'D' :
		case 'G':
		case 'J':
		case 'K':
			pPlayer->PlayerKeyEventOff(wParam, this);
			return(false);
		}
	}
	return(false);
}

bool CSceneInGame::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (CScene::OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam)) return true;

	switch (nMessageID)
	{
	case WM_RBUTTONUP:
		CInGamePlayer * pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer());
		int result = pPlayer->UseMagic();
		if (0 < result)
		{
			// 쏜 상태를 전송
			cs_packet_Behavior behavior_packet;
			behavior_packet.size = sizeof(cs_packet_Behavior);
			behavior_packet.type = CS_MAGIC_CASTING;
			CLIENT.GetWSASendBuffer().buf = reinterpret_cast<CHAR*>(CLIENT.GetUSendBuffer());
			CLIENT.GetWSASendBuffer().len = sizeof(cs_packet_Behavior);
			memcpy(CLIENT.GetUSendBuffer(), reinterpret_cast<UCHAR*>(&behavior_packet), sizeof(cs_packet_Behavior));
			DWORD ioBytes;
			int ret = WSASend(CLIENT.GetClientSocket(), &CLIENT.GetWSASendBuffer(), 1, &ioBytes, 0, NULL, NULL);
			if (ret)
			{
				int error_code = WSAGetLastError();
				if (WSA_IO_PENDING != error_code)
				{
					CLIENT.error_display(__FUNCTION__ " SC_PUT_PLAYER:WSASend", error_code);
				}
			}
		//	pPlayer->MagicShot();
		}
		else if (0 == result)
		{
			m_pUIShader->GetGameMessage(nullptr, eMessage::MSG_UI_DRAW_NEED_ELEMENT);
		}
		return true;
	}
	return false;
}

bool CSceneInGame::ProcessInput(HWND hWnd, float fFrameTime, POINT & pt)
{
	static UCHAR pKeyBuffer[256];
	//GetKeyboardState(pKeyBuffer);
	float cxDelta = 0.0f, cyDelta = 0.0f;
	/*마우스를 캡쳐했으면 마우스가 얼마만큼 이동하였는 가를 계산한다. 마우스 왼쪽 또는 오른쪽 버튼이 눌러질 때의 메시지(WM_LBUTTONDOWN, WM_RBUTTONDOWN)를 처리할 때 마우스를 캡쳐하였다. 그러므로 마우스가 캡쳐된 것은 마우스 버튼이 눌려진 상태를 의미한다. 마우스를 좌우 또는 상하로 움직이면 플레이어를 x-축 또는 y-축으로 회전한다.*/
	if (GetCapture() == hWnd)
	{
		//현재 마우스 커서의 위치를 가져온다.
		//GetCursorPos(&ptCursorPos);
		//마우스 버튼이 눌린 채로 이전 위치에서 현재 마우스 커서의 위치까지 움직인 양을 구한다.
		cxDelta = (float)(pt.x - m_ptOldCursorPos.x) * 0.5f;
		cyDelta = (float)(pt.y - m_ptOldCursorPos.y) * 0.5f;

		//ClientToScreen(hWnd, &m_ptOldCursorPos);
		SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		//m_pUIShader->GetGameMessage(nullptr, eMessage::MSG_MOUSE_DOWN_OVER, nullptr);
	}
	//else
		//m_pUIShader->GetGameMessage(nullptr, eMessage::MSG_MOUSE_UP_OVER, nullptr);
	CPlayer * pPlayer = m_pCamera->GetPlayer();
	//cout << "Num : " << pPlayer->GetPlayerNum() << endl;
	{
		DWORD dwDirection = 0;
		/*키보드의 상태 정보를 반환한다. 화살표 키(‘→’, ‘←’, ‘↑’, ‘↓’)를 누르면 플레이어를 오른쪽/왼쪽(로컬 x-축), 앞/뒤(로컬 z-축)로 이동한다. ‘Page Up’과 ‘Page Down’ 키를 누르면 플레이어를 위/아래(로컬 y-축)로 이동한다.*/
		if (GetKeyboardState(pKeyBuffer))
		{
#if 0
			if (pKeyBuffer[VK_UP]    & 0xF0) dwDirection |= DIR_FORWARD;
			if (pKeyBuffer[VK_DOWN]  & 0xF0) dwDirection |= DIR_BACKWARD;
			if (pKeyBuffer[VK_LEFT]  & 0xF0) dwDirection |= DIR_LEFT;
			if (pKeyBuffer[VK_RIGHT] & 0xF0) dwDirection |= DIR_RIGHT;
			//if (pKeyBuffer['W'] & 0x0F) cout << "WF!!";
			//if (pKeyBuffer['W'] & 0xF0) cout << "FW!!";
#endif
			if (pKeyBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
			if (pKeyBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
			if (pKeyBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
			if (pKeyBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;
		}
		//플레이어를 이동하거나(dwDirection) 회전한다(cxDelta 또는 cyDelta).
		const float fToTalDelta = cxDelta + cyDelta;
		if ( dwDirection || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (fToTalDelta)
			{
#if 0
				/*cxDelta는 y-축의 회전을 나타내고 cyDelta는 x-축의 회전을 나타낸다. 오른쪽 마우스 버튼이 눌려진 경우 cxDelta는 z-축의 회전을 나타낸다.*/
				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
				{
					if (pPlayer->GetStatus().IsAlive())
						pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
					else 
						m_pCamera->Rotate(cyDelta, 0.0f, -cxDelta);
				}
#endif
				if (!(pKeyBuffer[VK_RBUTTON] & 0xF0))
				{
					if (pPlayer->GetStatus().IsAlive())
					{
						pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
						DWORD iobyte;
						cs_packet_rotate rotate_packet;
						rotate_packet.size = sizeof(cs_packet_rotate);
						rotate_packet.type = CS_ROTATION;
						rotate_packet.cxDelta = cxDelta;
						rotate_packet.cyDelta = cyDelta;
						rotate_packet.LookVector = pPlayer->GetLookVector();
						CLIENT.GetWSASendBuffer().buf = reinterpret_cast<CHAR*>(CLIENT.GetUSendBuffer());
						CLIENT.GetWSASendBuffer().len = sizeof(cs_packet_rotate);
						memcpy(CLIENT.GetUSendBuffer(), reinterpret_cast<UCHAR*>(&rotate_packet), sizeof(cs_packet_rotate));
						DWORD ioBytes;
						int ret = WSASend(CLIENT.GetClientSocket(), &CLIENT.GetWSASendBuffer(), 1, &ioBytes, 0, NULL, NULL);
						if (ret)
						{
							int error_code = WSAGetLastError();
							if (WSA_IO_PENDING != error_code)
							{
								CLIENT.error_display(__FUNCTION__ " SC_PUT_PLAYER:WSASend", error_code);
							}
						}
						//CLIENT.SendPacket(reinterpret_cast<UCHAR*>(&rotate_packet));
					}
					else 
						m_pCamera->Rotate(cyDelta, cxDelta, 0.0f);
				}
			}
			/*플레이어를 dwDirection 방향으로 이동한다(실제로는 속도 벡터를 변경한다). 이동 거리는 시간에 비례하도록 한다. 플레이어의 이동 속력은 (50/초)로 가정한다. 만약 플레이어의 이동 속력이 있다면 그 값을 사용한다.*/
#if 0
			if (dwDirection != pPlayer->GetDirection())
			{
				pPlayer->SetDirection(dwDirection);//pPlayer->Move(dwDirection, 50.0f * fFrameTime, true);
				cs_packet_move_test packet;
				packet.direction = pPlayer->GetDirection();
				packet.pos = pPlayer->GetPosition();
			}
#endif
		}
		//플레이어를 실제로 이동하고 카메라를 갱신한다. 중력과 마찰력의 영향을 속도 벡터에 적용한다.
		//pPlayer->Update(fFrameTime);
		if (dwDirection != pPlayer->GetDirection())
		{
			DWORD iobyte;
			cs_packet_move_test movetest;
			movetest.size = sizeof(cs_packet_move_test);
			movetest.type = CS_MOVE;
			movetest.direction = dwDirection;
			movetest.Position = pPlayer->GetPosition();
			movetest.LookVector = pPlayer->GetLookVector();

			//cout << movetest.Position << endl;
			movetest.animation = pPlayer->GetAnimationState();//wdNextState;
		//	CLIENT.SendPacket(reinterpret_cast<UCHAR*>(&movetest));
			CLIENT.GetWSASendBuffer().buf = reinterpret_cast<CHAR*>(CLIENT.GetUSendBuffer());
			CLIENT.GetWSASendBuffer().len = sizeof(cs_packet_move_test);
			memcpy(CLIENT.GetUSendBuffer(), reinterpret_cast<UCHAR*>(&movetest), sizeof(cs_packet_move_test));
			DWORD ioBytes;
			int ret = WSASend(CLIENT.GetClientSocket(), &CLIENT.GetWSASendBuffer(), 1, &ioBytes, 0, NULL, NULL);
			if (ret)
			{
				int error_code = WSAGetLastError();
				if (WSA_IO_PENDING != error_code)
				{
					CLIENT.error_display(__FUNCTION__ " SC_PUT_PLAYER:WSASend", error_code);
				}
			}
			//int ret = WSASend(CLIENT.GetClientSocket(), &CLIENT.GetWSASendBuffer(), 1, &iobyte, 0, NULL, NULL);
		}
		pPlayer->SetDirection(dwDirection);
//		cout << "갱신 : " << dwDirection;
	}
	if (pKeyBuffer[VK_SPACE] & 0xF0)
	{
		FRAMEWORK.ChangeGameScene(new CSceneTitle());

		ZeroMemory(pKeyBuffer, 256);
		SetKeyboardState(pKeyBuffer);
	}
	return false;
}

void CSceneInGame::LightUpdate(float fTimeElapsed)
{
	if (m_pLights && m_pd3dcbLights)
	{
		//현재 카메라의 위치 벡터를 조명을 나타내는 상수 버퍼에 설정한다.
		XMFLOAT3 xv3CameraPosition = m_pCamera->GetPosition();
		m_pLights->m_xv4CameraPosition = XMFLOAT4(xv3CameraPosition.x, xv3CameraPosition.y, xv3CameraPosition.z, 1.0f);

		/*두 번째 조명은 플레이어가 가지고 있는 손전등(스팟 조명)이다. 그러므로 플레이어의 위치와 방향이 바뀌면 현재 플레이어의 위치와 z-축 방향 벡터를 스팟 조명의 위치와 방향으로 설정한다.*/
		CPlayer *pPlayer = m_pCamera->GetPlayer();
		m_pLights->m_pLights[0].m_xv3Position = pPlayer->GetPosition();
		m_pLights->m_pLights[0].m_xv3Direction = pPlayer->GetLookVector();

		//XMFLOAT3 pos = m_pPlayerShader->GetPlayer(0)->GetPosition();
		//pos.y += 20.f;
		//m_pLights->m_pLights[2].m_xv3Position = pos;
		//pos = m_pPlayerShader->GetPlayer(1)->GetPosition();
		//pos.y += 20.f;
		//m_pLights->m_pLights[3].m_xv3Position = pos;
	}

}

void CSceneInGame::AnimateObjects(float fTimeElapsed)
{
	LightUpdate(fTimeElapsed);

	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	}
	m_pPlayerShader->AnimateObjects(fTimeElapsed);
	m_pUIShader->AnimateObjects(fTimeElapsed);

#ifdef _QUAD_TREE
	QUADMgr.Update(m_pCamera);
#endif
	EVENTMgr.Update(fTimeElapsed);
	SYSTEMMgr.Update(fTimeElapsed);
}

void CSceneInGame::Render(ID3D11DeviceContext*pd3dDeviceContext, RENDER_INFO * pRenderInfo)
{
#ifdef _THREAD
	int index = pRenderInfo->ThreadID;

	if (index == m_nShaders - 1)
	{
		m_pPlayerShader->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
	}
//#ifndef _DEBUG
	//if (index == m_nThread - 1)
	//{
	//	UpdateLights(pd3dDeviceContext);
	//	pd3dDeviceContext->OMSetRenderTargets(1, &(pRenderInfo->ppd3dMrtRTV[MRT_SCENE]), nullptr);
	//	ShadowMgr.UpdateStaticShadowResource(pd3dDeviceContext);
	//	m_pSceneShader->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
	//	return;
	//}
//#endif
	m_ppShaders[index]->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
#else
	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
	}
#endif
	//m_ppShaders[m_nShaders - 1]->Render(pd3dDeviceContext, pCamera);
}

void CSceneInGame::UIRender(ID3D11DeviceContext * pd3dDeviceContext)
{
	m_pUIShader->Render(pd3dDeviceContext, DRAW_AND_ACTIVE, m_pCamera);
}

#ifdef PICKING
CGameObject *CScene::PickObjectPointedByCursor(int xClient, int yClient)
{
	if (!m_pCamera) return(nullptr);

	XMFLOAT4X4 xmtxView = m_pCamera->GetViewMatrix();
	XMFLOAT4X4 xmtxProjection = m_pCamera->GetProjectionMatrix();
	D3D11_VIEWPORT d3dViewport = m_pCamera->GetViewport();

	XMFLOAT3 xv3PickPosition;
	/*화면 좌표계의 점 (xClient, yClient)를 화면 좌표 변환의 역변환과 투영 변환의 역변환을 한다. 그 결과는 카메라 좌표계의 점이다. 투영 평면이 카메라에서 z-축으로 거리가 1이므로 z-좌표는 1로 설정한다.*/
	xv3PickPosition.x = (((2.0f * xClient) / d3dViewport.Width) - 1) / xmtxProjection._11;
	xv3PickPosition.y = -(((2.0f * yClient) / d3dViewport.Height) - 1) / xmtxProjection._22;
	xv3PickPosition.z = 1.0f;

	int nIntersected = 0;
	float fNearHitDistance = FLT_MAX;
	MESHINTERSECTINFO d3dxIntersectInfo;
	CGameObject *pIntersectedObject = nullptr, *pNearestObject = nullptr;
	//씬의 모든 쉐이더 객체에 대하여 픽킹을 처리하여 카메라와 가장 가까운 픽킹된 객체를 찾는다.
	for (int i = 0; i < m_nShaders; i++)
	{
		pIntersectedObject = m_ppShaders[i]->PickObjectByRayIntersection(&xv3PickPosition, &xmtxView, &d3dxIntersectInfo);
		if (pIntersectedObject && (d3dxIntersectInfo.m_fDistance < fNearHitDistance))
		{
			fNearHitDistance = d3dxIntersectInfo.m_fDistance;
			pNearestObject = pIntersectedObject;
		}
	}
	return(pNearestObject);
}
CHeightMapTerrain *CSceneInGame::GetTerrain()
{
	CTerrainShader *pTerrainShader = (CTerrainShader *)m_ppShaders[m_uHeightMapIndex];
	return(pTerrainShader->GetTerrain());
}
#endif

void CSceneInGame::GetGameMessage(CScene * byObj, eMessage eMSG, void * extra)
{
	//CInGamePlayer * pPlayer = static_cast<CInGamePlayer*>(extra);

	switch (eMSG)
	{
	case eMessage::MSG_ITEM_STAFF_CHANGE:
		static_cast<CInGameUIShader*>(m_pUIShader)->ChangeItemUI(static_cast<CStaff*>(extra));
		return;
	
	case eMessage::MSG_MAGIC_CAST:
	case eMessage::MSG_PARTICLE_ON:
	case eMessage::MSG_MAGIC_SHOT:
	case eMessage::MSG_MAGIC_AREA:
		m_ppShaders[m_nEffectShaderNum]->GetGameMessage(nullptr, eMSG, extra);
		return;
	
	case eMessage::MSG_MOUSE_DOWN:
		m_ptOldCursorPos = *(POINT*)extra;
	case eMessage::MSG_MOUSE_DOWN_OVER:
		m_pUIShader->GetGameMessage(nullptr, eMSG, extra);
		return;

	case eMessage::MSG_MOUSE_UP:
		m_pUIShader->GetGameMessage(nullptr, eMSG, extra);
	case eMessage::MSG_MOUSE_UP_OVER:
		return;

	case eMessage::MSG_EFFECT_RADIAL_ON :
	case eMessage::MSG_EFFECT_RADIAL_OFF:
	case eMessage::MSG_EFFECT_GLARE_ON  :
	case eMessage::MSG_EFFECT_GLARE_OFF :
		m_pSceneShader->GetGameMessage(nullptr, eMSG);
		return;

	case eMessage::MSG_ROUND_ENTER:
		SYSTEMMgr.RoundEnter();
		Reset();
		return;

	case eMessage::MSG_ROUND_START:
		SYSTEMMgr.RoundStart();
		m_pPlayerShader->RoundStart();
		return;

	case eMessage::MSG_ROUND_END:
		m_pPlayerShader->RoundEnd();
		SYSTEMMgr.RoundEnd();

		if (SYSTEMMgr.IsWinPlayer(static_cast<CInGamePlayer*>(m_pCamera->GetPlayer())))
			static_cast<CInGameUIShader*>(m_pUIShader)->UIReadyWinLogo(true);
		else
			static_cast<CInGameUIShader*>(m_pUIShader)->UIReadyLoseLogo(true);
		return;

	case eMessage::MSG_ROUND_CLEAR:
		static_cast<CInGameUIShader*>(m_pUIShader)->UIReadyLoseLogo(false);
		SYSTEMMgr.RoundClear();
		return;

	case eMessage::MSG_GAME_END:
		CLIENT.CloseConnect();
		FRAMEWORK.ChangeGameScene(new CSceneTitle());
		return;
	}
}

void CSceneInGame::SendGameMessage(CScene * toObj, eMessage eMSG, void * extra)
{
}

bool CSceneInGame::PacketProcess(LPARAM lParam)
{
	CLIENT.ReadPacket();
	//static const int SC_PUT_PLAYER = 1;
	static bool bFirstTime = true;
//	static int mId = -1;
	char * buffer = CLIENT.GetRecvBuffer();
	CInGamePlayer * pPlayer = nullptr;
	switch (buffer[1])
	{
	case SC_PUT_PLAYER:
	{	//sc_packet_put_player * my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
		sc_packet_put_player * my_packet = reinterpret_cast<sc_packet_put_player *>(buffer);
		int id = my_packet->id;

		if (bFirstTime)
		{
			bFirstTime = false;
		}
		if (id != CLIENT.GetClientID())
		{
			//m_pPlayerShader->SetPlayerID(FRAMEWORK.GetDevice(), id);
			//	m_pPlayerShader->GetPlayer(id);
			pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(id));
			//pPlayer->SetPlayerNum(id);
			pPlayer->InitPosition(XMFLOAT3(my_packet->x, my_packet->y, my_packet->z));
			pPlayer->SetVisible(true);

			//pPlayer->SetPosition(XMFLOAT3(my_packet->x, my_packet->y, my_packet->z));
			//pPlayer->SetPosition();
			//pPlayer->SetActive(true);
		}
		//pPlayer->SetActive(true);
		break;
	}
	case SC_POS:
	{
		/*sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(buffer);*/
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(buffer);
		
		int other_id = my_packet->id;
		
		
		if (other_id == CLIENT.GetClientID()) {
			pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(CLIENT.GetClientID()));
			if (pPlayer != NULL)
			{

				//pPlayer->ChangeAnimationState(my_packet->animationType, false, nullptr, 0);
				//pPlayer->Move(my_packet->Shift, true);
					pPlayer->SetPosition(my_packet->Position);
					
			}

		}
		if (other_id != CLIENT.GetClientID())
		{
			pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(other_id));
			if (pPlayer != NULL)
			{
				pPlayer->RenewPacket(*my_packet);
				if (my_packet->animationType == eANI_IDLE)
					pPlayer->ChangeAnimationState(eANI_IDLE, false, nullptr, 0);
				//pPlayer->SetDirection(my_packet->dwDirection);
				//pPlayer->SetPosition(my_packet->Position);
				//pPlayer->SetLo
			
				//	pPlayer->ChangeAnimationState(my_packet->animationType, false, nullptr, 0);
				//pPlayer->Move(my_packet->Shift, true);
			//		pPlayer->SetPosition(my_packet->Position);
			}
		}

		cout << "Packet Pos : " << my_packet->Position << endl;

		break;
	}
	case SC_ROUND_TIME:
	{
		/*sc_packet_RoundTime *my_packet = reinterpret_cast<sc_packet_RoundTime *>(buffer);
		cout << "count: " << my_packet->time << endl;*/

		break;
	}
	case SC_GAME_STATE:
	{
		sc_packet_GameState* my_packet = reinterpret_cast<sc_packet_GameState*>(buffer);
		my_packet->gamestate;



		break;
	}
	case SC_PLAYER_INFO:
	{
		sc_packet_playerInfo* my_packet = reinterpret_cast<sc_packet_playerInfo*>(buffer);
		pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(my_packet->id));
		pPlayer->GetStatus().SetHP(my_packet->HP);
		break;
	}
	case  SC_ROTATION:
	{

		sc_packet_rotate* my_packet = reinterpret_cast<sc_packet_rotate*>(buffer);
		pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(my_packet->id));
		pPlayer->RenewPacket(*my_packet);
		//pPlayer->Rotate(my_packet->cyDelta, my_packet->cxDelta, 0.0f);
		break;
	}
	case SC_DOMINATE:
	{
		sc_packet_dominate* my_packet = reinterpret_cast<sc_packet_dominate*>(buffer);
		SYSTEMMgr.DominatePortalGate(my_packet->id);


		break;
	}
	case SC_MAGIC_CASTING:
	{
		sc_packet_Behavior* my_packet = reinterpret_cast<sc_packet_Behavior*>(buffer);
		pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(my_packet->id));
		pPlayer->MagicShot();
		break;
	}
	case SC_ANI_IDLE:
	{
		sc_packet_Behavior* my_packet = reinterpret_cast<sc_packet_Behavior*>(buffer);
		pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(my_packet->id));
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", buffer[1]);
		break;
	}

	return false;
}
