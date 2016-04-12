#include "stdafx.h"
#include "ShaderEssential.h"
#include "MyInline.h"
//#include "ObjectsList.h"
#include <D3Dcompiler.h>

CInGamePlayer * gpPlayer = nullptr;
bool gbStartGlare = false;
bool gbStartRadial = false;

static float GaussianDistribution(float x, float y, float rho)
{
	float g = 1.0f / sqrtf(2.0f * XM_PI * rho * rho);
	g *= expf(-(x * x + y * y) / (2 * rho * rho));

	return g;
}

#pragma region PlayerShader

CSceneShader::CSceneShader() : CTexturedShader()
{
	m_fTotalTime = m_fFrameTime = 0;

	m_pMesh                 = nullptr;
	m_pTexture              = nullptr;
	m_pd3dDepthStencilState = nullptr;
	m_iDrawOption           = 0;
	m_ppd3dMrtSrv           = nullptr;

	m_pd3dPSFinal           = nullptr;
	m_pd3dPSOther           = nullptr;
	m_pInfoScene            = nullptr;
	m_pd3dPSDump            = nullptr;

	m_pd3dBloom16x16RTV = m_pd3dBloom4x4RTV = nullptr;
	m_pd3dBloom16x16SRV = m_pd3dBloom4x4SRV = nullptr;

	//m_pd3dShadowSrv = nullptr;
	m_pd3dLightPS = nullptr;

	m_pd3dComputeVertBlur  = m_pd3dComputeHorzBlur = nullptr;
	m_pd3dComputeVertBloom = m_pd3dComputeHorzBloom = nullptr;
	m_pd3dBackRTV = nullptr;

	m_pd3dCBComputeInfo = m_pd3dCBWeight = nullptr;
	m_pd3dCSAdaptLum = m_pd3dCSReduceToSingle = nullptr;

	ZeroMemory(&m_cbWeights, sizeof(m_cbWeights));
	//ZeroMemory(&m_csRadialBlur, sizeof(m_csRadialBlur));
	
	for (int i = 0; i < 2; ++i)
	{
		m_pd3dPostScaledSRV[i] = m_pd3dPostSRV[i] = nullptr;
		m_pd3dPostScaledUAV[i] = m_pd3dPostUAV[i] = nullptr;
	}

	m_pd3dLastReducedSRV = nullptr;
	m_pd3dLastReducedUAV = nullptr;

	m_pd3dPostRenderRTV  = nullptr;
	m_pd3dPostRenderSRV  = nullptr;

	m_pd3dCSRadialBlur   = nullptr;
	m_pd3dRadialSRV      = nullptr;
	m_pd3dRadialUAV      = nullptr;
	//	m_csReduce = POST_CS_REPEATABLE(); // new POST_CS_REPEATABLE();
		//m_csBloom = POST_CS_BLOOMING();
}

CSceneShader::~CSceneShader()
{
	//for (int i = 1; i < NUM_MRT; ++i) m_ppd3dMrtSrv[i]->Release();

	if (m_pMesh) m_pMesh->Release();
	if (m_pTexture) m_pTexture->Release();
	if (m_pInfoScene) m_pInfoScene->Release();

	if (m_pd3dDepthStencilState) m_pd3dDepthStencilState->Release();

	if (m_pd3dPSOther) m_pd3dPSOther->Release();
	//if (m_pd3dShadowSrv) m_pd3dShadowSrv->Release();
	if (m_pd3dLightPS) m_pd3dLightPS->Release();

	if (m_pd3dPSDump)m_pd3dPSDump->Release();
	if (m_pd3dPSFinal) m_pd3dPSFinal->Release();

	if (m_pd3dBloom4x4RTV) m_pd3dBloom4x4RTV->Release();
	if (m_pd3dBloom4x4SRV) m_pd3dBloom4x4SRV->Release();

	if (m_pd3dBloom16x16RTV) m_pd3dBloom16x16RTV->Release();
	if (m_pd3dBloom16x16SRV) m_pd3dBloom16x16SRV->Release();

	if (m_pd3dComputeHorzBlur) m_pd3dComputeHorzBlur->Release();
	if (m_pd3dComputeVertBlur) m_pd3dComputeVertBlur->Release();
	if (m_pd3dComputeVertBloom) m_pd3dComputeVertBloom->Release();
	if (m_pd3dComputeHorzBloom) m_pd3dComputeHorzBloom->Release();

	for (int i = 0; i < 2; ++i)
	{
		if (m_pd3dPostSRV[i]) m_pd3dPostSRV[i]->Release();
		if (m_pd3dPostUAV[i]) m_pd3dPostUAV[i]->Release();

		if (m_pd3dPostScaledSRV[i]) m_pd3dPostScaledSRV[i]->Release();
		if (m_pd3dPostScaledUAV[i]) m_pd3dPostScaledUAV[i]->Release();
	}

	if (m_pd3dCBWeight)m_pd3dCBWeight->Release();
	if (m_pd3dCBComputeInfo) m_pd3dCBComputeInfo->Release();
	if (m_pd3dCBBloomInfo) m_pd3dCBBloomInfo->Release();

	if (m_pd3dCSReduceToSingle) m_pd3dCSReduceToSingle->Release();
	if (m_pd3dCSAdaptLum)m_pd3dCSAdaptLum->Release();

	if (m_pd3dLastReducedSRV) m_pd3dLastReducedSRV->Release();
	if (m_pd3dLastReducedUAV) m_pd3dLastReducedUAV->Release();

	if (m_pd3dPostRenderRTV) m_pd3dPostRenderRTV->Release();
	if (m_pd3dPostRenderSRV) m_pd3dPostRenderSRV->Release();

	if (m_pd3dRadialSRV) m_pd3dRadialSRV->Release();
	if (m_pd3dRadialUAV) m_pd3dRadialUAV->Release();
	if (m_pd3dCSRadialBlur) m_pd3dCSRadialBlur->Release();

	//if (m_pd3dBackRTV) m_pd3dBackRTV->Release();
	//m_csReduce.ReleaseObjects();
	//m_csBloom.~POST_CS_BLOOMING();
	//if (m_pcsReduce) delete m_pcsReduce;
}

void CSceneShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Post.fx", "VSScreen", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Post.fx", "PSScreen", "ps_5_0", &m_pd3dPixelShader);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Post.fx", "InfoScreen", "ps_5_0", &m_pd3dPSOther);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Post.fx", "LightScreen", "ps_5_0", &m_pd3dLightPS);

	CreatePixelShaderFromFile(pd3dDevice, L"fx/Post.fx", "DumpMap", "ps_5_0", &m_pd3dPSDump);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Final.fx", "PSFinalPass", "ps_5_0", &m_pd3dPSFinal);

	CreateComputeShaderFromFile(pd3dDevice, L"fx/BlurAndBloom.fx", "HorizonBlur", "cs_5_0", &m_pd3dComputeHorzBlur);
	CreateComputeShaderFromFile(pd3dDevice, L"fx/BlurAndBloom.fx", "VerticalBlur", "cs_5_0", &m_pd3dComputeVertBlur);
	CreateComputeShaderFromFile(pd3dDevice, L"fx/BlurAndBloom.fx", "HorizonBloom", "cs_5_0", &m_pd3dComputeHorzBloom);
	CreateComputeShaderFromFile(pd3dDevice, L"fx/BlurAndBloom.fx", "VerticalBloom", "cs_5_0", &m_pd3dComputeVertBloom);

	CreateComputeShaderFromFile(pd3dDevice, L"fx/BlurAndBloom.fx", "RadialBlur", "cs_5_0", &m_pd3dCSRadialBlur);

	CreateComputeShaderFromFile(pd3dDevice, L"fx/Reduce.fx", "LumCompression", "cs_5_0", &m_csReduce.m_pd3dComputeShader);
	CreateComputeShaderFromFile(pd3dDevice, L"fx/Reduce.fx", "ReduceToSingle", "cs_5_0", &m_pd3dCSReduceToSingle);
	CreateComputeShaderFromFile(pd3dDevice, L"fx/Reduce.fx", "LumAdapted", "cs_5_0", &m_pd3dCSAdaptLum);
}

