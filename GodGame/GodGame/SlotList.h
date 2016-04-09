#pragma once

#ifndef __SLOTLIST
#define __SLOTLIST

////////////////////////////////////////////////////////////////////////
#define	CB_SLOT_VIEWPROJECTION	     0x00
#define	CB_SLOT_WORLD_MATRIX	     0x01
//#define CB_SLOT_CAMERAPOS		     0x02
#define CB_SLOT_DISPLACEMENT	     0x03
#define CB_SLOT_PARTICLE		     0x04
#define CB_SLOT_STATIC_SHADOWMAP	 0x05
#define CB_SLOT_DYNAMIC_SHADOWMAP	 0x06

#define CB_PS_SLOT_LIGHT		     0x00
#define CB_PS_SLOT_MATERIAL		     0x01

////////////////////////////////////////////////////////////////////////
#define TX_SLOT_TEXTURE				 0x00
#define SS_SLOT_SAMPLER_STATE		 0x00

#define	TX_SLOT_CUBE_TEXTURE		 0x02
#define	SS_SLOT_CUBE_SAMPLER_STATE   0x02

#define TX_SLOT_TEXTURE_ARRAY		10

#define TX_SLOT_SSAO_SCREEN			25

#define TX_SLOT_MRT_TXCOLOR			17
#define TX_SLOT_MRT_POS				18
#define TX_SLOT_MRT_DIFFUSE			19
#define TX_SLOT_MRT_SPECULAR		20
#define TX_SLOT_MRT_NORMAL			21

#define TX_MRT_STARTSLOT	TX_SLOT_MRT_TXCOLOR
#define TX_MRT_ENDSLOT		TX_SLOT_MRT_NORMAL

#define TX_SLOT_RANDOM1D			22
#define TX_SLOT_

#define TX_SLOT_STATIC_SHADOW		30
#define TX_SLOT_SHADOWMAP			31
#define SS_SLOT_SHADOWSAMPLE		 0x03

#endif