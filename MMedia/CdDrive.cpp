// CdDrive.cpp: implementation of the CCdDrive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "WaveSoapFront.h"
#include "CdDrive.h"
#include <Setupapi.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CCdDrive::LoadAspi()
{
	//m_hWinaspi32 = LoadLibrary(_T("wnaspi32.dll"));
	if (NULL != m_hWinaspi32)
	{
		GetASPI32DLLVersion = (DWORD (_cdecl * )())
							GetProcAddress(m_hWinaspi32, "GetASPI32DLLVersion");

		GetASPI32SupportInfo = (DWORD (_cdecl * )())
								GetProcAddress(m_hWinaspi32, "GetASPI32SupportInfo");

		SendASPI32Command = (DWORD (_cdecl * )(SRB * ))
							GetProcAddress(m_hWinaspi32, "SendASPI32Command");

		GetAspi32Buffer = (GETASPI32BUFFER)
						GetProcAddress(m_hWinaspi32, "GetASPI32Buffer");

		FreeAspi32Buffer = (FREEASPI32BUFFER)
							GetProcAddress(m_hWinaspi32, "FreeASPI32Buffer");

		TranslateAspi32Address = (TRANSLATEASPI32ADDRESS)
								GetProcAddress(m_hWinaspi32, "TranslateAspi32Address");

		GetAspi32DriveLetter = NULL;

		GetAspi32HaTargetLun = NULL;

		if (NULL == GetASPI32DLLVersion
			|| NULL == GetASPI32SupportInfo
			|| NULL == SendASPI32Command
			|| 1 != ((0xFF00 & GetASPI32SupportInfo()) >> 8))
		{
			SendASPI32Command = NULL;
			GetASPI32SupportInfo = NULL;
			GetASPI32DLLVersion = NULL;

			FreeLibrary(m_hWinaspi32);
			m_hWinaspi32 = NULL;
		}
	}

	if (NULL == m_hWinaspi32)
	{
		m_hWinaspi32 = LoadLibrary(_T("cdral.dll"));
		if (NULL != m_hWinaspi32)
		{
			GetASPI32DLLVersion = (DWORD (_cdecl * )())
								GetProcAddress(m_hWinaspi32, "GetASPI32DLLVersion");

			GetASPI32SupportInfo = (DWORD (_cdecl * )())
									GetProcAddress(m_hWinaspi32, "GetASPI32SupportInfo");

			SendASPI32Command = (DWORD (_cdecl * )(SRB * ))
								GetProcAddress(m_hWinaspi32, "SendASPI32Command");

			GetAspi32Buffer = (GETASPI32BUFFER)
							GetProcAddress(m_hWinaspi32, "GetASPI32Buffer");

			FreeAspi32Buffer = (FREEASPI32BUFFER)
								GetProcAddress(m_hWinaspi32, "FreeASPI32Buffer");

			TranslateAspi32Address = (TRANSLATEASPI32ADDRESS)
									GetProcAddress(m_hWinaspi32, "TranslateAspi32Address");

			GetAspi32DriveLetter = (GETASPI32DRIVELETTER)
									GetProcAddress(m_hWinaspi32, "GetASPI32DriveLetter");

			GetAspi32HaTargetLun = (GETASPI32HATARGETLUN)
									GetProcAddress(m_hWinaspi32, "GetASPI32HaTargetLun");

			if (NULL == GetASPI32DLLVersion
				|| NULL == GetASPI32SupportInfo
				|| NULL == SendASPI32Command
				|| 1 != ((0xFF00 & GetASPI32SupportInfo()) >> 8))
			{
				SendASPI32Command = NULL;
				GetASPI32SupportInfo = NULL;
				GetASPI32DLLVersion = NULL;

				FreeLibrary(m_hWinaspi32);
				m_hWinaspi32 = NULL;
			}
		}
	}
}

CCdDrive::CCdDrive(CCdDrive const & Drive, BOOL UseAspi)
	: m_hDrive(NULL),
	m_hDriveAttributes(NULL),
	m_DriveLetter(0),
	m_hWinaspi32(NULL),

	GetASPI32DLLVersion(NULL),
	GetASPI32SupportInfo(NULL),
	SendASPI32Command(NULL),
	GetAspi32Buffer(NULL),
	FreeAspi32Buffer(NULL),
	TranslateAspi32Address(NULL),
	GetAspi32DriveLetter(NULL),
	GetAspi32HaTargetLun(NULL),
	m_MediaChangeCount(-1),
	m_bMediaChangeNotificationDisabled(false),
	m_bDoorLocked(false)
{
	if (UseAspi)
	{
		LoadAspi();
	}
	*this = Drive;
}

