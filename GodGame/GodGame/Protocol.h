#pragma once
#include <DirectXMath.h>
using namespace DirectX;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned long      DWORD;


#define MAX_BUFF_SIZE   4000
#define MAX_PACKET_SIZE  255
#define MAX_USER 10

#define OP_RECV 1
#define OP_SEND 2
#define OP_OBJECT 3
#define OP_ROUND_TIME 4
#define OP_CHANGE_GAMESTATE 5
#define OP_START_COUNT 6
#define OP_TIME 7

#define CS_UP    1
#define CS_DOWN  2
#define CS_LEFT  3
#define CS_RIGHT    4
#define CS_CHAT		5
#define CS_ANI_IDLE 6
#define CS_ANI_MAGIC_CAST1 7
#define CS_ANI_MAGIC_CAST2 8
#define CS_HEIGHT 9
#define CS_READY 10
#define CS_INPUT 11
#define CS_CHAR_DAMAGE 12
#define CS_MOVE 13
#define CS_ROTATION 14
#define CS_DOMINATE 15
#define CS_MAGIC_CASTING 16
#define SERVER_CHEAT 99

#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_CHAT		4
#define SC_ANI_IDLE 5
#define SC_ANI_MAGIC_CAST1 6
#define SC_ANI_MAGIC_CAST2 7
#define SC_OBJECT_INIT 8
#define SC_ROUND_TIME 9
#define SC_GAME_STATE 10
#define SC_PLAYER_INFO 11
#define SC_ROTATION 12
#define SC_DOMINATE 13
#define SC_MAGIC_CASTING 14

#define GAMESTATE_WAIT 1
#define GAMESTATE_START 2
#define GAMESTATE_END 3

#define INFO_TYPE_HP 1
#define INFO_TYPE_ELEMENT 2
#define INFO_TYPE_ITEM 3

#define S_ELEMENT_NULL			-1
#define S_ELEMENT_LIGHT			0
#define S_ELEMENT_FIRE			1
#define S_ELEMENT_WIND			2
#define S_ELEMENT_ICE			3
#define S_ELEMENT_DARK			4
#define S_ELEMENT_ELECTRIC		5
#define S_ELEMENT_NUM				6

#define LIMIT_ROUND_TIME 80.f

	

enum SERVER_GAME_STATE
{
	STATE_READY = 0,
	STATE_ROUND_ENTER,
	STATE_ROUND_START,
	STATE_ROUND_CHANGE,
	STATE_ROUND_END,
	STATE_ROUND_CLEAR,
	STATE_GAME_END,
	STATE_TOTAL_NUM
};
#pragma pack (push, 1)
struct cs_packet_rotate
{
	BYTE size;
	BYTE type;
	float cxDelta;
	float cyDelta;
	XMFLOAT3 LookVector;
};
struct cs_packet_up {
	BYTE size;
	BYTE type;
};

struct cs_packet_down {
	BYTE size;
	BYTE type;
};

struct cs_packet_left {
	BYTE size;
	BYTE type;
};

struct cs_packet_right {
	BYTE size;
	BYTE type;
};

struct cs_packet_chat {
	BYTE size;
	BYTE type;
//	WCHAR message[100];
};

struct cs_packet_animation {
	BYTE size;
	BYTE type;
	BYTE animation_Type;
};

struct cs_packet_height {
	BYTE size;
	BYTE type;
	float x;
	float z;
};
struct cs_packet_move_test
{
	BYTE size;
	BYTE type;
	BYTE animation;
	DWORD direction;
	XMFLOAT3 LookVector;
	XMFLOAT3 Position;
}; 
struct cs_packet_dominate
{
	BYTE size;
	BYTE type;
};
struct cs_packet_Behavior
{
	BYTE size;
	BYTE type;
};
struct cs_packet_ready {
	BYTE size;
	BYTE type;
	BYTE id;
	bool ready;
};
struct cs_packet_damage
{
	BYTE size;
	BYTE type;
	BYTE id;
	short damage;
};
struct cs_packet_Info
{
	BYTE size;
	BYTE type;
	BYTE id;
	BYTE infoType;
	short HP;
	short Item;
	short element;
};
struct cs_packet_state {
	BYTE size;
	BYTE type;
	WORD id;
	XMFLOAT3 Position;
	XMFLOAT3 LookVector;
	XMFLOAT3 RightVector;
};
struct cs_packet_vector
{
	BYTE size;
	BYTE type;
	XMFLOAT3 LookVector;
	XMFLOAT3 RightVector;
};
struct cs_packet_move {
	BYTE size;
	BYTE type;
	BYTE mode;
	BYTE animation;
	XMFLOAT3 MoveDirection;
	XMFLOAT3 LookDirection;
	XMFLOAT3 Velocity;
};

struct cs_packet_serverCheat
{
	BYTE size;
	BYTE type;
	short mode;
	short roundTime;
};
struct sc_packet_rotate
{
	BYTE size;
	BYTE type;
	WORD id;
	float cxDelta;
	float cyDelta;
	XMFLOAT3 LookVector;
};
struct sc_packet_dominate
{
	BYTE size;
	BYTE type;
	WORD id;

};
struct sc_packet_Behavior
{
	BYTE size;
	BYTE type;
	WORD id;

};
struct sc_packet_pos {
	BYTE size;
	BYTE type;
	WORD id;
	DWORD animationType;
	DWORD dwDirection;
	XMFLOAT3 Position;
	XMFLOAT3 LookVector;
};
struct sc_packet_height {
	BYTE size;
	BYTE type;
	int mapWidth;
	int mapHeight;
	float fHeight[1000];
};
struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	WORD id;
	WORD x;
	WORD y;
	WORD z;
	short HP;
	short numberOfElement[S_ELEMENT_NUM];
};
//struct sc_packet_Init_player {
//	BYTE size;
//	BYTE type;
//	WORD id;
//	float x;
//	float y;
//	float z;
//};

struct sc_packet_Init_player {
	BYTE size;
	BYTE type;
	WORD id;
	XMFLOAT3 Position;
	XMFLOAT3 LookVector;
	XMFLOAT3 RightVector;
	short HP;
};
struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	WORD id;
};

struct sc_packet_chat {
	BYTE size;
	BYTE type;
	WORD id;
//	WCHAR message[100];
};
struct sc_packet_animation_player {
	BYTE size;
	BYTE type;
	WORD id;
	BYTE animation_Type;
};
struct sc_packet_readyID
{
	BYTE size;
	BYTE id;
};

struct sc_packet_RoundTime
{
	BYTE size;
	BYTE type;
	BYTE id;
	unsigned short round;
	unsigned short time;

};
struct sc_packet_GameState
{
	BYTE size;
	BYTE type;
	BYTE id;
	short gamestate;
	short round;
};
struct sc_packet_playerInfo
{
	BYTE size;
	BYTE type;
	BYTE id;
	short point;
	short kill;
	short death;
	short HP;
	short numberOfElement[S_ELEMENT_NUM];
	short Item;

};

#pragma pack (pop)