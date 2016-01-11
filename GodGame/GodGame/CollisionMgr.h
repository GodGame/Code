#pragma once


#ifndef __COLLISION
#define __COLLISION

class AABB
{
public:
	//바운딩 박스의 최소점과 최대점을 나타내는 벡터이다.
	XMFLOAT3 m_xv3Minimum;
	XMFLOAT3 m_xv3Maximum;

public:
	AABB() { m_xv3Minimum = XMFLOAT3(+FLT_MAX, +FLT_MAX, +FLT_MAX); m_xv3Maximum = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX); }
	AABB(XMFLOAT3 xv3Minimum, XMFLOAT3 xv3Maximum) { m_xv3Minimum = xv3Minimum; m_xv3Maximum = xv3Maximum; }

	//두 개의 바운딩 박스를 합한다.
	void Union(XMFLOAT3& xv3Minimum, XMFLOAT3& xv3Maximum);
	void Union(AABB *pAABB);
	//바운딩 박스의 8개의 꼭지점을 행렬로 변환하고 최소점과 최대점을 다시 계산한다.
	void Update(XMFLOAT4X4 *pxmtxTransform);
};

class CGameObject;
struct CPartitionNode
{
	bool m_bIsStatic;
	CGameObject* m_pObject;

};

class QuadTree
{
};


class OctTree
{
};

class CCollisionMgr
{
public:
	CCollisionMgr();
	~CCollisionMgr();
};


#endif