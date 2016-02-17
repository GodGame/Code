#pragma once

#ifndef __COMMON
#define __COMMON

//#pragma warning(disable : 4244)

#define NOT_PSUPDATE	0x01
#define	RS_SHADOWMAP	0x02

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
#define ASSERT(x) assert(x) 
#else
#define ASSERT(x) x
#endif

#define _QUAD_TREE

using namespace std;

ostream& operator<<(ostream& os, XMFLOAT3 & xmf3);
ostream& operator<<(ostream& os, XMFLOAT4 & xmf4);

ostream& operator<<(ostream& os, XMFLOAT4X4 & mtx);


#endif