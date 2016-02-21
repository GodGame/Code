#include "stdafx.h"
#include "CollisionMgr.h"
#include "Object.h"
#include "Camera.h"
#include <algorithm>

//int giTreeNum = 0;

ostream & operator<<(ostream & os, AABB & bb)
{
	os << "MAX : " << bb.m_xv3Maximum << endl;
	os << "MIN : " << bb.m_xv3Minimum;
	return os;
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

void AABB::Update(XMFLOAT4X4 &pmtxTransform, AABB * bbMesh)
{
	if (bbMesh)
	{
		m_xv3Maximum = bbMesh->m_xv3Maximum;
		m_xv3Minimum = bbMesh->m_xv3Minimum;
	}

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

	//XMVECTOR xmvPos = XMVectorSet( pmtxTransform->_41 , pmtxTransform->_42, pmtxTransform->_43, 0 );

	for (int i = 0; i < 8; i++)
	{
		//정점을 변환한다.
		mtxTransform = XMLoadFloat4x4(&pmtxTransform);
		//vVertices[i] += xmvPos;
		vVertices[i] = XMVector3TransformCoord(vVertices[i], mtxTransform);
		XMStoreFloat4(&xmvVertcies, vVertices[i]);
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
ePartition AABB::IsIncludeAABB(AABB & bbSmall, AABB & bbLarge, bool bCollideCheck)
{
	if (!bCollideCheck)
		if (!AABB::CollisionAABB(bbSmall, bbLarge))
			return ePartition::CULL_OUT;

	if (bbSmall.m_xv3Minimum.x < bbLarge.m_xv3Minimum.x) return ePartition::CONTAIN_PART;
	if (bbSmall.m_xv3Minimum.y < bbLarge.m_xv3Minimum.y) return ePartition::CONTAIN_PART;
	if (bbSmall.m_xv3Minimum.z < bbLarge.m_xv3Minimum.z) return ePartition::CONTAIN_PART;

	if (bbSmall.m_xv3Maximum.x > bbLarge.m_xv3Maximum.x) return ePartition::CONTAIN_PART;
	if (bbSmall.m_xv3Maximum.y > bbLarge.m_xv3Maximum.y) return ePartition::CONTAIN_PART;
	if (bbSmall.m_xv3Maximum.z > bbLarge.m_xv3Maximum.z) return ePartition::CONTAIN_PART;

	return ePartition::CONTAIN_ALL;
}
bool AABB::CollisionAABBBy2D(AABB & one, AABB & two)
{
	if (one.m_xv3Minimum.x > two.m_xv3Maximum.x) return false;
	if (one.m_xv3Minimum.z > two.m_xv3Maximum.z) return false;
	if (two.m_xv3Minimum.x > one.m_xv3Maximum.x) return false;
	if (two.m_xv3Minimum.z > one.m_xv3Maximum.z) return false;

	return true;
}
ePartition AABB::IsIncludeAABBBy2D(AABB & bbSmall, AABB & bbLarge, bool bCollideCheck = false)
{
	if (!bCollideCheck)
		if (!AABB::CollisionAABBBy2D(bbSmall, bbLarge))
			return ePartition::CULL_OUT;

	if (bbSmall.m_xv3Minimum.x < bbLarge.m_xv3Minimum.x) return ePartition::CONTAIN_PART;
	if (bbSmall.m_xv3Minimum.z < bbLarge.m_xv3Minimum.z) return ePartition::CONTAIN_PART;

	if (bbSmall.m_xv3Maximum.x > bbLarge.m_xv3Maximum.x) return ePartition::CONTAIN_PART;
	if (bbSmall.m_xv3Maximum.z > bbLarge.m_xv3Maximum.z) return ePartition::CONTAIN_PART;

	return ePartition::CONTAIN_ALL;
}

DirectQuadTree::DirectQuadTree()
{
	m_pNodesArray = nullptr;
	m_nTreeLevels = 0;
	m_nNodes = 0;

	m_nMapWidth = 0;
	m_nMapLength = 0;
	m_fXScaleInverse = 0;
	m_fZScaleInverse = 0;
}

DirectQuadTree::~DirectQuadTree()
{
	if (m_pNodesArray) delete m_pNodesArray;
}

void DirectQuadTree::BuildQuadTree(BYTE nTreeLevels, UINT MapWidth, UINT MapLength)
{
	float fXScale, fZScale;

	m_nTreeLevels = nTreeLevels;
	fXScale = m_nMapWidth = MapWidth;
	fZScale = m_nMapLength = MapLength;

	for (unsigned int lv = m_nTreeLevels; lv > 0; --lv)
	{
		fXScale *= 0.5f;
		fZScale *= 0.5f;
		m_nNodes += pow(4, lv);
	}
	m_fXScaleInverse = 1 / fXScale;
	m_fZScaleInverse = 1 / fZScale;

	m_pNodesArray = new CPartitionNode[++m_nNodes];

	for (unsigned int i = 0; i < m_nNodes; ++i)
		m_pNodesArray[i].m_uIndex = i;
}

int DirectQuadTree::GetNodeContainingIndex(AABB & bb)
{
	int xmin = (int)(bb.m_xv3Minimum.x * m_fXScaleInverse);
	int zmin = (int)(bb.m_xv3Minimum.y * m_fZScaleInverse);

	int xResult = xmin ^ (int)(bb.m_xv3Maximum.x * m_fXScaleInverse);
	int zResult = zmin ^ (int)(bb.m_xv3Maximum.z * m_fZScaleInverse);

	int nNodeLv = m_nTreeLevels;
	int nShift = 0;

	while (xResult + zResult != 0)	// 최 상단 비트 찾기
	{
		xResult >>= 1;
		zResult >>= 1;
		nNodeLv--;
		nShift++;
	}
	xmin >>= nShift;
	zmin >>= nShift;

	return (zmin << (nNodeLv - 1) + xmin);
}

CPartitionNode * DirectQuadTree::GetNodeContaining(AABB & objBoundingBox)
{
	return &(m_pNodesArray[GetNodeContainingIndex(objBoundingBox)]);
}

CPartitionNode * DirectQuadTree::GetNodeContaining(float fLeft, float fRight, float fNear, float fFar)
{
	AABB bb;
	bb.m_xv3Maximum = { fRight, 0.0f, fFar };
	bb.m_xv3Minimum = { fLeft, 0.0f, fNear };

	int index = GetNodeContainingIndex(bb);

	return &m_pNodesArray[index];
}

int DirectQuadTree::EntryObject(CGameObject * pObject)
{
	int index = GetNodeContainingIndex(pObject->m_bcMeshBoundingCube);

	m_pNodesArray[index].m_vpObjects.push_back(pObject);

	return index;
}

void DirectQuadTree::EntryObjects(CGameObject ** ppObjectArrays, int nObjects)
{
	for (int i = 0; i < nObjects; ++i)
		DirectQuadTree::EntryObject(ppObjectArrays[i]);
}

QuadTree::QuadTree()
{
	for (int i = 0; i < 5; ++i) m_pNodes[i] = nullptr;

	ZeroMemory(&m_xmi3Center, sizeof(XMINT3));
	m_uHalfLength = 0;
	m_uHalfWidth = 0;

	m_bLeaf = false;
	m_bCulled = true;

	m_uTreeNum = 0;
	m_uTreeLevel = 0;
}

QuadTree::~QuadTree()
{
	for (int i = 0; i < 4; ++i) if (m_pNodes[i]) delete m_pNodes[i];
}

void QuadTree::BuildNodes(XMFLOAT3 & xmf3Center, UINT uWidth, UINT uLength, QuadTree * pParent, UINT uLevel)
{
	static UINT nBuilds = 0;
	m_uTreeNum = nBuilds++;
	m_uTreeLevel = uLevel;

	m_pNodes[Location::LOC_PARENT] = pParent;

	m_xmi3Center = XMINT3(xmf3Center.x, 0, xmf3Center.z);
	m_uHalfWidth = (uWidth >> 1);
	m_uHalfLength = (uLength >> 1);

	if (m_uHalfWidth < QAUD_MIN_UNIT)
	{
		m_bLeaf = true;
		return;
	}
	UINT uNextHalfWidth = (m_uHalfWidth >> 1);
	UINT uNextHalfLength = (m_uHalfLength >> 1);

	XMFLOAT3 xmf3Temp = { 0, 0, 0 };
	xmf3Temp.x = (float)m_xmi3Center.x - uNextHalfWidth;
	xmf3Temp.z = (float)m_xmi3Center.z - uNextHalfLength;
	m_pNodes[Location::LOC_LB] = QuadTree::CreateQuadTrees(xmf3Temp, m_uHalfWidth, m_uHalfLength, this, m_uTreeLevel + 1);

	xmf3Temp.x = (float)m_xmi3Center.x + uNextHalfWidth;
	xmf3Temp.z = (float)m_xmi3Center.z - uNextHalfLength;
	m_pNodes[Location::LOC_RB] = QuadTree::CreateQuadTrees(xmf3Temp, m_uHalfWidth, m_uHalfLength, this, m_uTreeLevel + 1);

	xmf3Temp.x = (float)m_xmi3Center.x - uNextHalfWidth;
	xmf3Temp.z = (float)m_xmi3Center.z + uNextHalfLength;
	m_pNodes[Location::LOC_LT] = QuadTree::CreateQuadTrees(xmf3Temp, m_uHalfWidth, m_uHalfLength, this, m_uTreeLevel + 1);

	xmf3Temp.x = (float)m_xmi3Center.x + uNextHalfWidth;
	xmf3Temp.z = (float)m_xmi3Center.z + uNextHalfLength;
	m_pNodes[Location::LOC_RT] = QuadTree::CreateQuadTrees(xmf3Temp, m_uHalfWidth, m_uHalfLength, this, m_uTreeLevel + 1);
}

QuadTree * QuadTree::CreateQuadTrees(XMFLOAT3 & xmf3Center, UINT uWidth, UINT uLength, QuadTree * pParent, UINT uLevel)
{
	QuadTree * pTree = new QuadTree();

	pTree->BuildNodes(xmf3Center, uWidth, uLength, pParent, uLevel);

	return pTree;
}

void QuadTree::FrustumCulling(CCamera * pCamera)
{
	XMFLOAT3 xmfMin = XMFLOAT3(m_xmi3Center.x - m_uHalfWidth, -1000.0f, m_xmi3Center.z - m_uHalfLength);
	XMFLOAT3 xmfMax = XMFLOAT3(m_xmi3Center.x + m_uHalfWidth, 1000.0f, m_xmi3Center.z + m_uHalfLength);

	m_bCulled = !(pCamera->IsInFrustum(xmfMin, xmfMax));
	if (m_bCulled) return;
	CGameObject * pObj = nullptr;

	if (m_uHalfLength > 512.0f)
	{
		for (auto it = m_vpObjectList.begin(); it != m_vpObjectList.end(); ++it)
		{
			pObj = (*it);
			//pObj->UpdateBoundingBox();
			AABB & bb = pObj->m_bcMeshBoundingCube;
			if (pCamera->IsInFrustum(&bb))
				pObj->GetGameMessage(nullptr, eMessage::MSG_CULL_IN);// SetActive(pCamera->IsInFrustum(&bb));
		}
	}
	else
	{
		for (auto it = m_vpObjectList.begin(); it != m_vpObjectList.end(); ++it)
		{
			(*it)->GetGameMessage(nullptr, eMessage::MSG_CULL_IN);
		}
	}
	if (m_bLeaf) return;

	for (int i = 0; i < 4; ++i)
		m_pNodes[i]->FrustumCulling(pCamera);
	/*for (int i = 0; i < 4; ++i)
		m_pNodes[i]->PreCutCulling();*/
}

void QuadTree::PreCutCulling()
{
	m_bCulled = true;
	if (m_bLeaf) return;

	for (int i = 0; i < 4; ++i)
		m_pNodes[i]->PreCutCulling();
}

Location QuadTree::IsContained(CGameObject * pObject, bool bCheckCollide)
{
	AABB & bbObj = pObject->m_bcMeshBoundingCube;
	const XMFLOAT3 pos = pObject->GetPosition();

	AABB bbQuad;

	if (bCheckCollide)
	{
		bbQuad.m_xv3Maximum = { (float)m_xmi3Center.x + m_uHalfWidth, (float)m_xmi3Center.y, (float)m_xmi3Center.z + m_uHalfLength };
		bbQuad.m_xv3Minimum = { (float)m_xmi3Center.x - m_uHalfWidth, (float)m_xmi3Center.y, (float)m_xmi3Center.z - m_uHalfLength };

		if (!AABB::CollisionAABBBy2D(bbObj, bbQuad))
			return Location(LOC_NONE);
	}
	bbQuad.m_xv3Maximum = { (float)m_xmi3Center.x, (float)m_xmi3Center.y, (float)m_xmi3Center.z };
	bbQuad.m_xv3Minimum = { (float)m_xmi3Center.x, (float)m_xmi3Center.y, (float)m_xmi3Center.z };

	UINT uCheck = 0;
	if (pos.z > m_xmi3Center.z) uCheck |= 0x02;
	if (pos.x > m_xmi3Center.x) uCheck |= 0x01;

	switch (uCheck)
	{
	case Location::LOC_LB:
		bbQuad.m_xv3Minimum.x -= m_uHalfWidth;
		bbQuad.m_xv3Minimum.z -= m_uHalfLength;
		break;

	case Location::LOC_RB:
		bbQuad.m_xv3Maximum.x += m_uHalfWidth;
		bbQuad.m_xv3Minimum.z -= m_uHalfLength;
		break;

	case Location::LOC_LT:
		bbQuad.m_xv3Minimum.x -= m_uHalfWidth;
		bbQuad.m_xv3Maximum.z += m_uHalfWidth;
		break;

	case Location::LOC_RT:
		bbQuad.m_xv3Maximum.x += m_uHalfWidth;
		bbQuad.m_xv3Maximum.z += m_uHalfLength;
		break;

	default:
		ASSERT(SUCCEEDED(1));
		break;
	}
	if (bbObj.m_xv3Maximum.x > bbQuad.m_xv3Maximum.x) return Location::LOC_ALL;
	if (bbObj.m_xv3Maximum.z > bbQuad.m_xv3Maximum.z) return Location::LOC_ALL;

	if (bbObj.m_xv3Minimum.x < bbQuad.m_xv3Minimum.x) return Location::LOC_ALL;
	if (bbObj.m_xv3Minimum.z < bbQuad.m_xv3Minimum.z) return Location::LOC_ALL;

	return Location(uCheck);
}

void QuadTree::FindContainedObjects_InChilds(CGameObject * pObject, vector<CGameObject*> & vcArray)
{
	if (!m_bLeaf) //말단노드 아니면 계산해서 넣는다.
	{
		Location loc = IsContained(pObject, false);

		if (loc == Location::LOC_ALL)
		{
			for (auto it = m_vpObjectList.begin(); it != m_vpObjectList.end(); ++it)
				vcArray.push_back(*it);

			return;
		}
		else
			m_pNodes[loc]->FindContainedObjects_InChilds(pObject, vcArray);
	}
	else
		for (auto it = m_vpObjectList.begin(); it != m_vpObjectList.end(); ++it)
			vcArray.push_back(*it);
}

QuadTree * QuadTree::EntityObject(CGameObject * pObject)
{
	Location eLoc = IsContained(pObject, false);

	if (eLoc != LOC_ALL && !m_bLeaf)
		return m_pNodes[eLoc]->EntityObject(pObject);

	m_vpObjectList.push_back(pObject);
	//cout << "등록 : " << m_uTreeNum << endl;
	return this;
}

void QuadTree::DeleteObject(CGameObject * pObject)
{
	Location eLoc = IsContained(pObject, false);

	if (eLoc != LOC_ALL && !m_bLeaf) 
	{
		m_pNodes[eLoc]->DeleteObject(pObject);
		return;
	}

	EraseObject(pObject);
}

void QuadTree::EraseObject(CGameObject * pObject)
{
	auto it = find_if(m_vpObjectList.begin(), m_vpObjectList.end(), [=](const CGameObject * pObj) { return pObject == pObj; });
	m_vpObjectList.erase(it);
}

bool QuadTree::SphereCollision(CGameObject * pTarget, vector<CGameObject*>* pContainedArray)
{
	bool bCheck = false;
	if (m_pNodes[LOC_PARENT])
		bCheck |= m_pNodes[LOC_PARENT]->SphereCollision(pTarget, pContainedArray, true);

	if (!m_bLeaf)
	{
		for (int i = 0; i < 4; ++i)
			bCheck |= m_pNodes[i]->SphereCollision(pTarget, pContainedArray, false);
	}

	if (pContainedArray)
		bCheck |= COLLISION.SphereCollisionOneToMul(pTarget, m_vpObjectList, *pContainedArray);
	else
		bCheck |= COLLISION.SphereCollisionOneToMul(pTarget, m_vpObjectList);

	return bCheck;
}

bool QuadTree::SphereCollision(CGameObject * pTarget, vector<CGameObject*> * pContainedArray, bool bIsUp)
{
	bool bCheck = false;
	if (bIsUp && m_pNodes[LOC_PARENT])
		bCheck |= m_pNodes[LOC_PARENT]->SphereCollision(pTarget, pContainedArray, true);

	if (!bIsUp && !m_bLeaf)
	{
		for (int i = 0; i < 4; ++i)
			bCheck |= m_pNodes[i]->SphereCollision(pTarget, pContainedArray, false);
	}

	if (pContainedArray)
		bCheck |= COLLISION.SphereCollisionOneToMul(pTarget, m_vpObjectList, *pContainedArray);
	else
		bCheck |= COLLISION.SphereCollisionOneToMul(pTarget, m_vpObjectList);

	return bCheck;
}


QuadTree * QuadTree::RenewalObject(CGameObject * pObject, bool bStart)
{
	CGameObject * pObj = pObject;
	Location eContain = IsContained(pObj, true);

	if (eContain != Location::LOC_ALL) // 다른 이동할 곳이 존재할때만 갱신한다.
	{
		if (bStart)	// 시작지이면 리스트에서 제거한다.
		{
			vector<CGameObject*>::iterator it = find(m_vpObjectList.begin(), m_vpObjectList.end(), pObject);
			m_vpObjectList.erase(it);
			pObj->UpdateBoundingBox();
		}
		if (eContain == Location::LOC_NONE) // 충돌하지 않으면 부모로 올라가 찾아본다.
			return m_pNodes[Location::LOC_PARENT]->RenewalObject(pObj, false);

		if (!m_bLeaf)	// 말단 노드가 아니면 하위 노드를 찾아본다.
			return m_pNodes[eContain]->RenewalObject(pObj, false);

		// 말단 노드이면 오브젝트를 추가한다.
	//	pObj->UpdateBoundingBox();
		m_vpObjectList.push_back(pObj);
		pObj = nullptr;
	}

	if (pObj && bStart == false)
	{
		//pObj->UpdateBoundingBox();
		m_vpObjectList.push_back(pObj);
	}
	return this;
}

///////////////////////////////////////////////////////////////////////////////////////
CQuadTreeManager::CQuadTreeManager()
{
	m_pRootTree = nullptr;

	m_vcContainedArray.reserve(1000);
	m_vcDynamicArray.reserve(100);
}

CQuadTreeManager::~CQuadTreeManager()
{
	ReleaseQuadTree();
}

CQuadTreeManager & CQuadTreeManager::GetInstance()
{
	static CQuadTreeManager instance;
	return instance;
}

void CQuadTreeManager::BuildQuadTree(XMFLOAT3 & xmf3Center, UINT uWidth, UINT uLength, QuadTree * pParent)
{
	if (nullptr == m_pRootTree)
		m_pRootTree = QuadTree::CreateQuadTrees(xmf3Center, uWidth, uLength, pParent, 0);
}

void CQuadTreeManager::ReleaseQuadTree()
{
	if (m_pRootTree) delete m_pRootTree;
	m_pRootTree = nullptr;

	for (int i = 0; i < m_vcDynamicArray.size(); ++i)
		m_vcDynamicArray.pop_back();

	for (int i = 0; i < m_vcContainedArray.size(); ++i)
		m_vcContainedArray.pop_back();
}

void CQuadTreeManager::FrustumCullObjects(CCamera * pCamera)
{
	m_pRootTree->FrustumCulling(pCamera);
}

vector<CGameObject*>& CQuadTreeManager::CollisionCheckList(CGameObject * pObject)
{
	m_vcContainedArray.clear();
	QuadTree * pTree = GetDynamicInfo(pObject)->first;
	
	if (pTree->m_uTreeLevel > 3)
		pTree = pTree->GetParentNode();
	
	pTree->SphereCollision(pObject, &m_vcContainedArray);

	return m_vcContainedArray;
}

bool CQuadTreeManager::CollisionCheck(CGameObject * pObject)
{
	QuadTree * pTree = GetDynamicInfo(pObject)->first;

	if (pTree->m_uTreeLevel > 3)
		pTree = pTree->GetParentNode();

	return pTree->SphereCollision(pObject, nullptr);
}

UINT CQuadTreeManager::ContainedErase()
{
	UINT sz = m_vcContainedArray.size();
	if (sz > 0) 
	{
		for (int i = sz - 1; i >= 0; --i)
		{
			m_pRootTree->DeleteObject(m_vcContainedArray[i]);
			m_vcContainedArray.pop_back();
		}
	}
	return sz;
}

vector<CGameObject*>* CQuadTreeManager::GetContainedObjectList(CGameObject * pObject)
{
	m_vcContainedArray.clear();

	if (m_pRootTree) 
		m_pRootTree->FindContainedObjects_InChilds(pObject, m_vcContainedArray);

	return &m_vcContainedArray;
}

vector<pair<QuadTree*, CGameObject*>>::iterator CQuadTreeManager::GetDynamicInfo(CGameObject * pObject)
{
	return find_if(m_vcDynamicArray.begin(), m_vcDynamicArray.end(),
		[=](const DynamicInfo & a) { return pObject == a.second;});
}

QuadTree * CQuadTreeManager::EntityStaticObject(CGameObject * pObject)
{
	return m_pRootTree->EntityObject(pObject);
}

QuadTree * CQuadTreeManager::EntityDynamicObject(CGameObject * pObject)
{
	QuadTree * pTree = m_pRootTree->EntityObject(pObject);
	m_vcDynamicArray.push_back(DynamicInfo(pTree, pObject));
	return pTree;
}

void CQuadTreeManager::DeleteStaticObject(CGameObject * pObject)
{
	m_pRootTree->DeleteObject(pObject);
}

void CQuadTreeManager::DeleteDynamicObject(CGameObject * pObject)
{
	auto it = GetDynamicInfo(pObject);
	m_vcDynamicArray.erase(it);

	m_pRootTree->DeleteObject(pObject);
}

UINT CQuadTreeManager::RenewalDynamicObjects()
{
	UINT uCountRenewal = 0;
	QuadTree * pBefore = nullptr;
	QuadTree * pTree = nullptr;
	CGameObject * pObject = nullptr;

	for (auto it = m_vcDynamicArray.begin(); it != m_vcDynamicArray.end(); ++it)
	{
		pBefore = it->first;	
		if (pBefore->IsCulled()) continue;

		pObject = it->second;
		pObject->UpdateBoundingBox();
		pTree = pBefore->RenewalObject(pObject, true);

		if (pBefore != pTree)
		{
			it->first = pTree;
			cout << "새로운 트리 넘버 : " << pTree->m_uTreeNum << endl;
			++uCountRenewal;
		}
	}

	return uCountRenewal;
}

void CQuadTreeManager::Update(CCamera * pCamera)
{
	RenewalDynamicObjects();
	FrustumCullObjects(pCamera);
}

CCollisionMgr::CCollisionMgr()
{
	m_pQuadMgr = &CQuadTreeManager::GetInstance();
}

CCollisionMgr::~CCollisionMgr()
{
}

CCollisionMgr & CCollisionMgr::GetInstance()
{
	static CCollisionMgr instance;
	return instance;
}

CGameObject* CCollisionMgr::SphereCollisionObject(CGameObject * pTarget, vector<CGameObject*>& vcObjList)
{
	CGameObject * pObject = nullptr;
	m_bbSphereTarget.Center = pTarget->GetPosition();
	m_bbSphereTarget.Radius = pTarget->GetSize();

	for (auto it = vcObjList.begin(); it != vcObjList.end(); ++it)
	{
		pObject = *it;
		if (pTarget != pObject)
		{
			m_bbSphereOther.Center = pObject->GetPosition();
			m_bbSphereOther.Radius = pObject->GetSize();
			if (m_bbSphereTarget.Intersects(m_bbSphereOther))
			{
				return pObject;
			}
		}
	}
	return nullptr;
}

bool CCollisionMgr::SphereCollisionOneToMul(CGameObject * pTarget, vector<CGameObject*>& vcObjList)
{
	bool bCheck = false;
	CGameObject * pObject = nullptr;
	m_bbSphereTarget.Center = pTarget->GetPosition();
	m_bbSphereTarget.Radius = pTarget->GetSize();

	for (auto it = vcObjList.begin(); it != vcObjList.end(); ++it)
	{
		pObject = *it;
		if (pObject->CanCollide(pTarget))
		{
			m_bbSphereOther.Center = pObject->GetPosition();
			m_bbSphereOther.Radius = pObject->GetSize();
			if (m_bbSphereTarget.Intersects(m_bbSphereOther))
			{
				pTarget->SendGameMessage(pObject, eMessage::MSG_COLLIDE);
				if (!bCheck) bCheck = !bCheck;
			}
		}
	}
	return bCheck;
}

bool CCollisionMgr::SphereCollisionOneToMul(CGameObject * pTarget, vector<CGameObject*>& vcObjList, vector<CGameObject*>& vcContained)
{
	bool bCheck = false;
	CGameObject * pObject = nullptr;
	m_bbSphereTarget.Center = pTarget->GetPosition();
	m_bbSphereTarget.Radius = pTarget->GetSize();

	for (auto it = vcObjList.begin(); it != vcObjList.end(); ++it)
	{
		pObject = *it;
		if (pObject->CanCollide(pTarget))
		{
			m_bbSphereOther.Center = pObject->GetPosition();
			m_bbSphereOther.Radius = pObject->GetSize();
			if (m_bbSphereTarget.Intersects(m_bbSphereOther))
			{
				pTarget->SendGameMessage(pObject, eMessage::MSG_COLLIDE);
				vcContained.push_back(pObject);
				if (!bCheck) bCheck = !bCheck;
			}
		}
	}
	return bCheck;
}

