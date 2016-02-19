#pragma once
#include "Object.h"
#include "Camera.h"
#include "Player.h"

struct VS_VB_INSTANCE
{
	XMFLOAT4X4 m_d3dxTransform;
};

struct VS_CB_WORLD_MATRIX
{
	XMFLOAT4X4 m_d3dxTransform;
};

struct VS_VB_WORLD_POSITION
{
	XMFLOAT4 m_xv3Position;
};

struct CB_DISPLACEMENT
{
	XMFLOAT3 m_xv3BumpScale;
	float  m_fBumpMax;
};

// Constant buffer layout for transferring data to the PS
struct CB_PS
{
	float param[4];
};

struct CB_CS
{
	XMFLOAT4 param;	// x, y = dispatch , z , w = input size;
};


class CShader
{
public:
	CShader();
	virtual ~CShader();

	void CreateVertexShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11VertexShader **ppd3dVertexShader, D3D11_INPUT_ELEMENT_DESC *pd3dInputLayout, UINT nElements, ID3D11InputLayout **ppd3dVertexLayout);
	void CreatePixelShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11PixelShader **ppd3dPixelShader);
	void CreateGeometryShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11GeometryShader **ppd3dGeometryShader);
	void CreateGeometryStreamOutShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11GeometryShader **ppd3dGeometryShader, 
		D3D11_SO_DECLARATION_ENTRY * pSODeclaration, UINT NumEntries, UINT *pBufferStrides, UINT NumStrides, UINT RasterizedStream);
	void CreateHullShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11HullShader **ppd3dHullShader);
	void CreateDomainShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11DomainShader **ppd3dDomainShader);
	void CreateComputeShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11ComputeShader **ppd3dComputeShader);

	virtual void CreateShader(ID3D11Device *pd3dDevice);

	static void CreateShaderVariables(ID3D11Device *pd3dDevice);
	static void ReleaseShaderVariables();
	static void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, XMFLOAT4X4 & xmtxWorld);

	void EntityAllStaticObjects();
	void EntityAllDynamicObjects();

	static ID3D11ShaderResourceView * CreateRandomTexture1DSRV(ID3D11Device * pd3dDevice);
public:
	//게임 객체들을 생성하고 애니메이션 처리를 하고 렌더링하기 위한 함수이다.
	virtual void BuildObjects(ID3D11Device *pd3dDevice);
	virtual void ReleaseObjects();
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void OnPrepareRender(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	
	virtual void GetGameMessage(CShader * byObj, eMessage eMSG, void * extra = nullptr);
	virtual void SendGameMessage(CShader * toObj, eMessage eMSG, void * extra = nullptr);
	static  void MessageObjToObj(CShader * byObj, CShader * toObj, eMessage eMSG, void * extra = nullptr);

	virtual CGameObject * GetObj(int num = 0);
#ifdef PICKING
	virtual CGameObject *PickObjectByRayIntersection(XMFLOAT3 *pxv3PickPosition, XMFLOAT4X4 *pxmtxView, MESHINTERSECTINFO *pd3dxIntersectInfo);
#endif
protected:
	ID3D11VertexShader   * m_pd3dVertexShader;
	ID3D11InputLayout    * m_pd3dVertexLayout;
	ID3D11PixelShader    * m_pd3dPixelShader;
	ID3D11GeometryShader * m_pd3dGeometryShader;
	ID3D11HullShader	 * m_pd3dHullShader;
	ID3D11DomainShader   * m_pd3dDomainShader;
	//ID3D11ComputeShader  * m_p3dComputeShader;
protected:

	//쉐이더 객체가 게임 객체들의 리스트를 가진다.
	CGameObject **m_ppObjects;
	int m_nObjects;

	//월드 변환 행렬을 위한 상수 버퍼는 하나만 있어도 되므로 정적 멤버로 선언한다.
	static ID3D11Buffer *m_pd3dcbWorldMatrix;
};

class CDiffusedShader : public CShader
{
public:
	CDiffusedShader();
	virtual ~CDiffusedShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
};


class CIlluminatedShader : public CShader
{
public:
	CIlluminatedShader();
	virtual ~CIlluminatedShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);

	//재질을 설정하기 위한 상수 버퍼이다.
	static ID3D11Buffer	 *m_pd3dcbMaterial;

	static void CreateShaderVariables(ID3D11Device *pd3dDevice);
	static void ReleaseShaderVariables();
	//재질을 쉐이더 변수에 설정(연결)하기 위한 함수이다.
	static void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, MATERIAL *pMaterial);
};

class CTexturedShader : public CShader
{
public:
	CTexturedShader();
	virtual ~CTexturedShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
};

class CDetailTexturedShader : public CTexturedShader
{
public:
	CDetailTexturedShader();
	virtual ~CDetailTexturedShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
};

class CTexturedIlluminatedShader : public CIlluminatedShader
{
public:
	CTexturedIlluminatedShader();
	virtual ~CTexturedIlluminatedShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
};

class CDetailTexturedIlluminatedShader : public CTexturedIlluminatedShader
{
public:
	CDetailTexturedIlluminatedShader();
	virtual ~CDetailTexturedIlluminatedShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
};
//플레이어를 렌더링하기 위한 쉐이더 클래스이다.


class CNormalMapShader : public CShader
{
protected:
	CB_DISPLACEMENT m_Bump;
	ID3D11Buffer *m_pd3dcbBump;
public:
	CNormalMapShader();
	virtual ~CNormalMapShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void OnPrepareRender(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
	
	void UpdateBumpInfo(ID3D11DeviceContext *pd3dDeviceContext, SETSHADER setInfo = ( SET_SHADER_DS | SET_SHADER_PS ));
};

class CInstanceShader
{
protected:
	UINT m_nInstanceBufferStride;
	UINT m_nInstanceBufferOffset;

public:
	CInstanceShader();
	~CInstanceShader();

	virtual ID3D11Buffer *CreateInstanceBuffer(ID3D11Device *pd3dDevice, int nObjects, UINT nBufferStride, void *pBufferData);
};


class CSplatLightingShader : public CTexturedIlluminatedShader
{
protected:
	ID3D11BlendState * m_pd3dSplatBlendState;
public:
	CSplatLightingShader();
	virtual ~CSplatLightingShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);

};

