#include "stdafx.h"
#include "MyInline.h"
#include "Player.h"
#include "Camera.h"

CCamera::CCamera(CCamera *pCamera)
{
	//Chae::XMFloat4x4Identity(&m_xmf44ViewProjection);
	if (pCamera)
	{
		//카메라가 이미 있으면 기존 카메라의 정보를 새로운 카메라에 복사한다.
		m_xv3Position         = pCamera->GetPosition();
		m_xv3Right            = pCamera->GetRightVector();
		m_xv3Look             = pCamera->GetLookVector();
		m_xv3Up               = pCamera->GetUpVector();
		m_fPitch              = pCamera->GetPitch();
		m_fRoll               = pCamera->GetRoll();
		m_fYaw                = pCamera->GetYaw();

		m_xmf44View           = pCamera->GetViewMatrix();
		m_xmf44Projection     = pCamera->GetProjectionMatrix();
		m_xmf44ViewProjection = pCamera->GetViewProjectionMatrix();
		m_d3dViewport         = pCamera->GetViewport();
		m_xv3LookAtWorld      = pCamera->GetLookAtPosition();
		m_xv3Offset           = pCamera->GetOffset();
		m_fTimeLag            = pCamera->GetTimeLag();
		m_pPlayer             = pCamera->GetPlayer();
		m_pd3dcbCamera        = pCamera->GetCameraConstantBuffer();
		m_pd3dcbCameraPos     = pCamera->GetCameraConstantBuffer();
		if (m_pd3dcbCamera) m_pd3dcbCamera->AddRef();
	}
	else
	{
		//카메라가 없으면 기본 정보를 설정한다.
		m_xv3Position         = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xv3Right            = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_xv3Up               = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_xv3Look             = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_fPitch              = 0.0f;
		m_fRoll               = 0.0f;
		m_fYaw                = 0.0f;
		m_fTimeLag            = 0.0f;
		m_xv3LookAtWorld      = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xv3Offset           = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_nMode               = 0x00;
		m_pPlayer             = nullptr;
		Chae::XMFloat4x4Identity(&m_xmf44View);
		Chae::XMFloat4x4Identity(&m_xmf44Projection);
		m_pd3dcbCamera        = nullptr;
		m_pd3dcbCameraPos     = nullptr;
	}
}

CCamera::~CCamera()
{
	if (m_pd3dcbCamera) m_pd3dcbCamera->Release();
	if (m_pd3dcbCameraPos) m_pd3dcbCameraPos->Release();
}

void CCamera::SetViewport(ID3D11DeviceContext *pd3dDeviceContext, DWORD xTopLeft, DWORD yTopLeft, DWORD nWidth, DWORD nHeight, float fMinZ, float fMaxZ)
{
	m_d3dViewport.TopLeftX = float(xTopLeft);
	m_d3dViewport.TopLeftY = float(yTopLeft);
	m_d3dViewport.Width    = float(nWidth);
	m_d3dViewport.Height   = float(nHeight);
	m_d3dViewport.MinDepth = fMinZ;
	m_d3dViewport.MaxDepth = fMaxZ;
	pd3dDeviceContext->RSSetViewports(1, &m_d3dViewport);
}

void CCamera::SetViewport(ID3D11DeviceContext * pd3dDeviceContext, DWORD nWidth, DWORD nHeight)
{
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX      = 0.0f;//-nWidth;
	viewport.TopLeftY	   = 0.0f;//-nHeight;
	viewport.Width         = float(nWidth);
	viewport.Height        = float(nHeight);
	viewport.MinDepth      = 0.0f;
	viewport.MaxDepth      = 1.0f;
	pd3dDeviceContext->RSSetViewports(1, &viewport);
}

void CCamera::Update(XMFLOAT3& xv3LookAt, float fTimeElapsed)
{
}

void CCamera::GenerateViewMatrix()
{
	XMMATRIX xmtxView = XMMatrixLookAtLH(XMLoadFloat3(&m_xv3Position), XMLoadFloat3(&m_pPlayer->GetPosition()), XMLoadFloat3(&m_xv3Up));
	XMStoreFloat4x4(&m_xmf44View, xmtxView);
	MakeViewProjectionMatrix();
}