void CSceneShader::BuildObjects(ID3D11Device *pd3dDevice, ID3D11ShaderResourceView ** ppd3dMrtSrv, int nMrtSrv, ID3D11RenderTargetView * pd3dBackRTV)
{
	m_fInverseToneTex = 1.0f / (ToneMappingTexSize*ToneMappingTexSize);

	m_iDrawOption = nMrtSrv;
	m_ppd3dMrtSrv = ppd3dMrtSrv;
	m_pd3dBackRTV = pd3dBackRTV;	//m_pd3dBackRTV->AddRef();

	m_pMesh       = new CPlaneMesh(pd3dDevice, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	m_pTexture    = new CTexture(NUM_MRT - 1, 1, 17, 0, SET_SHADER_PS);
	m_pInfoScene  = new CTexture(1, 0, 0, 0, SET_SHADER_PS);

	for (int i = 1; i < NUM_MRT; ++i) m_pTexture->SetTexture(i - 1, ppd3dMrtSrv[i]);

	m_pd3dBloom4x4RTV = ViewMgr.GetRTV("sr2d_bloom4x4");	  m_pd3dBloom4x4RTV->AddRef();
	m_pd3dBloom4x4SRV = ViewMgr.GetSRV("sr2d_bloom4x4");	  m_pd3dBloom4x4SRV->AddRef();
	m_pd3dBloom16x16RTV = ViewMgr.GetRTV("sr2d_bloom16x16");  m_pd3dBloom16x16RTV->AddRef();
	m_pd3dBloom16x16SRV = ViewMgr.GetSRV("sr2d_bloom16x16");  m_pd3dBloom16x16SRV->AddRef();

	m_pd3dPostSRV[0] = ViewMgr.GetSRV("su2d_post0"); m_pd3dPostSRV[0]->AddRef();
	m_pd3dPostSRV[1] = ViewMgr.GetSRV("su2d_post1"); m_pd3dPostSRV[1]->AddRef();
	m_pd3dPostUAV[0] = ViewMgr.GetUAV("su2d_post0"); m_pd3dPostUAV[0]->AddRef();
	m_pd3dPostUAV[1] = ViewMgr.GetUAV("su2d_post1"); m_pd3dPostUAV[1]->AddRef();

	m_pd3dPostScaledSRV[0] = ViewMgr.GetSRV("su2d_postscaled0"); m_pd3dPostScaledSRV[0]->AddRef();
	m_pd3dPostScaledSRV[1] = ViewMgr.GetSRV("su2d_postscaled0"); m_pd3dPostScaledSRV[1]->AddRef();
	m_pd3dPostScaledUAV[0] = ViewMgr.GetUAV("su2d_postscaled0"); m_pd3dPostScaledUAV[0]->AddRef();
	m_pd3dPostScaledUAV[1] = ViewMgr.GetUAV("su2d_postscaled0"); m_pd3dPostScaledUAV[1]->AddRef();

	m_csReduce.m_pd3dUAVArray[0] = ViewMgr.GetUAV("su_reduce1"); m_csReduce.m_pd3dUAVArray[0]->AddRef();
	m_csReduce.m_pd3dUAVArray[1] = ViewMgr.GetUAV("su_reduce2"); m_csReduce.m_pd3dUAVArray[1]->AddRef();
	m_csReduce.m_pd3dSRVArray[0] = ViewMgr.GetSRV("su_reduce1"); m_csReduce.m_pd3dSRVArray[0]->AddRef();
	m_csReduce.m_pd3dSRVArray[1] = ViewMgr.GetSRV("su_reduce2"); m_csReduce.m_pd3dSRVArray[1]->AddRef();

	m_pd3dPostRenderRTV = ViewMgr.GetRTV("sr2d_PostResult");  m_pd3dPostRenderRTV->AddRef();
	m_pd3dPostRenderSRV = ViewMgr.GetSRV("sr2d_PostResult");  m_pd3dPostRenderSRV->AddRef();

	m_pd3dRadialSRV = ViewMgr.GetSRV("su2d_radial");  m_pd3dRadialSRV->AddRef();
	m_pd3dRadialUAV = ViewMgr.GetUAV("su2d_radial");  m_pd3dRadialUAV->AddRef();

	//m_pd3dLastReducedSRV = ViewMgr.GetSRV("su_4last_reduce"); m_pd3dLastReducedSRV->AddRef();
	//m_pd3dLastReducedUAV = ViewMgr.GetUAV("su_4last_reduce"); m_pd3dLastReducedUAV->AddRef();
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	D3D11_BUFFER_DESC DescBuffer;
	ZeroMemory(&DescBuffer, sizeof(D3D11_BUFFER_DESC));
	DescBuffer.BindFlags           = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	DescBuffer.ByteWidth           = sizeof(float) * 16;
	DescBuffer.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	DescBuffer.StructureByteStride = sizeof(float);
	DescBuffer.Usage               = D3D11_USAGE_DEFAULT;

	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format                 = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension          = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement    = 0;
	DescUAV.Buffer.NumElements     = DescBuffer.ByteWidth / sizeof(float);

	D3D11_SHADER_RESOURCE_VIEW_DESC DescRV;
	ZeroMemory(&DescRV, sizeof(DescRV));
	DescRV.Format                  = DXGI_FORMAT_UNKNOWN;
	DescRV.ViewDimension           = D3D11_SRV_DIMENSION_BUFFER;
	DescRV.Buffer.FirstElement     = DescUAV.Buffer.FirstElement;
	DescRV.Buffer.NumElements      = DescUAV.Buffer.NumElements;

	ID3D11Buffer * pBuffer         = nullptr;
	ASSERT_S(pd3dDevice->CreateBuffer(&DescBuffer, nullptr, &pBuffer));
	ASSERT_S(pd3dDevice->CreateUnorderedAccessView(pBuffer, &DescUAV, &m_pd3dLastReducedUAV));
	ASSERT_S(pd3dDevice->CreateShaderResourceView(pBuffer, &DescRV, &m_pd3dLastReducedSRV));
	pBuffer->Release();
}

void CSceneShader::CreateConstantBuffer(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(CB_WEIGHT);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;// 0;
	HRESULT hr = pd3dDevice->CreateBuffer(&desc, nullptr, &m_pd3dCBWeight);

	ASSERT_S(nullptr != (m_pd3dCBComputeInfo = ViewMgr.GetBuffer("cs_float4")));
	m_pd3dCBComputeInfo->AddRef();

	ASSERT_S(nullptr != (m_pd3dCBBloomInfo = ViewMgr.GetBuffer("cs_float4x4")));
	m_pd3dCBBloomInfo->AddRef();

//	desc.ByteWidth = sizeof(CB_CS_BLOOM);
//	ASSERT(SUCCEEDED(hr = pd3dDevice->CreateBuffer(&desc, nullptr, &m_pd3dCBBloomInfo)));

	UpdateShaderReosurces(pd3dDeviceContext);
}

void CSceneShader::UpdateShaderReosurces(ID3D11DeviceContext * pd3dDeviceContext)
{
	float fSigma = 3.0f;
	float fSum = 0.0f, f2Sigma = 10.0f * fSigma * fSigma;

	//m_cbWeights.fWeight[11] = m_fInverseToneTex;
	float fWeight;
	for (int i = 0; i < 5; ++i)
	{
		fWeight = ::GaussianDistribution((float)i, 0, fSigma);
		m_cbWeights.fWeight[i] = XMFLOAT4(fWeight, fWeight, fWeight, 0.0f);
		//fSum += m_cbWeights.fWeight[i];
	}
	fWeight = 1.25 * ::GaussianDistribution(0, 0, fSigma);
	m_cbWeights.fWeight[5] = XMFLOAT4(fWeight, fWeight, fWeight, 0.0f);

	for (int i = 6; i < 11; ++i)
	{
		m_cbWeights.fWeight[i] = m_cbWeights.fWeight[10 - i];//1.25f * ::GaussianDistribution((float)i, 0, fSigma);
		//fSum += m_cbWeights.fWeight[i];
	}
	MapConstantBuffer(pd3dDeviceContext, &m_cbWeights, sizeof(CB_WEIGHT), m_pd3dCBWeight);
	pd3dDeviceContext->CSSetConstantBuffers(SLOT_CS_CB_WEIGHT, 1, &m_pd3dCBWeight);
}

void CSceneShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera /*= nullptr*/)
{
	gpPlayer = (CInGamePlayer*) pCamera->GetPlayer();
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	//ShadowMgr.UpdateStaticShadowResource(pd3dDeviceContext);
	ShadowMgr.UpdateDynamicShadowResource(pd3dDeviceContext);

	//printf("Opt: %d \n", m_iDrawOption);
	//SetTexture(0, m_ppd3dMrtSrv[m_iDrawOption]);
	//pd3dDeviceContext->OMSetRenderTargets(1, &m_ppd3dMrtRtv[MRT_SCENE], nullptr);
	//if (m_iDrawOption == 0)
	//{
	UpdateShaders(pd3dDeviceContext);
	//}
	//else if (m_iDrawOption == 1)
	//{
	//	m_pInfoScene->SetTexture(0, m_pd3dShadowSrv);
	//	pd3dDeviceContext->PSSetShader(m_pd3dLightPS, nullptr, 0);
	//	m_pInfoScene->UpdateShaderVariable(pd3dDeviceContext);
	//}
	//else if (m_iDrawOption < 0)
	//{
	//	pd3dDeviceContext->PSSetShader(m_pd3dPSOther, nullptr, 0);
	//	m_pInfoScene->UpdateShaderVariable(pd3dDeviceContext);
	//}
	//else
	//{
	//	SetInfoTextures(pd3dDeviceContext);
	//}

//	pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dBackRTV, nullptr);

	m_pMesh->Render(pd3dDeviceContext, uRenderState);

	//PostProcessingRender(pd3dDeviceContext, uRenderState, pCamera);
}

void CSceneShader::AnimateObjects(float fTimeElapsed)
{
	if (gpPlayer && false == gbStartGlare &&gpPlayer->GetEnergyNum() > 10)
	{
		GetGameMessage(this, MSG_EFFECT_GLARE_ON);
		EVENTMgr.InsertDelayMessage(3.0f, MSG_EFFECT_GLARE_OFF, CGameEventMgr::MSG_TYPE_SHADER, (void*)this); //ShaderDelayMessage(3.0f, MSG_EFFECT_GLARE_OFF, (CShader*)this);
	//	EVENTMgr.InsertDelayMessage(2.0f, MSG_CULL_OUT, CGameEventMgr::MSG_TYPE_SHADER, (void*)this);
	//	EVENTMgr.ShaderDelayMessage(2.0f, MSG_CULL_OUT, (CShader*)this);
	}
	m_fFrameTime = fTimeElapsed;
	if (gbStartGlare) m_fTotalTime += fTimeElapsed;

	//cout << "SceneFrame : " << m_fTotalTime << endl;
}

