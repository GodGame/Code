#pragma once

#ifndef __COLLISION
#define __COLLISION

#include "stdafx.h"
#include <list>
enum ePartition { NONE = -1, CULL_OUT = 0, CONTAIN_PART, CONTAIN_ALL };

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
	void Update(XMFLOAT4X4 & xmtxTransform, AABB * bbMesh = nullptr);

	static bool CollisionAABB(AABB & one, AABB & two);
	static ePartition IsIncludeAABB(AABB & bbSmall, AABB & bbLarge, bool bCollideCheck);
	static bool CollisionAABBBy2D(AABB & one, AABB & two);
	static ePartition IsIncludeAABBBy2D(AABB & bbSmall, AABB & bbLarge, bool bCollideCheck);

};

class CGameObject;

struct CPartitionNode
{
	unsigned int		 m_uIndex;
	AABB				 m_BoundingBox;
	vector<CGameObject*> m_vpObjects;
};

//template <int nodes>
//class CPartitionTree
//{
//public:
//	static enum XDivde { Left = 0, Right };
//	static enum YDivde { Bottom = 0, Top };
//	static enum ZDivde { Front = 0, Far };
//
//	typedef list<CPartitionNode> NodeArray;
//protected:
//	NodeArray m_vlNodes;
//
//	BoundingBox bb;
//	AABB m_BoundingBox;
//	XMFLOAT3 m_xmf3Center;
//	CPartitionTree * m_pChildTrees[nodes];
//
//
////	ePartLoc m_e;
//public:
//	CPartitionTree(AABB & bbMapBox, int nLevels);
//	CPartitionTree(XMFLOAT3 & xmf3Min, XMFLOAT3 & xmf3Max, int nLevels);
//	~CPartitionTree();
//
//	virtual void AddChild(int nLevels) = 0;
//	//NodeArray & GetNodeArray();
//};
//
//#include "CollisionPartitionTree.cpp"

class DirectQuadTree
{
	CPartitionNode * m_pNodesArray;
	unsigned int m_nTreeLevels : 8;
	unsigned int m_nMapWidth : 12;
	unsigned int m_nMapLength : 12;

	unsigned int m_nNodes;

	float m_fXScaleInverse;
	float m_fZScaleInverse;

public:
	DirectQuadTree();
	~DirectQuadTree();

	void BuildQuadTree(BYTE nTreeLevels, UINT MapWidth, UINT MapLength);

public:
	int GetNodeContainingIndex(AABB & bb);

	CPartitionNode * GetNodeContaining(AABB & objBoundingBox);
	CPartitionNode * GetNodeContaining(float fLeft, float fRight, float fNear, float fFar);

	int EntryObject(CGameObject * pObject);
	void EntryObjects(CGameObject ** ppObjectArrays, int nObjects);
public:
	//CPartitionNode * GetChildNode(int index);
};

#define QAUD_MIN_UNIT 128

enum Location{ LOC_NONE = -1, LOC_LB, LOC_RB, LOC_LT, LOC_RT, LOC_PARENT, LOC_ALL };

class CCamera;
class QuadTree
{
public:
	

private:
	vector<CGameObject*> m_vpObjectList;
	QuadTree * m_pNodes[5];

private:
	UINT m_uHalfWidth : 15;
	UINT m_uHalfLength : 15;
	UINT m_bLeaf : 1;
	bool m_bCulled : 1;

	XMINT3 m_xmi3Center;

public:		
	QuadTree();
	~QuadTree();

	void BuildNodes(XMFLOAT3 & xmf3Center, UINT uWidth, UINT uLength, QuadTree * pParent);
	static QuadTree * CreateQuadTrees(XMFLOAT3 & xmf3Center, UINT uWidth, UINT uLength, QuadTree * pParent);

	void FrustumCulling(CCamera * pCamera);
	void PreCutCulling();
	bool IsCulled() { return m_bCulled; }

	Location IsContained(CGameObject * pObject, bool bCheckCollide);
	void FindContainedObjects_InChilds(CGameObject * pObject, vector<CGameObject*> & vcArray);

	QuadTree* EntityObject(CGameObject * pObject);
	QuadTree* RenewalObject(CGameObject * pObject, bool bStart = true);
	void DeleteObject(CGameObject * pObject);
	
//s	bool ReleaseTree();
};

class CCamera;
class CQuadTreeManager
{
private:
	typedef pair<QuadTree*, CGameObject*> DynamicInfo;
	CQuadTreeManager();
	~CQuadTreeManager();

private:
	QuadTree * m_pRootTree;
	
	vector<CGameObject*> m_vcContainedArray;
	vector<DynamicInfo> m_vcDynamicArray;

public:
	static CQuadTreeManager& GetInstance();
	void BuildQuadTree(XMFLOAT3 & xmf3Center, UINT uWidth, UINT uLength, QuadTree * pParent);
	void ReleaseQuadTree();

	QuadTree * GetRootTree() { return m_pRootTree; }

public:
	QuadTree* EntityStaticObject(CGameObject* pObject);
	QuadTree* EntityDynamicObject(CGameObject* pObject);

	void DeleteStaticObject(CGameObject* pObject);
	void DeleteDynamicObject(CGameObject* pObject);


	UINT RenewalDynamicObjects();

public:
	void FrustumCullObjects(CCamera * pCamera);
	bool IsCollide(CGameObject * pObject);
	vector<CGameObject*>* GetContainedObjectList(CGameObject * pObject);
};

class CCollisionMgr
{
	CCollisionMgr();
	~CCollisionMgr();

	CQuadTreeManager * m_pQuadMgr;

public:
	static CCollisionMgr& GetInstance();

};
#define QUADMgr CQuadTreeManager::GetInstance()

#endif