#pragma once

#ifndef __COLLISION
#define __COLLISION

#include "stdafx.h"
#include <list>
enum ePartLoc { NONE = 0, INCLUDE_ALL, INCLUDE_PART_MIN, INCLUDE_PART_MAX, INCLUDE_NOT };

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

	static bool CollisionAABB(AABB & one, AABB & two);
	static ePartLoc IsIncludeAABB(AABB & bbSmall, AABB & bbLarge);
	static bool CollisionAABBBy2D(AABB & one, AABB & two);
	static ePartLoc IsIncludeAABBBy2D(AABB & bbSmall, AABB & bbLarge);

};
class CGameObject;
struct CPartitionNode
{

	bool m_bIsStatic;
	CGameObject * m_pObject;
};

template <int nodes>
class CPartitionTree
{
public:
	static enum XDivde { Left = 0, Right };
	static enum YDivde { Bottom = 0, Top };
	static enum ZDivde { Front = 0, Far };

	typedef list<CPartitionNode> NodeArray;
protected:
	NodeArray m_vlNodes;

	BoundingBox bb;
	AABB m_BoundingBox;
	XMFLOAT3 m_xmf3Center;
	CPartitionTree * m_pChildTrees[nodes];


//	ePartLoc m_e;
public:
	CPartitionTree(AABB & bbMapBox, int nLevels);
	CPartitionTree(XMFLOAT3 & xmf3Min, XMFLOAT3 & xmf3Max, int nLevels);
	~CPartitionTree();

	virtual void AddChild(int nLevels) = 0;
	//NodeArray & GetNodeArray();
};

#include "CollisionPartitionTree.cpp"

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