void CSceneShader::GetGameMessage(CShader * byObj, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_EFFECT_GLARE_ON :
		gbStartGlare = true;
	case eMessage::MSG_EFFECT_RADIAL_ON:
		m_iDrawOption = 2;
		return;

	case eMessage::MSG_EFFECT_GLARE_OFF:
		m_fTotalTime        = 0.0f;
		gbStartGlare		= false;
		gpPlayer->UseEnergy(10, true);
	case eMessage::MSG_EFFECT_RADIAL_OFF :
		m_iDrawOption       = 0;
		return;

	default:
		system("cls");
		return;
	}
}

void CSceneShader::SendGameMessage(CShader * toObj, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_EFFECT_GLARE_ON:
	case eMessage::MSG_EFFECT_GLARE_OFF:
	default:
		toObj->GetGameMessage(this, eMSG);
		return;
	}
}

void CSceneShader::PostProcessingRender(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	// 이런...
	pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dBackRTV, nullptr);
	ID3D11ShaderResourceView * pSRVArrsy[] = { m_pd3dPostSRV[1], m_pd3dPostScaledSRV[1] };

	static ID3D11ShaderResourceView * pd3dNullSRV[1] = { nullptr };
	static ID3D11UnorderedAccessView * pd3dNullUAV[2] = { nullptr, nullptr };

	switch (m_iDrawOption)
	{
	case 0:
		MeasureLuminance(pd3dDeviceContext, uRenderState, pCamera);
		Blooming(pd3dDeviceContext, uRenderState, pCamera);

		pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dBackRTV, nullptr);
		HDRFinalRender(pd3dDeviceContext, pSRVArrsy, uRenderState, pCamera);
		break;

	case 1:
		MeasureLuminance(pd3dDeviceContext, uRenderState, pCamera);
		pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dBackRTV, nullptr);
		HDRFinalRender(pd3dDeviceContext, nullptr, uRenderState, pCamera);
		break;

	case 2:
		MeasureLuminance(pd3dDeviceContext, uRenderState, pCamera);
		Blooming(pd3dDeviceContext, uRenderState, pCamera);

		pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dPostRenderRTV, nullptr);
		HDRFinalRender(pd3dDeviceContext, pSRVArrsy, uRenderState, pCamera);
		pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dBackRTV, nullptr);

		RadialBlur(pd3dDeviceContext, uRenderState, pCamera);
		ScreenRender(pd3dDeviceContext, m_pd3dRadialSRV, uRenderState);
		break;

	default:
		pd3dDeviceContext->VSSetShader(m_pd3dVertexShader, nullptr, 0);
		Blooming(pd3dDeviceContext, uRenderState, pCamera);
		ScreenRender(pd3dDeviceContext, m_pd3dBloom16x16SRV, uRenderState);
		break;
	}

	pd3dDeviceContext->CSSetShader(nullptr, nullptr, 0);
	pd3dDeviceContext->CSSetUnorderedAccessViews(1, 2, pd3dNullUAV, nullptr);
	pd3dDeviceContext->CSSetShaderResources(0, 1, pd3dNullSRV);
}

void CSceneShader::ScreenRender(ID3D11DeviceContext * pd3dDeviceContext, ID3D11ShaderResourceView * pScreenSRV, UINT uRenderState)
{
	pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dBackRTV, nullptr);

	m_pInfoScene->SetTexture(0, pScreenSRV);
	//pScreenSRV->Release();
	pd3dDeviceContext->PSSetShader(m_pd3dPSOther, nullptr, 0);
	m_pInfoScene->UpdateShaderVariable(pd3dDeviceContext);

	m_pMesh->Render(pd3dDeviceContext, uRenderState);
}

void CSceneShader::HDRFinalRender(ID3D11DeviceContext * pd3dDeviceContext, ID3D11ShaderResourceView * pBloomSRV[], UINT uRenderState, CCamera * pCamera)
{
	//pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dBackRTV, nullptr);
	pd3dDeviceContext->VSSetShader(m_pd3dVertexShader, nullptr, 0);
	pd3dDeviceContext->PSSetShader(m_pd3dPSFinal, nullptr, 0);

	//	m_csBloom.m_pd3dSRVArray[1]
	ID3D11ShaderResourceView* pShaderViews[4] = { m_ppd3dMrtSrv[0], m_csReduce.m_pd3dSRVArray[1], pBloomSRV != nullptr ? pBloomSRV[0] : nullptr, pBloomSRV != nullptr ? pBloomSRV[1] : nullptr };
	ID3D11SamplerState * pSamplers[2] = { TXMgr.GetSamplerState("ss_point_clamp"),  TXMgr.GetSamplerState("ss_linear_point_wrap") };
	CB_PS cbPS = { m_fInverseToneTex, m_fInverseToneTex, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	{
		D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
		pd3dDeviceContext->Map(m_pd3dCBComputeInfo, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
		memcpy(d3dMappedResource.pData, &cbPS, sizeof(CB_PS));
		pd3dDeviceContext->Unmap(m_pd3dCBComputeInfo, 0);
		pd3dDeviceContext->PSSetConstantBuffers(0, 1, &m_pd3dCBComputeInfo);
	}

	pd3dDeviceContext->PSSetShaderResources(0, 4, pShaderViews);
	pd3dDeviceContext->PSSetSamplers(0, 2, pSamplers);

	m_pMesh->Render(pd3dDeviceContext, uRenderState);
	//pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dBackRTV, nullptr);
}

void CSceneShader::MeasureLuminance(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	//if (m_fTotalTime < 0.1f) return;

	int dimx = int(ceil(ToneMappingTexSize / 8.0f));
	int dimy = dimx;

	static ID3D11ShaderResourceView * pd3dNullSRV[1] = { nullptr };
	static ID3D11UnorderedAccessView * pd3dNullUAV[2] = { nullptr, nullptr };
	{
		CB_CS cbCS = { XMFLOAT4(dimx, dimy, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT) };
		MapConstantBuffer(pd3dDeviceContext, &cbCS, sizeof(CB_CS), m_pd3dCBComputeInfo);
		pd3dDeviceContext->CSSetConstantBuffers(0, 1, &m_pd3dCBComputeInfo);
	}
	{
		pd3dDeviceContext->CSSetShader(m_csReduce.m_pd3dComputeShader, nullptr, 0);
		pd3dDeviceContext->CSSetUnorderedAccessViews(1, 1, &m_csReduce.m_pd3dUAVArray[0], nullptr);
		pd3dDeviceContext->CSSetShaderResources(0, 1, m_ppd3dMrtSrv);

		pd3dDeviceContext->Dispatch(dimx, dimy, 1);
	}
	{
		int dim = dimx*dimy;
		int nNumToReduce = dim;
		dim = int(ceil(dim / 128.0f));
		if (nNumToReduce > 1)
		{
			while(true)
			{
				CB_CS cbCS = { XMFLOAT4(nNumToReduce, 0, m_fTotalTime, m_fFrameTime) };
				MapConstantBuffer(pd3dDeviceContext, &cbCS, sizeof(CB_CS), m_pd3dCBComputeInfo);
				pd3dDeviceContext->CSSetConstantBuffers(0, 1, &m_pd3dCBComputeInfo);

				pd3dDeviceContext->CSSetShader(m_pd3dCSReduceToSingle, nullptr, 0);
				pd3dDeviceContext->CSSetUnorderedAccessViews(1, 1, &m_csReduce.m_pd3dUAVArray[1], nullptr);
				//pd3dDeviceContext->CSSetUnorderedAccessViews(2, 1, &m_pd3dLastReducedUAV, nullptr);
				pd3dDeviceContext->CSSetShaderResources(1, 1, &m_csReduce.m_pd3dSRVArray[0]);
				//pd3dDeviceContext->CSSetShaderResources(2, 1, &m_csReduce.m_pd3dSRVArray[1]);
				pd3dDeviceContext->Dispatch(dim, 1, 1);

				nNumToReduce = dim;
				dim = int(ceil(dim / 128.0f));

				if (nNumToReduce == 1)
					break;

				m_csReduce.swap(0, 1);
			}
		}
		else
		{
			m_csReduce.swap(0, 1);
		}
	}
	{
		m_csReduce.swap(0, 1);
		// x = 기본 1, y = 플러스하면 밝아짐
		CB_CS cbCS;
//		if(pCamera->GetPlayer()->m_nEnergy < 10)
//			cbCS = { XMFLOAT4(1.0f, 0.0f, m_fTotalTime, m_fFrameTime) };
		cbCS = { XMFLOAT4(1.0f, gbStartGlare * 200000.0f, m_fTotalTime, m_fFrameTime) };

		MapConstantBuffer(pd3dDeviceContext, &cbCS, sizeof(CB_CS), m_pd3dCBComputeInfo);
		pd3dDeviceContext->CSSetConstantBuffers(0, 1, &m_pd3dCBComputeInfo);

		pd3dDeviceContext->CSSetShader(m_pd3dCSAdaptLum, nullptr, 0);
		pd3dDeviceContext->CSSetUnorderedAccessViews(1, 1, &m_csReduce.m_pd3dUAVArray[1], nullptr);
		pd3dDeviceContext->CSSetUnorderedAccessViews(2, 1, &m_pd3dLastReducedUAV, nullptr);
		pd3dDeviceContext->CSSetShaderResources(1, 1, &m_csReduce.m_pd3dSRVArray[0]);
		//pd3dDeviceContext->CSSetShaderResources(2, 1, &m_csReduce.m_pd3dSRVArray[1]);
		pd3dDeviceContext->Dispatch(1, 1, 1);

		pd3dDeviceContext->CSSetShader(nullptr, nullptr, 0);
		pd3dDeviceContext->CSSetUnorderedAccessViews(1, 2, pd3dNullUAV, nullptr);
		pd3dDeviceContext->CSSetShaderResources(0, 1, pd3dNullSRV);
	}

#ifdef CAL_IN_CPU
	{
		float fResult;

		D3D11_BOX box;
		box.left = 0;
		box.right = sizeof(float) * dimx * dimy;
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;
		pd3dDeviceContext->CopySubresourceRegion(m_pd3dComputeRead, 0, 0, 0, 0, m_pd3dBufferReduce[0], 0, &box);
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		ASSERT(SUCCEEDED(pd3dDeviceContext->Map(m_pd3dComputeRead, 0, D3D11_MAP_READ, 0, &MappedResource)));
		float *pData = reinterpret_cast<float*>(MappedResource.pData);
		fResult = 0;
		for (int i = 0; i < dimx * dimy; ++i)
		{
			fResult += pData[i];
		}
		pd3dDeviceContext->Unmap(m_pd3dComputeRead, 0);

		cout << "평균치는 : " << fResult << endl;
	}
#endif
}

void CSceneShader::SceneBlur(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	static UINT cxGroup = (UINT)ceilf(FRAME_BUFFER_WIDTH / 256.0f);
	static UINT cyGroup = (UINT)ceilf(FRAME_BUFFER_HEIGHT / 480.0f);

	ID3D11ShaderResourceView * pd3dNullSRV[2] = { nullptr, nullptr };
	ID3D11UnorderedAccessView * pd3dNullUAV[1] = { nullptr };

	CB_CS_BLOOM cbcs;
	ZeroMemory(&cbcs, sizeof(CB_CS_BLOOM)); //= XMFLOAT4(m_fInverseToneTex, 0, 0, 0);
	cbcs.m_uOutputSize.x = cbcs.m_uInputSize.x = FRAME_BUFFER_WIDTH;
	cbcs.m_uOutputSize.y = cbcs.m_uInputSize.y = FRAME_BUFFER_HEIGHT;
	cbcs.m_fInverse = m_fInverseToneTex;
	cbcs.m_fThreshold = 0.6f;

	MapConstantBuffer(pd3dDeviceContext, &cbcs, sizeof(CB_CS_BLOOM), m_pd3dCBBloomInfo);
	pd3dDeviceContext->CSSetConstantBuffers(SLOT_CS_CB_BLOOM, 1, &m_pd3dCBBloomInfo);

	//	pCamera->SetViewport(pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH * 0.5f, FRAME_BUFFER_HEIGHT * 0.5f, 0.0f, 1.0f);
	ID3D11ShaderResourceView * pd3dSRVArray[] = { m_ppd3dMrtSrv[0], nullptr };
	//for (int i = 0; i < 1; ++i)
	{
		pd3dDeviceContext->CSSetShader(m_pd3dComputeHorzBlur, nullptr, 0);
		pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, &m_pd3dPostUAV[0], nullptr);
		pd3dDeviceContext->CSSetShaderResources(0, 2, pd3dSRVArray);

		pd3dDeviceContext->Dispatch(cxGroup, FRAME_BUFFER_HEIGHT, 1);

		pd3dDeviceContext->CSSetShader(m_pd3dComputeVertBlur, nullptr, 0);
		pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, &m_pd3dPostUAV[1], nullptr);
		pd3dSRVArray[0] = m_pd3dPostSRV[0];
		pd3dDeviceContext->CSSetShaderResources(0, 2, pd3dSRVArray);

		pd3dDeviceContext->Dispatch(FRAME_BUFFER_WIDTH, cyGroup, 1);

		pd3dDeviceContext->CSSetShader(nullptr, nullptr, 0);
		pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, pd3dNullUAV, nullptr);
		pd3dDeviceContext->CSSetShaderResources(0, 2, pd3dNullSRV);
	}
	//	pCamera->SetViewport(pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
}

