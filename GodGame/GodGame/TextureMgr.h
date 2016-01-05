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

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.

//게임 객체는 하나 이상의 텍스쳐를 가질 수 있다. CTexture는 텍스쳐를 관리하기 위한 클래스이다.
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
	//텍스쳐 리소스의 개수이다.
	int m_nTextures;
	ID3D11ShaderResourceView **m_ppd3dsrvTextures;
	//텍스쳐 리소스를 연결할 시작 슬롯이다.
	int m_nTextureStartSlot;
	//샘플러 상태 객체의 개수이다.
	int m_nSamplers;
	ID3D11SamplerState **m_ppd3dSamplerStates;
	//샘플러 상태 객체를 연결할 시작 슬롯이다.
	int m_nSamplerStartSlot;

public:
	void ChangeSetShader(SETSHADER uInfo) { m_uTextureSet = uInfo; }
	void AddSetShader(SETSHADER uAdd) { m_uTextureSet |= (uAdd); }

	void SetTexture(int nIndex, ID3D11ShaderResourceView *pd3dsrvTexture);
	void SetSampler(int nIndex, ID3D11SamplerState *pd3dSamplerState);

	//텍스쳐 리소스와 샘플러 상태 객체에 대한 쉐이더 변수를 변경한다.
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext);
	//텍스쳐 리소스에 대한 쉐이더 변수를 변경한다.
	void UpdateTextureShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, int nIndex = 0, int nSlot = 0);
	//샘플러 상태 객체에 대한 쉐이더 변수를 변경한다.
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