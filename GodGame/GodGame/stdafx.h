#pragma once

#ifndef __COMMON
#define __COMMON

//#pragma warning(disable : 4244)

#define NOT_PSUPDATE	0x01
#define	RS_SHADOWMAP	0x02
#define DRAW_AND_ACTIVE 0x04

#define PI		3.141592
#define	FRAME_BUFFER_WIDTH		1280
#define	FRAME_BUFFER_HEIGHT		960

#define NUM_MRT		6
#define NUM_SHADER	6

//#ifdef _DEBUG
#define NUM_THREAD  NUM_SHADER
//#else
//#define NUM_THREAD	(NUM_SHADER + 1)
//#endif




#define _THREAD

#include "SlotList.h"
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>
#include <stdio.h>
#include <iostream>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

//#include <d3d11.h>
#include <D3DX11.h>
//#include <D3DX10Math.h>

//#include <xnamath.h>
#include <DirectXMath.h>
using namespace DirectX;
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

//#include <d3d9types.h>

#include <Mmsystem.h>
#include <math.h>
#include <process.h>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <time.h>
//#include <ASSERT.h>

#ifdef _DEBUG
#define ASSERT(x)   assert(x) 
#define ASSERT_S(x) assert(SUCCEEDED(x))
#define ASSERT_F(x) assert(FAILED(x))
#else
#define ASSERT(x)   x
#define ASSERT_S(x) x
#define ASSERT_F(x) x
#endif

#define _QUAD_TREE

using namespace std;

ostream& operator<<(ostream& os, POINT & pt);
ostream& operator<<(ostream& os, RECT  & rect);
ostream& operator<<(ostream& os, LPRECT  & rect);

ostream& operator<<(ostream& os, XMFLOAT2 & xmf2);
ostream& operator<<(ostream& os, XMFLOAT3 & xmf3);
ostream& operator<<(ostream& os, XMFLOAT4 & xmf4);

ostream& operator<<(ostream& os, XMFLOAT4X4 & mtx);


template <class T>
class MyPriorityPointerQueue
{
public:
	typedef list<T*> PointerType;
//	typedef PointerType::iterator TypeIt;

private:
	PointerType _Container;

public:
	MyPriorityPointerQueue() {}
	~MyPriorityPointerQueue() {}

	typename  list<T*>::iterator begin() { return _Container.begin(); }
	typename  list<T*>::iterator end()   { return _Container.end(); }
	size_t size()  { return _Container.size(); }
	bool   empty() { return _Container.empty(); }
	void   clear() { _Container.clear();}

	T* least()
	{
		if (_Container.empty())
			return nullptr;

		auto it = _Container.begin();
		T* data = *it;

		_Container.erase(it);
		return data;
	}
	T* dequeue() { return least(); }

	T* greatest()
	{
		if (_Container.empty())
			return nullptr;

		auto it = --_Container.end();
		T* data = *it;

		_Container.erase(it);
		return data;
	}
	T* pop() { return greatest(); }

	void insert(T * data)
	{
		T * pTemp = data;

		if (_Container.empty())
			_Container.push_back(data);
		else
		{
			list<T*>::iterator itEnd = _Container.end();
			list<T*>::iterator it = _Container.begin();

			while (it != itEnd && !operator<(*pTemp, *(*it)))
			{
				it++;
			}
			_Container.insert(it, data);
		}
	}
	void enqueue(T * data) { insert(data); }
	void push(T * data)    { insert(data); }

	void erase(T * data)   { _Container.erase(data); }
	typename  list<T*>::iterator find(T * data)  { return _Container.find(_Container.begin(), _Container.end(), data); }
	//	TypeIt find_if(void * p) { return _Container.find_if(_Container.begin(), _Container.end(), p);}
};

#endif