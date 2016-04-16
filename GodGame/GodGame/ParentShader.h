#pragma once

#ifndef __SHADER_PARENTS
#define __SHADER_PARENTS

#include "ShaderEssential.h"
#include "ShaderList.h"


class CParentShader : public CShader
{
protected:
	int m_nShaders;
	CShader ** m_ppShader;

public:
	CParentShader();
	virtual ~CParentShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void ReleaseObjects();
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

};

class CEnviromentShader : public CParentShader
{
public:
	CEnviromentShader();
	virtual ~CEnviromentShader();

	virtual void BuildObjects(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
};

class CEffectShader : public CParentShader
{
	const CHAR mTxAniEffectNum    = 0;
	const CHAR mParticleEffectNum = 1;

public:
	CEffectShader();
	virtual ~CEffectShader();

	void ShaderKeyEventOn(CPlayer * pPlayer, WORD key, void * extra);
	virtual void BuildObjects(ID3D11Device *pd3dDevice);
	virtual void GetGameMessage(CShader * byObj, eMessage eMSG, void * extra = nullptr);
	//virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
};

class CStaticModelingShader : public CParentShader
{
public:
	CStaticModelingShader();
	virtual ~CStaticModelingShader();

	virtual void BuildObjects(ID3D11Device * pd3dDevice, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr);
	//virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
};

#endif