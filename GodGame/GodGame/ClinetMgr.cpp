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
		//	int id = 0;//my_packet->id;
		int id = my_packet->id;
		if (bFirstTime) {
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
			//
			 //CInGamePlayer::InitPosition(XMFLOAT3(pTerrain->GetWidth()*0.5f + (rand() % 40 - 20), pTerrain->GetPeakHeight() + 10.0f, 300 + (rand() % 40 - 20)));
			 //pPlayer->SetPosition(XMFLOAT3(my_packet->x, my_packet->y, my_packet->z));
			 //pPlayer->SetPosition();
			pPlayer->SetActive(true);
			pPlayer->SetVisible(true);
			//pPlayer->GetStatus().ChangeHP(my_packet->HP);
			pPlayer->GetStatus().SetHP(my_packet->HP);

		/*	cs_packet_state* info_packet = reinterpret_cast<cs_packet_state*>(umSendBuffer);
			info_packet->size = sizeof(info_packet);
			mSendWsaBuffer.buf = reinterpret_cast<CHAR*>(umSendBuffer);
			mSendWsaBuffer.len = sizeof(info_packet);
			memcpy(umSendBuffer, reinterpret_cast<UCHAR*>(info_packet), sizeof(info_packet));
			DWORD ioBytes;
			int ret = WSASend(mSock, &mSendWsaBuffer, 1, &ioBytes, 0, NULL, NULL);
			if (ret)
			{
				int error_code = WSAGetLastError();
				error_display(__FUNCTION__ " SC_PUT_PLAYER:WSASend", error_code);
			}*/
			
			cs_packet_state info_packet;
			info_packet.id = mId;
			info_packet.size = sizeof(cs_packet_state);
			info_packet.type = CS_INPUT;
			info_packet.LookVector = pPlayer->GetLookVector();
			info_packet.RightVector = pPlayer->GetRightVector();
			//SendPacket(reinterpret_cast<unsigned  char*>(&info_packet),sizeof(cs_packet_state));
			mSendWsaBuffer.buf = reinterpret_cast<CHAR*>(&umSendBuffer);
			mSendWsaBuffer.len = sizeof(cs_packet_state);
			memcpy(umSendBuffer, reinterpret_cast<UCHAR*>(&info_packet), sizeof(cs_packet_state));
			DWORD ioBytes;
			int ret = WSASend(mSock, &mSendWsaBuffer, 1, &ioBytes, 0, NULL, NULL);
			if (ret)
			{
				int error_code = WSAGetLastError();
				if (WSA_IO_PENDING != error_code)
				{
					error_display(__FUNCTION__ " SC_PUT_PLAYER:WSASend", error_code);
				}
			}
			//CLIENT.SendPacket(reinterpret_cast<unsigned  char*>(&info_packet));
		}
		else if (id != mId)
		{
			//	m_pPlayerShader->SetPlayerID(FRAMEWORK.GetDevice(), id);
			//	m_pPlayerShader->GetPlayer(mId);
			//pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(id));
			//	pPlayer->SetPlayerNum(id);
		//pPlayer->InitPosition(XMFLOAT3(my_packet->x, my_packet->y, my_packet->z));
			//m_pPlayerShader->SetPlayerID(FRAMEWORK.GetDevice(), id);
			//m_pPlayerShader->GetPlayer(id);
			//pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(id));
			//pPlayer->SetPlayerNum(id);
			//pPlayer->SetPosition(XMFLOAT3(my_packet->x, my_packet->y, my_packet->z));
			////pPlayer->SetPosition();
			//pPlayer->SetActive(true);
			//pPlayer->SetActive(true);
		}
		break;
	}
	case SC_OBJECT_INIT:
	{
		//	cout << __FUNCTION__"case: SC_OBJECT_INIT" << endl;
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

void ClientMgr::SendPacket(unsigned char * packet)
{
	WSABUF buf;
	buf.buf = reinterpret_cast<CHAR*>(packet);
	buf.len = packet[0];
//	_wsetlocale(LC_ALL, L"korean");
	int res = WSASend(mSock, &buf, 1, NULL, 0, NULL, NULL);
	if (0 != res) // WSA_IO_PENDING은 IOCP에서 미처 다 보내지 못했을때 나오는 에러
	{
		int error_no = WSAGetLastError();
		if (WSA_IO_PENDING != error_no)
		{
			error_display(__FUNCTION__ "SendPacket:WSASend", error_no);
			//	cout << __FUNCTION__ "SendPacket:WSASend" << endl;;
		}
		//	error_display("SendPacket:WSASend", error_no);
		//while (true);
	}
	//ZeroMemory(&buf, sizeof(buf));
}
//
//void ClientMgr::SendPacket(unsigned char * packet)
//{
//	//WSABUF buf;
//	//buf.buf = reinterpret_cast<CHAR*>(mSendBuffer);
//	//buf.len = packet[0];
//	//memcpy(mSendBuffer, packet, packet[0]);
//	ZeroMemory(umSendBuffer, sizeof(umSendBuffer));
//	WSABUF buf;
//	buf.buf = reinterpret_cast<CHAR*>(&umSendBuffer);
//	buf.len = packet[0];
//	memcpy(umSendBuffer, reinterpret_cast<UCHAR*>(packet), packet[0]);
//	//_wsetlocale(LC_ALL, L"korean");
//	int res = WSASend(mSock, &buf, 1, NULL, 0, NULL, NULL);
//	if (0 != res) // WSA_IO_PENDING은 IOCP에서 미처 다 보내지 못했을때 나오는 에러
//	{
//		int error_no = WSAGetLastError();
//		if (WSA_IO_PENDING != error_no)
//		{
//			error_display(__FUNCTION__ "SendPacket:WSASend", error_no);
//			//	cout << __FUNCTION__ "SendPacket:WSASend" << endl;;
//		}
//		//	error_display("SendPacket:WSASend", error_no);
//		//while (true);
//	}
////	ZeroMemory(&buf, sizeof(buf));
//}
void ClientMgr::SendPacket( unsigned char * packet, ULONG len)
{
	ZeroMemory(umSendBuffer, sizeof(umSendBuffer));
	//WSABUF buf;
	mSendWsaBuffer.buf = reinterpret_cast<CHAR*>(&umSendBuffer);
	mSendWsaBuffer.len = len;
	memcpy(umSendBuffer,&packet, len);
	int res = WSASend(mSock, &mSendWsaBuffer, 1, NULL, 0, NULL, NULL);
	if (0 != res) // WSA_IO_PENDING은 IOCP에서 미처 다 보내지 못했을때 나오는 에러
	{
		int error_no = WSAGetLastError();
		if (WSA_IO_PENDING != error_no)
		{
			error_display(__FUNCTION__ "SendPacket:WSASend", error_no);
		}
	}
}
void ClientMgr::error_display(char * msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg);
	wprintf(L"에러 : %ws\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}
