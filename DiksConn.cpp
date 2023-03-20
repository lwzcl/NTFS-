//#include "pch.h"
#include "DiksConn.h"

BOOL DiksConn::GetPhysicalDriveInfoShort(PHYDRIVEINFOSHORT* pThePhyDriveInfo, int iDrive)
{
	HANDLE hPhysicalDriveIOCTL = 0;
	BOOL OK = FALSE;

	TCHAR driveName[32];

	_stprintf_s(driveName, _T("\\\\.\\PhysicalDrive%d"), iDrive);

	// ���豸�����������̾��
	hPhysicalDriveIOCTL = CreateFile(driveName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
	{
		STORAGE_PROPERTY_QUERY query;
		DWORD cbBytesReturned = 0;
		char buffer[10000];

		memset((void*)&query, 0, sizeof(query));
		query.PropertyId = StorageDeviceProperty;
		query.QueryType = PropertyStandardQuery;

		memset(buffer, 0, sizeof(buffer));

		if (DeviceIoControl(hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &buffer, sizeof(buffer), &cbBytesReturned, NULL))
		{
			STORAGE_DEVICE_DESCRIPTOR* descrip = (STORAGE_DEVICE_DESCRIPTOR*)&buffer;
			char serialNumber[100];
			char modelNumber[100];
			char vendorId[100];
			//char productRevision[100];

			//ת���ַ���
			flipAndCodeBytes(buffer, descrip->VendorIdOffset, 0, vendorId);
			flipAndCodeBytes(buffer, descrip->ProductIdOffset, 0, modelNumber);
			//flipAndCodeBytes(buffer, descrip->ProductRevisionOffset, 0, productRevision);
			flipAndCodeBytes(buffer, descrip->SerialNumberOffset, 1, serialNumber);

			strcpy_s(pThePhyDriveInfo->vendorId, vendorId);
			strcpy_s(pThePhyDriveInfo->modelNumber, modelNumber);
			//strcpy_s(pThePhyDriveInfo->productRevision, productRevision);
			strcpy_s(pThePhyDriveInfo->serialNumber, serialNumber);

			//
			//AfxMessageBox(vendorId);
			memset(buffer, 0, sizeof(buffer));
			if (DeviceIoControl(hPhysicalDriveIOCTL, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &buffer, sizeof(buffer), &cbBytesReturned, NULL))
			{
				DISK_GEOMETRY_EX* geom = (DISK_GEOMETRY_EX*)&buffer;
				pThePhyDriveInfo->fixed = (geom->Geometry.MediaType == FixedMedia);   //ý�����ͣ���Fixed��remoble��unknown
				pThePhyDriveInfo->size = geom->DiskSize.QuadPart;           //��С

				OK = TRUE;
			}
		}
	}
	CloseHandle(hPhysicalDriveIOCTL);
	return OK;
}

char* DiksConn::flipAndCodeBytes(const char* str, int pos, int flip, char* buf)
{

	int i;
	int j = 0;
	int k = 0;

	buf[0] = '\0';
	if (pos <= 0)
		return buf;

	if (!j)
	{
		char p = 0;

		//First try to gather all characters representing hex digits only.
		j = 1;
		k = 0;
		buf[k] = 0;
		for (i = pos; j && str[i] != '\0'; ++i)
		{
			char c = tolower(str[i]);

			if (isspace(c))
				c = '0';

			++p;
			buf[k] <<= 4;

			if (c >= '0' && c <= '9')
				buf[k] |= (unsigned char)(c - '0');
			else if (c >= 'a' && c <= 'f')
				buf[k] |= (unsigned char)(c - 'a' + 10);
			else
			{
				j = 0;
				break;
			}

			if (p == 2)
			{
				if (buf[k] != '\0' && !isprint(buf[k]))
				{
					j = 0;
					break;
				}
				++k;
				p = 0;
				buf[k] = 0;
			}

		}
	}

	if (!j)
	{
		// There are non-digit characters, gather them as is.
		j = 1;
		k = 0;
		for (i = pos; j && str[i] != '\0'; ++i)
		{
			char c = str[i];

			if (!isprint(c))
			{
				j = 0;
				break;
			}

			buf[k++] = c;
		}
	}

	if (!j)
	{
		// The characters are not there or are not printable.
		k = 0;
	}

	buf[k] = '\0';

	if (flip)
		// Flip adjacent characters
		for (j = 0; j < k; j += 2)
		{
			char t = buf[j];
			buf[j] = buf[j + 1];
			buf[j + 1] = t;
		}

	// Trim any beginning and end space
	i = j = -1;
	for (k = 0; buf[k] != '\0'; ++k)
	{
		if (!isspace(buf[k]))
		{
			if (i < 0)
				i = k;
			j = k;
		}
	}

	if ((i >= 0) && (j >= 0))
	{
		for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
			buf[k - i] = buf[k];
		buf[k - i] = '\0';
	}

	return buf;
}
//---------------------------------------------------------------------------------------------------------------------
//��ȡ�߼���������Ϣ,�������ֻ�ǵ�����ȡ�߼���������Ϣ��������ȥ�ж��Ƿ�����ĳ��������̣���GetLogicalDriveInfo��һ��
//����:��Ż�ȡ��Ϣ�Ľṹ��;�߼��������̷�.
BOOL DiksConn::GetLogicalDriveInfoOnly(LOGICALDRIVEINFO* logicalDriveInfo, char logicalDriveNumber)
{

	char path[128];
	logicalDriveInfo->szDrive = logicalDriveNumber;
	//��ȡ����������
	sprintf_s(path, "%c:\\", logicalDriveNumber);
	logicalDriveInfo->uDriveType = GetDriveType(path);
	//ʹ��GetVolumeInformation��ȡ
	if (!GetVolumeInformation(
		path,
		logicalDriveInfo->szDriveName,
		sizeof(logicalDriveInfo->szDriveName),
		&logicalDriveInfo->dwSerialNumber,
		&logicalDriveInfo->dwMaximumComponentLength,
		&logicalDriveInfo->dwFileSystemFlags,
		logicalDriveInfo->dwFileSystem,
		sizeof(logicalDriveInfo->dwFileSystem)
	))
	{
		return FALSE;
	}
	//ʹ��GetDiskFreeSpace��ȡ��Ϣ
	if (!GetDiskFreeSpace(
		path,
		&logicalDriveInfo->dwSectorsPerCluster,
		&logicalDriveInfo->dwBytesPerSector,
		&logicalDriveInfo->dwNumberOfFreeCluters,
		&logicalDriveInfo->dwTotalNumberOfClusters
	))
	{
		return FALSE;
	}
	return TRUE;


	return 0;
}

BOOL DiksConn::GetLogicalDriveInfo(LOGICALDRIVEINFO* logicalDriveInfo, char logicalDriveNumber, int phyDriveNumber)
{

	HANDLE hDrive;
	char path[128];
	STORAGE_DEVICE_NUMBER number;   //use this to get disk numbers
	DWORD readed;                   // discard results

	sprintf_s(path, "\\\\.\\%c:", logicalDriveNumber);
	hDrive = CreateFile(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDrive == INVALID_HANDLE_VALUE) // cannot open the drive
	{
		CloseHandle(hDrive);
		return FALSE;
	}
	if (DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &number, sizeof(number), &readed, NULL))
	{
		//�����ͬһ�����̣��򷵻�FALSE
		if ((number.DeviceNumber != phyDriveNumber) || phyDriveNumber < 0)
		{
			CloseHandle(hDrive);
			return FALSE;
		}
	}
	//OK,�������������������������ͬһ������,��ʼ��ȡ�߼���������Ϣ
	logicalDriveInfo->szDrive = logicalDriveNumber;
	//��ȡ����������
	sprintf_s(path, "%c:\\", logicalDriveNumber);
	logicalDriveInfo->uDriveType = GetDriveType(path);
	//ʹ��GetVolumeInformation��ȡ
	if (!GetVolumeInformation(
		path,
		logicalDriveInfo->szDriveName,
		sizeof(logicalDriveInfo->szDriveName),
		&logicalDriveInfo->dwSerialNumber,
		&logicalDriveInfo->dwMaximumComponentLength,
		&logicalDriveInfo->dwFileSystemFlags,
		logicalDriveInfo->dwFileSystem,
		sizeof(logicalDriveInfo->dwFileSystem)
	))
	{
		CloseHandle(hDrive);
		return FALSE;
	}
	//ʹ��GetDiskFreeSpace��ȡ��Ϣ
	if (!GetDiskFreeSpace(
		path,
		&logicalDriveInfo->dwSectorsPerCluster,
		&logicalDriveInfo->dwBytesPerSector,
		&logicalDriveInfo->dwNumberOfFreeCluters,
		&logicalDriveInfo->dwTotalNumberOfClusters
	))
	{
		CloseHandle(hDrive);
		return FALSE;
	}
	CloseHandle(hDrive);
	return TRUE;



}
//------------------------------------------------------------------------------------
//����Tree��.
//����:��ӵ�TreeView���;��ӵ��ַ���;���ڵ�;�Ƿ�Ϊ�ļ���
HTREEITEM DiksConn::AddItemToTree(HWND hwndTV, LPSTR lpszItem, HTREEITEM hParent, BOOL bFolder)
{
	TVITEM tvi;
	//TVITEM tParent;
	TVINSERTSTRUCT tvins;
	HTREEITEM hme;
	// ���TVITEM�ṹ
	// ͨ��TVITEM��mask�ֶΣ�˵����Щ�ֶ�����Ч��
	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

	// �ֽڵ��ı����ı�����
	tvi.pszText = lpszItem;
	tvi.cchTextMax = sizeof(tvi.pszText) / sizeof(tvi.pszText[0]);

	// ����bFolder����Ϊ�ڵ�ָ����ͬ��ͼ��
	if (bFolder)
	{
		tvi.iImage = 0;
		tvi.iSelectedImage = 1;
		tvi.cChildren = I_CHILDRENCALLBACK;
	}
	else
	{
		tvi.iImage = 2;
		tvi.iSelectedImage = 2;
		tvi.cChildren = 0;
	}

	// ���TVINSERTSTRUCT�ṹ
	tvins.item = tvi;
	// �²�����ӽڵ��λ�ã�TVI_SORT��ʾ���ı�����
	// ��������TVI_FIRST TVI_LAST�ȡ�	
	tvins.hInsertAfter = TVI_SORT;

	// ���hParentΪNULL�������Ľڵ�Ϊ���ڵ�;����hParentΪ�丸�ڵ�
	if (hParent == NULL)
	{
		tvins.hParent = TVI_ROOT;
	}
	else
	{
		tvins.hParent = hParent;
	}
	// ����TreeView_InsertItem�꣬�����½ڵ�
	hme = TreeView_InsertItem(hwndTV, &tvins);
	return hme;

}

