#include "stdafx.h"
#include "PacketMgr.h"
#include "Player.h"

CPacketMgr::CPacketMgr()
{
	m_vcRecvBuffer.reserve(10);
	m_vcSendBuffer.reserve(10);

	m_dwBytes = 0;
	m_dwSendFlags = 0;
	m_dwRecvFlags = 0;
}

CPacketMgr::~CPacketMgr()
{
	m_vcRecvBuffer.clear();
	m_vcSendBuffer.clear();
}

void CPacketMgr::err_quit(wchar_t * msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, //컴퓨터가 알아서 주소값 찾아준다. 일반적으로 쓰임
		NULL,			// 주소값을 자동으로 찾아주므로
		WSAGetLastError(),	//에러 코드를 찾아준다. 코드번호 입력하는 곳.
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	//언어 설정
		(LPTSTR)&lpMsgBuf, 0, NULL
		);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void CPacketMgr::err_display(char * msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, //컴퓨터가 알아서 주소값 찾아준다. 일반적으로 쓰임
		NULL,			// 주소값을 자동으로 찾아주므로
		WSAGetLastError(),	//에러 코드를 찾아준다. 코드번호 입력하는 곳.
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	//언어 설정
		(LPTSTR)&lpMsgBuf, 0, NULL
		);

	printf("[%s] %ws", msg, (wchar_t*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void CPacketMgr::error_display(char * msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg);
	wprintf(L"에러%s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void CPacketMgr::SendInputPacket(CPlayer * pPlayer, int id)
{
	static cs_packet_state info_packet;
	info_packet.id = id;
	info_packet.size = sizeof(cs_packet_state);
	info_packet.type = CS_INPUT;
	info_packet.LookVector = pPlayer->GetLookVector();
	info_packet.RightVector = pPlayer->GetRightVector();

	PACKET_MGR.Send(CLIENT.GetClientSocket(), reinterpret_cast<CHAR*>(&info_packet), info_packet.size);
}

void CPacketMgr::SendRotatePacket(CPlayer * pPlayer)
{
	static cs_packet_rotate rotate_packet;
	rotate_packet.size = sizeof(cs_packet_rotate);
	rotate_packet.type = CS_ROTATION;
	rotate_packet.cxDelta = 0;
	rotate_packet.cyDelta = 0;
	rotate_packet.LookVector = pPlayer->GetLookVector();

	PACKET_MGR.Send(CLIENT.GetClientSocket(), reinterpret_cast<CHAR*>(&rotate_packet), rotate_packet.size);
}

void CPacketMgr::SendPositionPacket(CPlayer * pPlayer, const DWORD & direction)
{
	static cs_packet_move_test movePacket;
	movePacket.size = sizeof(cs_packet_move_test);
	movePacket.type = CS_MOVE;
	movePacket.direction = direction;
	movePacket.Position = pPlayer->GetPosition();
	movePacket.LookVector = pPlayer->GetLookVector();
	movePacket.animation = pPlayer->GetAnimationState();

	PACKET_MGR.Send(CLIENT.GetClientSocket(), reinterpret_cast<CHAR*>(&movePacket), movePacket.size);
}

void CPacketMgr::SendBehaviorPacket()
{
	static cs_packet_Behavior behavior_packet;
	behavior_packet.size = sizeof(cs_packet_Behavior);
	behavior_packet.type = CS_MAGIC_CASTING;

	PACKET_MGR.Send(CLIENT.GetClientSocket(), reinterpret_cast<CHAR*>(&behavior_packet), behavior_packet.size);
}

void CPacketMgr::SendDominatePacket()
{
	static cs_packet_dominate dominate_packet;
	dominate_packet.size = sizeof(cs_packet_dominate);
	dominate_packet.type = CS_DOMINATE;

	PACKET_MGR.Send(CLIENT.GetClientSocket(), reinterpret_cast<CHAR*>(&dominate_packet), dominate_packet.size);
}

void CPacketMgr::PushSendDataPtr(char * pData, ULONG len)
{
	WSABUF tempBuf;
	tempBuf.buf = pData;
	tempBuf.len = len;

	m_vcSendBuffer.push_back(tempBuf);

	//cout << "Send Buffer Size : " << m_vcSendBuffer.size() << endl;
}

void CPacketMgr::PopSendDataPtr()
{
	if (!m_vcSendBuffer.empty())
		m_vcSendBuffer.pop_back();
}

void CPacketMgr::UpdateSendDataPtr(char * data, ULONG len, WORD wdSlot)
{
	m_vcSendBuffer[wdSlot].buf = data;
	m_vcSendBuffer[wdSlot].len = len;
}

DWORD CPacketMgr::Send(SOCKET sock, char * data, ULONG len)
{
	WSABUF buf;
	buf.buf = data;
	buf.len = len;

	m_retval = WSASend(sock, &buf, 1, &m_dwBytes, m_dwSendFlags, nullptr, nullptr);
	if (0 != m_retval)
	{
		static int error_no = 0;
		error_no = WSAGetLastError();
		if (WSA_IO_PENDING != error_no)
		{
			error_display(__FUNCTION__ "SendPacket:WSASend", error_no);
		}
	}
	return m_dwBytes;
}

DWORD CPacketMgr::PreparedSend(SOCKET sock, int offset, UINT nBufferLen)
{
	m_retval = WSASend(sock, &m_vcSendBuffer[offset], nBufferLen, &m_dwBytes, m_dwSendFlags, nullptr, nullptr);
	if (m_retval == SOCKET_ERROR)
	{
		err_display("WSASend()");
		return 0;
	}
	return m_dwBytes;
}

void CPacketMgr::PushRecvDataPtr(char * pData, ULONG len)
{
	WSABUF tempBuf;
	tempBuf.buf = pData;
	tempBuf.len = len;

	m_vcRecvBuffer.push_back(tempBuf);
}

void CPacketMgr::PopRecvDataPtr()
{
	if (!m_vcRecvBuffer.empty())
		m_vcRecvBuffer.pop_back();
}

void CPacketMgr::UpdateRecvDataPtr(char * data, ULONG len, WORD wdSlot)
{
	m_vcRecvBuffer[wdSlot].buf = data;
	m_vcRecvBuffer[wdSlot].len = len;
}

DWORD CPacketMgr::Recv(SOCKET sock, char * data, ULONG len)
{
	WSABUF tempBuf;
	tempBuf.buf = data;
	tempBuf.len = len;

	m_retval = WSARecv(sock, &tempBuf, 1, &m_dwBytes, &m_dwRecvFlags, nullptr, nullptr);
	if (m_retval == SOCKET_ERROR)
	{
		err_display("WSARecv()");
		return 0;
	}
	return m_dwBytes;
}

DWORD CPacketMgr::PreparedRecv(SOCKET sock, int offset, UINT nBufferLen)
{
	m_retval = WSARecv(sock, &m_vcRecvBuffer[offset], nBufferLen, &m_dwBytes, &m_dwSendFlags, nullptr, nullptr);
	if (m_retval == SOCKET_ERROR)
	{
		err_display("WSARecv()");
		return 0;
	}
	return m_dwBytes;
}