void CCamera::GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle)
{
	XMMATRIX xmtxFov = XMMatrixPerspectiveFovLH(XMConvertToRadians(fFOVAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
	XMStoreFloat4x4(&m_xmf44Projection, xmtxFov);
	MakeViewProjectionMatrix();
}

void CCamera::MakeViewProjectionMatrix()
{
	Chae::XMFloat4x4Mul(&m_xmf44ViewProjection, &m_xmf44View, &m_xmf44Projection);
}

void CCamera::SetPlayer(CPlayer *pPlayer)
{
	m_pPlayer = pPlayer;

	XMFLOAT3 xmfPlayerPos = pPlayer->GetPosition();

	xmfPlayerPos.x += m_xv3Offset.x;
	xmfPlayerPos.y += m_xv3Offset.y;
	xmfPlayerPos.z += m_xv3Offset.z;

	m_xv3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xv3Right    = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xv3Up       = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xv3Look     = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_fPitch      = 0.0f;
	m_fRoll       = 0.0f;
	m_fYaw        = 0.0f;

	SetPosition(xmfPlayerPos);
	SetLookAt(m_pPlayer->GetPosition());
	RegenerateViewMatrix();
}

void CCamera::SetOffset(XMFLOAT3 xv3Offset) {
	m_xv3Offset = xv3Offset;
	Chae::XMFloat3Add(&m_xv3Position, &m_xv3Position, &xv3Offset);
}

void CCamera::Move(XMFLOAT3& xv3Shift)
{
	Chae::XMFloat3Add(&m_xv3Position, &m_xv3Position, &xv3Shift);
	//m_xv3Position = m_xv3Position, xv3Shift;
}

void CCamera::RegenerateViewMatrix()
{
	XMVECTOR xmvLook = XMLoadFloat3(&m_xv3Look);
	XMVECTOR xmvUp = XMLoadFloat3(&m_xv3Up);
	XMVECTOR xmvRight;
	XMVECTOR xmvPostion = XMLoadFloat3(&m_xv3Position);

	//카메라의 z-축 벡터를 정규화한다.
	xmvLook = XMVector3Normalize(xmvLook); //Chae::XMFloat3Normalize(&m_xv3Look);
	//카메라의 z-축과 y-축에 수직인 벡터를 x-축으로 설정한다.
	xmvRight = XMVector3Cross(xmvUp, xmvLook); //xv3ec3Cross(&m_xv3Right, &m_xv3Up, &m_xv3Look);
	//카메라의 x-축 벡터를 정규화한다.
	XMVector3Normalize(xmvRight);
	//카메라의 z-축과 x-축에 수직인 벡터를 y-축으로 설정한다.
	xmvUp = XMVector3Cross(xmvLook, xmvRight);	//xv3ec3Cross(&m_xv3Up, &m_xv3Look, &m_xv3Right);
	//카메라의 y-축 벡터를 정규화한다.
	XMVector3Normalize(xmvUp);	//xv3ec3Normalize(&m_xv3Up, &m_xv3Up);

	m_xmf44View._11 = m_xv3Right.x;
	m_xmf44View._12 = m_xv3Up.x;
	m_xmf44View._13 = m_xv3Look.x;
	m_xmf44View._21 = m_xv3Right.y;
	m_xmf44View._22 = m_xv3Up.y;
	m_xmf44View._23 = m_xv3Look.y;
	m_xmf44View._31 = m_xv3Right.z;
	m_xmf44View._32 = m_xv3Up.z;
	m_xmf44View._33 = m_xv3Look.z;

	XMStoreFloat(&m_xmf44View._41, -XMVector3Dot(xmvPostion, xmvRight));
	XMStoreFloat(&m_xmf44View._42, -XMVector3Dot(xmvPostion, xmvUp)); 
	XMStoreFloat(&m_xmf44View._43, -XMVector3Dot(xmvPostion, xmvLook)); 
	//카메라의 위치와 방향이 바뀌면(카메라 변환 행렬이 바뀌면) 절두체 평면을 다시 계산한다.
	CalculateFrustumPlanes();

	XMStoreFloat3(&m_xv3Look, xmvLook);
	XMStoreFloat3(&m_xv3Up, xmvUp);
	XMStoreFloat3(&m_xv3Right, xmvRight);
}

void CCamera::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage          = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth      = sizeof(VS_CB_VIEWPROJECTION);
	bd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pd3dDevice->CreateBuffer(&bd, nullptr, &m_pd3dcbCamera);

	//bd.ByteWidth = sizeof(VS_CB_CAMERAPOS);
	//pd3dDevice->CreateBuffer(&bd, nullptr, &m_pd3dcbCameraPos);
}

void CCamera::UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext, XMFLOAT4X4 & xmf44ViewProj, XMFLOAT3 & xmfCameraPos)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	/*상수 버퍼의 메모리 주소를 가져와서 카메라 변환 행렬과 투영 변환 행렬을 복사한다. 쉐이더에서 행렬의 행과 열이 바뀌는 것에 주의하라.*/
	pd3dDeviceContext->Map(m_pd3dcbCamera, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	VS_CB_VIEWPROJECTION *pcbViewProjection = (VS_CB_VIEWPROJECTION *)d3dMappedResource.pData;
	pcbViewProjection->m_xf3CameraPos = XMFLOAT4(xmfCameraPos.x, xmfCameraPos.y, xmfCameraPos.z, 1);//XMFLOAT4(xmPos.x, xmPos.y, xmPos.z, 1.0f);
	Chae::XMFloat4x4Transpose(&pcbViewProjection->m_xmf44View, &xmf44ViewProj);	//XMFLOAT4X4Transpose(&pcbViewProjection->m_xmf44View, &m_xmf44View);
	pd3dDeviceContext->Unmap(m_pd3dcbCamera, 0);
	
	//상수 버퍼를 슬롯(VS_SLOT_CAMERA)에 설정한다.
	pd3dDeviceContext->VSSetConstantBuffers(CB_SLOT_VIEWPROJECTION, 1, &m_pd3dcbCamera);
	pd3dDeviceContext->GSSetConstantBuffers(CB_SLOT_VIEWPROJECTION, 1, &m_pd3dcbCamera);
	//pd3dDeviceContext->HSSetConstantBuffers(CB_SLOT_VIEWPROJECTION, 1, &m_pd3dcbCamera);
	//pd3dDeviceContext->DSSetConstantBuffers(CB_SLOT_VIEWPROJECTION, 1, &m_pd3dcbCamera);
}

