#include "stdafx.h"
#include "MyInline.h"
#include "EffectEntity.h"

///////////////////////////////////////////////////////////////////////////////////////
CEffect::CEffect() : CEntity()
{
	m_nStartVertex         = 0;
	m_nVertexOffsets       = 0;
	m_nVertexStrides       = 0;

	m_fDurability          = 0;

	m_bEnable              = false;
	m_bMove                = false;
	m_bTerminal            = false;
	m_bSubordinate         = false;
	m_bReserveDelete       = false;

	m_pd3dSRVImagesArrays  = nullptr;
	m_pd3dDrawVertexBuffer = nullptr;

	m_pMaster              = nullptr;
}

CEffect::~CEffect()
{
	if (m_pd3dSRVImagesArrays) m_pd3dSRVImagesArrays->Release();
	if (m_pd3dDrawVertexBuffer) m_pd3dDrawVertexBuffer->Release();
}

void CEffect::GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_CULL_IN:
		m_bActive = true;
		return;
	case eMessage::MSG_CULL_OUT:
		m_bActive = false;
		return;
	case eMessage::MSG_COLLIDE:
		return;
	case eMessage::MSG_COLLIDED:
		return;
	case eMessage::MSG_NORMAL:
		return;
	}
}

void CEffect::SendGameMessage(CEntity * toEntity, eMessage eMSG, void * extra)
{
	if (toEntity == m_pMaster || dynamic_cast<CAbsorbMarble*>(toEntity)) return;

	switch (eMSG)
	{
	case eMessage::MSG_NORMAL:
		return;
		// 반대로 메세지 전송하도록 하자
	case eMessage::MSG_COLLIDE:
		toEntity->GetGameMessage(this, MSG_COLLIDED);
		Collide();
		//cout << "충돌!! At : " << toEntity->GetPosition() << endl;
		return;
	case eMessage::MSG_COLLIDED:
		//toEntity->GetGameMessage(this, MSG_COLLIDE);
		//Collide();
		//cout << "충돌!! At : " << toEntity->GetPosition() << endl;
		return;
	}
}

void CEffect::Collide()
{
	if (false == m_bReserveDelete)
	{
		NextEffectOn();
	}
}

void CEffect::MoveUpdate(const float & fGameTime, const float & fTimeElapsed, XMFLOAT3 & xmf3Pos)
{
	XMVECTOR xmvVelocity;
	XMVECTOR xmvPos;

	if (m_bUseAccel)
	{
		xmvVelocity = (m_velocity.fWeightSpeed * 0.5f * XMLoadFloat3(&m_velocity.xmf3Accelate) * fGameTime * fGameTime) +
			(m_velocity.fWeightSpeed * XMLoadFloat3(&m_velocity.xmf3Velocity) * fGameTime);
		xmvPos = xmvVelocity + XMLoadFloat3(&m_velocity.xmf3InitPos);
	}
	else
	{
		xmvVelocity = XMLoadFloat3(&m_velocity.xmf3Velocity) * fTimeElapsed * m_velocity.fWeightSpeed;
		xmvPos = xmvVelocity + XMLoadFloat3(&xmf3Pos);
	}
	XMStoreFloat3(&xmf3Pos, xmvPos);
}
void CEffect::SetMoveVelocity(MoveVelocity & move, XMFLOAT3 * InitPos)
{
	m_velocity = move;

	if (Chae::XMFloat3NorValue(m_velocity.xmf3Velocity, 0.0f)) m_bMove = true;
	if (Chae::XMFloat3NorValue(m_velocity.xmf3Accelate, 0.0f)) m_bUseAccel = true;

	if (InitPos) m_velocity.xmf3InitPos = *InitPos;
}
///////////////////////////////////////////////////////////////////////////////////////

CTxAnimationObject::CTxAnimationObject()
{
	ZeroMemory(&m_cbInfo, sizeof(m_cbInfo));
	m_pNextEffect = nullptr;
	m_pd3dCSBuffer = nullptr;

	m_bUseAnimation = false;
}

CTxAnimationObject::~CTxAnimationObject()
{
	if (m_pNextEffect) delete m_pNextEffect;
	if (m_pd3dCSBuffer) m_pd3dCSBuffer->Release();
}

