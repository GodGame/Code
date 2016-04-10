#pragma once

#ifndef __MAP_MGR
#define __MAP_MGR


class CHeightMap
{
private:
	//높이 맵 이미지 픽셀(8-비트)들의 이차원 배열이다. 각 픽셀은 0~255의 값을 갖는다.
	BYTE* m_pHeightMapImage;
	//높이 맵 이미지의 가로와 세로 크기이다.
	int m_nWidth;
	int m_nLength;
	//높이 맵을 실제로 몇 배 확대하여 사용할 것인가를 나타내는 스케일 벡터이다.
	XMFLOAT3 m_xv3Scale;

public:
	CHeightMap(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3& xv3Scale);
	~CHeightMap(void);

	//높이 맵 이미지에서 (x, z) 위치의 픽셀 값에 기반한 지형의 높이를 반환한다.
	float GetHeight(float x, float z, bool bReverseQuad = false) const;
	//높이 맵 이미지에서 (x, z) 위치의 법선 벡터를 반환한다.
	XMFLOAT3& GetHeightMapNormal(int x, int z);
	XMFLOAT3& GetScale() { return(m_xv3Scale); }

	BYTE* GetHeightMapImage() const { return(m_pHeightMapImage); }
	int GetHeightMapWidth()   const { return(m_nWidth); }
	int GetHeightMapLength()  const { return(m_nLength); }
};


class CMapManager
{
private:
	CMapManager();
	~CMapManager();

private:
	//지형의 높이 맵으로 사용할 이미지이다.
	CHeightMap *m_pHeightMap;

	//지형의 가로와 세로 크기이다.
	int m_nWidth;
	int m_nLength;

	//지형을 실제로 몇 배 확대할 것인가를 나타내는 스케일 벡터이다.
	XMFLOAT3 m_xv3Scale;

	float m_fMaxHeight;
	float m_fMinHeight;

public:
	void SetMaxHeight(float fHeight) { m_fMaxHeight = fHeight; }
	void SetMinHeight(float fHeight) { m_fMinHeight = fHeight; }

	//지형의 실제 높이를 반환한다. 높이 맵의 높이에 스케일을 곱한 값이다.
	float GetHeight(float x, float z, bool bReverseQuad = false) { return(m_pHeightMap->GetHeight(x, z, bReverseQuad) * m_xv3Scale.y); }
	XMFLOAT3& GetNormal(float x, float z) { return(m_pHeightMap->GetHeightMapNormal(int(x / m_xv3Scale.x), int(z / m_xv3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMap->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMap->GetHeightMapLength()); }

	XMFLOAT3& GetScale() { return(m_xv3Scale); }
	//지형의 실제 크기(가로/세로)를 반환한다. 높이 맵의 크기에 스케일을 곱한 값이다.
	float GetWidth() { return(m_nWidth * m_xv3Scale.x); }
	float GetLength() { return(m_nLength * m_xv3Scale.z); }

	float GetPeakHeight() { return(m_fMaxHeight); }
	float GetLowHeight() { return m_fMinHeight; }

public:
	static CMapManager& GetInstance();

	void Build(LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3& xmfScale);
	CHeightMap * GetHeightMap() { return m_pHeightMap; }

};
#define MAPMgr CMapManager::GetInstance()

#endif