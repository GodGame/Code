#pragma once

#ifndef _PACKET_MGR
#define _PACKET_MGR

#include "stdafx.h"
//#include "../protocol.h"

class CPacketMgr
{
private:
	vector<WSABUF> m_vcSendBuffer;
	vector<WSABUF> m_vcRecvBuffer;

	DWORD  m_dwBytes;
	DWORD  m_dwSendFlags;
	DWORD  m_dwRecvFlags;

	int    m_retval;

private:
	CPacketMgr();
	~CPacketMgr();

public:
	static CPacketMgr & GetInstance() { static CPacketMgr instance; return instance; }
	static void err_quit(wchar_t * msg);
	static void err_display(char * msg);
	static void error_display(char *msg, int err_no);

public:
	void SetSendFlag(DWORD dwFlag) { m_dwSendFlags = dwFlag; }
	void SetRecvFlag(DWORD dwFlag) { m_dwRecvFlags = dwFlag; }

	LPWSABUF GetSendBuffer() { return &m_vcSendBuffer[0]; }
	LPWSABUF GetRecvBuffer() { return &m_vcRecvBuffer[0]; }

	vector<WSABUF> GetSendBufferVector() { return m_vcSendBuffer; }
	vector<WSABUF> GetRecvBufferVector() { return m_vcRecvBuffer; }

public:	// send
	void PushSendDataPtr(char * data, ULONG len);
	void PopSendDataPtr();
	void UpdateSendDataPtr(char * data, ULONG len, WORD wdSlot = 0);

	DWORD Send(SOCKET sock, char * data, ULONG len);
	DWORD PreparedSend(SOCKET sock, int offset, UINT nBufferLen);
	DWORD PreparedSend(SOCKET sock, int offset) { return PreparedSend(sock, offset, m_vcSendBuffer.size()); }

public:	// recv
	void PushRecvDataPtr(char * data, ULONG len);
	void PopRecvDataPtr();
	void UpdateRecvDataPtr(char * data, ULONG len, WORD wdSlot = 0);

	DWORD Recv(SOCKET sock, char * data, ULONG len);
	DWORD PreparedRecv(SOCKET sock, int offset, UINT nBufferLen);
	DWORD PreparedRecv(SOCKET sock, int offset) { return PreparedRecv(sock, offset, m_vcRecvBuffer.size()); }
};
#define ERR_QUIT(str) CPacketMgr::err_quit(str);
#define ERR_PRINT(str) CPacketMgr::err_display(str);
#define ERROR_PRINT(str, errNo) CPacketMgr::error_display(str, errNo);

#define PACKET_MGR CPacketMgr::GetInstance()

#endif