void CTxAnimationObject::Initialize(ID3D11Device * pd3dDevice)
{
	m_cbInfo.m_nColorNum = COLOR_WHITE;
	m_cbInfo.m_xmf3Pos = { 0, 0, 0 };

	MoveVelocity move;
	move.xmf3Velocity = { 0, 0, 0 };
	move.xmf3Accelate = { 0, 0, 0 };
	move.fWeightSpeed = 1.0f;
	SetMoveVelocity(move, &m_cbInfo.m_xmf3Pos);

	XMFLOAT2 xmf2Size{ 50, 50 };
	XMFLOAT2 xmf2ImageSize{ 1000, 200 };
	XMFLOAT2 xmf2FrameSize{ 100, 100 };
	CTxAnimationObject::CreateBuffers(pd3dDevice, xmf2Size, xmf2ImageSize, xmf2FrameSize, 15, 0.1f);
}

void CTxAnimationObject::CreateBuffers(ID3D11Device * pd3dDevice, XMFLOAT2 & xmf2ObjSize, XMFLOAT2 & xmf2ImageSize, XMFLOAT2 & xmf2FrameSize, UINT dwFrameNum, float dwFramePerTime)
{
	TX_ANIMATION_VERTEX vertex;
	vertex.xmf2FrameTotalSize = xmf2ImageSize;

	CTxAnimationObject::CalculateCSInfoTime(vertex, xmf2ObjSize, xmf2FrameSize, dwFrameNum, dwFramePerTime);

	m_nVertexStrides = sizeof(TX_ANIMATION_VERTEX);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = m_nVertexStrides * 1;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;// | D3D11_BIND_STREAM_OUTPUT;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = &vertex;// pQuadPatchVertices;
	ASSERT_S(pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dDrawVertexBuffer));

	m_pd3dCSBuffer = ViewMgr.GetBuffer("cs_tx_animation");
	m_pd3dCSBuffer->AddRef();

}

void CTxAnimationObject::CalculateCSInfoTime(TX_ANIMATION_VERTEX & vertex, XMFLOAT2 & xmf2ObjSize, XMFLOAT2 & xmf2FrameSize, UINT dwFrameNum, float dwFramePerTime)
{
	UINT uWidth = UINT(vertex.xmf2FrameTotalSize.x / xmf2FrameSize.x);
	UINT uHeight = UINT(vertex.xmf2FrameTotalSize.y / xmf2FrameSize.y);

	vertex.xmf2FrameRatePercent.x = 1.0f / (float)uWidth;
	vertex.xmf2FrameRatePercent.y = 1.0f / (float)uHeight;

	m_cbInfo.m_xmf2Size = xmf2ObjSize;
	m_cbInfo.m_fFramePerTime = dwFramePerTime;
	m_cbInfo.m_bMove = m_bMove;

	m_fDurability = (dwFramePerTime * dwFrameNum);// +0.01f;
}

void CTxAnimationObject::UpdateShaderVariable(ID3D11DeviceContext * pd3dDeviceContext)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dCSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	CB_TX_ANIMATION_INFO *pcbParticle = (CB_TX_ANIMATION_INFO*)d3dMappedResource.pData;
	memcpy(pcbParticle, &m_cbInfo, sizeof(CB_TX_ANIMATION_INFO));
	pd3dDeviceContext->Unmap(m_pd3dCSBuffer, 0);
}

bool CTxAnimationObject::Enable(CGameObject * pObj, XMFLOAT3 * pos, int nColorNum)
{
	if (pos)
	{
		m_cbInfo.m_xmf3Pos = *pos;
		m_velocity.xmf3InitPos = *pos;
	}
	m_pMaster = pObj;

	m_bEnable = true;
	m_bTerminal = false;
	m_bUseAnimation = true;

	m_cbInfo.m_fGameTime = 0.0;

	if (nColorNum != COLOR_NONE) m_cbInfo.m_nColorNum = nColorNum;

	m_bReserveDelete = false;

	return true;
}

bool CTxAnimationObject::Disable()
{
	cout << "exit time : " << m_cbInfo.m_fGameTime << endl;
	return (m_bEnable = false);
}

