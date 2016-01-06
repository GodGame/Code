#pragma once

#define PI		3.141592
#define	FRAME_BUFFER_WIDTH		1280
#define	FRAME_BUFFER_HEIGHT		960

#define	VS_SLOT_VIEWPROJECTION	0x00
#define	VS_SLOT_WORLD_MATRIX	0x01
#define VS_SLOT_CAMERAPOS		0x02
#define SLOT_DISPLACEMENT		0x03

#define SLOT_RANDOM1D			9
#define SLOT_SSAO_SCREEN		25
#define SLOT_CB_SSAO			6

#define	PS_SLOT_COLOR				0x00

//조명과 재질을 설정하기 위한 상수 버퍼의 슬롯 번호를 정의한다. 
#define PS_SLOT_LIGHT			0x00
#define PS_SLOT_MATERIAL		0x01

#define PS_SLOT_TEXTURE		0x00
#define PS_SLOT_SAMPLER_STATE		0x00

#define	PS_SLOT_CUBE_TEXTURE		0x02
#define	PS_SLOT_CUBE_SAMPLER_STATE  0x02

#define PS_SLOT_TEXTURE_ARRAY		10

#define SLOT_PARTICLE 0x04
#define SLOT_SHADOWMAP 0x05
#define PS_SLOT_SHADOWMAP 30
#define PS_SLOT_SHADOWSAMPLE 0x03

#define NUM_SHADER	3
#define NUM_THREAD	NUM_SHADER
#define NUM_MRT		6

#define _THREAD

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

#include <d3d9types.h>

#include <Mmsystem.h>
#include <math.h>
#include <process.h>
#include <iostream>
#include <vector>
#include <map>
#include <time.h>
using namespace std;

