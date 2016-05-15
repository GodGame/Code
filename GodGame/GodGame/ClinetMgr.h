#pragma once

//#include "PacketMgr.h"

#define		SERVERPORT	9000
#define		BUFF_SIZE 1024
class CPlayer;
class CPlayerShader;
class CScene;
class ClientMgr
{
private:
	char mSendBuffer[BUFF_SIZE];
	char mRecvBuffer[BUFF_SIZE];
	char mPacketBuffer[BUFF_SIZE];
	UCHAR umSendBuffer[BUFF_SIZE];
	WSABUF mSendWsaBuffer;
	WSABUF mRecvWsaBuffer;

	DWORD mInPacketSize;
	int  mSavedPacketSize;
	int  mId;
	bool mbConnecting : 1;

	SOCKET              mSock;
	CPlayer *			m_pPlayer;
	CPlayerShader *		m_pPlayerShader;
	CScene  *			m_pScene;

	HWND				mhWnd;

private:
	ClientMgr();
	~ClientMgr();
	int m_nRoundTime;
	int m_nRoundNum;
	const float mfENTER_TIME = 5.5f;
	const float mfEND_TIME = 5.f;
public:
	static ClientMgr & GetInstance() { static ClientMgr instance; return instance; }
	int GetRoundTime() { return m_nRoundTime; }
	int GetRoundNum() { return m_nRoundNum; }
	bool Connect(HWND hWnd, int iServerPort);
	bool Setting(HWND hWnd, int iServerPort);
	void SetAsyncSelect();

	void CloseConnect();

	void InsertKey(WPARAM wParam);
	void ProcessPacket(char * ptr);
	void ReadPacket();
	void SendPacket(unsigned char* packet);
	void SendPacket(unsigned char * packet, ULONG len);
public:
	void SetScene(CScene * pScene) { m_pScene = pScene; }
	void SetPlayerShader(CPlayerShader * pShader) { m_pPlayerShader = pShader; }
	char * GetRecvBuffer() { return mRecvBuffer; }
	char * GetSendBuffer() { return mSendBuffer; }
	WSABUF& GetWSARecvBuffer() { return mRecvWsaBuffer; }
	WSABUF& GetWSASendBuffer() { return mSendWsaBuffer; }
	SOCKET GetClientSocket() { return mSock; }
	UCHAR* GetUSendBuffer() { return umSendBuffer; }
	int GetClientID() { return mId; }
public:
	void error_display(char * msg, int err_no);
};
#define CLIENT ClientMgr::GetInstance()