void CSceneShader::RadialBlur(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	static UINT cxGroup = (UINT)ceilf(FRAME_BUFFER_WIDTH / 32.0f);
	static UINT cyGroup = (UINT)ceilf(FRAME_BUFFER_HEIGHT / 32.0f);

	ID3D11ShaderResourceView * pd3dNullSRV[2] = { nullptr, nullptr };
	ID3D11UnorderedAccessView * pd3dNullUAV[1] = { nullptr };

	CB_CS_BLOOM cbcs;
	ZeroMemory(&cbcs, sizeof(CB_CS_BLOOM)); //= XMFLOAT4(m_fInverseToneTex, 0, 0, 0);
	cbcs.m_uOutputSize.x = cbcs.m_uInputSize.x = FRAME_BUFFER_WIDTH;
	cbcs.m_uOutputSize.y = cbcs.m_uInputSize.y = FRAME_BUFFER_HEIGHT;
	cbcs.m_fInverse = m_fInverseToneTex;
	cbcs.m_fThreshold = 0.6f;

	MapConstantBuffer(pd3dDeviceContext, &cbcs, sizeof(CB_CS_BLOOM), m_pd3dCBBloomInfo);
	pd3dDeviceContext->CSSetConstantBuffers(SLOT_CS_CB_BLOOM, 1, &m_pd3dCBBloomInfo);

	ID3D11ShaderResourceView * pd3dSRVArray[] = { m_pd3dPostRenderSRV, nullptr };
	//for (int i = 0; i < 1; ++i)
	{
		pd3dDeviceContext->CSSetShader(m_pd3dCSRadialBlur, nullptr, 0);
		pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, &m_pd3dRadialUAV, nullptr);
		pd3dDeviceContext->CSSetShaderResources(0, 2, pd3dSRVArray);

		pd3dDeviceContext->Dispatch(cxGroup, cyGroup, 1);

		pd3dDeviceContext->CSSetShader(nullptr, nullptr, 0);
		pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, pd3dNullUAV, nullptr);
		pd3dDeviceContext->CSSetShaderResources(0, 2, pd3dNullSRV);
	}
}

void CSceneShader::Blooming(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	int ScreenWidth = ceilf(FRAME_BUFFER_WIDTH * 0.25f);
	int ScreenHeight = ceilf(FRAME_BUFFER_HEIGHT * 0.25f);

	DumpMap(pd3dDeviceContext, m_ppd3dMrtSrv[0], m_pd3dBloom4x4RTV, ScreenWidth, ScreenHeight, pCamera);

	UINT cxGroup = (UINT)ceilf(ScreenWidth / 256.0f);
	UINT cyGroup = (UINT)ceilf(ScreenHeight / 240.0f);

	CB_CS_BLOOM cbcs;
	ZeroMemory(&cbcs, sizeof(CB_CS_BLOOM)); //= XMFLOAT4(m_fInverseToneTex, 0, 0, 0);
	cbcs.m_uOutputSize.x = cbcs.m_uInputSize.x = ScreenWidth;
	cbcs.m_uOutputSize.y = cbcs.m_uInputSize.y = ScreenHeight;
	cbcs.m_fInverse = m_fInverseToneTex;
	cbcs.m_fThreshold = 0.6f;

	MapConstantBuffer(pd3dDeviceContext, &cbcs, sizeof(CB_CS_BLOOM), m_pd3dCBBloomInfo);
	pd3dDeviceContext->CSSetConstantBuffers(SLOT_CS_CB_BLOOM, 1, &m_pd3dCBBloomInfo);

	//	pCamera->SetViewport(pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH * 0.5f, FRAME_BUFFER_HEIGHT * 0.5f, 0.0f, 1.0f);
	ID3D11ShaderResourceView * pd3dSRVArray[] = { m_pd3dBloom4x4SRV, m_csReduce.m_pd3dSRVArray[1] };
	//for (int i = 0; i < 1; ++i)
	{
		pd3dDeviceContext->CSSetShader(m_pd3dComputeHorzBloom, nullptr, 0);
		pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, &m_pd3dPostUAV[0], nullptr);
		pd3dDeviceContext->CSSetShaderResources(0, 2, pd3dSRVArray);

		pd3dDeviceContext->Dispatch(cxGroup, ScreenHeight, 1);

		pd3dDeviceContext->CSSetShader(m_pd3dComputeVertBlur, nullptr, 0);	// bloom이 아니라 블러로 한다.
		pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, &m_pd3dPostUAV[1], nullptr);

		pd3dSRVArray[0] = m_pd3dPostSRV[0];

		pd3dDeviceContext->CSSetShaderResources(0, 2, pd3dSRVArray);
		pd3dDeviceContext->Dispatch(ScreenWidth, cyGroup, 1);
	}

	DumpMap(pd3dDeviceContext, m_pd3dPostSRV[1], m_pd3dBloom16x16RTV, FRAME_BUFFER_WIDTH * 0.0625, FRAME_BUFFER_HEIGHT * 0.0625, pCamera);

	ScreenWidth = ceilf(FRAME_BUFFER_WIDTH * 0.0625);
	ScreenHeight = ceilf(FRAME_BUFFER_HEIGHT * 0.0625);

	cxGroup = (UINT)ceilf(ScreenWidth / 256.0f);
	cyGroup = (UINT)ceilf(ScreenHeight / 240.0f);
	{
		cbcs.m_uOutputSize.x = cbcs.m_uInputSize.x = ScreenWidth;
		cbcs.m_uOutputSize.y = cbcs.m_uInputSize.y = ScreenHeight;

		MapConstantBuffer(pd3dDeviceContext, &cbcs, sizeof(CB_CS_BLOOM), m_pd3dCBBloomInfo);
		pd3dDeviceContext->CSSetConstantBuffers(SLOT_CS_CB_BLOOM, 1, &m_pd3dCBBloomInfo);

		pd3dDeviceContext->CSSetShader(m_pd3dComputeHorzBlur, nullptr, 0);	// bloom이 아니라 블러로 한다.
		pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, &m_pd3dPostScaledUAV[1], nullptr);

		pd3dSRVArray[0] = m_pd3dPostScaledSRV[1];

		pd3dDeviceContext->CSSetShaderResources(0, 2, pd3dSRVArray);
		pd3dDeviceContext->Dispatch(cxGroup, ScreenHeight, 1);

		pd3dDeviceContext->CSSetShader(m_pd3dComputeVertBlur, nullptr, 0);	// bloom이 아니라 블러로 한다.
		pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, &m_pd3dPostScaledUAV[0], nullptr);

		pd3dSRVArray[0] = m_pd3dPostScaledSRV[1];

		pd3dDeviceContext->CSSetShaderResources(0, 2, pd3dSRVArray);
		pd3dDeviceContext->Dispatch(ScreenWidth, cyGroup, 1);
	}
}

