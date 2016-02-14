// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// GodGame.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"

// TODO: 필요한 추가 헤더는
// 이 파일이 아닌 STDAFX.H에서 참조합니다.
ostream& operator<<(ostream& os, XMFLOAT3 & xmf3)
{
	os << xmf3.x << ", " << xmf3.y << ", " << xmf3.z;
	return os;
}

ostream & operator<<(ostream & os, XMFLOAT4 & xmf4)
{
	os << xmf4.x << ", " << xmf4.y << ", " << xmf4.z <<", " << xmf4.w;
	return os;
}

ostream & operator<<(ostream & os, XMFLOAT4X4 & mtx)
{
	os << mtx._11 << ", " << mtx._12 << ", " << mtx._13 << ", " << mtx._14 << endl;
	os << mtx._21 << ", " << mtx._22 << ", " << mtx._23 << ", " << mtx._24 << endl;
	os << mtx._31 << ", " << mtx._32 << ", " << mtx._33 << ", " << mtx._34 << endl;
	os << mtx._41 << ", " << mtx._42 << ", " << mtx._43 << ", " << mtx._44;
	return os;
}