void CCamera::UpdateCameraPositionCBBuffer(ID3D11DeviceContext *pd3dDeviceContext)
{
	//D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
}
CSpaceShipCamera::CSpaceShipCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = SPACESHIP_CAMERA;
}

void CSpaceShipCamera::Rotate(float x, float y, float z)
{
	if (x == 0.0f && y == 0.0f && z == 0.0f) return;

	XMMATRIX mtxRotate;
	XMVECTOR xmvRight = XMLoadFloat3(&m_xv3Right);
	XMVECTOR xmvUp = XMLoadFloat3(&m_xv3Up);
	XMVECTOR xmvLook = XMLoadFloat3(&m_xv3Look);
	XMVECTOR xmvAxisVector;

	XMVECTOR xmvCameraPos = XMLoadFloat3(&m_xv3Position);
	XMVECTOR xmvPlayerPos = XMLoadFloat3(&m_pPlayer->GetPosition());

	if (m_pPlayer && (x != 0.0f))
	{
		xmvAxisVector = XMLoadFloat3(&m_pPlayer->GetRightVector());

		//플레이어의 로컬 x-축에 대한 x 각도의 회전 행렬을 계산한다.
		mtxRotate = XMMatrixRotationAxis(xmvAxisVector, XMConvertToRadians(x));	//XMFLOAT4X4RotationAxis(&mtxRotate, &m_pPlayer->GetRightVector(), (float)D3DXToRadian(x));
		//카메라의 로컬 x-축, y-축, z-축을 회전한다.
		xmvRight = XMVector3TransformNormal(xmvRight, mtxRotate);
		xmvUp = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvLook = XMVector3TransformNormal(xmvLook, mtxRotate);
		/*카메라의 위치 벡터에서 플레이어의 위치 벡터를 뺀다. 결과는 플레이어 위치를 기준으로 한 카메라의 위치 벡터이다.*/
		xmvCameraPos -= xmvPlayerPos; //m_xv3Position -= m_pPlayer->GetPosition();
		//플레이어의 위치를 중심으로 카메라의 위치 벡터(플레이어를 기준으로 한)를 회전한다.
		xmvCameraPos = XMVector3TransformCoord(xmvCameraPos, mtxRotate);	//xv3ec3TransformCoord(&m_xv3Position, &m_xv3Position, &mtxRotate);
		//회전시킨 카메라의 위치 벡터에 플레이어의 위치를 더한다.
		xmvCameraPos += xmvPlayerPos;//m_xv3Position += m_pPlayer->GetPosition();
	}
	if (m_pPlayer && (y != 0.0f))
	{
		xmvAxisVector    = XMLoadFloat3(&m_pPlayer->GetUpVector());
		mtxRotate        = XMMatrixRotationAxis(xmvAxisVector, XMConvertToRadians(y));

		xmvRight         = XMVector3TransformNormal(xmvRight, mtxRotate);
		xmvUp            = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvLook          = XMVector3TransformNormal(xmvLook, mtxRotate);
		xmvCameraPos    -= xmvPlayerPos;
		xmvCameraPos     = XMVector3TransformCoord(xmvCameraPos, mtxRotate);
		xmvCameraPos    += xmvPlayerPos;
	}
	if (m_pPlayer && (z != 0.0f))
	{
		xmvAxisVector    = XMLoadFloat3(&m_pPlayer->GetLookVector());
		mtxRotate        = XMMatrixRotationAxis(xmvAxisVector, XMConvertToRadians(z));

		xmvRight         = XMVector3TransformNormal(xmvRight, mtxRotate);
		xmvUp            = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvLook          = XMVector3TransformNormal(xmvLook, mtxRotate);
		xmvCameraPos    -= xmvPlayerPos;
		xmvCameraPos     = XMVector3TransformCoord(xmvCameraPos, mtxRotate);
		xmvCameraPos    += xmvPlayerPos;
	}
	XMStoreFloat3(&m_xv3Right, xmvRight);
	XMStoreFloat3(&m_xv3Up, xmvUp);
	XMStoreFloat3(&m_xv3Look, xmvLook);

	XMStoreFloat3(&m_xv3Position, xmvCameraPos);
}