mydiskinfo DiksConn::myStringSP(CString hddname)
{
	mydiskinfo info{};
	if (!hddname.IsEmpty())
	{
		CString index = hddname.Left(4);

		//info.index = hddname.Left(1);
		int fast = hddname.Find(_T("("));
		int end = hddname.Find(_T(")"));

		//m_prot.SetWindowText(hddname.Left(1));
		info.index = hddname.Left(1);

		//CString ok = hddname.Mid(2, fast - 2);
		//m_mode.SetWindowText(ok);
		info.szDriveName = hddname.Mid(2, fast - 2);

		//m_size.SetWindowText(hddname.Mid(fast + 1, end - fast - 4));
		info.dwTotalNumberOfClusters = hddname.Mid(fast + 1, end - fast - 4);

	}
	return info;
	
	
}

void DiksConn::IsDiskInfo()
{
	DWORD tempDrive;
	LOGICALDRIVEINFO theLogicalDriveInfo;
	tempDrive = GetLogicalDrives();
	char tempChar[128] = "";
	//LOGICALDRIVEINFO logicalDrive;

	//char szBuff[100];   //��ʱ��������
	PHYDRIVEINFOSHORT thePhyDIS;  //��̵�������������Ϣ
	int nDrive = 0;    //����ͳ���ж��ٸ�����������

	/*HTREEITEM tree1 = m_diskTree.InsertItem(_T("��1"));
	HTREEITEM tree2 = m_diskTree.InsertItem(_T("��2"));
	HTREEITEM tree3 = m_diskTree.InsertItem(_T("��3"));*/

	/*m_diskTree.InsertItem(_T("��1 ������"), tree1);
	m_diskTree.InsertItem(_T("��2 ������"), tree2);
	m_diskTree.InsertItem(_T("��3 ������"), tree3);*/

	while (GetPhysicalDriveInfoShort(&thePhyDIS, nDrive))
	{
		//����Ҫ��ʾ���ַ���,��������������,�ͺ�,����
		//sprintf_s(szBuff, "%d %s  %s (%I64d GB)", nDrive, thePhyDIS.modelNumber, thePhyDIS.serialNumber, thePhyDIS.size / 1000 / 1000 / 1000);
		//SendMessageA(hwndComboBo x,CB_ADDSTRING,0,(LPARAM)&szBuff);
		//����һ����Ϣ��Ӳ���б�,���һ����Ϣ
		//SendMessageA(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)szBuff);
		// 
		// 
		// 
		//m_diskTree.InsertItem(szBuff);
		for (int i = 0; i < 31; i++, tempDrive = tempDrive >> 1)
		{
			//tempDrive&1��ʾ��ǰ�����������ڣ��������ڵ��ú�����ȡ���һ�ȡ�ɹ��˲Ų���
			if (tempDrive & 1 && GetLogicalDriveInfoOnly(&theLogicalDriveInfo, 'A' + i))
			{
				sprintf_s(tempChar, "%c:  %s   %s   (%.2f GB)   ",
					theLogicalDriveInfo.szDrive,
					strlen(theLogicalDriveInfo.szDriveName) == 0 ? "���ش���" : theLogicalDriveInfo.szDriveName,
					theLogicalDriveInfo.dwFileSystem,
					(__int64)theLogicalDriveInfo.dwTotalNumberOfClusters * (__int64)theLogicalDriveInfo.dwSectorsPerCluster * (__int64)theLogicalDriveInfo.dwBytesPerSector / 1024 / 1024 / 1024.00);
				//SendMessageA(hwndMainCBLogicalDrive, CB_ADDSTRING, 0, (LPARAM)tempChar);
				//m_diskTree.InsertItem(tempChar);

				if (GetLogicalDriveInfo(&theLogicalDriveInfo, theLogicalDriveInfo.szDrive, nDrive))
				{

					printf("OK");
				}


			}
		}


		nDrive++;
	}

	


}
