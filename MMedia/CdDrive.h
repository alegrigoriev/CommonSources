// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
// CdDrive.h: interface for the ICdDrive class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "KInterlocked.h"
#define USE_ASPI_ENABLE  0

#if USE_ASPI_ENABLE
#define SRB_HAInquiry    _SRB_HAInquiry
#define PSRB_HAInquiry   _PSRB_HAInquiry
#define SRB_ExecSCSICmd  _SRB_ExecSCSICmd
#define PSRB_ExecSCSICmd _PSRB_ExecSCSICmd
#define SRB_Abort        _SRB_Abort
#define PSRB_Abort       _PSRB_Abort
#define SRB_BusDeviceReset _SRB_BusDeviceReset
#define PSRB_BusDeviceReset _PSRB_BusDeviceReset
#define SRB_GetDiskInfo     _SRB_GetDiskInfo
#define PSRB_GetDiskInfo    _PSRB_GetDiskInfo

#include "wnaspi32.h"

#undef  SRB_HAInquiry
#undef PSRB_HAInquiry
#undef SRB_ExecSCSICmd
#undef PSRB_ExecSCSICmd
#undef SRB_Abort
#undef PSRB_Abort
#undef SRB_BusDeviceReset
#undef PSRB_BusDeviceReset
#undef SRB_GetDiskInfo
#undef PSRB_GetDiskInfo
#endif
#pragma pack(push, 1)

enum CdMediaChangeState
{
	CdMediaStateUnknown,
	CdMediaStateNotReady,
	CdMediaStateReady,
	CdMediaStateDiskChanged,
	CdMediaStateBusy,
	CdMediaStateNoDrives,
};

struct CdAddressMSF
{
	UCHAR reserved;
	UCHAR Minute;
	UCHAR Second;
	UCHAR Frame;
	LONG SectorNumber(int FramesPerSecond = 75) const
	{
		return Frame + (Second + Minute * 60) * FramesPerSecond;
	}
	operator LONG() const
	{
		return Frame + (Second + Minute * 60) * 75;
	}

	CdAddressMSF & operator =(LONG sector)
	{
		Frame = UCHAR((sector % 75) & 0xFF);
		Second = UCHAR(((sector / 75) % 60) & 0xFF);
		Minute = UCHAR((sector / (75 * 60)) & 0xFF);
		return *this;
	}
};

#pragma pack(pop)

enum { CDDASectorSize = 2352} ;

struct RiffCddaFmt
{
	DWORD Riff; //"RIFF"
	DWORD Fmt; // "fmt "
	DWORD Size; // 0x18
	USHORT Session; // 1 ??
	USHORT Track;   //
	USHORT VolumeSerialNumber;
	ULONG FirstSector;
	ULONG LengthSectors;
	CdAddressMSF FirstSectorMsf;
	CdAddressMSF LengthMsf;
};

struct CdTrackInfo
{
	CString Artist;
	CString Album;
	CString Track;
	CString TrackFileName;
	bool Checked;
	bool IsAudio;
	CdAddressMSF TrackBegin;
	LONG NumSectors;
	CdTrackInfo()
	{
		Checked = FALSE;
		NumSectors = 0;
		TrackBegin.reserved = 0;
		TrackBegin.Minute = 0;
		TrackBegin.Second = 0;
		TrackBegin.Frame = 0;
	}
};

class ICdDrive
{
public:
	virtual ~ICdDrive() {}

	virtual int FindCdDrives(TCHAR Drives['Z' - 'A' + 1]) = 0;
	virtual BOOL Open(TCHAR letter) = 0;
	virtual void Close() = 0;
	virtual DWORD GetDiskID() = 0;

	virtual BOOL GetMaxReadSpeed(int * pMaxSpeed, int * pCurrentSpeed) = 0; // bytes/s

	virtual BOOL SetReadSpeed(ULONG BytesPerSec, ULONG BeginLba = 0, ULONG NumSectors = 0) = 0;
	virtual BOOL ReadCdData(void * pBuf, long Address, int nSectors) = 0;
	virtual BOOL ReadCdData(void * pBuf, CdAddressMSF Address, int nSectors) = 0;
	//virtual BOOL SetStreaming(long BytesPerSecond) = 0;

	//virtual CString GetLastScsiErrorText() = 0;
	//virtual BOOL GetMediaChanged() = 0; // TRUE if disk was changed after previous call
	virtual BOOL EnableMediaChangeDetection(bool Enable = true) = 0;
	virtual BOOL DisableMediaChangeDetection() = 0;
	virtual BOOL LockDoor(bool Lock = true) = 0;
	virtual BOOL UnlockDoor() = 0;
	virtual BOOL ReadToc() = 0;

	virtual BOOL IsTrackCDAudio(unsigned track) = 0;
	virtual UCHAR GetNumberOfTracks() = 0;
	virtual UCHAR GetFirstTrack() = 0;
	virtual UCHAR GetLastTrack() = 0;
	virtual CdAddressMSF GetTrackBegin(unsigned track) = 0;
	virtual LONG GetNumSectors(unsigned track) = 0;
	virtual LONG GetTrackNumber(unsigned track) = 0;

	//virtual BOOL ReadSessions() = 0;
	virtual void StopAudioPlay() = 0;

	virtual bool CanEjectMedia() = 0;
	virtual bool CanLoadMedia() = 0;
	virtual void EjectMedia() = 0;
	virtual void LoadMedia() = 0;
	virtual BOOL IsTrayOpen() = 0;
	virtual BOOL EjectSupported() const = 0;

	virtual BOOL IsSlotType() const = 0;
	virtual BOOL IsTrayType() const = 0;

	virtual void SetTrayOut(bool Out) = 0;

	virtual CdMediaChangeState CheckForMediaChange() = 0;
	virtual void ForceMountCD() = 0;

	virtual BOOL QueryVendor(CString & Vendor) = 0;
	virtual void StopDrive() = 0;

	virtual void SetDriveBusy(bool Busy = true) = 0;
	virtual bool IsDriveBusy(TCHAR letter)const  = 0;
	virtual bool IsDriveBusy() const = 0;

	//virtual BOOL GetEcMode(BOOL * C2ErrorPointersSupported) = 0;

	//virtual BOOL StartReading(int speed) = 0;

	virtual ICdDrive * Clone() const = 0;

};

ICdDrive * CreateCdDrive(BOOL UseAspi = USE_ASPI_ENABLE);

