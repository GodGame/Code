#include "CollisionMgr.h"


template<int nodes>
CPartitionTree<nodes>::CPartitionTree(AABB & bbMapBox, int nLevels)
{
	m_BoundingBox = bbMapBox;

	XMVECTOR xmvMin = XMLoadFloat3(&m_BoundingBox.m_xv3Minimum);
	XMVECTOR xmvMax = XMLoadFloat3(&m_BoundingBox.m_xv3Maximum);

	XMStoreFloat3(&m_xmf3Center, (xmvMin + xmvMax) * 0.5f );
//	AddChild(nLevels);
}

template<int nodes>
CPartitionTree<nodes>::CPartitionTree(XMFLOAT3 & xmf3Min, XMFLOAT3 & xmf3Max, int nLevels)
{
	m_BoundingBox.m_xv3Maximum = xmf3Max;
	m_BoundingBox.m_xv3Minimum = xmf3Min;

	XMVECTOR xmvMin = XMLoadFloat3(&m_BoundingBox.m_xv3Minimum);
	XMVECTOR xmvMax = XMLoadFloat3(&m_BoundingBox.m_xv3Maximum);

	XMStoreFloat3(&m_xmf3Center, (xmvMin + xmvMax) * 0.5f);
//	AddChild(nLevels);
}

template<int nodes>
CPartitionTree<nodes>::~CPartitionTree()
{
	for (int i = 0; i < nodes; ++i)
	{
		if (m_pChildTrees[i]) 
			delete m_pChildTrees[i];
	}

}

//template<int nodes>
//void CPartitionTree<nodes>::AddChild(int nLevels)
//{
//}
