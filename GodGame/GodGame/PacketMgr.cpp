#include "stdafx.h"
#include "PacketMgr.h"

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
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, //��ǻ�Ͱ� �˾Ƽ� �ּҰ� ã���ش�. �Ϲ������� ����
		NULL,			// �ּҰ��� �ڵ����� ã���ֹǷ�
		WSAGetLastError(),	//���� �ڵ带 ã���ش�. �ڵ��ȣ �Է��ϴ� ��.
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	//��� ����
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
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, //��ǻ�Ͱ� �˾Ƽ� �ּҰ� ã���ش�. �Ϲ������� ����
		NULL,			// �ּҰ��� �ڵ����� ã���ֹǷ�
		WSAGetLastError(),	//���� �ڵ带 ã���ش�. �ڵ��ȣ �Է��ϴ� ��.
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	//��� ����
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
	wprintf(L"����%s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
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
	if (m_retval == SOCKET_ERROR)
	{
		err_display("WSASend()");
		return 0;
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

