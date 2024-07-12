#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <vector>  
#include <string>  
#include <atlbase.h>
#include <atlstr.h>
#include <ShellApi.h>
//#include <resource.h>
using namespace std;

HANDLE hCom[20] = {0}; //全局变量，串口句柄
//#define CMAX	20
#define MYWM_NOTIFYICON WM_USER+1

int opencom(char number[], int bps ,int par, int keyboardnum)
{
	hCom[keyboardnum] = CreateFile(number,//COM口
		GENERIC_READ | GENERIC_WRITE, //允许读和写
		0, //独占方式
		NULL,
		OPEN_EXISTING, //打开而不是创建
		0, //同步方式
		NULL);
	if (hCom[keyboardnum] == (HANDLE)-1)
	{
		return FALSE;
	}

	SetupComm(hCom[keyboardnum], 1024, 1024); //输入缓冲区和输出缓冲区的大小都是1024
	COMMTIMEOUTS TimeOuts; //设定读超时
	TimeOuts.ReadIntervalTimeout = 1000;
	TimeOuts.ReadTotalTimeoutMultiplier = 50;
	TimeOuts.ReadTotalTimeoutConstant = 20; //设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	SetCommTimeouts(hCom[keyboardnum], &TimeOuts); //设置超时
	DCB dcb;
	GetCommState(hCom[keyboardnum], &dcb);
	dcb.BaudRate = bps; //波特率
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_ENABLE;
	dcb.ByteSize = 8; //每个字节有8位
	dcb.Parity = par; //无奇偶校验位
	dcb.StopBits = ONESTOPBIT; //1个停止位
	SetCommState(hCom[keyboardnum], &dcb);
	PurgeComm(hCom[keyboardnum], PURGE_TXCLEAR | PURGE_RXCLEAR);
	//puts("o");
	return TRUE;
}

struct readTem {
	int readNum;
	char *str;
	int isWork;
};
readTem readcom(int comNum/*, char *cstr*/)
{
	readTem ret;
	char rStr[255];
	DWORD wCount; //读取的字节数
	BOOL bReadStat;
	bReadStat = ReadFile(hCom[comNum], rStr, 255, &wCount, NULL);
	if (!bReadStat)
		ret.isWork=0;
	//cstr = str;
	ret.readNum = wCount;
	ret.str = rStr;
	ret.isWork = 1;
	return ret;
}

