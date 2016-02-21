#pragma once

#include "ShaderType.h"

class CStaticShader : public CShader
{
public:
	CStaticShader();
	virtual ~CStaticShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial * pMaterial);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
};

class CInstancingShader : public CInstanceShader, public CTexturedShader
{

public:
	CInstancingShader();
	~CInstancingShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial *pMaterial, CTexture *pTexture, int k);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
private:
	CMaterial *m_pMaterial;
	CTexture *m_pTexture;

	//UINT m_nInstanceBufferStride;
	//UINT m_nInstanceBufferOffset;

	ID3D11Buffer *m_pd3dCubeInstanceBuffer;
	ID3D11Buffer *m_pd3dSphereInstanceBuffer;

public:
	//인스턴싱 버퍼를 생성한다.
	//ID3D11Buffer *CreateInstanceBuffer(ID3D11Device *pd3dDevice, int nObjects, UINT nBufferStride, void *pBufferData);
};

class CBillboardShader : public CShader, public CInstanceShader
{
private:
	ID3D11Buffer * m_pd3dTreeInstanceBuffer;
	int m_nTrees;
public:
	CBillboardShader();
	~CBillboardShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void AnimateObjects(float fTimeElapsed);
	void AllRender(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

	static void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera);
private:
	CTexture *m_pTexture;

public:
	//인스턴싱 버퍼를 생성한다.
	//ID3D11Buffer *CreateInstanceBuffer(ID3D11Device *pd3dDevice, int nObjects, UINT nBufferStride, void *pBufferData);
};

class CPointInstanceShader : public CShader, public CInstanceShader
{
private:
	ID3D11Buffer * m_pd3dCubeInstanceBuffer;
	int m_nCubes;
public:
	CPointInstanceShader();
	~CPointInstanceShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial * pMaterial);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void AnimateObjects(float fTimeElapsed);
	//static void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera);
private:
	CTexture  * m_pTexture;
	CMaterial * m_pMaterial;

public:
	//인스턴싱 버퍼를 생성한다.
	//ID3D11Buffer *CreateInstanceBuffer(ID3D11Device *pd3dDevice, int nObjects, UINT nBufferStride, void *pBufferData);
};

class CNormalShader : public CNormalMapShader
{
	CTexture  * m_pTexture;
	CMaterial * m_pMaterial;
public:
	CNormalShader();
	~CNormalShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial * pMaterial);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

};

class CParticleShader : public CShader
{
	ID3D11VertexShader       * m_pd3dVSSO;
	ID3D11GeometryShader     * m_pd3dGSSO;

private:
	ID3D11DepthStencilState  * m_pd3dSODepthStencilState;
	ID3D11DepthStencilState  * m_pd3dDepthStencilState;

	ID3D11BlendState         * m_pd3dBlendState;

	ID3D11ShaderResourceView * m_pd3dRandomSRV;
	ID3D11SamplerState       * m_pd3dSamplerState;

	ID3D11ShaderResourceView ** m_ppd3dParticleImageSRV;
	int m_nImages;

private:
	CParticle    ** m_ppParticle;
	ID3D11Buffer *  m_pd3dcbGameInfo;

	//typedef CParticle* ParticleInfo;
	typedef pair<UINT, CParticle*> ParticleInfo;
	vector<CParticle*>   m_vcAbleParticleArray;
	vector<ParticleInfo> m_vcUsingParticleArray;

public :
	CParticleShader();
	~CParticleShader();

	void	CreateStates(ID3D11Device * pd3dDevice);
	virtual void CreateShader(ID3D11Device *pd3dDevice);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain, CMaterial * pMaterial);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void AnimateObjects(float fTimeElapsed);

	void CreateShaderVariables(ID3D11Device *pd3dDevice);
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, CParticle * pParticle);

	void SOSetState(ID3D11DeviceContext * pd3dDeviceContext);

	bool SetParticle(int index, XMFLOAT3 * pos = nullptr)
	{ 
		if (m_ppParticle[index]->IsAble() ) return false;
		return m_ppParticle[index]->Enable(pos); 
	}
	CParticle * GetParticle(int index) { return m_ppParticle[index]; }

	void ParticleOn(XMFLOAT3 * pos = nullptr)
	{
		if (m_vcAbleParticleArray.empty()) return;

		auto it = m_vcAbleParticleArray.end() - 1;
		(*it)->Enable(pos);
		m_vcAbleParticleArray.pop_back();
	}

	void ParticleOn(int num, XMFLOAT3 * pos = nullptr, XMFLOAT3 * vel = nullptr, XMFLOAT3 * acc = nullptr)
	{
		m_ppParticle[num]->Enable(pos);
		if (vel) m_ppParticle[num]->SetMoveVelocity(*vel);
		if (acc) m_ppParticle[num]->SetMoveAccel(*acc);
	}
};