void CTxAnimationObject::NextEffectOn()
{
	if (m_pNextEffect)
	{
		m_pNextEffect->Enable(m_pMaster, &m_cbInfo.m_xmf3Pos);
		m_bTerminal = true;
	}
	else
		Disable();

	m_bReserveDelete = true;
}

void CTxAnimationObject::Animate(float fTimeElapsed)
{
	if (m_bTerminal)
	{
		m_pNextEffect->Animate(fTimeElapsed);
		if (m_pNextEffect->IsTermainal())
			Disable();
	}
	else if (m_bUseAnimation)
	{
		m_cbInfo.m_fGameTime += fTimeElapsed;

		if (m_cbInfo.m_fGameTime > m_fDurability)
		{
			NextEffectOn();
		}
		else if (m_bMove)
		{
			MoveUpdate(m_cbInfo.m_fGameTime, fTimeElapsed, m_cbInfo.m_xmf3Pos);
			m_cbInfo.m_xmf3LookVector = m_velocity.xmf3Velocity;
		}
	}
}

void CTxAnimationObject::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld)
{
	if (m_bTerminal)
	{
		m_pNextEffect->Render(pd3dDeviceContext, uRenderState, pCamera, pmtxParentWorld);
	}
	else
	{
		OnPrepare(pd3dDeviceContext);

		UpdateShaderVariable(pd3dDeviceContext);
		pd3dDeviceContext->GSSetConstantBuffers(0x04, 1, &m_pd3dCSBuffer);

		pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dDrawVertexBuffer, &m_nVertexStrides, &m_nVertexOffsets);
		pd3dDeviceContext->Draw(1, 0);
	}
}

void CCircleMagic::Initialize(ID3D11Device * pd3dDevice)
{
	m_cbInfo.m_nColorNum = COLOR_GRAY;
	m_cbInfo.m_xmf3Pos = { 0, 0, 0 };

	MoveVelocity move;
	move.xmf3Velocity = { 0, 0, 0 };
	move.xmf3Accelate = { 0, 0, 0 };
	move.fWeightSpeed = 1.0f;
	SetMoveVelocity(move, &m_cbInfo.m_xmf3Pos);

	float fSize = m_uSize = 60;
	XMFLOAT2 xmf2Size{ fSize, fSize };
	XMFLOAT2 xmf2ImageSize{ 960, 768 };
	XMFLOAT2 xmf2FrameSize{ 192, 192 };
	CTxAnimationObject::CreateBuffers(pd3dDevice, xmf2Size, xmf2ImageSize, xmf2FrameSize, 20, 0.05f);

	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_sprite_circle.png"));
}

void CIceSpear::Initialize(ID3D11Device * pd3dDevice)
{
	m_cbInfo.m_nColorNum = COLOR_WHITE;
	m_cbInfo.m_xmf3Pos = { 0, 0, 0 };

	MoveVelocity move;
	move.xmf3Velocity = { 0, 0, 0 };
	move.xmf3Accelate = { 0, 0, 0 };
	move.fWeightSpeed = 1.0f;
	SetMoveVelocity(move, &m_cbInfo.m_xmf3Pos);

	float fSize = m_uSize = 20;
	XMFLOAT2 xmf2Size{ fSize, fSize };
	XMFLOAT2 xmf2ImageSize{ 960, 576 };
	XMFLOAT2 xmf2FrameSize{ 192, 192 };
	CTxAnimationObject::CreateBuffers(pd3dDevice, xmf2Size, xmf2ImageSize, xmf2FrameSize, 13, 0.05f);

	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_sprite_ice01.png"));
}

void CElementSpike::Initialize(ID3D11Device * pd3dDevice)
{
	m_cbInfo.m_nColorNum = COLOR_WHITE;
	m_cbInfo.m_xmf3Pos = { 0, 0, 0 };

	MoveVelocity move;
	move.xmf3Velocity = { 0, 0, 0 };
	move.xmf3Accelate = { 0, 0, 0 };
	move.fWeightSpeed = 1.0f;
	SetMoveVelocity(move, &m_cbInfo.m_xmf3Pos);

	float fSize = m_uSize = 20;
	XMFLOAT2 xmf2Size{ fSize, fSize };
	XMFLOAT2 xmf2ImageSize{ 640, 128 };
	XMFLOAT2 xmf2FrameSize{ 128, 128 };
	CTxAnimationObject::CreateBuffers(pd3dDevice, xmf2Size, xmf2ImageSize, xmf2FrameSize, 5, 0.1f);
	m_cbInfo.m_bMove = false;

	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_sprite_spike.png"));
}