CCdDrive & CCdDrive::operator =(CCdDrive const & Drive)
{
	m_DriveLetter = Drive.m_DriveLetter;
	m_MediaChangeCount = Drive.m_MediaChangeCount;
	m_ScsiAddr = Drive.m_ScsiAddr;

	DuplicateHandle(GetCurrentProcess(), Drive.m_hDrive,
					GetCurrentProcess(), & m_hDrive,
					0, FALSE, DUPLICATE_SAME_ACCESS);
	DuplicateHandle(GetCurrentProcess(), Drive.m_hDriveAttributes,
					GetCurrentProcess(), & m_hDriveAttributes,
					0, FALSE, DUPLICATE_SAME_ACCESS);
	return *this;
}

CCdDrive::CCdDrive(BOOL UseAspi)
	: m_hDrive(NULL),
	m_hDriveAttributes(NULL),
	m_DriveLetter(0),
	m_hWinaspi32(NULL),
	GetASPI32DLLVersion(NULL),
	GetASPI32SupportInfo(NULL),
	SendASPI32Command(NULL),
	GetAspi32Buffer(NULL),
	FreeAspi32Buffer(NULL),
	TranslateAspi32Address(NULL),
	GetAspi32DriveLetter(NULL),
	GetAspi32HaTargetLun(NULL),
	m_MaxTransferSize(0x10000),
	m_BufferAlignment(1),
	m_MediaChangeCount(-1),
	m_bMediaChangeNotificationDisabled(false),
	m_bDoorLocked(false)
{
	memzero(m_ScsiAddr);
	if (UseAspi)
	{
		LoadAspi();
	}
#if 0//def _DEBUG
	SetLastError(0);
	HDEVINFO di = SetupDiGetClassDevs( & GUID_DEVINTERFACE_CDROM,
										NULL, AfxGetMainWnd()->m_hWnd, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	TRACE("HDEVINFO=%X, last error=%d\n", di, GetLastError());

	for (int i = 0; ; i++)
	{
		SP_DEVICE_INTERFACE_DATA spdid;
		spdid.cbSize = sizeof spdid;
		spdid.InterfaceClassGuid = GUID_DEVINTERFACE_CDROM;

		if ( ! SetupDiEnumDeviceInterfaces(di,
											NULL, & GUID_DEVINTERFACE_CDROM,
											i, & spdid))
		{
			TRACE("No more devices, last error=%d\n", GetLastError());
			break;
		}
		TRACE("CDROM Device interface found!\n");
		struct IntDetailData: SP_DEVICE_INTERFACE_DETAIL_DATA
		{
			TCHAR MorePath[256];
		} DetailData;
		DetailData.cbSize = sizeof SP_DEVICE_INTERFACE_DETAIL_DATA;
		DetailData.DevicePath[0] = 0;
		DetailData.DevicePath[1] = 0;
		DWORD FilledSize = 0;;
		SP_DEVINFO_DATA sdi;
		sdi.cbSize = sizeof sdi;

		SetupDiGetDeviceInterfaceDetail(di, &spdid,
										NULL, 0, & FilledSize, NULL);
		TRACE("Required data size=%d\n", FilledSize);

		if ( ! SetupDiGetDeviceInterfaceDetail(di, & spdid,
												& DetailData, sizeof DetailData, NULL, NULL))
		{
			TRACE("SetupDiGetDeviceInterfaceDetail failed, error=%d\n", GetLastError());
			continue;
		}

		HANDLE hFile = CreateFile(DetailData.DevicePath,
								GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL, OPEN_EXISTING,
								0, // flags
								NULL);
		if (NULL == hFile || INVALID_HANDLE_VALUE == hFile)
		{
			TRACE("Failed to open device for READ access, error=%d\n", GetLastError());
		}
		else
		{
			TRACE("Opened device for READ access\n");
			CloseHandle(hFile);
		}
		hFile = CreateFile(DetailData.DevicePath,
							0, FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL, OPEN_EXISTING,
							0, // flags
							NULL);
		if (NULL == hFile || INVALID_HANDLE_VALUE == hFile)
		{
			TRACE("Failed to open device for ATTRIBUTE access, error=%d\n", GetLastError());
		}
		else
		{
			TRACE("Opened device for ATTRIBUTE access\n");
			CloseHandle(hFile);
		}

		hFile = CreateFile(DetailData.DevicePath,
							GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL, OPEN_EXISTING,
							0, // flags
							NULL);
		if (NULL == hFile || INVALID_HANDLE_VALUE == hFile)
		{
			TRACE("Failed to open device for WRITE access, error=%d\n", GetLastError());
		}
		else
		{
			TRACE("Opened device for WRITE access\n");
			CloseHandle(hFile);
		}

		hFile = CreateFile(DetailData.DevicePath,
							GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL, OPEN_EXISTING,
							0, // flags
							NULL);
		if (NULL == hFile || INVALID_HANDLE_VALUE == hFile)
		{
			TRACE("Failed to open device for READ/WRITE access, error=%d\n", GetLastError());
		}
		else
		{
			TRACE("Opened device for READ/WRITE access\n");
			CloseHandle(hFile);
		}

		hFile = CreateFile(DetailData.DevicePath,
							MAXIMUM_ALLOWED, FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL, OPEN_EXISTING,
							0, // flags
							NULL);
		if (NULL == hFile || INVALID_HANDLE_VALUE == hFile)
		{
			TRACE("Failed to open device for MAXIMIM_ALLOWED access, error=%d\n", GetLastError());
		}
		else
		{
			TRACE("Opened device for MAXIMIM_ALLOWED access\n");
			DWORD bytes;

			BOOL res = DeviceIoControl(hFile, IOCTL_SCSI_GET_ADDRESS,
										NULL, 0,
										& m_ScsiAddr, sizeof m_ScsiAddr,
										& bytes, NULL);

			struct OP : SCSI_PASS_THROUGH, SCSI_SenseInfo
			{
				char CdConfig[1024];
			} spt;
			spt.Length = sizeof spt;
			spt.PathId = m_ScsiAddr.PathId;
			spt.TargetId = m_ScsiAddr.TargetId;
			spt.Lun = m_ScsiAddr.Lun;
			spt.SenseInfoLength = sizeof SCSI_SenseInfo;
			spt.SenseInfoOffset = sizeof SCSI_PASS_THROUGH;

			GetConfigurationCDB cdb(sizeof spt.CdConfig);

			memcpy(spt.Cdb, & cdb, sizeof cdb);

			spt.CdbLength = sizeof cdb;
			spt.DataIn = FALSE;

			spt.DataTransferLength = sizeof spt.CdConfig;
			spt.DataBufferOffset = sizeof SCSI_PASS_THROUGH + sizeof SCSI_SenseInfo;
			spt.TimeOutValue = 1000;

			res = DeviceIoControl(hFile, IOCTL_SCSI_PASS_THROUGH,
								& spt, sizeof spt,
								& spt, sizeof spt,
								& bytes, NULL);

			TRACE("IOCTL_SCSI_PASS_THROUGH returned %d, error=%d\n", res, GetLastError());

			CloseHandle(hFile);
		}
	}
	SetupDiDestroyDeviceInfoList(di);
#endif
}

CCdDrive::~CCdDrive()
{
	Close();
	if (NULL != m_hWinaspi32)
	{
		FreeLibrary(m_hWinaspi32);
		m_hWinaspi32 = NULL;
	}
}

BOOL CCdDrive::Open(TCHAR letter)
{
	BOOL res;
	Close();
	CString path;
	path.Format("\\\\.\\%c:", letter);

	// access MAXIMUM_ALLOWED is necessary to have READ_ATTRIBUTES privilege
	HANDLE Drive = CreateFile(
							path,
							MAXIMUM_ALLOWED,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							0,
							NULL);

	if (INVALID_HANDLE_VALUE == Drive || NULL == Drive)
	{
		return FALSE;
	}

	m_hDrive = Drive;   // we will needit

	Drive = CreateFile(
						path,
						0,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);

	if (INVALID_HANDLE_VALUE == Drive || NULL == Drive)
	{
		Close();
		return FALSE;
	}

	m_hDriveAttributes = Drive;   // we will needit
	m_MediaChangeCount = -1;

	m_DriveLetter = letter;

	DWORD bytes =0;
	res = DeviceIoControl(m_hDrive, IOCTL_SCSI_GET_ADDRESS,
						NULL, 0,
						& m_ScsiAddr, sizeof m_ScsiAddr,
						& bytes, NULL);

	if ( ! res)
	{
		Close();
		return FALSE;
	}
	// find its equivalent in ASPI. The adapter number is supposed to be the same.
	// use either ASPI or IOCTL to find max transfer length
	IO_SCSI_CAPABILITIES ScsiCaps;
	memzero(ScsiCaps);
	memzero(m_ScsiAddr);

	if (NULL != GetAspi32HaTargetLun)
	{
		HaTargetLun Addr = GetAspi32HaTargetLun(letter);
		TRACE("Drive %c SCSI addr returned by cdral=%08X\n",
			Addr);
		m_ScsiAddr.PortNumber = Addr.HaId;
		m_ScsiAddr.Lun = Addr.Lun;
		m_ScsiAddr.TargetId = Addr.TargetId;
	}
	else
	{
		// WINASPI is used or IOCTL_SCSI_PASS_THROUGH
		res = DeviceIoControl(m_hDrive, IOCTL_SCSI_GET_ADDRESS,
							NULL, 0,
							& m_ScsiAddr, sizeof m_ScsiAddr,
							& bytes, NULL);
	}

	res = DeviceIoControl(m_hDrive, IOCTL_SCSI_GET_CAPABILITIES,
						NULL, 0,
						& ScsiCaps, sizeof ScsiCaps,
						& bytes, NULL);

	m_MaxTransferSize = ScsiCaps.MaximumTransferLength;
	m_BufferAlignment = ScsiCaps.AlignmentMask;

	TRACE("MaxTransferSize = %d, buffer alignment = %x, \n",
		m_MaxTransferSize, m_BufferAlignment
		);

	return TRUE;
}

void CCdDrive::Close()
{
	if (NULL != m_hDriveAttributes)
	{
		UnlockDoor();
		EnableMediaChangeDetection();
		CloseHandle(m_hDriveAttributes);
		m_hDriveAttributes = NULL;
	}

	if (NULL != m_hDrive)
	{
		CloseHandle(m_hDrive);
		m_hDrive = NULL;
	}

	m_DriveLetter = 0;
}

BOOL CCdDrive::EnableMediaChangeDetection()
{
	if ( ! m_bMediaChangeNotificationDisabled)
	{
		return TRUE;
	}

	if (NULL == m_hDriveAttributes)
	{
		return FALSE;
	}
	DWORD BytesReturned;
	BOOLEAN McnDisable = FALSE;

	BOOL res = DeviceIoControl(m_hDriveAttributes,IOCTL_STORAGE_MCN_CONTROL,
								& McnDisable, sizeof McnDisable,
								NULL, 0,
								&BytesReturned, NULL);
	m_bMediaChangeNotificationDisabled = false;

	return TRUE;
}

BOOL CCdDrive::DisableMediaChangeDetection()
{
	if (m_bMediaChangeNotificationDisabled)
	{
		return TRUE;
	}

	if (NULL == m_hDriveAttributes)
	{
		return FALSE;
	}
	DWORD BytesReturned;
	BOOLEAN McnDisable = TRUE;

	BOOL res = DeviceIoControl(m_hDriveAttributes, IOCTL_STORAGE_MCN_CONTROL,
								& McnDisable, sizeof McnDisable,
								NULL, 0,
								&BytesReturned, NULL);
	TRACE("DisableMediaChangeDetection IOCTL returned %d\n", res);
	m_bMediaChangeNotificationDisabled = true;

	return TRUE;
}

BOOL CCdDrive::LockDoor()
{
	if (m_bDoorLocked)
	{
		return TRUE;
	}

	if (NULL == m_hDriveAttributes)
	{
		return FALSE;
	}
	DWORD BytesReturned;
	BOOLEAN LockMedia = TRUE;

	BOOL res = DeviceIoControl(m_hDriveAttributes, IOCTL_STORAGE_EJECTION_CONTROL,
								& LockMedia, sizeof LockMedia,
								NULL, 0,
								&BytesReturned, NULL);

	m_bDoorLocked = true;

	return TRUE;
}

BOOL CCdDrive::UnlockDoor()
{
	if ( ! m_bDoorLocked)
	{
		return TRUE;
	}

	if (NULL == m_hDriveAttributes)
	{
		return FALSE;
	}

	DWORD BytesReturned;
	BOOLEAN LockMedia = FALSE;

	BOOL res = DeviceIoControl(m_hDriveAttributes, IOCTL_STORAGE_EJECTION_CONTROL,
								& LockMedia, sizeof LockMedia,
								NULL, 0,
								&BytesReturned, NULL);

	m_bDoorLocked = false;

	return TRUE;
}

BOOL CCdDrive::SendScsiCommand(CD_CDB * pCdb,
								void * pData, DWORD * pDataLen,
								int DataDirection,
								SCSI_SenseInfo * pSense)  // SCSI_IOCTL_DATA_IN, SCSI_IOCTL_DATA_OUT,
{
	// issue IOCTL_SCSI_PASS_THROUGH_DIRECT or IOCTL_SCSI_PASS_THROUGH
	int CdbLength;
	DWORD bytes;

	switch (pCdb->Opcode >> 5)  // command group
	{
	case 0:
		CdbLength = 6;
		break;
	case 1:
	case 2:
		CdbLength = 10;
		break;
	case 4:
		CdbLength = 16;
		break;
	case 5:
		CdbLength = 12;
		break;
	case 3:
	case 6:
	case 7:
		CdbLength = 16; // unknown??
		break;
	}
	if (NULL != m_hWinaspi32)
	{
		SRB_ExecSCSICmd srb;
		HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		memset( & srb, 0, sizeof srb);
		srb.Command = SC_EXEC_SCSI_CMD;
		srb.HostAdapter = m_ScsiAddr.PortNumber;
		srb.Target = m_ScsiAddr.TargetId;
		srb.Lun = m_ScsiAddr.Lun;

		srb.hCompletionEvent = hEvent;
		srb.Flags |= SRB_EVENT_NOTIFY;

		if (SCSI_IOCTL_DATA_IN == DataDirection)
		{
			// to host
			srb.Flags |= SRB_DIR_IN;
		}
		else if (SCSI_IOCTL_DATA_OUT == DataDirection)
		{
			// from host
			srb.Flags |= SRB_DIR_OUT;
		}

		srb.BufLen = * pDataLen;
		srb.BufPointer = pData;
		srb.SenseLen = sizeof srb.SenseInfo;
		srb.CDBLen = CdbLength;
		memcpy(& srb.CDBByte, pCdb, CdbLength);

		DWORD status = SendASPI32Command( & srb);
		if (SS_PENDING == status)
		{
			TRACE("Scsi request pending\n");
			WaitForSingleObject(hEvent, INFINITE);
		}
		CloseHandle(hEvent);
		if (pSense)
		{
			memcpy(pSense, & srb.SenseInfo, sizeof * pSense);
		}

		return SS_COMP == srb.Status;
	}
	else
	{
		// try to use IOCTL_SCSI_PASS_TROUGH or
		// IOCTL_SCSI_PASS_TROUGH_DIRECT
		if (*pDataLen < 256)
		{
			// use IOCTL_SCSI_PASS_TROUGH
			struct OP : SCSI_PASS_THROUGH, SCSI_SenseInfo
			{
				char MoreBuffer[256];
			} spt;

			spt.Length = sizeof SCSI_PASS_THROUGH;
			spt.PathId = m_ScsiAddr.PathId;
			spt.TargetId = m_ScsiAddr.TargetId;
			spt.Lun = m_ScsiAddr.Lun;

			spt.SenseInfoLength = sizeof SCSI_SenseInfo;
			spt.SenseInfoOffset = sizeof SCSI_PASS_THROUGH;

			memcpy(spt.Cdb, pCdb, CdbLength);

			spt.CdbLength = CdbLength;
			spt.DataIn = SCSI_IOCTL_DATA_IN == DataDirection;

			spt.DataTransferLength = * pDataLen;
			spt.DataBufferOffset = offsetof (OP, MoreBuffer);
			spt.TimeOutValue = 1000;

			BOOL res = DeviceIoControl(m_hDrive, IOCTL_SCSI_PASS_THROUGH,
										& spt, sizeof spt,
										& spt, sizeof spt,
										& bytes, NULL);

			TRACE("IOCTL_SCSI_PASS_THROUGH returned %d, error=%d\n", res, GetLastError());

			if (res && pSense)
			{
				memcpy(pSense, (SCSI_SenseInfo *) & spt, sizeof * pSense);
			}

			return res && SS_COMP == spt.ScsiStatus;
		}
		else
		{
		}
		return FALSE;
	}

}

BOOL CCdDrive::ScsiInquiry(SRB_HAInquiry * pInq)
{
	pInq->Command = SC_HA_INQUIRY;
	pInq->Flags = 0;
	pInq->HostAdapter = m_ScsiAddr.PortNumber;
	DWORD status = SendASPI32Command(pInq);
	return status == SS_COMP;
}

BOOL CCdDrive::ReadToc(CDROM_TOC * pToc)
{
	DWORD dwReturned = 0;
	BOOL res = DeviceIoControl(m_hDrive, IOCTL_CDROM_READ_TOC,
								NULL, 0,
								pToc, sizeof *pToc,
								& dwReturned,
								NULL);

	TRACE("Get TOC IoControl returned %x, bytes: %d, last error = %d, First track %d, last track: %d, Length:%02X%02X\n",
		res, dwReturned, GetLastError(),
		pToc->FirstTrack, pToc->LastTrack, pToc->Length[1], pToc->Length[0]);
	return res;
}

BOOL CCdDrive::ReadSessions(CDROM_TOC * pToc)
{
	DWORD dwReturned = 0;
	BOOL res = DeviceIoControl(m_hDrive, IOCTL_CDROM_GET_LAST_SESSION,
								NULL, 0,
								pToc, sizeof *pToc,
								& dwReturned,
								NULL);

	TRACE("Get Last Session IoControl returned %x, bytes: %d, last error = %d, First track %d, last track: %d, Length:%02X%02X\n",
		res, dwReturned, GetLastError(),
		pToc->FirstTrack, pToc->LastTrack, pToc->Length[1], pToc->Length[0]);
	return res;
}

CdMediaChangeState CCdDrive::CheckForMediaChange()
{
	if (NULL != m_hDrive)
	{
		DWORD MediaChangeCount = 0;
		DWORD BytesReturned;
		DWORD res = DeviceIoControl(m_hDrive,
									IOCTL_STORAGE_CHECK_VERIFY2,
									NULL, 0,
									& MediaChangeCount, sizeof MediaChangeCount,
									& BytesReturned, NULL);

		TRACE("GetLastError=%d, MediaChange=%d\n",
			GetLastError(),
			MediaChangeCount);

		if (! res && GetLastError() != ERROR_NOT_READY)
		{
			res = DeviceIoControl(m_hDrive,
								IOCTL_STORAGE_CHECK_VERIFY,
								NULL, 0,
								& MediaChangeCount, sizeof MediaChangeCount,
								& BytesReturned, NULL);
			TRACE("GetLastError=%d, MediaChange=%d\n",
				GetLastError(),
				MediaChangeCount);
		}

		if (! res)
		{
			if (-1 != m_MediaChangeCount)
			{
				m_MediaChangeCount = -1;
				TRACE("device not ready\n");
				return CdMediaStateNotReady;
			}
		}
		else if (MediaChangeCount != m_MediaChangeCount)
		{
			m_MediaChangeCount = MediaChangeCount;
			return CdMediaStateChanged;
		}
		else
		{
			return CdMediaStateSameMedia;
		}
	}
	return CdMediaStateNotReady;

}

int CCdDrive::FindCdDrives(TCHAR Drives['Z' - 'A' + 1])
{
	int NumberOfDrives = 0;
	for (int letter = 'A'; letter <= 'Z'; letter++)
	{
		CString s;
		s.Format("%c:", letter);
		if (DRIVE_CDROM == GetDriveType(s))
		{
			Drives[NumberOfDrives] = letter;
			NumberOfDrives++;
		}
	}
	return NumberOfDrives;
}

DWORD CCdDrive::GetDiskID()
{
	DWORD MaxCompLength, FilesysFlags, DiskId = 0;

	TCHAR root[8];
	sprintf(root, "%c:\\", m_DriveLetter);

	if (GetVolumeInformation(root, NULL, 0, & DiskId,
							& MaxCompLength, & FilesysFlags, NULL, 0))
	{
		TRACE("CD Volume label %08X\n", DiskId);
	}
	return DiskId;
}

BOOL CCdDrive::GetMaxReadSpeed(int * pMaxSpeed) // bytes/s
{
	SCSI_SenseInfo ssi;
	DWORD DataLen;
	BOOL res;
	// try to use Get Performance command
	// if the command is not supported, use Set Speed and get the max speed
	// from mech status mode page (offset 8 / 176.4)
#if 0//def _DEBUG
	InquiryData inq;
	InquiryCDB iqcdb(sizeof inq);
	DataLen = sizeof inq;

	res = SendScsiCommand( & iqcdb, & inq, & DataLen, SCSI_IOCTL_DATA_IN, & ssi);

	struct FEATURE : FeatureHeader
	{
		UCHAR MoreData[1024];
	} feature;

	DataLen = sizeof feature;
	GetConfigurationCDB ConfigCdb(sizeof feature);

	res = SendScsiCommand( & ConfigCdb, & feature, & DataLen, SCSI_IOCTL_DATA_IN, & ssi);


	ProfileListDesc pld;
	GetConfigurationCDB GetProfiles(sizeof pld, 0x1E,
									GetConfigurationCDB::RequestedTypeOneDescriptor);
	DataLen = sizeof pld;
	res = SendScsiCommand( & GetProfiles, & pld, & DataLen, SCSI_IOCTL_DATA_IN, & ssi);

#endif
	struct PERF : CdPerformanceDataHeader
	{
		CdNominalPerformanceDescriptor desc[10];
	} perf;
	GetPerformanceCDB cdb(10, 0);
	DataLen = sizeof perf;

	if (SendScsiCommand( & cdb, & perf, & DataLen, SCSI_IOCTL_DATA_IN, & ssi))
	{
		// check that header is valid
		const DWORD DataLength = perf.DataLength;
		if (DataLength > 10 * sizeof (CdNominalPerformanceDescriptor)
			|| 0 != DataLength % sizeof (CdNominalPerformanceDescriptor)
			|| 0 != DataLength)
		{
			TRACE("Wrong performance data length=%d\n", DataLength);
			return FALSE;
		}
		DWORD MaxPerformance = 0;
		for (int i = 0; i < DataLength / sizeof (CdNominalPerformanceDescriptor); i++)
		{
			DWORD tmp = perf.desc[i].StartPerformance;
			if (MaxPerformance < tmp)
			{
				MaxPerformance = tmp;
			}

			tmp = perf.desc[i].EndPerformance;
			if (MaxPerformance < tmp)
			{
				MaxPerformance = tmp;
			}
		}
		* pMaxSpeed = MaxPerformance * 1024;
		return TRUE;
	}
	else
	{
		// use CD MAE current capabilities page
		// use mech status mode page
		struct CdCaps: ModeInfoHeader, CDCapabilitiesMechStatusModePage
		{
		} cdmp;
		ModeSenseCDB msdb(sizeof cdmp, cdmp.Code);
		DataLen = sizeof cdmp;

		res = SendScsiCommand( & msdb, & cdmp, & DataLen, SCSI_IOCTL_DATA_IN, & ssi);

		if (res)
		{
			* pMaxSpeed = cdmp.MaxReadSpeedSupported * 1000;
			return TRUE;
		}

		struct CdCaps: ModeInfoHeader, CDCurrentCapabilitiesModePage
		{
		} cdcp;
		ModeSenseCDB msdb2(sizeof cdcp, cdcp.Code);
		DataLen = sizeof cdcp;

		res = SendScsiCommand( & msdb2, & cdcp, & DataLen, SCSI_IOCTL_DATA_IN, & ssi);

		if (res)
		{
			* pMaxSpeed = cdcp.MaximumReadSpeed * 1000;
			return TRUE;
		}

		return FALSE;

	}
}

void CCdDrive::StopAudioPlay()
{
	if (NULL != m_hDrive)
	{
		DWORD bytes =0;
		BOOL res = DeviceIoControl(m_hDrive, IOCTL_CDROM_STOP_AUDIO,
									NULL, 0,
									NULL, 0,
									& bytes, NULL);
	}
}

BOOL CCdDrive::ReadCdData(void * pBuf, long Address, int nSectors)
{
	DWORD Length = nSectors * 2352;
	ReadCD_CDB rcd(Address, Length);

	return SendScsiCommand( & rcd, pBuf, & Length,
							SCSI_IOCTL_DATA_IN, NULL);
}

BOOL CCdDrive::ReadCdData(void * pBuf, CdAddressMSF Address, int nSectors)
{
	DWORD Length = nSectors * 2352;
	CdAddressMSF EndAddress;
	EndAddress = LONG(Address) + nSectors;
	ReadCD_MSF_CDB rcd(Address, EndAddress);

	return SendScsiCommand( & rcd, pBuf, & Length,
							SCSI_IOCTL_DATA_IN, NULL);
}

