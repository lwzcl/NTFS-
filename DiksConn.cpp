//#include "pch.h"
#include "DiksConn.h"

BOOL DiksConn::GetPhysicalDriveInfoShort(PHYDRIVEINFOSHORT* pThePhyDriveInfo, int iDrive)
{
	HANDLE hPhysicalDriveIOCTL = 0;
	BOOL OK = FALSE;

	TCHAR driveName[32];

	_stprintf_s(driveName, _T("\\\\.\\PhysicalDrive%d"), iDrive);

	// 打开设备，获得物理磁盘句柄
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

			//转化字符串
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
				pThePhyDriveInfo->fixed = (geom->Geometry.MediaType == FixedMedia);   //媒体类型，即Fixed，remoble，unknown
				pThePhyDriveInfo->size = geom->DiskSize.QuadPart;           //大小

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
//获取逻辑驱动器信息,这个函数只是单纯获取逻辑驱动器信息，并不会去判断是否属于某个物理磁盘，和GetLogicalDriveInfo不一样
//参数:存放获取信息的结构体;逻辑驱动器盘符.
BOOL DiksConn::GetLogicalDriveInfoOnly(LOGICALDRIVEINFO* logicalDriveInfo, char logicalDriveNumber)
{

	char path[128];
	logicalDriveInfo->szDrive = logicalDriveNumber;
	//获取驱动器类型
	sprintf_s(path, "%c:\\", logicalDriveNumber);
	logicalDriveInfo->uDriveType = GetDriveType(path);
	//使用GetVolumeInformation获取
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
	//使用GetDiskFreeSpace获取信息
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
		//如果不同一个磁盘，则返回FALSE
		if ((number.DeviceNumber != phyDriveNumber) || phyDriveNumber < 0)
		{
			CloseHandle(hDrive);
			return FALSE;
		}
	}
	//OK,有这个物理驱动器，而且属于同一个磁盘,开始获取逻辑驱动器信息
	logicalDriveInfo->szDrive = logicalDriveNumber;
	//获取驱动器类型
	sprintf_s(path, "%c:\\", logicalDriveNumber);
	logicalDriveInfo->uDriveType = GetDriveType(path);
	//使用GetVolumeInformation获取
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
	//使用GetDiskFreeSpace获取信息
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
//添加项到Tree中.
//参数:添加到TreeView句柄;添加的字符串;父节点;是否为文件夹
HTREEITEM DiksConn::AddItemToTree(HWND hwndTV, LPSTR lpszItem, HTREEITEM hParent, BOOL bFolder)
{
	TVITEM tvi;
	//TVITEM tParent;
	TVINSERTSTRUCT tvins;
	HTREEITEM hme;
	// 填充TVITEM结构
	// 通过TVITEM的mask字段，说明哪些字段是有效的
	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

	// 字节的文本及文本长度
	tvi.pszText = lpszItem;
	tvi.cchTextMax = sizeof(tvi.pszText) / sizeof(tvi.pszText[0]);

	// 根据bFolder参数为节点指定不同的图标
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

	// 填充TVINSERTSTRUCT结构
	tvins.item = tvi;
	// 新插入的子节点的位置，TVI_SORT表示按文本排序
	// 还可以是TVI_FIRST TVI_LAST等。	
	tvins.hInsertAfter = TVI_SORT;

	// 如果hParent为NULL，则插入的节点为根节点;否则hParent为其父节点
	if (hParent == NULL)
	{
		tvins.hParent = TVI_ROOT;
	}
	else
	{
		tvins.hParent = hParent;
	}
	// 调用TreeView_InsertItem宏，插入新节点
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

	//char szBuff[100];   //临时辅助变量
	PHYDRIVEINFOSHORT thePhyDIS;  //简短的物理驱动器信息
	int nDrive = 0;    //用来统计有多少个物理驱动器

	/*HTREEITEM tree1 = m_diskTree.InsertItem(_T("根1"));
	HTREEITEM tree2 = m_diskTree.InsertItem(_T("根2"));
	HTREEITEM tree3 = m_diskTree.InsertItem(_T("根3"));*/

	/*m_diskTree.InsertItem(_T("根1 的了项"), tree1);
	m_diskTree.InsertItem(_T("根2 的了项"), tree2);
	m_diskTree.InsertItem(_T("根3 的了项"), tree3);*/

	while (GetPhysicalDriveInfoShort(&thePhyDIS, nDrive))
	{
		//构造要显示的字符串,包括驱动器名称,型号,容量
		//sprintf_s(szBuff, "%d %s  %s (%I64d GB)", nDrive, thePhyDIS.modelNumber, thePhyDIS.serialNumber, thePhyDIS.size / 1000 / 1000 / 1000);
		//SendMessageA(hwndComboBo x,CB_ADDSTRING,0,(LPARAM)&szBuff);
		//发送一个消息到硬盘列表,添加一条信息
		//SendMessageA(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)szBuff);
		// 
		// 
		// 
		//m_diskTree.InsertItem(szBuff);
		for (int i = 0; i < 31; i++, tempDrive = tempDrive >> 1)
		{
			//tempDrive&1表示当前的驱动器存在，存在了在调用函数获取，且获取成功了才插入
			if (tempDrive & 1 && GetLogicalDriveInfoOnly(&theLogicalDriveInfo, 'A' + i))
			{
				sprintf_s(tempChar, "%c:  %s   %s   (%.2f GB)   ",
					theLogicalDriveInfo.szDrive,
					strlen(theLogicalDriveInfo.szDriveName) == 0 ? "本地磁盘" : theLogicalDriveInfo.szDriveName,
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