void CSceneShader::DumpMap(ID3D11DeviceContext * pd3dDeviceContext, ID3D11ShaderResourceView * pSRVsource, ID3D11RenderTargetView * pRTVTarget, DWORD dFrameWidth, DWORD dFrameHeight, CCamera * pCamera)
{
	DWORD dWidth = dFrameWidth;
	DWORD dHeight = dFrameHeight;

	pCamera->SetViewport(pd3dDeviceContext, 0, 0, dWidth, dHeight, 0.0f, 1.0f);

	ID3D11SamplerState * pSamplers[2] = { TXMgr.GetSamplerState("ss_point_clamp"),  TXMgr.GetSamplerState("ss_linear_clamp") };

	pd3dDeviceContext->PSSetShader(m_pd3dPSDump, nullptr, 0);

	CB_PS cbPS = { FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, (float)dWidth, (float)dHeight };
	MapConstantBuffer(pd3dDeviceContext, &cbPS, sizeof(CB_PS), m_pd3dCBComputeInfo);
	pd3dDeviceContext->PSSetConstantBuffers(0, 1, &m_pd3dCBComputeInfo);
	
	pd3dDeviceContext->PSSetShaderResources(0, 1, &pSRVsource);
	pd3dDeviceContext->PSSetSamplers(0, 2, pSamplers);
	pd3dDeviceContext->OMSetRenderTargets(1, &pRTVTarget, nullptr);

	m_pMesh->Render(pd3dDeviceContext, 0);

	pd3dDeviceContext->PSSetShader(nullptr, nullptr, 0);

	pCamera->SetViewport(pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dBackRTV, nullptr);
}


void CSceneShader::SetTexture(int index, ID3D11ShaderResourceView * m_pSceneSRV)
{
	m_pInfoScene->SetTexture(index, m_pSceneSRV);
}

void CSceneShader::SetInfoTextures(ID3D11DeviceContext * pd3dDeviceContext)
{
	m_pInfoScene->SetTexture(0, m_ppd3dMrtSrv[m_iDrawOption]);
	pd3dDeviceContext->PSSetShader(m_pd3dPSOther, nullptr, 0);
	m_pInfoScene->UpdateShaderVariable(pd3dDeviceContext);
}

void CSceneShader::UpdateShaders(ID3D11DeviceContext * pd3dDeviceContext)
{
	m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPlayerShader::CPlayerShader() : CShader()
{
}
CPlayerShader::~CPlayerShader()
{
}
void CPlayerShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);

	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSNormalAndSF", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSNormalAndSF", "ps_5_0", &m_pd3dPixelShader);
}

void CPlayerShader::BuildObjects(ID3D11Device *pd3dDevice, CShader::BUILD_RESOURCES_MGR & mgrScene)
{
	m_nObjects = 2;
	m_ppObjects = new CGameObject*[m_nObjects];

	CMaterial * pPlayerMaterial = MaterialMgr.GetObjects("White");

	CMesh * pMesh[eANI_TOTAL_NUM] = { nullptr, };

	pMesh[eANI_IDLE]            = mgrScene.mgrMesh.GetObjects("scene_aure_idle");
	pMesh[eANI_RUN_FORWARD]     = mgrScene.mgrMesh.GetObjects("scene_aure_run_forwad");
	pMesh[eANI_WALK_BACK]       = mgrScene.mgrMesh.GetObjects("scene_aure_walk_Back");
	pMesh[eANI_WALK_RIGHT]      = mgrScene.mgrMesh.GetObjects("scene_aure_walk_right");
	pMesh[eANI_WALK_LEFT]       = mgrScene.mgrMesh.GetObjects("scene_aure_walk_left");

	pMesh[eANI_1H_CAST]         = mgrScene.mgrMesh.GetObjects("scene_aure_magic_cast01");
	pMesh[eANI_1H_MAGIC_ATTACK] = mgrScene.mgrMesh.GetObjects("scene_aure_magic_attack01");
	pMesh[eANI_1H_MAGIC_AREA]   = mgrScene.mgrMesh.GetObjects("scene_aure_magic_area01");

	pMesh[eANI_DAMAGED_FRONT_01] = mgrScene.mgrMesh.GetObjects("scene_aure_damaged_f01");
	pMesh[eANI_DAMAGED_FRONT_02] = mgrScene.mgrMesh.GetObjects("scene_aure_damaged_f02");
	pMesh[eANI_DEATH_FRONT]      = mgrScene.mgrMesh.GetObjects("scene_aure_death_f");

	CMapManager * pTerrain = &MAPMgr;
	for (int j = 0; j < m_nObjects; ++j)
	{
		CInGamePlayer *pPlayer = new CInGamePlayer(eANI_TOTAL_NUM);
		CTexture * pTexture = mgrScene.mgrTexture.GetObjects("scene_aure");
		pPlayer->BuildObject(pMesh, eANI_TOTAL_NUM, pTexture, pPlayerMaterial);
		pPlayer->SetCollide(true);
		pPlayer->AddRef();
		m_ppObjects[j] = pPlayer;

		if (j == 0)	// 플레이어일 때, 카메라를 셋팅해준다.
		{
			pPlayer->ChangeCamera(pd3dDevice, THIRD_PERSON_CAMERA, 0.0f);
			pPlayer->Rotate(0, 180, 0);
			continue;
		}
		else
		{
			float fHeight = pTerrain->GetHeight(1180, 255, true);// +10.f;
			pPlayer->SetPosition(XMFLOAT3(1180, fHeight, 255));
		}

		char name[56];
		for (int i = 1; i <= 7; ++i)
		{
			CRevolvingObject * pObject = nullptr;
			pObject = new CRevolvingObject(1);
			pObject->SetRevolutionAxis(XMFLOAT3(0, 1, 0));
			pObject->SetRevolutionSpeed(60.0f);

			sprintf(name, "scene_staff1_%d", i);

			pObject->SetMesh(mgrScene.mgrMesh.GetObjects(name));
			pObject->SetTexture(mgrScene.mgrTexture.GetObjects(name));
			pObject->SetMaterial(pPlayerMaterial);
			pObject->SetPosition(cosf(XMConvertToRadians(i * 51)) * 15, 10, sinf(XMConvertToRadians(i * 51)) * 15);
			pObject->Rotate(0, 0, 0);

			pPlayer->SetChild(pObject);
			//pBrickTexture->Release();
		}
	}
	EntityAllDynamicObjects();
}

void CPlayerShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);
	//printf("%0.2f %0.2f %0.2f \n", pos.x, pos.y, pos.z);
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	m_ppObjects[0]->SetActive(true);//m_ppObjects[1]->SetActive(true);

	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		CShader::Render(pd3dDeviceContext, uRenderState);
	}
}
void CPlayerShader::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nObjects; ++i)
	{
		m_ppObjects[i]->Animate(fTimeElapsed);
	}
}
#pragma endregion PlayerShader

CTerrainShader::CTerrainShader() : CSplatLightingShader()
{
	m_nLayerNumber = 0;
	m_pptxLayerMap = nullptr;
}

void CTerrainShader::CreateShader(ID3D11Device *pd3dDevice)
{
#ifdef TS_TERRAIN
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 } };
	UINT nElements = ARRAYSIZE(d3dInputLayout);

	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSTerrain", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreateHullShaderFromFile(pd3dDevice, L"fx/Effect.fx", "HSTerrain", "hs_5_0", &m_pd3dHullShader);
	CreateDomainShaderFromFile(pd3dDevice, L"fx/Effect.fx", "DSTerrain", "ds_5_0", &m_pd3dDomainShader);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSTerrain", "ps_5_0", &m_pd3dPixelShader);
	CreateShaderVariables(pd3dDevice);
#else
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSSplatTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSSplatTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader);
#endif
}

CTerrainShader::~CTerrainShader()
{
	for (int i = 0; i < m_nLayerNumber; ++i)
		if (m_pptxLayerMap[i])
			m_pptxLayerMap[i]->Release();
	delete[] m_pptxLayerMap;
}

