#pragma once


#ifndef __COLLISION
#define __COLLISION

class AABB
{
public:
	//�ٿ�� �ڽ��� �ּ����� �ִ����� ��Ÿ���� �����̴�.
	XMFLOAT3 m_xv3Minimum;
	XMFLOAT3 m_xv3Maximum;

public:
	AABB() { m_xv3Minimum = XMFLOAT3(+FLT_MAX, +FLT_MAX, +FLT_MAX); m_xv3Maximum = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX); }
	AABB(XMFLOAT3 xv3Minimum, XMFLOAT3 xv3Maximum) { m_xv3Minimum = xv3Minimum; m_xv3Maximum = xv3Maximum; }

	//�� ���� �ٿ�� �ڽ��� ���Ѵ�.
	void Union(XMFLOAT3& xv3Minimum, XMFLOAT3& xv3Maximum);
	void Union(AABB *pAABB);
	//�ٿ�� �ڽ��� 8���� �������� ��ķ� ��ȯ�ϰ� �ּ����� �ִ����� �ٽ� ����Ѵ�.
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