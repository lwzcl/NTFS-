#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#include "DiksConn.h"


void ShowHddInfo()
{

	DiksConn disk;
	DWORD tempDrive;
	LOGICALDRIVEINFO theLogicalDriveInfo;

	char tempChar[128] = {0};
	//LOGICALDRIVEINFO logicalDrive;

	char szBuff[100];   //临时辅助变量
	PHYDRIVEINFOSHORT thePhyDIS;  //简短的物理驱动器信息
	int nDrive = 0;    //用来统计有多少个物理驱动器

	while (disk.GetPhysicalDriveInfoShort(&thePhyDIS, nDrive))
	{
		//构造要显示的字符串,包括驱动器名称,型号,容量
		sprintf_s(szBuff, "%d %s  %s (%I64d GB)\n", nDrive, thePhyDIS.modelNumber, thePhyDIS.serialNumber, thePhyDIS.size / 1024 / 1024 / 1024);

		printf(szBuff);

		//Pitem = m_diskTree.InsertItem(szBuff, NULL, NULL, root);
		tempDrive = GetLogicalDrives();

		for (int i = 0; i < 31; i++, tempDrive = tempDrive >> 1)
		{
			//tempDrive&1表示当前的驱动器存在，存在了在调用函数获取，且获取成功了才插入
			if (tempDrive & 1 && disk.GetLogicalDriveInfoOnly(&theLogicalDriveInfo, 'A' + i))
			{
				memset(tempChar, 0, 128);
				sprintf_s(tempChar, "         %c:  %s   %s   (%.2f GB)   \n",
					theLogicalDriveInfo.szDrive,
					strlen(theLogicalDriveInfo.szDriveName) == 0 ? "本地磁盘" : theLogicalDriveInfo.szDriveName,
					theLogicalDriveInfo.dwFileSystem,
					(__int64)theLogicalDriveInfo.dwTotalNumberOfClusters * (__int64)theLogicalDriveInfo.dwSectorsPerCluster * (__int64)theLogicalDriveInfo.dwBytesPerSector / 1024 / 1024 / 1024.00);
				

				if (disk.GetLogicalDriveInfo(&theLogicalDriveInfo, theLogicalDriveInfo.szDrive, nDrive))
				{
				
					printf(tempChar);
				}


			}
		}


		nDrive++;
	}


}

void HddToHdd(char* srcprot, char* dstprot,DWORDLONG hddsize)
{
	char srchdd[32] = { 0 };
	char dsthdd[32] = { 0 };

	//
	HANDLE srcHandle = CreateFileA(srchdd, GENERIC_READ , FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);

	HANDLE dstHandle = CreateFileA(srchdd, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if (srcHandle != INVALID_HANDLE_VALUE    ||  dstHandle != INVALID_HANDLE_VALUE)
	{


	}




}
void startmnu(char * srcnum,char * dstnum ,ULONG srcSize)
{




}
int main()
{
	char srcnum;
	char dstnum;
	ULONG* srcSize = 0;

	//显示磁盘
	ShowHddInfo();
	//操作界面
	//startmnu(&srcnum, &dstnum, srcSize);
	printf("############################请输入故障盘编号#######################################\n");
	scanf("%c", &srcnum);

	Sleep(100);

	printf("############################请输入目标盘编号#######################################\n");

	scanf("%c", &dstnum);

	Sleep(100);

	printf("############################请输入硬盘容量#######################################\n");
	

	scanf("%d", srcSize);
		

	//HddToHdd(srcHdd, dstHdd, srcSize);

	

	getchar();

}