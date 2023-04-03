#include <stdafx.h>

#include "RemoteDispatcher.h"

void ErrorOutWithMsg(const std::string &&szMsg);

RemoteDispatcher::RemoteDispatcher() : Component()
{
	m_hPipeHandle =
	    CreateNamedPipe(L"\\\\.\\pipe\\ChaosModVEffectDispatcherPipe", PIPE_ACCESS_INBOUND,
	                                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT, 1, 0, 256, 0, NULL);

	if (m_hPipeHandle == INVALID_HANDLE_VALUE)
	{
		ErrorOutWithMsg("Error while creating a named pipe. Try reloading the mod.");

		return;
	}
}

RemoteDispatcher::~RemoteDispatcher()
{
	OnModPauseCleanup();
}

void RemoteDispatcher::OnModPauseCleanup()
{
	if (m_hPipeHandle != INVALID_HANDLE_VALUE)
	{
		DisconnectNamedPipe(m_hPipeHandle);
		CloseHandle(m_hPipeHandle);

		m_hPipeHandle = INVALID_HANDLE_VALUE;
	}
}


void RemoteDispatcher::OnRun()
{
	if (!ConnectNamedPipe(m_hPipeHandle, NULL))
	{
		switch (GetLastError())
		{
		case ERROR_PIPE_CONNECTED:
			break;
		case ERROR_PIPE_LISTENING:
			return;
		case ERROR_NO_DATA:
			LOG("Pipe disconnected");
			DisconnectNamedPipe(m_hPipeHandle);
			return;
		default:
			return;
		}
	}

	char cBuffer[256];
	DWORD ulBytesRead;
	if (!ReadFile(m_hPipeHandle, cBuffer, sizeof(cBuffer) - 1, &ulBytesRead, NULL))
	{
		while (GetLastError() == ERROR_IO_PENDING)
		{
			WAIT(0);
		}
	}

	if (ulBytesRead > 0)
	{
		cBuffer[ulBytesRead] = '\0';

		LOG("Recieved message: " << cBuffer);

		GetComponent<EffectDispatcher>()->DispatchEffect(std::string(cBuffer));
	}
}

void ErrorOutWithMsg(const std::string &&szMsg)
{
	std::wstring wStr = { szMsg.begin(), szMsg.end() };
	MessageBox(NULL, wStr.c_str(), L"ChaosModV Error", MB_OK | MB_ICONERROR);
}