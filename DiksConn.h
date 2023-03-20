#pragma once
#include <afxwin.h>
#include<stdio.h>


#include <WinUser.h>
#include <process.h>
#include <afx.h>
#include <ShlObj.h>
#include <WinIoCtl.h>

//�߼���������������Ϣ�ṹ��
typedef struct {
	char szDrive; //������·��
	UINT uDriveType;     //����
	TCHAR szDriveName[32];    //����������   
	DWORD dwSerialNumber;    //Ӳ�����к�
	DWORD dwMaximumComponentLength;
	DWORD dwFileSystemFlags;      //�ж϶�������������ֵ����FILE_VOLUME_QUOTAS
	char dwFileSystem[16];       //�ļ�ϵͳ
	DWORD dwSectorsPerCluster;   //ÿ�ض��ٸ�����
	DWORD dwBytesPerSector;    //ÿ���������ֽ���
	DWORD dwNumberOfFreeCluters;   //���дش�С
	DWORD dwTotalNumberOfClusters;  //�ܴش�С
}LOGICALDRIVEINFO, * PLOGICALDRIVEINFO;

//����洢�豸�����Ϣ
typedef struct {
	char serialNumber[100];
	char modelNumber[100];
	char vendorId[100];
	char productRevision[100];
	int fixed;   //ý�����ͣ���Fixed��remoble��unknown
	__int64 size;  //��С
}PHYDRIVEINFOSHORT, * PPHYDRIVEINFOSHORT;

//����洢�豸�Ƚ���ϸ��Ϣ��S.M.A.R.T.ʹ��
typedef struct {
	char serialNumber[100];
	char modelNumber[100];
	char contrllorRivision[100];
	__int64 bufferSize;  //��������С����byteΪ��λ
	int driveType;   //ý�����ͣ���Fixed��remoble��unknown
	__int64 size;  //��С,��byteΪ��λ
	int controllerType; //Primary - 0 ; Secondary - 1  ;  Tertiary - 2  ;   Quaternary - 4
	char masterorSlave[20]; // Master - 0   ;  Slave - 1 ;
}PHYDRIVEINFOSMART, * PPHYDRIVEINFOSMART;

typedef struct {
	DWORD dwSourceDrive; //Դ����
	char modelNumber[100];
	char serialNumber[100];
	__int64 iDesAllSector;   //Ŀ���ж��ٸ�����
}SelectInfo, * PSelectInfo;

typedef struct {
	HWND hDlg; //�����ھ��
	HWND hPSSectorClone;  //���������
	HWND hStaticSectorClone; //�����ʾ��Ϣ�ľ�̬�ı���
	DWORD dwSourceDrive; //Դ����
	DWORD dwDesDrive;   //Ŀ�Ĵ���
	__int64 iSourceAllSector;   //Դ�ж��ٸ�����
	__int64 iDesAllSector;   //Ŀ���ж��ٸ�����
	BOOL bContinue;    //�Ƿ����
}sectorClonePARAMS, * PsectorClonePARAMS;


//�߼���������������Ϣ�ṹ��
typedef struct {
	CString index; //�������˿�
	CString szDriveName;    //����������   
	CString dwSerialNumber;    //Ӳ�����к�
	CString dwTotalNumberOfClusters;  //�ܴش�С
}mydiskinfo, * Pmydiskinfo;

typedef struct {
	INT64 time3ms;   //С��<=3
	INT64 time15ms; //С��<=15
	INT64 time150ms;////С��<=150
	INT64 timeG150ms; //����150
	INT64 bd; //����
}Myjingdu, * PMyjingdu;

class DiksConn
{
public:

	BOOL GetPhysicalDriveInfoShort(PHYDRIVEINFOSHORT* pThePhyDriveInfo, int iDrive);
	char* flipAndCodeBytes(const char* str, int pos, int flip, char* buf);


	BOOL GetLogicalDriveInfoOnly(LOGICALDRIVEINFO* logicalDriveInfo, char logicalDriveNumber);   //ֻ��ȡ�߼���������Ϣ�ĺ���


	//////////////////////////////////////////////////////////////////////////
//��ȡ�߼���������Ϣ��
//������LOGICALDRIVEINFO�洢�߼�����������Ϣ��LPCTSTR����ڼ����߼���������int ����ڼ��������������������-1��������Ǻ�����������������������ж�
//����ֵ������߼��������������������У�����TRUE��������ڻ����߼������������ڣ�����FALSE
	BOOL GetLogicalDriveInfo(LOGICALDRIVEINFO* logicalDriveInfo, char logicalDriveNumber, int phyDriveNumber);

	//------------------------------------------------------------------------------------
	//����Tree��.
	//����:��ӵ�TreeView���;��ӵ��ַ���;���ڵ�;�Ƿ�Ϊ�ļ���
	HTREEITEM AddItemToTree(HWND hwndTV, LPSTR lpszItem, HTREEITEM hParent, BOOL bFolder);
	//�ַ�������
	mydiskinfo myStringSP(CString hddname);


	//�ж��߼������Ƿ������������
	void IsDiskInfo();
};

