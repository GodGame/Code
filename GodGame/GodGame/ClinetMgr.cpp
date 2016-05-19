#include "stdafx.h"
#include "PacketMgr.h"
#include "ClinetMgr.h"
#include "SceneInGame.h"
#include "GameFramework.h"
#include "Protocol.h"
//#include "../protocol.h"

ClientMgr::ClientMgr()
{
	ZeroMemory(mSendBuffer, sizeof(char) * BUFF_SIZE);
	ZeroMemory(mRecvBuffer, sizeof(char) * BUFF_SIZE);
	ZeroMemory(mPacketBuffer, sizeof(char) * BUFF_SIZE);
	mSendWsaBuffer.buf = mSendBuffer;
	mSendWsaBuffer.len = BUFF_SIZE;
	mRecvWsaBuffer.buf = mRecvBuffer;
	mRecvWsaBuffer.len = BUFF_SIZE;

	mInPacketSize = 0;
	mSavedPacketSize = 0;
	mId = 0;

	m_pPlayer = nullptr;
	m_pPlayerShader = nullptr;
	m_pScene = nullptr;
	m_nRoundNum = 1;
	mSock = NULL;
	mbConnecting = false;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		CPacketMgr::err_quit(_T("WsaStart"));

	_wsetlocale(LC_ALL, L"korean");
}


ClientMgr::~ClientMgr()
{
	if (mSock) closesocket(mSock);

	WSACleanup();
}

bool ClientMgr::Connect(HWND hWnd, int iServerPort)
{
	char cIpAddress[32];
	printf("서버 IP주소를 입력하세요. : ");
	fgets(cIpAddress, sizeof(cIpAddress), stdin);

	// connect
	int iPortAddress = iServerPort;
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(cIpAddress);
	serveraddr.sin_port = htons(iPortAddress);
	int retval = WSAConnect(mSock, reinterpret_cast<SOCKADDR *>(&serveraddr), sizeof(serveraddr), NULL, NULL, NULL, NULL);
	if (retval == SOCKET_ERROR)
	{
		CPacketMgr::err_quit(_T("connect()"));
		return false;
	}
	mhWnd = hWnd;
	//SendInitialPacket();
	//RecvInitialPacket();

	PACKET_MGR.PushSendDataPtr(reinterpret_cast<char*>(mSendBuffer), sizeof(mSendBuffer));
	PACKET_MGR.PushRecvDataPtr(reinterpret_cast<char*>(mRecvBuffer), sizeof(mRecvBuffer));

	return mbConnecting = true;
}

bool ClientMgr::Setting(HWND hWnd, int iServerPort)
{
	mSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (mSock == INVALID_SOCKET)
	{
		CPacketMgr::err_quit(_T("socket()"));
		return false;
	}
	return ClientMgr::Connect(hWnd, iServerPort);
}

void ClientMgr::SetAsyncSelect()
{
	WSAAsyncSelect(mSock, mhWnd, WM_SOCKET, FD_CLOSE | FD_READ);
}

void ClientMgr::CloseConnect()
{
	if (mSock) closesocket(mSock);
	mSock = NULL;
	mbConnecting = false;
}

void ClientMgr::InsertKey(WPARAM wParam)
{
#if 0
	int x = 0, y = 0;
	if (wParam == VK_RIGHT)	x += 1;
	if (wParam == VK_LEFT)	x -= 1;
	if (wParam == VK_UP)	y -= 1;
	if (wParam == VK_DOWN)	y += 1;
	cs_packet_up * my_packet = reinterpret_cast<cs_packet_up *>(mSendBuffer);
	my_packet->size = sizeof(cs_packet_up);

	DWORD iobyte;
	if (0 != x)
	{
		if (1 == x) my_packet->type = CS_RIGHT;
		else my_packet->type = CS_LEFT;

		iobyte = PACKET_MGR.Send(mSock, reinterpret_cast<char*>(my_packet), sizeof(cs_packet_up));
		// << "Send Size : " << iobyte << endl;
	}
	if (0 != y)
	{
		if (1 == y) my_packet->type = CS_DOWN;
		else my_packet->type = CS_UP;

		iobyte = PACKET_MGR.Send(mSock, reinterpret_cast<char*>(my_packet), sizeof(cs_packet_up));
	}
	cout << iobyte << " Insert Key" << endl;
#endif
}

