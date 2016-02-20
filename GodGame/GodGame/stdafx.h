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

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:
#include <windows.h>
#include <stdio.h>
#include <iostream>
// C ��Ÿ�� ��� �����Դϴ�.
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

#endif