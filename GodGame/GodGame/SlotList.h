#pragma once

#ifndef __SLOTLIST
#define __SLOTLIST

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


#endif