void CIceBolt::Initialize(ID3D11Device * pd3dDevice)
{
	m_cbInfo.m_nColorNum = COLOR_WHITE;
	m_cbInfo.m_xmf3Pos = { 0, 0, 0 };

	MoveVelocity move;
	move.xmf3Velocity = { 0, 0, 1 };
	move.xmf3Accelate = { 0, 0, 1 };
	move.fWeightSpeed = 10.0f;
	SetMoveVelocity(move, &m_cbInfo.m_xmf3Pos);

	float fSize = m_uSize = 10;
	XMFLOAT2 xmf2Size{ fSize, fSize };
	XMFLOAT2 xmf2ImageSize{ 128, 128 };
	XMFLOAT2 xmf2FrameSize{ 128, 128 };
	CTxAnimationObject::CreateBuffers(pd3dDevice, xmf2Size, xmf2ImageSize, xmf2FrameSize, 1, 5.0f);
	m_cbInfo.m_bMove = false;

	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_sprite_ice_bolt.png"));

	m_pNextEffect = new CElementSpike();
	m_pNextEffect->Initialize(pd3dDevice);
}

void CElectricBolt::Initialize(ID3D11Device * pd3dDevice)
{
	m_cbInfo.m_nColorNum = COLOR_WHITE;
	m_cbInfo.m_xmf3Pos = { 0, 0, 0 };

	MoveVelocity move;
	move.xmf3Velocity = { 0, 0, 1 };
	move.xmf3Accelate = { 0, 0, 1 };
	move.fWeightSpeed = 10.0f;
	SetMoveVelocity(move, &m_cbInfo.m_xmf3Pos);

	float fSize = m_uSize = 8;
	XMFLOAT2 xmf2Size{ fSize, fSize };
	XMFLOAT2 xmf2ImageSize{ 128, 128 };
	XMFLOAT2 xmf2FrameSize{ 128, 128 };
	CTxAnimationObject::CreateBuffers(pd3dDevice, xmf2Size, xmf2ImageSize, xmf2FrameSize, 1, 5.0f);
	m_cbInfo.m_bMove = false;

	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_sprite_electric_bolt.png"));

	m_pNextEffect = new CElementSpike();
	m_pNextEffect->Initialize(pd3dDevice);
}

////////////////////////////////////////////////////////////////////////////////////////
CParticle::CParticle() : CEffect()
{
	m_pd3dInitialVertexBuffer = nullptr;
	m_pd3dStreamOutVertexBuffer = nullptr;

	m_pcNextParticle = nullptr;

	ZeroMemory(&m_cbParticle, sizeof(CB_PARTICLE));

	m_bInitilize = false;

	m_pd3dCSParticleBuffer = nullptr;
}

CParticle::~CParticle()
{
	if (m_pd3dStreamOutVertexBuffer) m_pd3dStreamOutVertexBuffer->Release();
	if (m_pd3dInitialVertexBuffer) m_pd3dInitialVertexBuffer->Release();

	if (m_pcNextParticle) delete m_pcNextParticle;
	if (m_pd3dCSParticleBuffer) m_pd3dCSParticleBuffer->Release();
}

void CParticle::Initialize(ID3D11Device *pd3dDevice)
{
	CB_PARTICLE cbParticle;
	ZeroMemory(&cbParticle, sizeof(CB_PARTICLE));
	MoveVelocity mov;

	CParticle::SetParticle(cbParticle, mov, 1.0f, 200.0f);

	PARTICLE_INFO cParticle;
	ZeroMemory(&cParticle, sizeof(PARTICLE_INFO));

	CParticle::CreateParticleBuffer(pd3dDevice, cParticle, 200.0f);
}

void CParticle::SetParticle(CB_PARTICLE & info, MoveVelocity & Velocity, float fDurability, UINT uMaxParticle)
{
	SetDurabilityTime(fDurability);
	Velocity.xmf3InitPos = m_cbParticle.m_vParticleEmitPos;

	m_bEnable = false;
	m_bInitilize = true;
	ZeroMemory(&m_cbParticle, sizeof(CB_PARTICLE));

	m_cbParticle = info;
	m_cbParticle.m_bEnable = 1.0f;
	SetMoveVelocity(Velocity, &m_cbParticle.m_vParticleEmitPos);
}