CFirstPersonCamera::CFirstPersonCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = FIRST_PERSON_CAMERA;
	if (pCamera)
	{
		/*1인칭 카메라로 변경하기 이전의 카메라가 스페이스-쉽 카메라이면 카메라의 Up 벡터를 월드좌표의 y-축이 되도록 한다. 이것은 스페이스-쉽 카메라의 로컬 y-축 벡터가 어떤 방향이든지 1인칭 카메라(대부분 사람인 경우)의 로컬 y-축 벡터가 월드좌표의 y-축이 되도록 즉, 똑바로 서있는 형태로 설정한다는 의미이다. 그리고 로컬 x-축 벡터와 로컬 z-축 벡터의 y-좌표가 0.0f가 되도록 한다. 이것은 <그림 8>과 같이 로컬 x-축 벡터와 로컬 z-축 벡터를 xz-평면(지면)으로 투영하는 것을 의미한다. 즉, 1인칭 카메라의 로컬 x-축 벡터와 로컬 z-축 벡터는 xz-평면에 평행하다.*/
		if (pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			m_xv3Up           = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xv3Right.y      = 0.0f;
			m_xv3Look.y       = 0.0f;

			XMVECTOR xmvRight = XMLoadFloat3(&m_xv3Right);
			XMVECTOR xmvLook  = XMLoadFloat3(&m_xv3Look);

			xmvRight          = XMVector3Normalize(xmvRight);
			xmvLook           = XMVector3Normalize(xmvLook);

			XMStoreFloat3(&m_xv3Right, xmvRight);
			XMStoreFloat3(&m_xv3Look, xmvLook);
		}
	}
}