void ClientMgr::ProcessPacket(char * ptr)
{
	//cout << "Process Packet!! " << endl;
	CInGamePlayer * pPlayer = static_cast<CInGamePlayer*>(m_pPlayer);

	static bool bFirstTime = true;
	switch (ptr[1])
	{
	case SC_PUT_PLAYER:
	{
		sc_packet_put_player * my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
		int id = my_packet->id;
		if (bFirstTime) 
		{
			bFirstTime = false;
			mId = id;
		}
		if (id == mId)
		{
			m_pPlayerShader->SetPlayerID(FRAMEWORK.GetDevice(), mId);
			m_pPlayerShader->GetPlayer(mId);
			pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(mId));
			pPlayer->SetPlayerNum(mId);
			pPlayer->InitPosition(XMFLOAT3(my_packet->x, my_packet->y, my_packet->z));
			pPlayer->UpdateBoundingBox();
			cout << "Player " << mId << pPlayer->GetPosition() << endl;

			pPlayer->SetActive(true);
			pPlayer->SetVisible(true);
			pPlayer->GetStatus().SetHP(my_packet->HP);

			PACKET_MGR.SendInputPacket(pPlayer, mId);
		}
		else if (id != mId)
		{

		}
		break;
	}
	case SC_OBJECT_INIT:
	{
		sc_packet_Init_player * my_packet = reinterpret_cast<sc_packet_Init_player *>(ptr);
		int id = 0;
		id = my_packet->id;
		if (bFirstTime)
		{
			bFirstTime = false;
			mId = id;
			//m_vxPlayerPosition[mId] = XMFLOAT3(my_packet->x, my_packet->y, my_packet->z);
		}
		break;
	}
	case SC_POS:
	{
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int other_id = my_packet->id;
		if (other_id == mId) {

		}
		else
		{

		}

		break;
	}
	case SC_ROUND_TIME:
	{
		sc_packet_RoundTime *my_packet = reinterpret_cast<sc_packet_RoundTime *>(ptr);
		SYSTEMMgr.SetRoundTime(my_packet->time);
		SYSTEMMgr.SetRoundNUm(my_packet->round);
		m_nRoundTime = my_packet->time;
		m_nRoundNum = my_packet->round;
		//cout << "count: " << my_packet->round << endl;

		break;
	}
	case SC_GAME_STATE:
	{
		sc_packet_GameState *my_packet = reinterpret_cast<sc_packet_GameState *>(ptr);
		switch (my_packet->gamestate)
		{
		case STATE_READY://STATE_READY
			cout << "STATE_READY " << endl;
			//EVENTMgr.InsertDelayMessage(0.f, eMessage::MSG_ROUND_START, CGameEventMgr::MSG_TYPE_SCENE, SYSTEMMgr.GetNowScene());
			break;
		case STATE_ROUND_ENTER:
			EVENTMgr.InsertDelayMessage(0.f, eMessage::MSG_ROUND_ENTER, CGameEventMgr::MSG_TYPE_SCENE, SYSTEMMgr.GetNowScene());
			break;
		case STATE_ROUND_START://STATE_ROUND_START
			cout << "STATE_ROUND_START: " << endl;
			EVENTMgr.InsertDelayMessage(0.f, eMessage::MSG_ROUND_START, CGameEventMgr::MSG_TYPE_SCENE, SYSTEMMgr.GetNowScene());
			break;
		case STATE_ROUND_CHANGE:
			break;
		case STATE_ROUND_END:
			cout << "STATE_ROUND_END: " << endl;
			EVENTMgr.InsertDelayMessage(0.f, eMessage::MSG_ROUND_END, CGameEventMgr::MSG_TYPE_SCENE, SYSTEMMgr.GetNowScene());
			break;
		case STATE_ROUND_CLEAR:
			cout << "STATE_ROUND_CLEAR: " << endl;
			EVENTMgr.InsertDelayMessage(1.f, eMessage::MSG_ROUND_CLEAR, CGameEventMgr::MSG_TYPE_SCENE, SYSTEMMgr.GetNowScene());
			break;
		case STATE_GAME_END:
			cout << "STATE_GAME_END: " << endl;
			EVENTMgr.InsertDelayMessage(0.f, eMessage::MSG_GAME_END, CGameEventMgr::MSG_TYPE_SCENE, SYSTEMMgr.GetNowScene());
			break;
		case STATE_TOTAL_NUM:
			break;
		default:
			break;
		}
		//my_packet->gamestate;
		//m_nRoundTime = my_packet->time;
		//m_nRoundNum = my_packet->round;
		//cout << "count: " << my_packet->round << endl;
		break;
	}
	case SC_PLAYER_INFO:
	case SC_ROTATION:
	case SC_DOMINATE:
	case SC_MAGIC_CASTING:
	case SC_ANI_IDLE:
		break;
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void ClientMgr::ReadPacket()
{
	if (mbConnecting == false) return;

	DWORD iobyte = 0, ioflag = 0;
	//	iobyte = PACKET_MGR.Recv(mSock, mRecvBuffer, BUFF_SIZE);
	int ret = WSARecv(mSock, &mRecvWsaBuffer, 1, &iobyte, &ioflag, NULL, NULL);
	BYTE *ptr = reinterpret_cast<BYTE *>(mRecvBuffer);

	while (0 != iobyte)
	{
		if (0 == mInPacketSize)
			mInPacketSize = ptr[0];

		if (iobyte + mSavedPacketSize >= mInPacketSize)
		{
			memcpy(mPacketBuffer + mSavedPacketSize, ptr, mInPacketSize - mSavedPacketSize);

			ProcessPacket(mPacketBuffer);

			ptr += mInPacketSize - mSavedPacketSize;
			iobyte -= mInPacketSize - mSavedPacketSize;

			mInPacketSize = 0;
			mSavedPacketSize = 0;
		}
		else
		{
			memcpy(mPacketBuffer + mSavedPacketSize, ptr, iobyte);
			mSavedPacketSize += iobyte;
			iobyte = 0;
		}
	}
}

void ClientMgr::SendPacket(char * packet)
{
	PACKET_MGR.Send(mSock, packet, packet[0]);
}

void ClientMgr::SendPacket(char * packet, ULONG len)
{
	PACKET_MGR.Send(mSock, packet, len);
}