void CParticle::CreateParticleBuffer(ID3D11Device * pd3dDevice, PARTICLE_INFO & pcInfo, UINT nMaxNum)
{
	m_nVertexStrides = sizeof(PARTICLE_INFO);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = m_nVertexStrides;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;// | D3D11_BIND_STREAM_OUTPUT;

	HRESULT hr = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = &pcInfo;// pQuadPatchVertices;
	ASSERT_S(hr = pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dInitialVertexBuffer));

	d3dBufferDesc.ByteWidth = m_nVertexStrides * nMaxNum;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	// SO으로 받을 버퍼는 서브리소스 데이터 값을 nullptr로 해야한다!!
	ASSERT_S(hr = pd3dDevice->CreateBuffer(&d3dBufferDesc, nullptr, &m_pd3dStreamOutVertexBuffer));
	ASSERT_S(hr = pd3dDevice->CreateBuffer(&d3dBufferDesc, nullptr, &m_pd3dDrawVertexBuffer));

	m_pd3dCSParticleBuffer = ViewMgr.GetBuffer("cs_particle");
	m_pd3dCSParticleBuffer->AddRef();
	//D3D11_QUERY_DESC qd;
	//qd.Query = D3D11_QUERY_SO_STATISTICS;
	//qd.MiscFlags = D3D11_QUERY_MISC_PREDICATEHINT;

	//ASSERT_S(pd3dDevice->CreateQuery(&qd, &m_pd3dQuery));
}

void CParticle::StreamOut(ID3D11DeviceContext *pd3dDeviceContext)
{
	pd3dDeviceContext->SOSetTargets(1, &m_pd3dStreamOutVertexBuffer, &m_nVertexOffsets);
	// 원인은 무엇인가??
	static const UINT strides[] = { (m_nVertexStrides * 3) };

	if (m_bTerminal)
		m_pcNextParticle->StreamOut(pd3dDeviceContext);
	else
	{
		UpdateShaderVariable(pd3dDeviceContext);
		pd3dDeviceContext->GSSetConstantBuffers(0x04, 1, &m_pd3dCSParticleBuffer);

		if (m_bInitilize)
		{
			pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dInitialVertexBuffer, strides, &m_nVertexOffsets);
			pd3dDeviceContext->Draw(1, 0);
			m_bInitilize = false;
		}
		else
		{
			pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dDrawVertexBuffer, strides, &m_nVertexOffsets);
			pd3dDeviceContext->DrawAuto();
		}
	}
}

void CParticle::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld)
{
	if (m_bTerminal)
	{
		m_pcNextParticle->Render(pd3dDeviceContext, uRenderState, pCamera, pmtxParentWorld);
	}
	else
	{
		swap(m_pd3dDrawVertexBuffer, m_pd3dStreamOutVertexBuffer);

		OnPrepare(pd3dDeviceContext);
		UpdateShaderVariable(pd3dDeviceContext);
		pd3dDeviceContext->VSSetConstantBuffers(0x04, 1, &m_pd3dCSParticleBuffer);

		pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dDrawVertexBuffer, &m_nVertexStrides, &m_nVertexOffsets);
		pd3dDeviceContext->DrawAuto();
	}
}

void CParticle::UpdateShaderVariable(ID3D11DeviceContext * pd3dDeviceContext)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dCSParticleBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	CB_PARTICLE *pcbParticle = (CB_PARTICLE *)d3dMappedResource.pData;
	memcpy(pcbParticle, GetCBParticle(), sizeof(CB_PARTICLE));
	pd3dDeviceContext->Unmap(m_pd3dCSParticleBuffer, 0);
}