void CFirstPersonCamera::Rotate(float x, float y, float z)
{
	if (x == 0.0f && y == 0.0f && z == 0.0f) return;

	XMMATRIX mtxRotate;
	XMVECTOR xmvRight = XMLoadFloat3(&m_xv3Right);
	XMVECTOR xmvUp = XMLoadFloat3(&m_xv3Up);
	XMVECTOR xmvLook = XMLoadFloat3(&m_xv3Look);
	XMVECTOR xmvAxisVector;

	XMVECTOR xmvCameraPos = XMLoadFloat3(&m_xv3Position);
	XMVECTOR xmvPlayerPos = XMLoadFloat3(&m_pPlayer->GetPosition());

	if (x != 0.0f)
	{
		xmvAxisVector    = XMLoadFloat3(&m_pPlayer->GetRightVector());
		//카메라의 로컬 x-축을 기준으로 회전하는 행렬을 생성한다. 고개를 끄떡이는 동작이다.
		mtxRotate        = XMMatrixRotationAxis(xmvAxisVector, XMConvertToRadians(x));
		//카메라의 로컬 x-축, y-축, z-축을 회전한다.
		xmvRight         = XMVector3TransformNormal(xmvRight, mtxRotate);
		xmvUp            = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvLook          = XMVector3TransformNormal(xmvLook, mtxRotate);
	}
	if (m_pPlayer && (y != 0.0f))
	{
		xmvAxisVector    = XMLoadFloat3(&m_pPlayer->GetUpVector());
		//플레이어의 로컬 y-축을 기준으로 회전하는 행렬을 생성한다.
		mtxRotate        = XMMatrixRotationAxis(xmvAxisVector, XMConvertToRadians(y));
		//카메라의 로컬 x-축, y-축, z-축을 회전한다.
		xmvRight         = XMVector3TransformNormal(xmvRight, mtxRotate);
		xmvUp            = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvLook          = XMVector3TransformNormal(xmvLook, mtxRotate);
	}
	if (m_pPlayer && (z != 0.0f))
	{
		xmvAxisVector    = XMLoadFloat3(&m_pPlayer->GetLookVector());
		//플레이어의 로컬 z-축을 기준으로 회전하는 행렬을 생성한다.
		mtxRotate        = XMMatrixRotationAxis(xmvAxisVector, XMConvertToRadians(z));
		//카메라의 위치 벡터를 플레이어 좌표계로 표현한다(오프셋 벡터).
		xmvCameraPos    -= xmvPlayerPos;
		//오프셋 벡터 벡터를 회전한다.
		xmvCameraPos     = XMVector3TransformCoord(xmvCameraPos, mtxRotate);
		//회전한 카메라의 위치를 월드 좌표계로 표현한다.
		xmvCameraPos    += xmvPlayerPos;
		//카메라의 로컬 x-축, y-축, z-축을 회전한다.
		xmvRight         = XMVector3TransformNormal(xmvRight, mtxRotate);
		xmvUp            = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvLook          = XMVector3TransformNormal(xmvLook, mtxRotate);
	}
	XMStoreFloat3(&m_xv3Right, xmvRight);
	XMStoreFloat3(&m_xv3Up, xmvUp);
	XMStoreFloat3(&m_xv3Look, xmvLook);

	XMStoreFloat3(&m_xv3Position, xmvCameraPos);
}

