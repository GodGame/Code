#include "stdafx.h"
#include "MyInline.h"
#include "SceneTitle.h"
#include "SceneInGame.h"
#include "GameFramework.h"

CSceneTitle::CSceneTitle() : CScene()
{
}

CSceneTitle::~CSceneTitle()
{
}

void CSceneTitle::BuildObjects(ID3D11Device *pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext, SceneShaderBuildInfo * SceneInfo)
{
	m_nThread = m_nShaders = 1;
	m_ppShaders = new CShader*[m_nShaders];

	CUIScreenShader * pTitle = new CUIScreenShader(); //new CUIShader();
	pTitle->CreateShader(pd3dDevice);
	pTitle->BuildObjects(pd3dDevice, SceneInfo->pd3dBackRTV);
	m_ppShaders[0] = pTitle;

	CreateShaderVariables(pd3dDevice);
	BuildStaticShadowMap(pd3dDeviceContext);
}

void CSceneTitle::ReleaseObjects()
{
	CScene::ReleaseObjects();
}

bool CSceneTitle::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		//		m_pSelectedObject = PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	return false;
}

void CSceneTitle::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
}

void CSceneTitle::ReleaseShaderVariables()
{
	if (m_pLights) delete m_pLights;
	if (m_pd3dcbLights) m_pd3dcbLights->Release();
}

void CSceneTitle::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights)
{
}

bool CSceneTitle::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	CMaterial * pMat;

	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:
			//			FRAMEWORK.ChangeGameScene(new CSceneInGame());
			break;
		}
		break;
	}
	return(false);
}

bool CSceneTitle::ProcessInput()
{
	static UCHAR pKeyBuffer[256];
	if (GetKeyboardState(pKeyBuffer))
	{
		if (pKeyBuffer[VK_SPACE] & 0xF0) FRAMEWORK.ChangeGameScene(new CSceneInGame());
	}
	return false;
}

void CSceneTitle::AnimateObjects(float fTimeElapsed)
{
}

void CSceneTitle::Render(ID3D11DeviceContext * pd3dDeviceContext, RENDER_INFO * pRenderInfo)
{
	m_ppShaders[0]->Render(pd3dDeviceContext, 0, nullptr);
}

void CSceneTitle::UIRender(ID3D11DeviceContext * pd3dDeviceContext)
{
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
#endif