void CTerrainShader::BuildObjects(ID3D11Device *pd3dDevice)
{
	m_nObjects = 1;
	m_ppObjects = new CGameObject*[m_nObjects];

	m_nLayerNumber = 1;
	m_pptxLayerMap = new CTexture *[m_nLayerNumber];

#ifdef TS_TERRAIN
	wchar_t  **ppHeigtName, ** ppTextureName;
	ppTextureName = new wchar_t *[m_nLayerNumber];
	ppHeigtName = new wchar_t *[m_nLayerNumber];

	for (int i = 0; i < m_nLayerNumber; ++i)
	{
		m_pptxLayerMap[i] = new CTexture(2, 2, 0, 0, (SET_SHADER_PS | SET_SHADER_VS | SET_SHADER_DS));
		ppTextureName[i] = new wchar_t[128];
		ppHeigtName[i] = new wchar_t[128];
	}
	ID3D11ShaderResourceView **ppd3dsrvHeight, **ppd3dsrvTexture;
	ppd3dsrvTexture = new ID3D11ShaderResourceView *[m_nLayerNumber];
	ppd3dsrvHeight = new ID3D11ShaderResourceView *[m_nLayerNumber];

	ppTextureName[0] = _T("../Assets/Image/Terrain/Detail_Texture_2.jpg");

	ppHeigtName[0] = _T("../Assets/Image/Terrain/HeightMap.jpg");

	for (int fileIndex = 0; fileIndex < m_nLayerNumber; fileIndex++)
	{
		D3DX11CreateShaderResourceViewFromFile(pd3dDevice, ppHeigtName[fileIndex], nullptr, nullptr, &ppd3dsrvHeight[fileIndex], nullptr);
		m_pptxLayerMap[fileIndex]->SetTexture(0, ppd3dsrvHeight[fileIndex]);
		m_pptxLayerMap[fileIndex]->SetSampler(0, TXMgr.GetSamplerState("ss_linear_clamp"));
		ppd3dsrvHeight[fileIndex]->Release();

		D3DX11CreateShaderResourceViewFromFile(pd3dDevice, ppTextureName[fileIndex], nullptr, nullptr, &ppd3dsrvTexture[fileIndex], nullptr);
		m_pptxLayerMap[fileIndex]->SetTexture(1, ppd3dsrvTexture[fileIndex]);
		m_pptxLayerMap[fileIndex]->SetSampler(1, TXMgr.GetSamplerState("ss_linear_wrap"));
		ppd3dsrvTexture[fileIndex]->Release();
	}

	delete[] ppTextureName;
	delete[] ppd3dsrvHeight;
	delete[] ppd3dsrvTexture;
#else
	wchar_t ** ppTextureName, **ppAlphaName, **ppEntityTexture;
	ppEntityTexture = new wchar_t *[m_nLayerNumber];
	ppTextureName   = new wchar_t *[m_nLayerNumber];
	ppAlphaName     = new wchar_t *[m_nLayerNumber];

	for (int i = 0; i < m_nLayerNumber; ++i)
	{
		m_pptxLayerMap[i]  = new CTexture(3, 2, 0, 0, SET_SHADER_PS);
		ppTextureName[i]   = new wchar_t[128];
		ppAlphaName[i]     = new wchar_t[128];
		ppEntityTexture[i] = new wchar_t[128];
	}
	ID3D11ShaderResourceView **ppd3dsrvTexture, **ppd3dsrvAlphaTexture;
	ppd3dsrvTexture = new ID3D11ShaderResourceView *[m_nLayerNumber];
	ppd3dsrvAlphaTexture = new ID3D11ShaderResourceView *[m_nLayerNumber];

	ppTextureName[0] = _T("../Assets/Image/Terrain/Detail_Texture_8.jpg");
	//ppTextureName[1] = _T("../Assets/Image/Terrain/Detail_Texture_6.jpg");
	//ppTextureName[2] = _T("../Assets/Image/Terrain/flower.jpg");

	ppAlphaName[0] = _T("../Assets/Image/Terrain/Alpha0.png");
	//ppAlphaName[1] = _T("../Assets/Image/Terrain/Alpha1.png");
	//ppAlphaName[2] = _T("../Assets/Image/Terrain/Alpha2.png");

	ppEntityTexture[0] = _T("../Assets/Image/Terrain/Base_Texture1.jpg");

	for (int fileIndex = 0; fileIndex < m_nLayerNumber; fileIndex++) 
	{
		D3DX11CreateShaderResourceViewFromFile(pd3dDevice, ppTextureName[fileIndex], nullptr, nullptr, &ppd3dsrvTexture[fileIndex], nullptr);
		m_pptxLayerMap[fileIndex]->SetTexture(0, ppd3dsrvTexture[fileIndex]);
		m_pptxLayerMap[fileIndex]->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));
		ppd3dsrvTexture[fileIndex]->Release();

		D3DX11CreateShaderResourceViewFromFile(pd3dDevice, ppAlphaName[fileIndex], nullptr, nullptr, &ppd3dsrvAlphaTexture[fileIndex], nullptr);
		m_pptxLayerMap[fileIndex]->SetTexture(1, ppd3dsrvAlphaTexture[fileIndex]);
		m_pptxLayerMap[fileIndex]->SetSampler(1, TXMgr.GetSamplerState("ss_linear_clamp"));
		ppd3dsrvAlphaTexture[fileIndex]->Release();

		D3DX11CreateShaderResourceViewFromFile(pd3dDevice, ppEntityTexture[fileIndex], nullptr, nullptr, &ppd3dsrvTexture[fileIndex], nullptr);
		m_pptxLayerMap[fileIndex]->SetTexture(2, ppd3dsrvTexture[fileIndex]);
		ppd3dsrvTexture[fileIndex]->Release();
	}

	delete[] ppTextureName;
	delete[] ppAlphaName;
	delete[] ppd3dsrvTexture;
	delete[] ppd3dsrvAlphaTexture;
	delete[] ppEntityTexture;
#endif

	//지형을 확대할 스케일 벡터이다. x-축과 z-축은 8배, y-축은 2배 확대한다.
	XMFLOAT3 xv3Scale(8.0f, 1.5f, 8.0f);
	const int ImageWidth = 255;
	const int ImageLength = 255;

	/*지형을 높이 맵 이미지 파일을 사용하여 생성한다. 높이 맵 이미지의 크기는 가로x세로(257x257)이고 격자 메쉬의 크기는 가로x세로(17x17)이다.
	지형 전체는 가로 방향으로 16개, 세로 방향으로 16의 격자 메쉬를 가진다. 지형을 구성하는 격자 메쉬의 개수는 총 256(16x16)개가 된다.*/

	m_ppObjects[0] = new CHeightMapTerrain(pd3dDevice, _T("../Assets/Image/Terrain/HeightMap.raw"), ImageWidth + 1, ImageLength + 1, ImageWidth + 1, ImageLength + 1, xv3Scale);
	m_ppObjects[0]->AddRef();

	XMFLOAT3 xv3Size = XMFLOAT3(ImageWidth * xv3Scale.x, 0, ImageWidth * xv3Scale.z);
	QUADMgr.BuildQuadTree(XMFLOAT3(xv3Size.x * 0.5f, 0, xv3Size.z * 0.5f), xv3Size.x, xv3Size.z, nullptr);

	m_ppObjects[0]->SetMaterial(MaterialMgr.GetObjects("Terrain"));
}
void CTerrainShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);
	//float pBlendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	//pd3dDeviceContext->OMSetBlendState(m_pd3dSplatBlendState, pBlendFactor, 0xffffffff);
	//for (int i = 0; i < m_nLayerNumber; ++i)
	for (int i = m_nLayerNumber - 1; i >= 0; --i)
	{
		//m_ppObjects[0]->SetTexture(m_pptxLayerMap[0], false);
		m_pptxLayerMap[i]->UpdateShaderVariable(pd3dDeviceContext);
		m_ppObjects[i]->SetActive(true);
		m_ppObjects[i]->Render(pd3dDeviceContext, uRenderState, pCamera);
	}
	//pd3dDeviceContext->OMSetBlendState(nullptr, pBlendFactor, 0xffffffff);
}

//CHeightMapTerrain *CTerrainShader::GetTerrain()
//{
//	return((CHeightMapTerrain *)m_ppObjects[0]);
//}

CWaterShader::CWaterShader() : CTexturedShader()
{
	m_pd3dWaterBlendState = nullptr;
}

CWaterShader::~CWaterShader()
{
	if (m_pd3dWaterBlendState) m_pd3dWaterBlendState->Release();
}

void CWaterShader::CreateShader(ID3D11Device *pd3dDevice)
{
	CTexturedShader::CreateShader(pd3dDevice);
}

void CWaterShader::BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain)
{
	//m_pTexture = pTexture;
	//if (pTexture) pTexture->AddRef();

	ID3D11SamplerState *pd3dSamplerState = nullptr;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	d3dSamplerDesc.Filter                = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU              = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.AddressV              = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.AddressW              = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.ComparisonFunc        = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD                = 0;
	d3dSamplerDesc.MaxLOD                = 0;
	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);

	ID3D11SamplerState *pd3dDetailSamplerState = nullptr;
	d3dSamplerDesc.AddressU                    = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressV                    = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressW                    = D3D11_TEXTURE_ADDRESS_WRAP;
	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dDetailSamplerState);

	/////////////////////////////////////////////////

	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	/// 물 텍스쳐///

	CTexture *pWaterTexture = new CTexture(1, 1, 0, 0);
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Terrain/water01.png"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	pWaterTexture->SetTexture(0, pd3dsrvTexture);
	pWaterTexture->SetSampler(0, pd3dSamplerState);
	pd3dsrvTexture->Release();
	pd3dSamplerState->Release();

	//ID3D11ShaderResourceView *pd3dsrvDetailTexture = nullptr;
	//D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Terrain/Water_Detail_Texture_0.dds"), nullptr, nullptr, &pd3dsrvDetailTexture, nullptr);
	//pWaterTexture->SetTexture(1, pd3dsrvDetailTexture);
	//pWaterTexture->SetSampler(1, pd3dDetailSamplerState);
	//pd3dsrvDetailTexture->Release();
	//pd3dDetailSamplerState->Release();

	//CCubeMeshTexturedIlluminated *pCubeMesh = new CCubeMeshTexturedIlluminated(pd3dDevice, 12.0f, 12.0f, 12.0f);
	CSphereMeshTexturedIlluminated *pSphereMesh = new CSphereMeshTexturedIlluminated(pd3dDevice, 5.0f, 20, 20);

	m_nObjects = 1;
	m_ppObjects = new CGameObject*[m_nObjects];

	CRotatingObject *pObject = nullptr;

	/// 이상 스테틱 객체들

	m_ppObjects[0] = new CGameObject(1);
	CCubeMeshTexturedIlluminated * CWaterMesh = new CCubeMeshTexturedIlluminated(pd3dDevice, 2048, 100, 2048);
	m_ppObjects[0]->SetMesh(CWaterMesh);
	m_ppObjects[0]->SetTexture(pWaterTexture);
	m_ppObjects[0]->SetPosition(1024, 98, 1024);

	//CMaterial *pWaterMaterial = new CMaterial();
	//pWaterMaterial->m_Material.m_xcDiffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//pWaterMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.1f, 0.1f, 0.3f, 1.0f);
	//pWaterMaterial->m_Material.m_xcEmissive = XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f);
	//pWaterMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	//m_ppObjects[0]->SetMaterial(pWaterMaterial);

	SetBlendState(pd3dDevice);
}

