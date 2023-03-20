#pragma once
#include <afxwin.h>
#include<stdio.h>


#include <WinUser.h>
#include <process.h>
#include <afx.h>
#include <ShlObj.h>
#include <WinIoCtl.h>

//逻辑驱动器驱动器信息结构体
typedef struct {
	char szDrive; //驱动器路径
	UINT uDriveType;     //类型
	TCHAR szDriveName[32];    //驱动器名称   
	DWORD dwSerialNumber;    //硬盘序列号
	DWORD dwMaximumComponentLength;
	DWORD dwFileSystemFlags;      //判断多种驱动器属性值，如FILE_VOLUME_QUOTAS
	char dwFileSystem[16];       //文件系统
	DWORD dwSectorsPerCluster;   //每簇多少个扇区
	DWORD dwBytesPerSector;    //每个扇区的字节数
	DWORD dwNumberOfFreeCluters;   //空闲簇大小
	DWORD dwTotalNumberOfClusters;  //总簇大小
}LOGICALDRIVEINFO, * PLOGICALDRIVEINFO;

//物理存储设备简短信息
typedef struct {
	char serialNumber[100];
	char modelNumber[100];
	char vendorId[100];
	char productRevision[100];
	int fixed;   //媒体类型，即Fixed，remoble，unknown
	__int64 size;  //大小
}PHYDRIVEINFOSHORT, * PPHYDRIVEINFOSHORT;

//物理存储设备比较详细信息，S.M.A.R.T.使用
typedef struct {
	char serialNumber[100];
	char modelNumber[100];
	char contrllorRivision[100];
	__int64 bufferSize;  //缓冲区大小，以byte为单位
	int driveType;   //媒体类型，即Fixed，remoble，unknown
	__int64 size;  //大小,以byte为单位
	int controllerType; //Primary - 0 ; Secondary - 1  ;  Tertiary - 2  ;   Quaternary - 4
	char masterorSlave[20]; // Master - 0   ;  Slave - 1 ;
}PHYDRIVEINFOSMART, * PPHYDRIVEINFOSMART;

typedef struct {
	DWORD dwSourceDrive; //源磁盘
	char modelNumber[100];
	char serialNumber[100];
	__int64 iDesAllSector;   //目的有多少个扇区
}SelectInfo, * PSelectInfo;

typedef struct {
	HWND hDlg; //主窗口句柄
	HWND hPSSectorClone;  //进度条句柄
	HWND hStaticSectorClone; //输出提示信息的静态文本框
	DWORD dwSourceDrive; //源磁盘
	DWORD dwDesDrive;   //目的磁盘
	__int64 iSourceAllSector;   //源有多少个扇区
	__int64 iDesAllSector;   //目的有多少个扇区
	BOOL bContinue;    //是否继续
}sectorClonePARAMS, * PsectorClonePARAMS;


//逻辑驱动器驱动器信息结构体
typedef struct {
	CString index; //驱动器端口
	CString szDriveName;    //驱动器名称   
	CString dwSerialNumber;    //硬盘序列号
	CString dwTotalNumberOfClusters;  //总簇大小
}mydiskinfo, * Pmydiskinfo;

typedef struct {
	INT64 time3ms;   //小于<=3
	INT64 time15ms; //小于<=15
	INT64 time150ms;////小于<=150
	INT64 timeG150ms; //高于150
	INT64 bd; //坏道
}Myjingdu, * PMyjingdu;

class DiksConn
{
public:

	BOOL GetPhysicalDriveInfoShort(PHYDRIVEINFOSHORT* pThePhyDriveInfo, int iDrive);
	char* flipAndCodeBytes(const char* str, int pos, int flip, char* buf);


	BOOL GetLogicalDriveInfoOnly(LOGICALDRIVEINFO* logicalDriveInfo, char logicalDriveNumber);   //只获取逻辑驱动器信息的函数


	//////////////////////////////////////////////////////////////////////////
//获取逻辑驱动器信息，
//参数：LOGICALDRIVEINFO存储逻辑驱动器的信息，LPCTSTR代表第几个逻辑驱动器，int 代表第几个物理驱动器，如果是-1，则代表是忽略这个物理驱动器，不做判断
//返回值：如果逻辑驱动器在物理驱动器中，返回TRUE，如果不在或者逻辑驱动器不存在，返回FALSE
	BOOL GetLogicalDriveInfo(LOGICALDRIVEINFO* logicalDriveInfo, char logicalDriveNumber, int phyDriveNumber);

	//------------------------------------------------------------------------------------
	//添加项到Tree中.
	//参数:添加到TreeView句柄;添加的字符串;父节点;是否为文件夹
	HTREEITEM AddItemToTree(HWND hwndTV, LPSTR lpszItem, HTREEITEM hParent, BOOL bFolder);
	//字符串分离
	mydiskinfo myStringSP(CString hddname);


	//判断逻辑磁盘是否在物理磁盘上
	void IsDiskInfo();
};