int writecom(int comNum, char lpOutBuffer[], DWORD dwBytesWrite)
{
	//DWORD dwBytesWrite = 255;
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	BOOL bWriteStat;
	ClearCommError(hCom[comNum], &dwErrorFlags, &ComStat);
	bWriteStat = WriteFile(hCom[comNum], lpOutBuffer, dwBytesWrite, &dwBytesWrite, NULL);
	if (!bWriteStat)
		return FALSE;
	PurgeComm(hCom[comNum], PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	return TRUE;
}

int closecom(int keyboardnum)
{
	if (!CloseHandle(hCom[keyboardnum]))
		return FALSE;
	return TRUE;
}

char hexr[255] = { 0 };
int BinToHex(int bin[], int HexMaxNum)
{
	//char rtemp[255] = { 0 };
	for (int i = 0; i < HexMaxNum; i++)
	{
		char temp[255] = { 0 };
		char itemp[255] = { 0 };
		sprintf(itemp, "%d%d%d%d%d%d%d%d", bin[i * 8], bin[i * 8 + 1],
			bin[i * 8 + 2], bin[i * 8 + 3], bin[i * 8 + 4],
			bin[i * 8 + 5], bin[i * 8 + 6], bin[i * 8 + 7]);
		sprintf(temp, "%c", strtol(itemp, 0, 2));
		//printf("%s", temp);
		hexr[i] = temp[0];
	}
	//rc = &rtemp;
	return 1;
}


BYTE scan_code(DWORD pKey)
{
	const DWORD result = MapVirtualKey(pKey, MAPVK_VK_TO_VSC);
	return static_cast<BYTE>(result);
}

int setkey(int UpDown,int keyNum[5])
{
	int mouseLeft = MOUSEEVENTF_LEFTUP;
	int mouseMiddle = MOUSEEVENTF_MIDDLEUP;
	int mouseRight = MOUSEEVENTF_RIGHTUP;
	DWORD keyUpDown = KEYEVENTF_KEYUP;
	if (UpDown == 0)
	{
		mouseLeft = MOUSEEVENTF_LEFTDOWN;
		mouseMiddle = MOUSEEVENTF_MIDDLEDOWN;
		mouseRight = MOUSEEVENTF_RIGHTDOWN;
		keyUpDown = 0;
	}

	INPUT inputs;
	HWND hWnd = GetForegroundWindow();
	for (int i = 0; i < 5; i++)
	{
		if (keyNum[i] <= 255)
			keybd_event(keyNum[i], scan_code(keyNum[i]), keyUpDown, 0);
		/**
		if (keyNum[i] <= 255 && keyNum[i] > 0)
		{
			inputs.type = INPUT_KEYBOARD;
			inputs.ki = { (WORD)keyNum[i], 0, keyUpDown, 0, 0 };
			SendInput(1, &inputs, sizeof(INPUT));
		}*/
		else if (keyNum[i] == 256)
			mouse_event(mouseLeft, 0, 0, 0, 0);
		else if (keyNum[i] == 257)
			mouse_event(mouseMiddle, 0, 0, 0, 0);
		else if (keyNum[i] == 258)
			mouse_event(mouseRight, 0, 0, 0, 0);
		else if (keyNum[i] == 259)
			SendMessage(hWnd,WM_MOUSEWHEEL, WHEEL_DELTA << 16, 0); //上滚
		else if (keyNum[i] == 260)
			SendMessage(hWnd,WM_MOUSEWHEEL, -WHEEL_DELTA << 16, 0); //下滚


		
		//Sleep(125);
	}

	return 1;
}

#define WM_IAWENTRAY    WM_USER+5  //系统托盘的自定义消息  

//定义全局变量  
HINSTANCE g_hInst;
NOTIFYICONDATA nid;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#define WCLASSNAME "WindowClassName"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define COLOR_BPP 32

HWND hwndDOS = NULL;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,PSTR szCmdLine, int iCmdShow)
{
	g_hInst = hInstance; //获取该程序实例句柄  

	static TCHAR szAppName[] = TEXT("TimeWork");
	MSG          msg;
	WNDCLASS     wndclass;
	HWND         hwnd;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			szAppName, MB_ICONERROR);
		return 0;
	}

		hwnd = CreateWindow(szAppName,                  // window class name  
		TEXT(""), // window caption  
		WS_OVERLAPPEDWINDOW,        // window style  
		CW_USEDEFAULT,              // initial x position  
		CW_USEDEFAULT,              // initial y position  
		CW_USEDEFAULT,              // initial x size  
		CW_USEDEFAULT,              // initial y size  
		NULL,                       // parent window handle  
		NULL,                       // window menu handle  
		hInstance,                  // program instance handle  
		NULL);                     // creation parameters  

	//初始化NOTIFYICONDATA结构  
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = 101;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_IAWENTRAY;
	nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(101));
	_tcscpy(nid.szTip, TEXT("串口外设驱动程序\r\n左键打开主界面\r\n中键显示/隐藏调试界面\r\n右键关闭")); //_tcscpy是windows宏，需包含头文件tchar.h  
	Shell_NotifyIcon(NIM_ADD, &nid); //最小化时隐藏窗口并设置系统托盘


	char inipath[255] = { 0 };
	char path1[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, path1);
	sprintf(inipath, "%s\\Default\\config.ini", path1);
	LPTSTR lpPath = new char[MAX_PATH];
	strcpy(lpPath, inipath);

	if (AllocConsole())
	{
		freopen("CONOUT$", "w", stdout);
		hwndDOS = GetForegroundWindow();

		if (GetPrivateProfileInt("Software", "WHide", 0, lpPath))
			ShowWindow(hwndDOS, SW_HIDE); //隐藏了获取的窗口.
		printf("串口外设驱动程序\n");
		system("title COMDriver - Build4430 Alpha3 - 20240125 - Copyright 351Workshop");
	}

	int knum = GetPrivateProfileInt("KeyBoard", "Num", 0, lpPath);
	char ch_KeyboardCom[255];
	char ch_temp[255] = { 0 };

	for (int i = 0; i < knum; i++)
	{
		_itoa(i, ch_temp, 255);
		LPTSTR lpPath1 = new char[MAX_PATH];
		sprintf(inipath, "%s\\Default\\%d\\config.ini", path1, i);
		strcpy(lpPath1, inipath);
		GetPrivateProfileInt("KeyBoard", "Num", 0, lpPath);
		GetPrivateProfileString("KeyBoard", ch_temp, "nul", ch_KeyboardCom, 255, lpPath);
		int ocom=opencom(ch_KeyboardCom, GetPrivateProfileInt("KeyBoard", "Bps", 0, lpPath1), 
						GetPrivateProfileInt("KeyBoard", "Par", 0, lpPath1),i);
		printf("配置：序号%s 串口号%s 波特率%d 校验%d 键盘数%d\n", ch_temp, ch_KeyboardCom,
				GetPrivateProfileInt("KeyBoard", "Bps", 0, lpPath1),
				GetPrivateProfileInt("KeyBoard", "Par", 0, lpPath1),knum);
		if (ocom == 0)
		{
			puts("error1");
			ShowWindow(hwndDOS, SW_SHOW);
			while (1);
		}
	}

	for (int i = 0; i < knum; i++)
	{
		printf("配置键盘中：序号%d",i);
		_itoa(i, ch_temp, 255);
		LPTSTR lpPath1 = new char[MAX_PATH];

		
		char txtpath[255] = { 0 };
		char lpOutBuffer[255] = { 0 };
		LPTSTR lpPath2 = new char[MAX_PATH];
		sprintf(txtpath, "%s\\Default\\%d\\StartConfig.scg", path1, i);
		strcpy(lpPath2, txtpath);
		HANDLE hFile;
		hFile = CreateFile(txtpath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD dwReads;
		ReadFile(hFile, lpOutBuffer, 255, &dwReads, NULL);
		lpOutBuffer[dwReads] = 0;
		CloseHandle(hFile);
		
		char och[2] = {0};
		int cnum = 0;
		if (dwReads > 0)
		{
			while (1)
			{
				och[0] = lpOutBuffer[cnum];
				/**
				DWORD dwBytesWrite = 1;
				COMSTAT ComStat;
				DWORD dwErrorFlags;
				BOOL bWriteStat;
				ClearCommError(hCom[i], &dwErrorFlags, &ComStat);
				bWriteStat = WriteFile(hCom[i], och, dwBytesWrite, &dwBytesWrite, NULL);
				if (!bWriteStat)
				{
					printf("error2 %s %s %s\n", txtpath, lpOutBuffer, och);
					ShowWindow(hwndDOS, SW_SHOW);
					//while (1);
				}
				PurgeComm(hCom[i], PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);*/
				if (!writecom(i, och,1))
				{
					printf("error2 %s %s %s\n", txtpath, lpOutBuffer, och);
					ShowWindow(hwndDOS, SW_SHOW);
				}

				cnum++;

				if (cnum == dwReads)
					break;
			}
			
		}
		printf("	完成!\n");
	}


	for (int i = 0; i < knum; i++)
	{
		_itoa(i, ch_temp, 255);
		LPTSTR lpPath1 = new char[MAX_PATH];
		sprintf(inipath, "%s\\Default\\%d\\config.ini", path1, i);
		strcpy(lpPath1, inipath);

		int sv=GetPrivateProfileInt("KeyBoard", "StartVidio", 0, lpPath1);
		int svn = GetPrivateProfileInt("KeyBoard", "SVMaxNum", 0, lpPath1);
		if (sv)
		{
			char txtpath[255] = { 0 };
			char lpOutBuffer[25] = { 0 };
			HANDLE hFile;
			DWORD dwReads;
			LPTSTR lpPath2 = new char[MAX_PATH];
			//DWORD dwBytesWrite = 255;
			//COMSTAT ComStat;
			//DWORD dwErrorFlags;
			//BOOL bWriteStat;

			for (int j = 0; j < svn; j++)
			{
				printf("显示动画中：序号%d 第%d帧", i, j);
				sprintf(txtpath, "%s\\Default\\%d\\StartVidio%d.tvo", path1, i, j);
				strcpy(lpPath2, txtpath);
				hFile = CreateFile(txtpath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				ReadFile(hFile, lpOutBuffer, 255, &dwReads, NULL);
				lpOutBuffer[dwReads] = 0;
				CloseHandle(hFile);

				/**
				ClearCommError(hCom[i], &dwErrorFlags, &ComStat);
				bWriteStat = WriteFile(hCom[i], lpOutBuffer, /*dwBytesWritedwReads, &dwBytesWrite, NULL);
				if (!bWriteStat)
				{
					printf("error3 %s %s\n", txtpath, lpOutBuffer);
					ShowWindow(hwndDOS, SW_SHOW);
					//while (1);
				}
				PurgeComm(hCom[i], PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);*/

				if (!writecom(i, lpOutBuffer, dwReads))
				{
					printf("error3 %s %s\n", txtpath, lpOutBuffer);
					ShowWindow(hwndDOS, SW_SHOW);
				}

				Sleep(GetPrivateProfileInt("KeyBoard", "SVSleepTime", 0, lpPath1));
				printf("	完成!\n");
			}
		}
	}
	printf("---------------------接收键盘信号---------------------\n");
	int locktable[255][255] = { 0 };
	int lighttable[255][255] = { 0 };
	while (1)
	{
		PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		//puts("a");
		for (int i = 0; i < knum; i++)
		{
			//puts("a");
			DWORD wCount; //读取的字节数
			int bReadStat;
			unsigned char tmp[255] = { 0 };
			bReadStat = ReadFile(hCom[i],tmp, 1, &wCount, NULL);
			if (wCount==0)
			{
				printf("失败!序号%d 输入%X\n", i, tmp[0]);//***
			}
			else if (!bReadStat)
			{
				printf("error4!\n");//***
			}
			else
			{
				printf("开始执行：序号%d 输入%X 个数%d 锁定%d", i, tmp[0],wCount,
					GetPrivateProfileInt(ch_temp, "Lock", 0, lpPath));
				sprintf(inipath, "%s\\Default\\%d\\config.ini", path1,i);
				strcpy(lpPath, inipath);

				//_itoa(tmp[0], ch_temp, 255);
				sprintf(ch_temp, "%X", tmp[0]);
				//ch_temp[0] = tmp[0];
				int k1 = GetPrivateProfileInt(ch_temp, "1", -1, lpPath);
				int k2 = GetPrivateProfileInt(ch_temp, "2", -1, lpPath);
				int k3 = GetPrivateProfileInt(ch_temp, "3", -1, lpPath);
				int k4 = GetPrivateProfileInt(ch_temp, "4", -1, lpPath);
				int k5 = GetPrivateProfileInt(ch_temp, "5", -1, lpPath);
				int keyNums[5] = { k1, k2, k3, k4, k5 };

				setkey(0, keyNums);

				int keynum = GetPrivateProfileInt(ch_temp, "KeyNum", 0, lpPath);
				int kmnum = GetPrivateProfileInt("Keyboard", "KeyMaxNum", 0, lpPath);
				if (GetPrivateProfileInt(ch_temp, "LightMode", 0, lpPath) == 1)
				{
					if (lighttable[i][keynum] == 0)
					{
						lighttable[i][keynum] = 1;

						BinToHex(lighttable[i], kmnum);
						//strtol(lighttable, 0, 2);
						//char chanum[255] = {0};
						//strcpy(chanum, cchanum);

						char lpOutBuffer[255] = {0};
						lpOutBuffer[0] = GetPrivateProfileInt("Keyboard", "LightCtrl", 0, lpPath);
						for (int j = 0; j < kmnum / 8; j++)
						{
							lpOutBuffer[j + 1] = hexr[j];
						}
						/*
						DWORD dwBytesWrite = kmnum/8+1;
						COMSTAT ComStat;
						DWORD dwErrorFlags;
						BOOL bWriteStat;
						ClearCommError(hCom[i], &dwErrorFlags, &ComStat);
						bWriteStat = WriteFile(hCom[i], lpOutBuffer, dwBytesWrite, &dwBytesWrite, NULL);
						if (!bWriteStat) { printf("error4"); }
						PurgeComm(hCom[i], PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);*/

						if (!writecom(i, lpOutBuffer, kmnum / 8 + 1))
						{
							printf("error3");
							ShowWindow(hwndDOS, SW_SHOW);
						}
					}
					else
					{
						lighttable[i][keynum] = 0;
						BinToHex(lighttable[i], kmnum);
						//string canum = IntToStr(lighttable);
						//string hanum = BinToHex(canum.data(), kmnum,0);

						//const char *cchanum = hanum.data();
						//char chanum[255] = { 0 };
						//strcpy(chanum, cchanum);

						//char lpOutBuffer[100];
						char lpOutBuffer[255] = {0};
						lpOutBuffer[0] = GetPrivateProfileInt("Keyboard", "LightCtrl", 0, lpPath);
						for (int j = 0; j < kmnum / 8; j++)
						{
							lpOutBuffer[j + 1] = hexr[j];
						}
						/*DWORD dwBytesWrite = kmnum / 8 + 1;
						COMSTAT ComStat;
						DWORD dwErrorFlags;
						BOOL bWriteStat;
						ClearCommError(hCom[i], &dwErrorFlags, &ComStat);
						bWriteStat = WriteFile(hCom[i], lpOutBuffer, dwBytesWrite, &dwBytesWrite, NULL);
						if (!bWriteStat) { printf("error4"); }
						PurgeComm(hCom[i], PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);*/
						if (!writecom(i, lpOutBuffer, kmnum / 8 + 1))
						{
							printf("error3");
							ShowWindow(hwndDOS, SW_SHOW);
						}
					}
				}
				else if (GetPrivateProfileInt(ch_temp, "LightMode", 0, lpPath) == 2)
				{
					lighttable[i][keynum] = 1;

					BinToHex(lighttable[i], kmnum);
					//strtol(lighttable, 0, 2);
					//char chanum[255] = {0};
					//strcpy(chanum, cchanum);

					//char lpOutBuffer[100];
					char lpOutBuffer[255];
					lpOutBuffer[0] = GetPrivateProfileInt("Keyboard", "LightCtrl", 0, lpPath);
					for (int j = 0; j < kmnum / 8; j++)
					{
						lpOutBuffer[j + 1] = hexr[j];
					}
					/*DWORD dwBytesWrite = kmnum / 8 + 1;
					COMSTAT ComStat;
					DWORD dwErrorFlags;
					BOOL bWriteStat;
					ClearCommError(hCom[i], &dwErrorFlags, &ComStat);
					bWriteStat = WriteFile(hCom[i], lpOutBuffer, dwBytesWrite, &dwBytesWrite, NULL);
					if (!bWriteStat) { printf("error4"); }
					PurgeComm(hCom[i], PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);*/
					if (!writecom(i, lpOutBuffer, kmnum / 8 + 1))
					{
						printf("error3");
						ShowWindow(hwndDOS, SW_SHOW);
					}
				}



				Sleep(25);

				if (GetPrivateProfileInt(ch_temp, "Lock", 0, lpPath))
				{
					if (locktable[i][keynum] == 0)
					{
						locktable[i][keynum] = 1;
					}
					else
					{
						setkey(1, keyNums);

						locktable[i][keynum] = 0;
					}
				}
				else
				{
					setkey(1, keyNums);
				}

				if (GetPrivateProfileInt(ch_temp, "LightMode", 0, lpPath) == 2)
				{
					lighttable[i][keynum] = 0;

					BinToHex(lighttable[i], kmnum);
					//strtol(lighttable, 0, 2);
					//char chanum[255] = {0};
					//strcpy(chanum, cchanum);

					//char lpOutBuffer[100];
					char lpOutBuffer[255];
					lpOutBuffer[0] = GetPrivateProfileInt("Keyboard", "LightCtrl", 0, lpPath);
					for (int j = 0; j < kmnum / 8; j++)
					{
						lpOutBuffer[j + 1] = hexr[j];
					}
					
					/*DWORD dwBytesWrite = kmnum / 8 + 1;
					COMSTAT ComStat;
					DWORD dwErrorFlags;
					BOOL bWriteStat;
					ClearCommError(hCom[i], &dwErrorFlags, &ComStat);
					bWriteStat = WriteFile(hCom[i], lpOutBuffer, dwBytesWrite, &dwBytesWrite, NULL);
					if (!bWriteStat) { printf("error4"); }
					PurgeComm(hCom[i], PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);*/
					if (!writecom(i, lpOutBuffer, kmnum / 8 + 1))
					{
						printf("error3");
						ShowWindow(hwndDOS, SW_SHOW);
					}
				}
				printf("	完成!\n");
			}
		}
	}
}

int hidemode=1;
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC         hdc;
	PAINTSTRUCT ps;
	RECT        rect;

	switch (message)
	{
	case WM_CREATE:
		return 0;

		case WM_PAINT:
			return 0;

	case WM_IAWENTRAY:
		if (wParam == 101) {
			if (lParam == WM_RBUTTONDOWN) {
				Sleep(250);
				exit(0);
				return TRUE;
			}
			else if (lParam == WM_LBUTTONDOWN) {
				char path1[MAX_PATH] = { 0 };
				GetCurrentDirectory(MAX_PATH, path1);
				SHELLEXECUTEINFO lpExecInfo = { 0 };
				char exepath[100];
				sprintf(exepath, "%s\\setup.exe", path1);
				lpExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
				lpExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
				lpExecInfo.lpVerb = "Open";
				lpExecInfo.hwnd = NULL;
				lpExecInfo.lpFile = exepath;
				lpExecInfo.lpDirectory = NULL;
				lpExecInfo.nShow = SW_SHOW;
				lpExecInfo.lpParameters = NULL;
				lpExecInfo.hInstApp = NULL;
				ShellExecuteEx(&lpExecInfo);
				return TRUE;
			}
			else if (lParam == WM_MBUTTONDOWN)
			{
				if (hidemode)
				{
					ShowWindow(hwndDOS, SW_SHOW);
					hidemode = 0;
				}
				else
				{
					ShowWindow(hwndDOS, SW_HIDE);
					hidemode = 1;
				}
			}
		}
		return FALSE;
		
	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_CLOSE:
			DestroyWindow(hwnd);
			PostQuitMessage(0);
			break;
		case SC_MINIMIZE:
			ShowWindow(hwnd, SW_HIDE);
			break;
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	
	return 1;
}