void CWaterShader::SetBlendState(ID3D11Device *pd3dDevice)
{
	D3D11_BLEND_DESC	d3dBlendDesc;
	ZeroMemory(&d3dBlendDesc, sizeof(D3D11_BLEND_DESC));

	d3dBlendDesc.AlphaToCoverageEnable                 = false;
	d3dBlendDesc.IndependentBlendEnable                = false;
	d3dBlendDesc.RenderTarget[0].BlendEnable           = true;
	d3dBlendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;

	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;	// 파란색 위주로 한다.

	pd3dDevice->CreateBlendState(&d3dBlendDesc, &m_pd3dWaterBlendState);
}

void CWaterShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	float pBlendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pd3dDeviceContext->OMSetBlendState(m_pd3dWaterBlendState, pBlendFactor, 0xffffffff);

	m_ppObjects[0]->Render(pd3dDeviceContext, uRenderState, pCamera);
	pd3dDeviceContext->OMSetBlendState(nullptr, pBlendFactor, 0xffffffff);
}

CSkyBoxShader::CSkyBoxShader()
{
}

CSkyBoxShader::~CSkyBoxShader()
{
}

void CSkyBoxShader::CreateShader(ID3D11Device *pd3dDevice)
{
#define _WITH_SKYBOX_TEXTURE_CUBE
#ifdef _WITH_SKYBOX_TEXTURE_CUBE
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSSkyBoxTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSSkyBoxTexturedColor", "ps_5_0", &m_pd3dPixelShader);
#else
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSTexturedColor", "ps_5_0", &m_pd3dPixelShader);
#endif
}

void CSkyBoxShader::BuildObjects(ID3D11Device *pd3dDevice)
{
	m_nObjects = 1;
	m_ppObjects = new CGameObject*[m_nObjects];

	CSkyBox *pSkyBox = new CSkyBox(pd3dDevice, 2);
	m_ppObjects[0] = pSkyBox;
	pSkyBox->AddRef();
}

void CSkyBoxShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	CShader::OnPrepareRender(pd3dDeviceContext, uRenderState);

	m_ppObjects[0]->Render(pd3dDeviceContext, uRenderState, pCamera);
}

CSSAOShader::CSSAOShader()
{
	m_pMesh = nullptr;
	m_pd3dSRVSSAO = nullptr;
	ZeroMemory(&m_ssao, sizeof(m_ssao));
}

CSSAOShader::~CSSAOShader()
{
	if (m_pd3dSRVSSAO) m_pd3dSRVSSAO->Release();
}

void CSSAOShader::CreateShader(ID3D11Device * pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/SSAO.fx", "VSSCeneSpaceAmbient", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/SSAO.fx", "PSSCeneSpaceAmbient", "ps_5_0", &m_pd3dPixelShader);
}

void CSSAOShader::BuildObjects(ID3D11Device * pd3dDevice)
{
	BuildSSAO(pd3dDevice);

	CPlaneMesh * pMesh = new CPlaneMesh(pd3dDevice, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	m_pMesh = pMesh;
}

void CSSAOShader::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);
	UpdateShaderVariable(pd3dDeviceContext, pCamera);
	TXMgr.UpdateShaderVariable(pd3dDeviceContext, "srv_random1d");
	//TXMgr.UpdateShaderVariable(pd3dDeviceContext, "srv_rtvSSAO");

	m_pMesh->Render(pd3dDeviceContext, uRenderState);
}

void CSSAOShader::BuildSSAO(ID3D11Device * pd3dDevice)
{
	float aspect = (float)FRAME_BUFFER_WIDTH / (float)FRAME_BUFFER_HEIGHT;
	float farZ = 1000.0f;
	float halfHeight = farZ * tanf(XMConvertToRadians(0.5f * 60.0f));
	float halfWidth = aspect * halfHeight;

	m_ssao.m_gFrustumCorners[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	m_ssao.m_gFrustumCorners[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	m_ssao.m_gFrustumCorners[2] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);
	m_ssao.m_gFrustumCorners[3] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);

	int index = 0;
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	m_ssao.m_gOffsetVectors[index++] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < NUM_SSAO_OFFSET; ++i)
	{
		int iMinus = i % 2 ? -1 : 1;
		float s = Chae::RandomFloat(0.0f, 0.8f) + 0.2f; //iMinus + (0.2 * iMinus);

		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&m_ssao.m_gOffsetVectors[i]));
		XMStoreFloat4(&m_ssao.m_gOffsetVectors[i], v);
	}
}

void CSSAOShader::CreateShaderVariable(ID3D11Device * pd3dDevice)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(CB_SSAO_INFO);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = pd3dDevice->CreateBuffer(&bd, nullptr, &m_pd3dcbSSAOInfo);
	if (FAILED(hr))
		printf("오류입니다!!");
}

void CSSAOShader::UpdateShaderVariable(ID3D11DeviceContext * pd3dDeviceContext, CCamera * pCamera)
{
	float fw = FRAME_BUFFER_WIDTH * 0.5f;
	float fh = FRAME_BUFFER_HEIGHT * 0.5f;
	static const XMMATRIX T(
		+fw, 0.0f, 0.0f, 0.0f,
		0.0f, -fh, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		+fw, +fh, 0.0f, 1.0f);

	XMMATRIX VP = XMLoadFloat4x4(&pCamera->GetViewProjectionMatrix());// camera.Proj();
	XMMATRIX PT = XMMatrixMultiply(VP, T);
	XMStoreFloat4x4(&m_ssao.m_gViewToTexSpace, PT);

	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbSSAOInfo, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	CB_SSAO_INFO *pcbSSAO = (CB_SSAO_INFO *)d3dMappedResource.pData;
	memcpy(pcbSSAO->m_gFrustumCorners, m_ssao.m_gFrustumCorners, sizeof(m_ssao.m_gFrustumCorners));
	memcpy(pcbSSAO->m_gOffsetVectors, m_ssao.m_gOffsetVectors, sizeof(m_ssao.m_gOffsetVectors));
	Chae::XMFloat4x4Transpose(&pcbSSAO->m_gViewToTexSpace, &m_ssao.m_gViewToTexSpace);
	pd3dDeviceContext->Unmap(m_pd3dcbSSAOInfo, 0);

	//상수 버퍼를 디바이스의 슬롯(CB_SLOT_WORLD_MATRIX)에 연결한다.
	//pd3dDeviceContext->VSSetConstantBuffers(CB_SLOT_SSAO, 1, &m_pd3dcbSSAOInfo);
	//pd3dDeviceContext->PSSetConstantBuffers(CB_SLOT_SSAO, 1, &m_pd3dcbSSAOInfo);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

CUIShader::CUIShader() : CShader()
{
	m_pBackRTV             = nullptr;
	m_pd3dScreenInfoBuffer = nullptr;
	m_pMousePoint          = nullptr;
	m_pd3dBlendState       = nullptr;
}

CUIShader::~CUIShader()
{
	//if (m_pBackRTV) m_pBackRTV->Release();
	if (m_pd3dScreenInfoBuffer) m_pd3dScreenInfoBuffer->Release();
	if (m_pMousePoint) m_pMousePoint->Release();
	if (m_pd3dBlendState) m_pd3dBlendState->Release();
}

void CUIShader::OnPrepareRender(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState)
{
	CShader::OnPrepareRender(pd3dDeviceContext, uRenderState);

	ID3D11SamplerState * pSampler = TXMgr.GetSamplerState("ss_point_wrap");
	pd3dDeviceContext->PSSetSamplers(0, 1, &pSampler);
}

void CUIShader::BuildObjects(ID3D11Device * pd3dDevice, ID3D11RenderTargetView * pBackRTV, CScene * pScene)
{
	CreateUIResources(pd3dDevice);
}

void CUIShader::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	static const float m_fWidth  = FRAME_BUFFER_WIDTH;
	static const float m_fHeight = FRAME_BUFFER_HEIGHT;

	CB_PS UIInfo = { m_fWidth, m_fHeight, 1.0f, 1.0f };
	XMFLOAT3 pos;

	pd3dDeviceContext->OMSetRenderTargets(1, &m_pBackRTV, nullptr);
	CUIShader::OnPrepareRender(pd3dDeviceContext, uRenderState);

	//pd3dDeviceContext->OMSetBlendState(m_pd3dBlendState, nullptr, 0xffffffff);

	for (int i = 0; i < m_nObjects; i++)
	{
		pos = m_ppObjects[i]->GetPosition();
		memcpy(&UIInfo.param[2], &pos, sizeof(XMFLOAT2));

		MapConstantBuffer(pd3dDeviceContext, &UIInfo, sizeof(XMFLOAT4), m_pd3dScreenInfoBuffer);
		pd3dDeviceContext->GSSetConstantBuffers(0, 1, &m_pd3dScreenInfoBuffer);

		m_ppObjects[i]->Render(pd3dDeviceContext, uRenderState, pCamera);
	}
	//pd3dDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	if (m_pMousePoint)
	{
		pos = m_pMousePoint->GetPosition();
		memcpy(&UIInfo.param[2], &pos, sizeof(XMFLOAT2));

		MapConstantBuffer(pd3dDeviceContext, &UIInfo, sizeof(XMFLOAT4), m_pd3dScreenInfoBuffer);
		pd3dDeviceContext->GSSetConstantBuffers(0, 1, &m_pd3dScreenInfoBuffer);

		m_pMousePoint->Render(pd3dDeviceContext, uRenderState, pCamera);
	}

}

