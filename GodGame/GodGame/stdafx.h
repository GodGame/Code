#pragma once

#ifndef __COMMON
#define __COMMON

#define NOT_PSUPDATE	1<<0
#define	RS_SHADOWMAP	NOT_PSUPDATE

#define PI		3.141592
#define	FRAME_BUFFER_WIDTH		1280
#define	FRAME_BUFFER_HEIGHT		960


#define NUM_SHADER	4
#define NUM_THREAD	NUM_SHADER
#define NUM_MRT		6

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
#include <time.h>
using namespace std;



#endif