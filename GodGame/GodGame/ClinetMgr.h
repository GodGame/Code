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

public:
	static ClientMgr & GetInstance() { static ClientMgr instance; return instance; }

	bool Connect(HWND hWnd, int iServerPort);
	bool Setting(HWND hWnd, int iServerPort);
	void SetAsyncSelect();

	void CloseConnect();

	void InsertKey(WPARAM wParam);
	void ProcessPacket(char * ptr);
	void ReadPacket();

public:
	void SetScene(CScene * pScene) { m_pScene = pScene; }
	void SetPlayerShader(CPlayerShader * pShader) { m_pPlayerShader = pShader; }
	char * GetRecvBuffer() { return mRecvBuffer; }

};
#define CLIENT ClientMgr::GetInstance()