void CUIShader::CreateShader(ID3D11Device * pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT nElements = ARRAYSIZE(d3dInputLayout);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Final.fx", "VS_UI_Draw", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Final.fx", "PS_UI_Draw", "ps_5_0", &m_pd3dPixelShader);
	CreateGeometryShaderFromFile(pd3dDevice, L"fx/Final.fx", "GS_UI_Draw", "gs_5_0", &m_pd3dGeometryShader);
}

void CUIShader::CreateUIResources(ID3D11Device * pd3dDevice)
{
	if (m_pd3dScreenInfoBuffer) return;
	ASSERT_S(nullptr != ( m_pd3dScreenInfoBuffer = ViewMgr.GetBuffer("cs_float4") ));
	m_pd3dScreenInfoBuffer->AddRef();

	//D3D11_BLEND_DESC d3dBlendStateDesc;
	//ZeroMemory(&d3dBlendStateDesc, sizeof(D3D11_BLEND_DESC));
	//d3dBlendStateDesc.IndependentBlendEnable                    = false;
	//int index                                                   = 0;
	//ZeroMemory(&d3dBlendStateDesc.RenderTarget[index], sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
	//d3dBlendStateDesc.AlphaToCoverageEnable                     = true;
	//d3dBlendStateDesc.RenderTarget[index].BlendEnable           = true;
	//d3dBlendStateDesc.RenderTarget[index].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
	//d3dBlendStateDesc.RenderTarget[index].DestBlend             = D3D11_BLEND_DEST_ALPHA;
	//d3dBlendStateDesc.RenderTarget[index].BlendOp               = D3D11_BLEND_OP_ADD;
	//d3dBlendStateDesc.RenderTarget[index].SrcBlendAlpha         = D3D11_BLEND_ZERO;
	//d3dBlendStateDesc.RenderTarget[index].DestBlendAlpha        = D3D11_BLEND_ZERO;
	//d3dBlendStateDesc.RenderTarget[index].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	//d3dBlendStateDesc.RenderTarget[index].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	//pd3dDevice->CreateBlendState(&d3dBlendStateDesc, &m_pd3dBlendState);
}

void CUIShader::MouseDown(HWND hWnd, POINT & pt)
{
	POINT point = pt;
	ScreenToClient(hWnd, &point);
	point.y = FRAME_BUFFER_HEIGHT - point.y;

	for (int i = 0; i < m_nObjects; ++i)
	{
		if (((CUIObject*)m_ppObjects[i])->CollisionCheck(point))
		{
			GetGameMessage(nullptr, MSG_COLLIDE_UI, &((CUIObject*)m_ppObjects[i])->GetUIMessage());
			break;
		}
	}
}

CTitleScreenShader::CTitleScreenShader() : CUIShader()
{
}

CTitleScreenShader::~CTitleScreenShader()
{
}

void CTitleScreenShader::BuildObjects(ID3D11Device * pd3dDevice, ID3D11RenderTargetView * pBackRTV, CScene * pScene)
{
	m_nObjects = 2;
	m_ppObjects = new CGameObject*[m_nObjects];
	m_pBackRTV = pBackRTV;
	//m_pBackRTV->AddRef();
	
	m_pScene = pScene;

	CPoint2DMesh * pUIMesh = nullptr;
	CGameObject  * pObject = nullptr;
	CTexture     * pTexture = nullptr;

	XMFLOAT4 InstanceData[2] =
	{
		XMFLOAT4(FRAME_BUFFER_WIDTH * 0.5f, FRAME_BUFFER_HEIGHT * 0.5f, FRAME_BUFFER_WIDTH * 0.5f, FRAME_BUFFER_HEIGHT * 0.5f),
		XMFLOAT4(FRAME_BUFFER_WIDTH * 0.5f, FRAME_BUFFER_HEIGHT * 0.5f, 600, 200)
	};
	string   UIName[2] = { { "srv_title_jpg"}, { "srv_loading.png"} };

	//m_pTexture->SetTexture(0, TXMgr.GetShaderResourceView("srv_title_jpg"));
	//m_pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));

	for (int i = 0; i < m_nObjects; i++)
	{
		pUIMesh = new CPoint2DMesh(pd3dDevice, InstanceData[i]);
		pObject = new CUIObject(1, UIMgr.GetObjects("ui_title_start"));
		pObject->SetMesh(pUIMesh);

		pTexture = new CTexture(1, 0, 0, 0, SET_SHADER_PS);
		pTexture->SetTexture(0, TXMgr.GetShaderResourceView(UIName[i]));

		pObject->SetTexture(pTexture);
		pObject->SetActive(true);
		pObject->AddRef();

		m_ppObjects[i] = pObject;
	}

	m_ppObjects[1]->SetActive(false);

	{
		pUIMesh = new CPoint2DMesh(pd3dDevice, XMFLOAT4(0.0, 0.0, 15.0f, 20.0f));
		pTexture = new CTexture(1, 0, 0, 0, SET_SHADER_PS);
		pTexture->SetTexture(0, TXMgr.GetShaderResourceView("srv_mouse1.png"));

		m_pMousePoint = new CGameObject(1);
		m_pMousePoint->SetMesh(pUIMesh);
		m_pMousePoint->SetTexture(pTexture);
		m_pMousePoint->SetActive(true);
		m_pMousePoint->AddRef();
	}
	CUIShader::CreateUIResources(pd3dDevice);
}

void CTitleScreenShader::GetGameMessage(CShader * byObj, eMessage eMSG, void * extra)
{
	UIMessage msg = *(UIMessage*)extra;

	switch (eMSG)
	{
	case eMessage::MSG_COLLIDE_UI:
		switch (msg)
		{
		case UIMessage::MSG_UI_TITLE_INSERT_INGAME :
			EVENTMgr.InsertDelayMessage(0.1f, eMessage::MSG_SCENE_CHANGE, CGameEventMgr::MSG_TYPE_SCENE, m_pScene);
			m_ppObjects[1]->SetActive(true);
			return;
		}
		return;
	default:
		return;
	}
}

CInGameUIShader::CInGameUIShader() : CUIShader()
{
}

CInGameUIShader::~CInGameUIShader()
{
}

void CInGameUIShader::BuildObjects(ID3D11Device * pd3dDevice, ID3D11RenderTargetView * pBackRTV, CScene * pScene)
{
	//m_nObjects = 1;
	//m_ppObjects = new CGameObject*[m_nObjects];
	m_pBackRTV = pBackRTV;
	//m_pBackRTV->AddRef();

	m_pScene = pScene;

	CPoint2DMesh * pUIMesh = nullptr;
	CGameObject  * pObject = nullptr;
	CTexture     * pTexture = nullptr;

	//XMFLOAT4 InstanceData[1] = { XMFLOAT4(FRAME_BUFFER_WIDTH * 0.5f, FRAME_BUFFER_HEIGHT * 0.5f, FRAME_BUFFER_WIDTH * 0.5f, FRAME_BUFFER_HEIGHT * 0.5f) };
	//string   UIName[1] = { { "srv_title_jpg", } };

	////m_pTexture->SetTexture(0, TXMgr.GetShaderResourceView("srv_title_jpg"));
	////m_pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));

	//for (int i = 0; i < m_nObjects; i++)
	//{
	//	pUIMesh = new CPoint2DMesh(pd3dDevice, InstanceData[i]);
	//	pObject = new CGameObject(1);
	//	pObject->SetMesh(pUIMesh);

	//	pTexture = new CTexture(1, 0, 0, 0, SET_SHADER_PS);
	//	pTexture->SetTexture(0, TXMgr.GetShaderResourceView(UIName[i]));

	//	pObject->SetTexture(pTexture);
	//	m_ppObjects[i] = pObject;
	//}
	{
		pUIMesh = new CPoint2DMesh(pd3dDevice, XMFLOAT4(0.0, 0.0, 15.0f, 20.0f));
		pTexture = new CTexture(1, 0, 0, 0, SET_SHADER_PS);
		pTexture->SetTexture(0, TXMgr.GetShaderResourceView("srv_mouse1.png"));
		m_pMousePoint = new CUIObject(1);
		m_pMousePoint->SetMesh(pUIMesh);
		m_pMousePoint->SetTexture(pTexture);
		m_pMousePoint->SetActive(true);
		m_pMousePoint->AddRef();
	}
	CUIShader::CreateUIResources(pd3dDevice);
}

void CInGameUIShader::GetGameMessage(CShader * byObj, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_MOUSE_DOWN:
	case eMessage::MSG_MOUSE_DOWN_OVER:
		m_pMousePoint->SetActive(false);
		return;
	case eMessage::MSG_MOUSE_UP:
		m_pMousePoint->SetActive(true);
		return;
	case eMessage::MSG_MOUSE_UP_OVER:
		return;
	}
}
