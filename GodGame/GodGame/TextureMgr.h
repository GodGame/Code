#pragma once
#ifndef __TXMGR
#define __TXMGR


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
	void UpdateTextureShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, int nIndex = 0, int nSlot = 0);
	//���÷� ���� ��ü�� ���� ���̴� ������ �����Ѵ�.
	void UpdateSamplerShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, int nIndex = 0, int nSlot = 0);

	ID3D11ShaderResourceView * GetSRV(int index) {return m_ppd3dsrvTextures[index];}
	ID3D11SamplerState * GetSampler(int index) { return m_ppd3dSamplerStates[index]; }
	ID3D11ShaderResourceView ** GetSRVList(void) { return m_ppd3dsrvTextures; }
	ID3D11SamplerState ** GetSamplerList(void) { return m_ppd3dSamplerStates; }

	static ID3D11ShaderResourceView * CreateTexture2DArraySRV(ID3D11Device *pd3dDevice, wchar_t *ppstrFilePaths, UINT nTextures);
};



class CTextureMgr
{
private:
	CTextureMgr();
	~CTextureMgr();

	map<string, CTexture*> m_vpTextureArray;
//	vector<CTexture*> m_vpTextureArray;

public:
	static CTextureMgr& GetInstance();
	bool InsertTexture(CTexture * pTexture, string name);
	bool InsertShaderResourceView(ID3D11ShaderResourceView * pSRV, string name, UINT uSlotNum, SETSHADER nSetInfo = SET_SHADER_PS);
	bool InsertSamplerState(ID3D11SamplerState * pSamplerState, string name, UINT uSlotNum, SETSHADER nSetInfo = SET_SHADER_PS);

	void EraseTexture(string name);

	CTexture * GetTexture(string name);
	ID3D11ShaderResourceView * GetShaderResourceView(string name)
	{
		return m_vpTextureArray[name]->GetSRV(0);
	}
	ID3D11SamplerState * GetSamplerState(string name)
	{
		return m_vpTextureArray[name]->GetSampler(0);
	}

public:
	void BuildResources(ID3D11Device *pd3dDevice);

	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, string name);
};

#define TXMgr CTextureMgr::GetInstance()
#define TXList CTextureMgr::eTxList




#endif