void CCamera::CalculateFrustumPlanes()
{
	/*카메라 변환 행렬과 원근 투영 변환 행렬을 곱한 행렬을 사용하여 절두체 평면들을 구한다. 즉 월드 좌표계에서 절두체 컬링을 한다.*/
	XMFLOAT4X4 mtxViewProject;
	Chae::XMFloat4x4Mul(&mtxViewProject, &m_xmf44View, &m_xmf44Projection);

	//절두체의 왼쪽 평면
	m_xmpFrustumPlanes[0].x = -(mtxViewProject._14 + mtxViewProject._11);
	m_xmpFrustumPlanes[0].y = -(mtxViewProject._24 + mtxViewProject._21);
	m_xmpFrustumPlanes[0].z = -(mtxViewProject._34 + mtxViewProject._31);
	m_xmpFrustumPlanes[0].w = -(mtxViewProject._44 + mtxViewProject._41);

	//절두체의 오른쪽 평면
	m_xmpFrustumPlanes[1].x = -(mtxViewProject._14 - mtxViewProject._11);
	m_xmpFrustumPlanes[1].y = -(mtxViewProject._24 - mtxViewProject._21);
	m_xmpFrustumPlanes[1].z = -(mtxViewProject._34 - mtxViewProject._31);
	m_xmpFrustumPlanes[1].w = -(mtxViewProject._44 - mtxViewProject._41);

	//절두체의 위쪽 평면
	m_xmpFrustumPlanes[2].x = -(mtxViewProject._14 - mtxViewProject._12);
	m_xmpFrustumPlanes[2].y = -(mtxViewProject._24 - mtxViewProject._22);
	m_xmpFrustumPlanes[2].z = -(mtxViewProject._34 - mtxViewProject._32);
	m_xmpFrustumPlanes[2].w = -(mtxViewProject._44 - mtxViewProject._42);

	//절두체의 아래쪽 평면
	m_xmpFrustumPlanes[3].x = -(mtxViewProject._14 + mtxViewProject._12);
	m_xmpFrustumPlanes[3].y = -(mtxViewProject._24 + mtxViewProject._22);
	m_xmpFrustumPlanes[3].z = -(mtxViewProject._34 + mtxViewProject._32);
	m_xmpFrustumPlanes[3].w = -(mtxViewProject._44 + mtxViewProject._42);

	//절두체의 근평면
	m_xmpFrustumPlanes[4].x = -(mtxViewProject._13);
	m_xmpFrustumPlanes[4].y = -(mtxViewProject._23);
	m_xmpFrustumPlanes[4].z = -(mtxViewProject._33);
	m_xmpFrustumPlanes[4].w = -(mtxViewProject._43);

	//절두체의 원평면
	m_xmpFrustumPlanes[5].x = -(mtxViewProject._14 - mtxViewProject._13);
	m_xmpFrustumPlanes[5].y = -(mtxViewProject._24 - mtxViewProject._23);
	m_xmpFrustumPlanes[5].z = -(mtxViewProject._34 - mtxViewProject._33);
	m_xmpFrustumPlanes[5].w = -(mtxViewProject._44 - mtxViewProject._43);

	/*절두체의 각 평면의 법선 벡터 (a, b. c)의 크기로 a, b, c, d를 나눈다. 즉, 법선 벡터를 정규화하고 원점에서 평면까지의 거리를 계산한다.*/
	XMVECTOR xmPlane;
	for (int i = 0; i < 6; i++)
	{
		xmPlane = XMPlaneNormalize(XMLoadFloat4(&m_xmpFrustumPlanes[i]));
		XMStoreFloat4(&m_xmpFrustumPlanes[i], xmPlane);
		//D3DXPlaneNormalize(&m_xmpFrustumPlanes[i], &m_xmpFrustumPlanes[i]);
	}
}