//void CParticle::GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra)
//{
//	switch (eMSG)
//	{
//	case eMessage::MSG_CULL_IN:
//		m_bActive = true;
//		return;
//	case eMessage::MSG_CULL_OUT:
//		m_bActive = false;
//		return;
//	case eMessage::MSG_COLLIDE:
//		return;
//	case eMessage::MSG_COLLIDED:
//		return;
//	case eMessage::MSG_NORMAL:
//		return;
//	}
//}
//
//void CParticle::SendGameMessage(CEntity * toEntity, eMessage eMSG, void * extra)
//{
//	switch (eMSG)
//	{
//	case eMessage::MSG_NORMAL:
//		return;
//		// 반대로 메세지 전송하도록 하자
//	case eMessage::MSG_COLLIDE:
//		toEntity->GetGameMessage(this, MSG_COLLIDED);
//		Disable();
//		return;
//	case eMessage::MSG_COLLIDED:
//		toEntity->GetGameMessage(this, MSG_COLLIDE);
//		return;
//	}
//}

void CParticle::Update(float fTimeElapsed)
{
	if (m_bTerminal)
	{
		m_pcNextParticle->Update(fTimeElapsed);
		if (m_pcNextParticle->IsTermainal())
			Disable();
	}
	else
	{
		m_cbParticle.m_fGameTime += fTimeElapsed;
		m_cbParticle.m_fTimeStep = fTimeElapsed;
		float fGameTime = m_cbParticle.m_fGameTime;

		LifeUpdate(fGameTime, fTimeElapsed);
		if (m_bMove && m_cbParticle.m_bEnable)
		{
			QUADMgr.CollisionCheck(this);
			MoveUpdate(fGameTime, fTimeElapsed, m_cbParticle.m_vParticleEmitPos);
		}
	}
}

void CParticle::LifeUpdate(const float & fGameTime, const float & fTimeElapsed)
{
	if (fGameTime > m_fDurability + m_cbParticle.m_fLifeTime)
	{
		m_cbParticle.m_fGameTime = 0.0f;
		if (false == m_bTerminal) Disable();
	}
	else if (fGameTime > m_fDurability)
	{
		m_cbParticle.m_bEnable = 0;
		NextEffectOn();
	}
}

bool CParticle::Enable(CGameObject * pObj, XMFLOAT3 * pos, int nColorNum)
{
	if (pos)
	{
		SetEmitPosition(*pos);
		m_velocity.xmf3InitPos = *pos;
	}

	if (m_bMove)
	{
		UpdateBoundingBox();
		cout << "BB : " << m_bcMeshBoundingCube << endl;
		QUADMgr.InsertDynamicEntity(this);
	}
	//else QUADMgr.InsertStaticEntity(this);
	m_pMaster = pObj;

	m_bReserveDelete = false;
	m_bActive = true;
	m_bEnable = true;
	m_bTerminal = false;
	m_bInitilize = true;
	m_cbParticle.m_bEnable = 1;
	m_cbParticle.m_fGameTime = 0;

	if (nColorNum != COLOR_NONE) m_cbParticle.m_nColorNum = nColorNum;
	return true;
}

bool CParticle::Disable()
{
	if (m_bMove) QUADMgr.DeleteDynamicEntity(this);
	//else QUADMgr.DeleteStaticEntity(this);

	return (m_bEnable = false);
}

void CParticle::NextEffectOn()
{
	if (m_pcNextParticle)
	{
		m_pcNextParticle->Enable(m_pMaster, &m_cbParticle.m_vParticleEmitPos);
		m_bTerminal = true;
	}
	else
	{
		m_cbParticle.m_fGameTime = m_fDurability;
		Disable();
	}
	m_bReserveDelete = true;
}

void CSmokeBoomParticle::Initialize(ID3D11Device * pd3dDevice)
{
	static CB_PARTICLE cbParticle;
	ZeroMemory(&cbParticle, sizeof(CB_PARTICLE));

	cbParticle.m_fLifeTime = 1.0f;
	cbParticle.m_vAccel = XMFLOAT3(20, 20, 20);
	cbParticle.m_vParticleEmitPos = XMFLOAT3(0, 0, 0);
	cbParticle.m_vParticleVelocity = XMFLOAT3(20, 20, 20);
	cbParticle.m_fNewTime = 0.008f;
	cbParticle.m_fMaxSize = 8.0f;
	cbParticle.m_nColorNum = COLOR_GRAY;

	m_uSize = 20;

	MoveVelocity mov;

	CParticle::SetParticle(cbParticle, mov, 0.5f, m_nMaxParticlenum);
	//m_bMove = false;

	PARTICLE_INFO cParticle;
	ZeroMemory(&cParticle, sizeof(PARTICLE_INFO));

	CParticle::CreateParticleBuffer(pd3dDevice, cParticle, m_nMaxParticlenum);
	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_particle_smoke_array"));
}

