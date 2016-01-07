#pragma once
#ifndef __TXMGR
#define __TXMGR

#include "MgrType.h"

typedef UINT SETSHADER;
#define SET_SHADER_NONE 0
#define SET_SHADER_VS	(1 << 0)
#define SET_SHADER_HS	(1 << 1)
#define SET_SHADER_DS	(1 << 2)
#define SET_SHADER_GS	(1 << 3)
#define SET_SHADER_PS	(1 << 4)
#define SET_SHADER_CS	(1 << 5)

// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.

//���� ��ü�� �ϳ� �̻��� �ؽ��ĸ� ���� �� �ִ�. CTexture�� �ؽ��ĸ� �����ϱ� ���� Ŭ�����̴�.
class CTexture
{
public:
	CTexture(int nTextures = 1, int nSamplers = 1, int nTextureStartSlot = 0, int nSamplerStartSlot = 0, SETSHADER nSetInfo = SET_SHADER_PS);
	virtual ~CTexture();

private:
	SETSHADER	m_uTextureSet;
	int m_nReferences;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

private:
	//�ؽ��� ���ҽ��� �����̴�.
	int m_nTextures;
	ID3D11ShaderResourceView **m_ppd3dsrvTextures;
	//�ؽ��� ���ҽ��� ������ ���� �����̴�.
	int m_nTextureStartSlot;
	//���÷� ���� ��ü�� �����̴�.
	int m_nSamplers;
	ID3D11SamplerState **m_ppd3dSamplerStates;
	//���÷� ���� ��ü�� ������ ���� �����̴�.
	int m_nSamplerStartSlot;

public:
	void ChangeSetShader(SETSHADER uInfo) { m_uTextureSet = uInfo; }
	void AddSetShader(SETSHADER uAdd) { m_uTextureSet |= (uAdd); }

	void SetTexture(int nIndex, ID3D11ShaderResourceView *pd3dsrvTexture);
	void SetSampler(int nIndex, ID3D11SamplerState *pd3dSamplerState);

	//�ؽ��� ���ҽ��� ���÷� ���� ��ü�� ���� ���̴� ������ �����Ѵ�.
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext);
	//�ؽ��� ���ҽ��� ���� ���̴� ������ �����Ѵ�.
	void UpdateTextureShaderVariable(ID3D11DeviceContext *pd3dDeviceContext);
	//���÷� ���� ��ü�� ���� ���̴� ������ �����Ѵ�.
	void UpdateSamplerShaderVariable(ID3D11DeviceContext *pd3dDeviceContext);

public:
	ID3D11ShaderResourceView * GetSRV(int index) {return m_ppd3dsrvTextures[index];}
	ID3D11SamplerState * GetSampler(int index) { return m_ppd3dSamplerStates[index]; }
	ID3D11ShaderResourceView ** GetSRVList(void) { return m_ppd3dsrvTextures; }
	ID3D11SamplerState ** GetSamplerList(void) { return m_ppd3dSamplerStates; }

	bool IsSampler() { return m_nSamplers > 0; }
	bool IsSRV() { return m_nTextures > 0; }

	static ID3D11ShaderResourceView * CreateTexture2DArraySRV(ID3D11Device *pd3dDevice, wchar_t *ppstrFilePaths, UINT nTextures);
};


class CTextureMgr : public CMgr<CTexture>
{
private:
	CTextureMgr();
	virtual ~CTextureMgr();

public:
	static CTextureMgr& GetInstance();
	bool InsertShaderResourceView(ID3D11ShaderResourceView * pSRV, string name, UINT uSlotNum, SETSHADER nSetInfo = SET_SHADER_PS);
	bool InsertSamplerState(ID3D11SamplerState * pSamplerState, string name, UINT uSlotNum, SETSHADER nSetInfo = SET_SHADER_PS);

	ID3D11ShaderResourceView * GetShaderResourceView(string name) { return m_mpList[name]->GetSRV(0);}
	ID3D11SamplerState * GetSamplerState(string name) { return m_mpList[name]->GetSampler(0);}

public:
	virtual void BuildResources(ID3D11Device *pd3dDevice);
	void BuildSamplers(ID3D11Device *pd3dDevice);
	void BuildTextures(ID3D11Device *pd3dDevice);

	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, string name);

	static ID3D11ShaderResourceView * CreateRandomTexture1DSRV(ID3D11Device * pd3dDevice);
};

#define TXMgr CTextureMgr::GetInstance()
///////////////////////////////////////////////////////////////////////////////////////////////////////

//������ ������ ��Ÿ���� ����ü�̴�.
struct MATERIAL
{
	XMFLOAT4 m_xcAmbient;
	XMFLOAT4 m_xcDiffuse;
	XMFLOAT4 m_xcSpecular; //(r,g,b,a=power)
	XMFLOAT4 m_xcEmissive;
};

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

private:
	SETSHADER	m_uMaterialSet;
	int m_nReferences;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	MATERIAL m_Material;
};


class CMaterialMgr : public CMgr<CMaterial>
{
private:
	CMaterialMgr();
	virtual ~CMaterialMgr();

public:
	static CMaterialMgr& GetInstance();

	virtual void BuildResources(ID3D11Device *pd3dDevice);

};

#define MaterialMgr CMaterialMgr::GetInstance()
#endif