bool CCamera::IsInFrustum(XMFLOAT3& xv3Minimum, XMFLOAT3& xv3Maximum)
{
	XMFLOAT3 xv3NearPoint, xv3FarPoint, xv3Normal;
	for (int i = 0; i < 6; i++)
	{
		/*절두체의 각 평면에 대하여 바운딩 박스의 근접점을 계산한다. 근접점의 x, y, z 좌표는 법선 벡터의 각 요소가 음수이면 바운딩 박스의 최대점의 좌표가 되고 그렇지 않으면 바운딩 박스의 최소점의 좌표가 된다.*/
		xv3Normal = XMFLOAT3(m_xmpFrustumPlanes[i].x, m_xmpFrustumPlanes[i].y, m_xmpFrustumPlanes[i].z);
		if (xv3Normal.x >= 0.0f) {
			if (xv3Normal.y >= 0.0f) {
				if (xv3Normal.z >= 0.0f) {
					//법선 벡터의 x, y, z 좌표의 부호가 모두 양수이므로 근접점은 바운딩 박스의 최소점이다.
					xv3NearPoint.x = xv3Minimum.x; xv3NearPoint.y = xv3Minimum.y; xv3NearPoint.z = xv3Minimum.z;
				}
				else {
					/*법선 벡터의 x, y 좌표의 부호가 모두 양수이므로 근접점의 x, y 좌표는 바운딩 박스의 최소점의 x, y 좌표이고 법선 벡터의 z 좌표가 움수이므로 근접점의 z 좌표는 바운딩 박스의 최대점의 z 좌표이다.*/
					xv3NearPoint.x = xv3Minimum.x; xv3NearPoint.y = xv3Minimum.y; xv3NearPoint.z = xv3Maximum.z;
				}
			}
			else {
				if (xv3Normal.z >= 0.0f) {
					/*법선 벡터의 x, z 좌표의 부호가 모두 양수이므로 근접점의 x, z 좌표는 바운딩 박스의 최소점의 x, z 좌표이고 법선 벡터의 y 좌표가 움수이므로 근접점의 y 좌표는 바운딩 박스의 최대점의 y 좌표이다.*/
					xv3NearPoint.x = xv3Minimum.x; xv3NearPoint.y = xv3Maximum.y; xv3NearPoint.z = xv3Minimum.z;
				}
				else {
					/*법선 벡터의 y, z 좌표의 부호가 모두 음수이므로 근접점의 y, z 좌표는 바운딩 박스의 최대점의 y, z 좌표이고 법선 벡터의 x 좌표가 양수이므로 근접점의 x 좌표는 바운딩 박스의 최소점의 x 좌표이다.*/
					xv3NearPoint.x = xv3Minimum.x; xv3NearPoint.y = xv3Maximum.y; xv3NearPoint.z = xv3Maximum.z;
				}
			}
		}
		else {
			if (xv3Normal.y >= 0.0f) {
				if (xv3Normal.z >= 0.0f) {
					/*법선 벡터의 y, z 좌표의 부호가 모두 양수이므로 근접점의 y, z 좌표는 바운딩 박스의 최소점의 y, z 좌표이고 법선 벡터의 x 좌표가 음수이므로 근접점의 x 좌표는 바운딩 박스의 최대점의 x 좌표이다.*/
					xv3NearPoint.x = xv3Maximum.x; xv3NearPoint.y = xv3Minimum.y; xv3NearPoint.z = xv3Minimum.z;
				}
				else {
					/*법선 벡터의 x, z 좌표의 부호가 모두 음수이므로 근접점의 x, z 좌표는 바운딩 박스의 최대점의 x, z 좌표이고 법선 벡터의 y 좌표가 양수이므로 근접점의 y 좌표는 바운딩 박스의 최소점의 y 좌표이다.*/
					xv3NearPoint.x = xv3Maximum.x; xv3NearPoint.y = xv3Minimum.y; xv3NearPoint.z = xv3Maximum.z;
				}
			}
			else {
				if (xv3Normal.z >= 0.0f) {
					/*법선 벡터의 x, y 좌표의 부호가 모두 음수이므로 근접점의 x, y 좌표는 바운딩 박스의 최대점의 x, y 좌표이고 법선 벡터의 z 좌표가 양수이므로 근접점의 z 좌표는 바운딩 박스의 최소점의 z 좌표이다.*/
					xv3NearPoint.x = xv3Maximum.x; xv3NearPoint.y = xv3Maximum.y; xv3NearPoint.z = xv3Minimum.z;
				}
				else {
					//법선 벡터의 x, y, z 좌표의 부호가 모두 음수이므로 근접점은 바운딩 박스의 최대점이다.
					xv3NearPoint.x = xv3Maximum.x; xv3NearPoint.y = xv3Maximum.y; xv3NearPoint.z = xv3Maximum.z;
				}
			}
		}
		if ((Chae::XMFloat3Dot(&xv3Normal, &xv3NearPoint) + m_xmpFrustumPlanes[i].w) > 0.0f) return(false);
	}
	return(true);
}

bool CCamera::IsInFrustum(AABB *pAABB)
{
	return(IsInFrustum(pAABB->m_xv3Minimum, pAABB->m_xv3Maximum));
}

CThirdPersonCamera::CThirdPersonCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = THIRD_PERSON_CAMERA;
	if (pCamera)
	{
		/*3인칭 카메라로 변경하기 이전의 카메라가 스페이스-쉽 카메라이면 카메라의 Up 벡터를 월드좌표의 y-축이 되도록 한다. 이것은 스페이스-쉽 카메라의 로컬 y-축 벡터가 어떤 방향이든지 3인칭 카메라(대부분 사람인 경우)의 로컬 y-축 벡터가 월드좌표의 y-축이 되도록 즉, 똑바로 서있는 형태로 설정한다는 의미이다. 그리고 로컬 x-축 벡터와 로컬 z-축 벡터의 y-좌표가 0.0f가 되도록 한다. 이것은 로컬 x-축 벡터와 로컬 z-축 벡터를 xz-평면(지면)으로 투영하는 것을 의미한다. 즉, 3인칭 카메라의 로컬 x-축 벡터와 로컬 z-축 벡터는 xz-평면에 평행하다.*/
		if (pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			m_xv3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xv3Right.y = 0.0f;
			m_xv3Look.y = 0.0f;
			Chae::XMFloat3Normalize(&m_xv3Right);
			Chae::XMFloat3Normalize(&m_xv3Look);
		}
	}
}

