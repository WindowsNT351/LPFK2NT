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
#include <conio.h>
//#include <resource.h>
using namespace std;

HANDLE hCom[20] = { 0 }; //ȫ�ֱ��������ھ��
//#define CMAX	20
#define MYWM_NOTIFYICON WM_USER+1

int opencom(char number[], int bps, int par, int keyboardnum)
{
	hCom[keyboardnum] = CreateFile(number,//COM��
		GENERIC_READ | GENERIC_WRITE, //�������д
		0, //��ռ��ʽ
		NULL,
		OPEN_EXISTING, //�򿪶����Ǵ���
		0, //ͬ����ʽ
		NULL);
	if (hCom[keyboardnum] == (HANDLE)-1)
	{
		return FALSE;
	}

	SetupComm(hCom[keyboardnum], 1024, 1024); //���뻺����������������Ĵ�С����1024
	COMMTIMEOUTS TimeOuts; //�趨����ʱ
	TimeOuts.ReadIntervalTimeout = 1000;
	TimeOuts.ReadTotalTimeoutMultiplier = 50;
	TimeOuts.ReadTotalTimeoutConstant = 20; //�趨д��ʱ
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	SetCommTimeouts(hCom[keyboardnum], &TimeOuts); //���ó�ʱ
	DCB dcb;
	GetCommState(hCom[keyboardnum], &dcb);
	dcb.BaudRate = bps; //������
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_ENABLE;
	dcb.ByteSize = 8; //ÿ���ֽ���8λ
	dcb.Parity = par; //����żУ��λ
	dcb.StopBits = ONESTOPBIT; //1��ֹͣλ
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
	DWORD wCount; //��ȡ���ֽ���
	BOOL bReadStat;
	bReadStat = ReadFile(hCom[comNum], rStr, 255, &wCount, NULL);
	if (!bReadStat)
		ret.isWork = 0;
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

DWORD SaveDataToFile(
	LPSTR szFilePath,
	LPVOID lpData,
	DWORD dwDataSize)
{
	//�ļ����
	HANDLE hFileWrite;
	//�ɹ�д������ݴ�С
	DWORD dwWritedDateSize;
	//���Ѿ����ڵ��ļ�����ȡ����
	hFileWrite = CreateFileA(szFilePath,//Ҫ�򿪵��ļ���
		GENERIC_WRITE,//��д��ʽ�򿪿�
		0,//�ɹ����
		NULL,//Ĭ�ϰ�ȫ����
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,//�������Դ�
		NULL);//��ģ��
	//�ж��Ƿ�ɹ���
	if (hFileWrite == INVALID_HANDLE_VALUE)
	{
		printf("���ļ�ʧ�ܣ� %d\n", GetLastError());
		while (1);
	}
	//�����ļ�ָ�뵽�ļ�Ϊ
	SetFilePointer(hFileWrite, 0, 0, FILE_END);
	//������д���ļ�
	if (!WriteFile(hFileWrite, lpData, dwDataSize, &dwWritedDateSize, NULL))
	{
		printf("д�ļ�ʧ�ܣ� %d\n", GetLastError());
		while (1);
	}
	else
	{
		printf("д�ļ��ɹ���д��%d�ֽڡ�\n", dwWritedDateSize);
	}
	CloseHandle(hFileWrite);
	return 0;
} 

HWND hwndDOS = NULL;
int main()
{
	char inipath[255] = { 0 };
	char path1[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, path1);
	sprintf(inipath, "%s\\Default\\config.ini", path1);
	LPTSTR lpPath = new char[MAX_PATH];
	strcpy(lpPath, inipath);

	int knum = GetPrivateProfileInt("KeyBoard", "Num", 0, lpPath);
	char ch_KeyboardCom[255];
	char ch_temp[255] = { 0 };

	system("title LEDEdit - Build4430 Alpha3 - 20240125 - Copyright 351Workshop");
	for (int i = 0; i < knum; i++)
	{
		_itoa(i, ch_temp, 255);
		LPTSTR lpPath1 = new char[MAX_PATH];
		sprintf(inipath, "%s\\Default\\%d\\config.ini", path1, i);
		strcpy(lpPath1, inipath);
		GetPrivateProfileInt("KeyBoard", "Num", 0, lpPath);
		GetPrivateProfileString("KeyBoard", ch_temp, "nul", ch_KeyboardCom, 255, lpPath);
		int ocom = opencom(ch_KeyboardCom, GetPrivateProfileInt("KeyBoard", "Bps", 0, lpPath1),
			GetPrivateProfileInt("KeyBoard", "Par", 0, lpPath1), i);
		printf("���ã����%s ���ں�%s ������%d У��%d ������%d\n", ch_temp, ch_KeyboardCom,
			GetPrivateProfileInt("KeyBoard", "Bps", 0, lpPath1),
			GetPrivateProfileInt("KeyBoard", "Par", 0, lpPath1), knum);
	}

	for (int i = 0; i < knum; i++)
	{
		printf("���ü����У����%d", i);
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

		char och[2] = { 0 };
		int cnum = 0;
		if (dwReads > 0)
		{
			while (1)
			{
				och[0] = lpOutBuffer[cnum];
				if (!writecom(i, och, 1))
				{
					printf("error2 %s %s %s\n", txtpath, lpOutBuffer, och);
				}

				cnum++;

				if (cnum == dwReads)
					break;
			}

		}
		printf("	���!\n");
	}

	int videoCount = 0;
	printf("---------------------���ռ����ź�---------------------\n");
	printf("---------------------��¼��......---------------------\n");
	int locktable[255][255] = { 0 };
	int lighttable[255][255] = { 0 };
	while (1)
	{
		/*PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
		TranslateMessage(&msg);
		DispatchMessage(&msg);*/
		//puts("a");
		for (int i = 0; i < knum; i++)
		{
			char lpOutBuffer[255] = { 0 };
			//puts("a");
			DWORD wCount; //��ȡ���ֽ���
			int bReadStat;
			unsigned char tmp[255] = { 0 };
			int kmnum = GetPrivateProfileInt("Keyboard", "KeyMaxNum", 0, lpPath);
			bReadStat = ReadFile(hCom[i], tmp, 1, &wCount, NULL);
			if (wCount == 0)
			{
				printf("ʧ��!���%d ����%X\n", i, tmp[0]);//***
			}
			else if (!bReadStat)
			{
				printf("error4!\n");//***
			}
			else
			{
				printf("��ʼִ�У����%d ����%X ����%d ����%d", i, tmp[0], wCount,
					GetPrivateProfileInt(ch_temp, "Lock", 0, lpPath));
				sprintf(inipath, "%s\\Default\\%d\\config.ini", path1, i);
				strcpy(lpPath, inipath);
				
				sprintf(ch_temp, "%X", tmp[0]);

				int keynum = GetPrivateProfileInt(ch_temp, "KeyNum", 0, lpPath);

				if (lighttable[i][keynum] == 0)
				{
					lighttable[i][keynum] = 1;

					BinToHex(lighttable[i], kmnum);

					lpOutBuffer[0] = GetPrivateProfileInt("Keyboard", "LightCtrl", 0, lpPath);
					for (int j = 0; j < kmnum / 8; j++)
					{
						lpOutBuffer[j + 1] = hexr[j];
					}

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

					lpOutBuffer[0] = GetPrivateProfileInt("Keyboard", "LightCtrl", 0, lpPath);
					for (int j = 0; j < kmnum / 8; j++)
					{
						lpOutBuffer[j + 1] = hexr[j];
					}
						
					if (!writecom(i, lpOutBuffer, kmnum / 8 + 1))
					{
						printf("error3");
						ShowWindow(hwndDOS, SW_SHOW);
					}
				}
				
				printf("	���!\n");
			}

			if (_kbhit())
			{
				if (_getche() == 0x30 + knum)
				{
					char savePath[255] = { 0 };
					sprintf(savePath, "newVid\\StartVidio%d.tvo", videoCount++);
					printf("ת��%s��		", savePath);

					BinToHex(lighttable[i], kmnum);
					lpOutBuffer[0] = GetPrivateProfileInt("Keyboard", "LightCtrl", 0, lpPath);
					for (int j = 0; j < kmnum / 8; j++)
					{
						lpOutBuffer[j + 1] = hexr[j];
					}

					SaveDataToFile(savePath, lpOutBuffer, kmnum / 8 + 1);

				}
			}
		}
	}
}