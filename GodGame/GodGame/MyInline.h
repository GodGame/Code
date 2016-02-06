#include "stdafx.h"

#pragma once

#ifndef MYINLINE
#define MYINLINE

// xmfloat를 간단하게 변환해서 연산할 경우 이걸 사용하자
namespace Chae
{
	// 벡터용
	inline void XMFloat3Add(XMFLOAT3 * Result, XMFLOAT3 * v1, XMFLOAT3 * v2)
	{
		XMStoreFloat3(Result, XMLoadFloat3(v1) + XMLoadFloat3(v2));
	}
	inline void XMFloat3Sub(XMFLOAT3 * Result, XMFLOAT3 * v1, XMFLOAT3 * v2)
	{
		XMStoreFloat3(Result, XMLoadFloat3(v1) - XMLoadFloat3(v2));
	}
	inline void XMFloat3Mul(XMFLOAT3 * Result, XMFLOAT3 * v1, XMFLOAT3 * v2)
	{
		XMStoreFloat3(Result, XMLoadFloat3(v1) * XMLoadFloat3(v2));
	}
	inline void XMFloat3Div(XMFLOAT3 * Result, XMFLOAT3 * v1, XMFLOAT3 * v2)
	{
		XMStoreFloat3(Result, XMLoadFloat3(v1) / XMLoadFloat3(v2));
	}
	inline float XMFloat3Dot(XMFLOAT3 * v1, XMFLOAT3 * v2)
	{
		float fResult;
		XMStoreFloat(&fResult, XMVector3Dot(XMLoadFloat3(v1), XMLoadFloat3(v2)));
		return fResult;
	}
	inline void XMFloat3Normalize(XMFLOAT3 * pXMF3Target)
	{
		XMStoreFloat3(pXMF3Target, XMVector3Normalize(XMLoadFloat3(pXMF3Target)));
	}
	inline void XMFloat3Normalize(XMFLOAT3 * pOut, XMFLOAT3 * pXMF3)
	{
		XMStoreFloat3(pOut, XMVector3Normalize(XMLoadFloat3(pXMF3)));
	}
	// 배열용
	inline void XMFloat4x4Identity(XMFLOAT4X4 * xmtxTarget)
	{
		XMMATRIX xmtxMatrix = XMMatrixIdentity();
		XMStoreFloat4x4(xmtxTarget, xmtxMatrix);
	}
	inline void XMFloat4x4Transpose(XMFLOAT4X4 * pResult, XMFLOAT4X4 * pM1)
	{
		XMMATRIX mtxResult = XMMatrixTranspose(XMLoadFloat4x4(pM1));
		XMStoreFloat4x4(pResult, mtxResult);
	}
	inline void XMFloat4x4Mul(XMFLOAT4X4 * pResult, XMFLOAT4X4* pM1, XMFLOAT4X4* pM2)
	{
		XMMATRIX M1 = XMLoadFloat4x4(pM1);
		XMMATRIX M2 = XMLoadFloat4x4(pM2);
		XMStoreFloat4x4(pResult, XMMatrixMultiply(M1, M2));
	}

	inline float RandomFloat(float a, float b)
	{
		return (a + (float)(rand() / (float)RAND_MAX) * (b - a));
	}

};
#endif