void CThirdPersonCamera::Update(XMFLOAT3& xv3LookAt, float fTimeElapsed)
{
	/*플레이어의 회전에 따라 3인칭 카메라도 회전해야 한다.*/
	XMFLOAT4X4 mtxRotate;
	Chae::XMFloat4x4Identity(&mtxRotate);
	XMFLOAT3 xv3Right  = m_pPlayer->GetRightVector();
	XMFLOAT3 xv3Up     = m_pPlayer->GetUpVector();
	XMFLOAT3 xv3Look   = m_pPlayer->GetLookVector();
	XMFLOAT3 xv3Player = m_pPlayer->GetPosition();
	m_xv3Position.y = max(m_xv3Position.y, xv3Player.y + m_xv3Offset.y);

	//플레이어의 로컬 x-축, y-축, z-축 벡터로부터 회전 행렬을 생성한다.
	mtxRotate._11 = xv3Right.x; mtxRotate._21 = xv3Up.x; mtxRotate._31 = xv3Look.x;
	mtxRotate._12 = xv3Right.y; mtxRotate._22 = xv3Up.y; mtxRotate._32 = xv3Look.y;
	mtxRotate._13 = xv3Right.z; mtxRotate._23 = xv3Up.z; mtxRotate._33 = xv3Look.z;

	XMMATRIX xmtxRotate = XMLoadFloat4x4(&mtxRotate);
	XMVECTOR xmv3Offset = XMLoadFloat3(&m_xv3Offset);
	XMVECTOR xmvCameraPos = XMLoadFloat3(&m_xv3Position);
	XMVECTOR xmvPlayerPos = XMLoadFloat3(&m_pPlayer->GetPosition());

	xmv3Offset = XMVector3TransformCoord(xmv3Offset, xmtxRotate); // xv3ec3TransformCoord(&xv3Offset, &m_xv3Offset, &mtxRotate);
	//회전한 카메라의 위치는 플레이어의 위치에 회전한 카메라 오프셋 벡터를 더한 것이다.
	XMVECTOR xv3Position = (xmvPlayerPos + xmv3Offset);
	//현재의 카메라의 위치에서 회전한 카메라의 위치까지의 벡터이다.
	XMVECTOR xv3Direction = xv3Position - xmvCameraPos;

	float fLength;
	XMStoreFloat(&fLength, XMVector3Length(xv3Direction));
	xv3Direction = XMVector3Normalize(xv3Direction);
	/*3인칭 카메라의 래그(Lag)는 플레이어가 회전하더라도 카메라가 동시에 따라서 회전하지 않고 약간의 시차를 두고 회전하는 효과를 구현하기 위한 것이다. m_fTimeLag가 1보다 크면 fTimeLagScale이 작아지고 실제 회전이 적게 일어날 것이다.*/

	float fTimeLagScale = (m_fTimeLag) ? fTimeElapsed * (1.0f / m_fTimeLag) : 1.0f;
	float fDistance = fLength * fTimeLagScale;
	if (fDistance > fLength) fDistance = fLength;
	if (fLength < 0.01f) fDistance = fLength;
	if (fDistance > 0)
	{
		xmvCameraPos += xv3Direction * fDistance;
		SetLookAt(xv3LookAt);
	}
	XMStoreFloat3(&m_xv3Position, xmvCameraPos);

}

void CThirdPersonCamera::SetLookAt(XMFLOAT3& xv3LookAt)
{
	XMMATRIX mtxLookAt = XMMatrixLookAtLH(XMLoadFloat3(&m_xv3Position), XMLoadFloat3(&xv3LookAt), XMLoadFloat3(&m_pPlayer->GetUpVector()));
	XMStoreFloat4x4(&m_xmf44View, mtxLookAt);
	//XMFLOAT4X4 mtxLookAt;
	//XMFLOAT4X4LookAtLH(&mtxLookAt, &m_xv3Position, &xv3LookAt, &m_pPlayer->GetUpVector());

	m_xv3Right = XMFLOAT3(m_xmf44View._11, m_xmf44View._21, m_xmf44View._31);
	m_xv3Up    = XMFLOAT3(m_xmf44View._12, m_xmf44View._22, m_xmf44View._32);
	m_xv3Look  = XMFLOAT3(m_xmf44View._13, m_xmf44View._23, m_xmf44View._33);

	MakeViewProjectionMatrix();
}