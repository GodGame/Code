#include "stdafx.h"
#include "CollisionMgr.h"


CCollisionMgr::CCollisionMgr()
{
}


CCollisionMgr::~CCollisionMgr()
{
}



void AABB::Union(XMFLOAT3& xv3Minimum, XMFLOAT3& xv3Maximum)
{
	if (xv3Minimum.x < m_xv3Minimum.x) m_xv3Minimum.x = xv3Minimum.x;
	if (xv3Minimum.y < m_xv3Minimum.y) m_xv3Minimum.y = xv3Minimum.y;
	if (xv3Minimum.z < m_xv3Minimum.z) m_xv3Minimum.z = xv3Minimum.z;
	if (xv3Maximum.x > m_xv3Maximum.x) m_xv3Maximum.x = xv3Maximum.x;
	if (xv3Maximum.y > m_xv3Maximum.y) m_xv3Maximum.y = xv3Maximum.y;
	if (xv3Maximum.z > m_xv3Maximum.z) m_xv3Maximum.z = xv3Maximum.z;
}

void AABB::Union(AABB *pAABB)
{
	Union(pAABB->m_xv3Minimum, pAABB->m_xv3Maximum);
}

void AABB::Update(XMFLOAT4X4 *pmtxTransform)
{
	XMVECTOR vVertices[8];
	vVertices[0] = XMVectorSet(m_xv3Minimum.x, m_xv3Minimum.y, m_xv3Minimum.z, 1);
	vVertices[1] = XMVectorSet(m_xv3Minimum.x, m_xv3Minimum.y, m_xv3Maximum.z, 1);
	vVertices[2] = XMVectorSet(m_xv3Maximum.x, m_xv3Minimum.y, m_xv3Maximum.z, 1);
	vVertices[3] = XMVectorSet(m_xv3Maximum.x, m_xv3Minimum.y, m_xv3Minimum.z, 1);
	vVertices[4] = XMVectorSet(m_xv3Minimum.x, m_xv3Maximum.y, m_xv3Minimum.z, 1);
	vVertices[5] = XMVectorSet(m_xv3Minimum.x, m_xv3Maximum.y, m_xv3Maximum.z, 1);
	vVertices[6] = XMVectorSet(m_xv3Maximum.x, m_xv3Maximum.y, m_xv3Maximum.z, 1);
	vVertices[7] = XMVectorSet(m_xv3Maximum.x, m_xv3Maximum.y, m_xv3Minimum.z, 1);
	m_xv3Minimum = XMFLOAT3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	m_xv3Maximum = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	//8개의 정점에서 x, y, z 좌표의 최소값과 최대값을 구한다.
	XMMATRIX mtxTransform;
	XMFLOAT4 xmvVertcies;
	for (int i = 0; i < 8; i++)
	{
		//정점을 변환한다.
		mtxTransform = XMLoadFloat4x4(pmtxTransform);
		vVertices[i] = XMVector3TransformCoord(vVertices[i], mtxTransform);
		XMStoreFloat4(&xmvVertcies, vVertices[i]);
		//xv3ec3TransformCoord(&vVertices[i], &vVertices[i], pmtxTransform);
		if (xmvVertcies.x < m_xv3Minimum.x) m_xv3Minimum.x = xmvVertcies.x;
		if (xmvVertcies.y < m_xv3Minimum.y) m_xv3Minimum.y = xmvVertcies.y;
		if (xmvVertcies.z < m_xv3Minimum.z) m_xv3Minimum.z = xmvVertcies.z;
		if (xmvVertcies.x > m_xv3Maximum.x) m_xv3Maximum.x = xmvVertcies.x;
		if (xmvVertcies.y > m_xv3Maximum.y) m_xv3Maximum.y = xmvVertcies.y;
		if (xmvVertcies.z > m_xv3Maximum.z) m_xv3Maximum.z = xmvVertcies.z;
	}
}
bool AABB::CollisionAABB(AABB & one, AABB & two)
{
	if (CollisionAABBBy2D(one, two)) return true;

	if (one.m_xv3Minimum.z > two.m_xv3Maximum.z) return false;
	if (two.m_xv3Minimum.z > two.m_xv3Maximum.z) return false;

	return true;
}
ePartLoc AABB::IsIncludeAABB(AABB & bbSmall, AABB & bbLarge)
{
	if (!AABB::CollisionAABB(bbSmall, bbLarge)) return ePartLoc::INCLUDE_NOT;

	if (bbSmall.m_xv3Minimum.x < bbLarge.m_xv3Minimum.x) return ePartLoc::INCLUDE_PART_MAX;
	if (bbSmall.m_xv3Minimum.y < bbLarge.m_xv3Minimum.y) return ePartLoc::INCLUDE_PART_MAX;
	if (bbSmall.m_xv3Minimum.z < bbLarge.m_xv3Minimum.z) return ePartLoc::INCLUDE_PART_MAX;

	if (bbSmall.m_xv3Maximum.x > bbLarge.m_xv3Maximum.x) return ePartLoc::INCLUDE_PART_MIN;
	if (bbSmall.m_xv3Maximum.y > bbLarge.m_xv3Maximum.y) return ePartLoc::INCLUDE_PART_MIN;
	if (bbSmall.m_xv3Maximum.z > bbLarge.m_xv3Maximum.z) return ePartLoc::INCLUDE_PART_MIN;

	return ePartLoc::INCLUDE_ALL;
}
bool AABB::CollisionAABBBy2D(AABB & one, AABB & two)
{
	if (one.m_xv3Minimum.x > two.m_xv3Maximum.x) return false;
	if (one.m_xv3Minimum.y > two.m_xv3Maximum.y) return false;
	if (two.m_xv3Minimum.x > two.m_xv3Maximum.x) return false;
	if (two.m_xv3Minimum.y > two.m_xv3Maximum.y) return false;

	return true;
}
ePartLoc AABB::IsIncludeAABBBy2D(AABB & bbSmall, AABB & bbLarge)
{
	if (!AABB::CollisionAABBBy2D(bbSmall, bbLarge)) return ePartLoc::INCLUDE_NOT;

	if (bbSmall.m_xv3Minimum.x < bbLarge.m_xv3Minimum.x) return ePartLoc::INCLUDE_PART_MAX;
	if (bbSmall.m_xv3Minimum.y < bbLarge.m_xv3Minimum.y) return ePartLoc::INCLUDE_PART_MAX;

	if (bbSmall.m_xv3Maximum.x > bbLarge.m_xv3Maximum.x) return ePartLoc::INCLUDE_PART_MIN;
	if (bbSmall.m_xv3Maximum.y > bbLarge.m_xv3Maximum.y) return ePartLoc::INCLUDE_PART_MIN;

	return ePartLoc::INCLUDE_ALL;
}