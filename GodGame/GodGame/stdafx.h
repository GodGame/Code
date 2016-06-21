#pragma once

#ifndef __COMMON
#define __COMMON

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
//#pragma warning(disable : 4244)
#define	WM_SOCKET	 WM_USER + 1
#define SERVER_PORT 9000

#define NOT_PSUPDATE	0x01
#define	RS_SHADOWMAP	0x02
#define DRAW_AND_ACTIVE 0x04

#define PI		3.141592
#define	FRAME_BUFFER_WIDTH		1280
#define	FRAME_BUFFER_HEIGHT		960

#define NUM_MRT		6
#define NUM_SHADER	7

//#ifdef _DEBUG
//#define NUM_THREAD  NUM_SHADER
//#else
//#define NUM_THREAD	(NUM_SHADER + 1)
//#endif

#define _THREAD
//#define _SAVE_DATA
#define _LOAD_DATA
#define _RANDOM_POS

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

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
using namespace D2D1;

#include <Mmsystem.h>r
#include <math.h>
#include <process.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <time.h>
//#include <atlconv.h>
#include "FW1FontWrapper_1_1/FW1FontWrapper.h"
//#ifdef _WIN64
#pragma comment(lib, "FW1FontWrapper")
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

#define COLOR_NONE   -1
#define COLOR_WHITE  0
#define COLOR_RED    1
#define COLOR_GREEN  2
#define COLOR_BLUE   3
#define COLOR_BLACK  4
#define COLOR_YELLOW 5
#define COLOR_MAGENT 6
#define COLOR_CYAN   7
#define COLOR_GRAY   8
#define COLOR_BLACK_GRAY   9

#define COS_10 0.984807
#define COS_15 0.965925
#define COS_20 0.939692
#define COS_25 0.906307
#define COS_30 0.866025
#define COS_35 0.819152
#define COS_40 0.766044
#define COS_45 0.707106
#define COS_50 0.642787
#define COS_55 0.573576
#define COS_60 0.5
#define COS_65 0.422618
#define COS_70 0.34202
#define COS_75 0.258819
#define COS_80 0.173648
#define COS_85 0.087155

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