void CFireBallParticle::Initialize(ID3D11Device * pd3dDevice)
{
	static CB_PARTICLE cbParticle;
	ZeroMemory(&cbParticle, sizeof(CB_PARTICLE));

	cbParticle.m_fLifeTime = 1.0f;
	cbParticle.m_vAccel = XMFLOAT3(-20, 20, -20);
	cbParticle.m_vParticleEmitPos = XMFLOAT3(0, 0, 0);
	cbParticle.m_vParticleVelocity = XMFLOAT3(20, -20, 20);
	cbParticle.m_fNewTime = 0.01f;
	cbParticle.m_fMaxSize = 16.0f;
	cbParticle.m_nColorNum = COLOR_WHITE;

	m_uSize = 10;

	MoveVelocity mov;
	mov.xmf3Velocity = XMFLOAT3(0, 0, 10);
	mov.xmf3Accelate = XMFLOAT3(0, 0, -10);
	mov.fWeightSpeed = 100.0f;

	CParticle::SetParticle(cbParticle, mov, 2.0f, m_nMaxParticlenum);

	PARTICLE_INFO cParticle;
	ZeroMemory(&cParticle, sizeof(PARTICLE_INFO));

	CParticle::CreateParticleBuffer(pd3dDevice, cParticle, m_nMaxParticlenum);
	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_particle_fire_array"));

	m_pcNextParticle = new CSmokeBoomParticle();
	m_pcNextParticle->Initialize(pd3dDevice);
	CB_PARTICLE * pParticle = m_pcNextParticle->GetCBParticle();
	pParticle->m_fMaxSize = 8.0f;
	pParticle->m_fNewTime = 0.0005f;
	pParticle->m_vParticleVelocity = XMFLOAT3(20, 20, 20);
	pParticle->m_vAccel = XMFLOAT3(40, 40, 40);
	pParticle->m_nColorNum = COLOR_GRAY;
}
//
//void CRainParticle::StreamOut(ID3D11DeviceContext * pd3dDeviceContext)
//{
//	pd3dDeviceContext->SOSetTargets(1, &m_pd3dStreamOutVertexBuffer, &m_nVertexOffsets);
//	// 원인은 무엇인가??
//	static const UINT strides[] = { (m_nVertexStrides) };
//	//pd3dDeviceContext->Begin(m_pd3dQuery);
//
//	UpdateShaderVariable(pd3dDeviceContext);
//	if (m_bInitilize)
//	{
//		pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dInitialVertexBuffer, strides, &m_nVertexOffsets);
//		pd3dDeviceContext->Draw(1, 0);
//		m_bInitilize = false;
//	}
//	else
//	{
//		pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dDrawVertexBuffer, strides, &m_nVertexOffsets);
//		pd3dDeviceContext->DrawAuto();
//	}
//}

void CRainParticle::Initialize(ID3D11Device * pd3dDevice)
{
	CB_PARTICLE cbParticle;
	ZeroMemory(&cbParticle, sizeof(CB_PARTICLE));

	cbParticle.m_fLifeTime = 1.0f;
	cbParticle.m_vAccel = XMFLOAT3(0, -40, 0);
	cbParticle.m_vParticleEmitPos = XMFLOAT3(512, 380, 256);
	cbParticle.m_vParticleVelocity = XMFLOAT3(0, -300, 0);
	cbParticle.m_fNewTime = 0.01f;
	cbParticle.m_fMaxSize = 10.0f;
	cbParticle.m_nColorNum = COLOR_WHITE;

	MoveVelocity mov = MoveVelocity();

	CParticle::SetParticle(cbParticle, mov, 100.0f, m_nMaxParticlenum);

	PARTICLE_INFO cParticle;
	ZeroMemory(&cParticle, sizeof(PARTICLE_INFO));

	CParticle::CreateParticleBuffer(pd3dDevice, cParticle, m_nMaxParticlenum);
}