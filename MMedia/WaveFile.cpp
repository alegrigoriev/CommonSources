// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
// WaveFile.cpp
#include "stdafx.h"
#include "DirectFile.h"
#include "WaveFile.h"
#include <atlbase.h>
#include <atlpath.h>

CMmioFile::CMmioFile()
	: m_hmmio(NULL),
	m_RiffckType(0)
{
}

CMmioFile::~CMmioFile()
{
	Close();

}

CMmioFile & CMmioFile::operator=(CMmioFile & SourceFile)
{
	Close();
	Attach( & SourceFile);

	MMIOINFO mmii;
	memzero(mmii);
	mmii.fccIOProc = 0;
	mmii.pIOProc = BufferedIOProc;

	mmii.adwInfo[0] = (DWORD)this;
	m_hmmio = mmioOpen(NULL, & mmii, MMIO_READ //| MMIO_ALLOCBUF
						);
	Seek(0);
	return *this;
}

static DWORD GetSectorSize(LPCTSTR szFilename)
{
	CString curr_dir;
	TCHAR c;
	DWORD size = GetCurrentDirectory(0, & c);
	if (0 == size)
	{
		return 0x1000;
	}
	LPTSTR pBuf = curr_dir.GetBuffer(size);
	if (NULL == pBuf)
	{
		return 0x1000;
	}

	if (GetCurrentDirectory(size, pBuf) > size)
	{
		return 0x1000;
	}
	curr_dir.ReleaseBuffer();

	CString file_dir;
	LPTSTR pFilenamePart;
	size = GetFullPathName(szFilename, 0, &c, & pFilenamePart);
	if (0 == size)
	{
		return 0x1000;
	}

	pBuf = file_dir.GetBuffer(size);
	if (NULL == pBuf)
	{
		return 0x1000;
	}

	if (GetFullPathName(szFilename, size, pBuf, & pFilenamePart) > size)
	{
		return 0x1000;
	}

	*pFilenamePart = 0;
	file_dir.ReleaseBuffer();
	if (0 != file_dir.CompareNoCase(curr_dir))
	{
		SetCurrentDirectory(file_dir);
	}

	DWORD sector_size = 0;
	DWORD sectors_per_cluster;
	DWORD free_clusters;
	DWORD total_clusters;
	if (! GetDiskFreeSpace(NULL, & sectors_per_cluster,
							& sector_size, & free_clusters, & total_clusters)
		|| 0 == sector_size)
	{
		sector_size = 0x1000;
	}

	if (0 != file_dir.CompareNoCase(curr_dir))
	{
		SetCurrentDirectory(curr_dir);
	}
	return sector_size;
}

BOOL CMmioFile::Open(LPCTSTR szFileName, UINT nOpenFlags)
{
	Close();
	DWORD DirectFileOpenFlags = 0;
	if (nOpenFlags & MmioFileOpenReadOnly)
	{
		DirectFileOpenFlags |= CDirectFile::OpenReadOnly | CDirectFile::OpenExisting;
	}
	else if (nOpenFlags & MmioFileOpenExisting)
	{
		DirectFileOpenFlags |= CDirectFile::OpenExisting;
		if (nOpenFlags & MmioFileAllowReadOnlyFallback)
		{
			DirectFileOpenFlags |= CDirectFile::OpenAllowReadOnlyFallback;
		}
	}
	else
	{
		if (nOpenFlags & MmioFileOpenCreateAlways)
		{
			DirectFileOpenFlags |= CDirectFile::CreateAlways;
		}
		else if (nOpenFlags & MmioFileOpenCreateNew)
		{
			DirectFileOpenFlags |= CDirectFile::CreateNew;
		}
		if (nOpenFlags & MmioFileOpenDeleteAfterClose)
		{
			DirectFileOpenFlags |= CDirectFile::OpenDeleteAfterClose;
		}
		if (nOpenFlags & MmioFileMemoryFile)
		{
			DirectFileOpenFlags |= CDirectFile::CreateMemoryFile;
		}
	}

	if ( ! CDirectFile::Open(szFileName, DirectFileOpenFlags))
	{
		return FALSE;
	}

	if (0 == (nOpenFlags & (MmioFileOpenCreateNew | MmioFileOpenCreateAlways)))
	{

		MMIOINFO mmii;
		memzero(mmii);
		mmii.fccIOProc = 0;
		mmii.pIOProc = BufferedIOProc;

		mmii.adwInfo[0] = (DWORD)this;

		m_hmmio = mmioOpen(NULL, & mmii,
							MMIO_READ //| MMIO_ALLOCBUF
							);

		if (NULL == m_hmmio)
		{
			Close();
			return FALSE;
		}

		if (0 == (nOpenFlags & MmioFileOpenDontLoadRiff)
			&& ! LoadRiffChunk())
		{
			Close();
			return FALSE;
		}
		AllocateInstanceData<InstanceDataMm>();
	}
	else
	{
		// new file created
		if (nOpenFlags & MmioFileMemoryFile)
		{
			SetFileLength(0x40);
		}
		MMIOINFO mmii;
		memzero(mmii);
		mmii.fccIOProc = 0;
		mmii.pIOProc = BufferedIOProc;

		mmii.adwInfo[0] = (DWORD)this;

		m_hmmio = mmioOpen(NULL, & mmii,
							MMIO_READ | MMIO_WRITE //| MMIO_ALLOCBUF
							);

		if (NULL == m_hmmio)
		{
			Close();
			return FALSE;
		}

		LPMMCKINFO pRiffck = & AllocateInstanceData<InstanceDataMm>()->riffck;

		if (0 == (nOpenFlags & MmioFileOpenDontCreateRiff))
		{
			if (FOURCC_RIFF != pRiffck->ckid)
			{
				// if RIFF chunk was filled before, it means that it is already read
				pRiffck->ckid = FOURCC_RIFF;
				pRiffck->cksize = 0;
				pRiffck->fccType = m_RiffckType; // derived class can set it
				pRiffck->dwDataOffset = 0;
				pRiffck->dwFlags = 0;
				if ( ! CreateRiff( * pRiffck))
				{
					Close();
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

BOOL CMmioFile::LoadRiffChunk()
{
	LPMMCKINFO pRiffck = & AllocateInstanceData<InstanceDataMm>()->riffck;
	if (FOURCC_RIFF == pRiffck->ckid)
	{
		// RIFF chunk is already read
		return TRUE;
	}

	pRiffck->ckid = FOURCC_RIFF;
	pRiffck->cksize = 0;
	pRiffck->fccType = m_RiffckType; // derived class can set it
	pRiffck->dwDataOffset = 0;
	pRiffck->dwFlags = 0;

	if ( ! FindRiff())
	{
		Close();
		return FALSE;
	}
	// if RIFF chunk was allocated before, it means that it is already read
	return TRUE;
}

LRESULT PASCAL CMmioFile::BufferedIOProc(LPSTR lpmmioinfo, UINT wMsg,
										LPARAM lParam1, LPARAM lParam2)
{
	LPMMIOINFO pmmi = (LPMMIOINFO) lpmmioinfo;
	CMmioFile * pFile = (CMmioFile *) pmmi->adwInfo[0];

	switch(wMsg)
	{
	case MMIOM_OPEN:
		return MMSYSERR_NOERROR;
		break;
	case MMIOM_CLOSE:
		return MMSYSERR_NOERROR;
		break;
	case MMIOM_READ:
	{
		//TRACE("MMIOM_READ at %08x, %x bytes\n", pmmi->lDiskOffset, lParam2);
		DWORD cbRead = pFile->ReadAt((LPVOID) lParam1, lParam2, pmmi->lDiskOffset);
		if (-1 == cbRead)
			return -1;
		pmmi->lDiskOffset += cbRead;
		return cbRead;
	}
		break;
	case MMIOM_WRITEFLUSH:
		TRACE("MMIOM_WRITEFLUSH\n");
	case MMIOM_WRITE:
	{
		//TRACE("MMIOM_WRITE at %08x, %x bytes\n", pmmi->lDiskOffset, lParam2);
		DWORD cbWritten = pFile->WriteAt((LPVOID) lParam1, lParam2, ULONG(pmmi->lDiskOffset));
		if (-1 == cbWritten)
			return -1;
		pmmi->lDiskOffset += cbWritten;
		return cbWritten;
	}
		break;
	case MMIOM_SEEK:
		switch (lParam2)
		{
		case SEEK_CUR:
			pmmi->lDiskOffset += lParam1;
			//TRACE("MMIOM_SEEK SEEK_CUR pmmi->lDiskOffset=%08x\n", pmmi->lDiskOffset);
			return pmmi->lDiskOffset;
			break;
		case SEEK_END:
			pmmi->lDiskOffset = long(pFile->CDirectFile::GetLength()) + lParam1;
			//TRACE("MMIOM_SEEK SEEK_END pmmi->lDiskOffset=%08x\n", pmmi->lDiskOffset);
			return pmmi->lDiskOffset;
			break;
		case SEEK_SET:
			pmmi->lDiskOffset = lParam1;
			//TRACE("MMIOM_SEEK SEEK_SET pmmi->lDiskOffset=%08x\n", pmmi->lDiskOffset);
			return pmmi->lDiskOffset;
			break;
		default:
			return -1;
		}
		break;
	default:
		return -1;
	}
}

void CMmioFile::Close( )
{
	if (m_hmmio != NULL)
	{
		mmioClose(m_hmmio, 0);
		m_hmmio = NULL;
	}
	CDirectFile::Close(0);
}


CWaveFile::CWaveFile()
{
	m_RiffckType = mmioFOURCC('W', 'A', 'V', 'E');
}

CWaveFile & CWaveFile::operator =(CWaveFile & SourceFile)
{
	CMmioFile::operator=(SourceFile);
	m_FactSamples = SourceFile.m_FactSamples;
	// waveformat will be read as needed
	return *this;
}

CWaveFile::~CWaveFile()
{
}

BOOL CWaveFile::Open( LPCTSTR lpszFileName, UINT nOpenFlags)
{
	if ( ! CMmioFile::Open(lpszFileName, nOpenFlags))
	{
		return FALSE;
	}
	AllocateInstanceData<InstanceDataWav>();
	return TRUE;
}

void CWaveFile::Close( )
{
	CMmioFile::Close();
	m_FactSamples = -1;
}

BOOL CWaveFile::LoadWaveformat()
{

	MMCKINFO ck = {mmioFOURCC('f', 'm', 't', ' '), 0, 0, 0, 0};
	if ( ! FindChunk(ck, GetRiffChunk()))
	{
		return FALSE;
	}
	if (ck.cksize > 0x20000) // 128K
	{
		TRACE("fmt chunk is too big: > 128K\n");
		Ascend(ck);
		return FALSE;
	}
	// allocate structure
	LPWAVEFORMATEX pWf = AllocateWaveformat(ck.cksize);

	if (NULL != pWf)
	{
		if (ck.cksize == Read(pWf, ck.cksize))
		{
			Ascend(ck);
			// try to find 'fact' chunk
			MMCKINFO * pFact = GetFactChunk();
			pFact->ckid = mmioFOURCC('f', 'a', 'c', 't');
			// save current position
			DWORD CurrPos = Seek(0, SEEK_CUR);
			m_FactSamples = -1;
			if (FindChunk(* pFact, GetRiffChunk()))
			{
				if (pFact->cksize >= sizeof (DWORD))
				{
					// read real number of samples
					Read(& m_FactSamples, sizeof m_FactSamples);
				}
				Ascend(* pFact);
			}
			Seek(CurrPos);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		Ascend(ck);
	}
	return FALSE;
}

BOOL CWaveFile::FindData()
{
	LPMMCKINFO pDatack = & AllocateInstanceData<InstanceDataWav>()->datack;
	pDatack->ckid = mmioFOURCC('d', 'a', 't', 'a');
	return FindChunk( * pDatack, GetRiffChunk());
}

BOOL CWaveFile::LoadMetadata()
{
	InstanceDataWav * pInstData = AllocateInstanceData<InstanceDataWav>();
	pInstData->ResetMetadata();

	DWORD id;
	MMCKINFO chunk;
	CMmioFile::Seek(pInstData->riffck.dwDataOffset + 4);

	while(Descend(chunk, & pInstData->riffck))
	{
		// scan subchunks and parse for data
		switch (chunk.ckid)
		{
		case FOURCC_LIST:
			if (! LoadListMetadata(chunk))
			{
				return FALSE;
			}
			break;
		case mmioFOURCC('c', 'u', 'e', ' '):
			if ( ! ReadCueSheet(chunk))
			{
				return FALSE;
			}
			break;
		case mmioFOURCC('p', 'l', 's', 't'):
			if ( ! ReadPlaylist(chunk))
			{
				return FALSE;
			}
			break;
		case mmioFOURCC('D', 'I', 'S', 'P'):
			if (chunk.cksize < 5
				|| sizeof id != Read( & id, sizeof id))
			{
				return FALSE;
			}
			if (CF_TEXT == id
				|| ! ReadChunkString(chunk.cksize - sizeof id,
									pInstData->DisplayTitle))
			{
				return FALSE;
			}
			// TODO: read arbitrary DISP types ( they can be several in one file
			break;
		}
		Ascend(chunk);
	}
	return TRUE;
}

BOOL CMmioFile::ReadChunkString(ULONG Length, CString & String)
{
	// the string may be unicode or text
	if (0 == Length)
	{
		String.Empty();
		return TRUE;
	}

	CStringA s;
	if (Length != Read(s.GetBuffer(Length), Length))
	{
		return FALSE;
	}
	if (Length >= 4
		&& UCHAR(s[0]) == 0xFE
		&& UCHAR(s[1]) == 0xFF)
	{
		// UNICODE marker
		Length &= ~1;
		s.SetAt(Length - 1, 0);
		s.SetAt(Length - 2, 0);
		String = PCWSTR(LPCSTR(s) + 2);
	}
	else
	{
		s.ReleaseBuffer(Length - 1);
		String = s;
	}
	return TRUE;
}

BOOL CMmioFile::ReadChunkStringW(ULONG Length, CString & String)
{
	// the string may be unicode or text
	if (0 == Length)
	{
		String.Empty();
		return TRUE;
	}

	CStringW s;
	if (Length != Read(s.GetBuffer(Length / sizeof (WCHAR)), Length))
	{
		return FALSE;
	}

	s.ReleaseBuffer(Length - 1);
	String = s;
	return TRUE;
}

BOOL CWaveFile::ReadCueSheet(MMCKINFO & chunk)
{
	InstanceDataWav * pInstData = AllocateInstanceData<InstanceDataWav>();
	DWORD count;
	if (chunk.cksize < sizeof count
		|| sizeof count != Read( & count, sizeof count))
	{
		return FALSE;
	}
	if (count * sizeof (CuePointChunkItem) + sizeof count != chunk.cksize)
	{
		return FALSE;
	}
	pInstData->Markers.reserve(count);
	pInstData->Markers.clear();

	CuePointChunkItem item;
	for (unsigned i = 0; i < count; i++)
	{
		if (sizeof item != Read( & item, sizeof item)
			|| item.fccChunk != 'atad')
		{
			return FALSE;
		}
		WaveMarker * pMarker = GetCueItem(item.NameId);
		pMarker->StartSample = item.SamplePosition;

	}
	return TRUE;
}

BOOL CWaveFile::ReadPlaylist(MMCKINFO & chunk)
{
	InstanceDataWav * pInstData = AllocateInstanceData<InstanceDataWav>();
	DWORD count;
	if (chunk.cksize < sizeof count
		|| sizeof count != Read( & count, sizeof count))
	{
		return FALSE;
	}

	if (count * sizeof (WavePlaylistItem) + sizeof count != chunk.cksize)
	{
		return FALSE;
	}
	pInstData->Playlist.reserve(count);
	pInstData->Playlist.clear();

	WavePlaylistItem item;
	for (unsigned i = 0; i < count; i++)
	{
		if (sizeof item != Read( & item, sizeof item))
		{
			return FALSE;
		}
		WaveMarker * pMarker = GetCueItem(item.MarkerIndex);
		// TODO: read playlist
	}
	return TRUE;
}

WaveMarker * CWaveFile::GetCueItem(DWORD CueId)
{
	InstanceDataWav * pInstData = AllocateInstanceData<InstanceDataWav>();
	for (unsigned i = 0; i < pInstData->Markers.size(); i++)
	{
		if (CueId == pInstData->Markers[i].CueId)
		{
			return & pInstData->Markers[i];
		}
	}
	WaveMarker marker;
	marker.CueId = CueId;
	marker.fccRgn = ' ngr';
	marker.LengthSamples = 0;
	marker.StartSample = 0;

	std::vector<WaveMarker>::iterator mm =
		pInstData->Markers.insert(pInstData->Markers.end(), marker);
	return mm.operator->();
}

BOOL CWaveFile::LoadListMetadata(MMCKINFO & chunk)
{
	// fccType is already read
	MMCKINFO subchunk;
	InstanceDataWav * pInstData = AllocateInstanceData<InstanceDataWav>();

	switch (chunk.fccType)
	{
	case mmioFOURCC('I', 'N', 'F', 'O'):
		while(Descend(subchunk, & chunk))
		{
			BOOL res = TRUE;
			switch (subchunk.ckid)
			{
			case RIFFINFO_IART:
				res = ReadChunkString(subchunk.cksize, pInstData->Author);
				break;
			case RIFFINFO_ICMT:
				res = ReadChunkString(subchunk.cksize, pInstData->Comment);
				break;
			case RIFFINFO_ICOP:
				res = ReadChunkString(subchunk.cksize, pInstData->Copyright);
				break;
			case RIFFINFO_IENG:
				res = ReadChunkString(subchunk.cksize, pInstData->RecordingEngineer);
				break;
			case RIFFINFO_IGNR:
				res = ReadChunkString(subchunk.cksize, pInstData->Genre);
				break;
			case RIFFINFO_IKEY:
				res = ReadChunkString(subchunk.cksize, pInstData->Keywords);
				break;
			case RIFFINFO_IMED:
				res = ReadChunkString(subchunk.cksize, pInstData->Medium);
				break;
			case RIFFINFO_INAM:
				res = ReadChunkString(subchunk.cksize, pInstData->Title);
				break;
			case RIFFINFO_ISRC:
				res = ReadChunkString(subchunk.cksize, pInstData->Source);
				break;
			case RIFFINFO_ITCH:
				res = ReadChunkString(subchunk.cksize, pInstData->Digitizer);
				break;
			case RIFFINFO_ISBJ:
				res = ReadChunkString(subchunk.cksize, pInstData->Subject);
				break;
			case RIFFINFO_ISRF:
				res = ReadChunkString(subchunk.cksize, pInstData->DigitizationSource);
				break;
			}
			if ( ! res)
			{
				return FALSE;
			}
			Ascend(subchunk);
		}
		break;

	case mmioFOURCC('U', 'N', 'F', 'O'):    // UNICODE INFO
		while(Descend(subchunk, & chunk))
		{
			BOOL res = TRUE;
			switch (subchunk.ckid)
			{
			case RIFFINFO_IART:
				res = ReadChunkStringW(subchunk.cksize, pInstData->Author);
				break;
			case RIFFINFO_ICMT:
				res = ReadChunkStringW(subchunk.cksize, pInstData->Comment);
				break;
			case RIFFINFO_ICOP:
				res = ReadChunkStringW(subchunk.cksize, pInstData->Copyright);
				break;
			case RIFFINFO_IENG:
				res = ReadChunkStringW(subchunk.cksize, pInstData->RecordingEngineer);
				break;
			case RIFFINFO_IGNR:
				res = ReadChunkStringW(subchunk.cksize, pInstData->Genre);
				break;
			case RIFFINFO_IKEY:
				res = ReadChunkStringW(subchunk.cksize, pInstData->Keywords);
				break;
			case RIFFINFO_IMED:
				res = ReadChunkStringW(subchunk.cksize, pInstData->Medium);
				break;
			case RIFFINFO_INAM:
				res = ReadChunkStringW(subchunk.cksize, pInstData->Title);
				break;
			case RIFFINFO_ISRC:
				res = ReadChunkStringW(subchunk.cksize, pInstData->Source);
				break;
			case RIFFINFO_ITCH:
				res = ReadChunkStringW(subchunk.cksize, pInstData->Digitizer);
				break;
			case RIFFINFO_ISBJ:
				res = ReadChunkStringW(subchunk.cksize, pInstData->Subject);
				break;
			case RIFFINFO_ISRF:
				res = ReadChunkStringW(subchunk.cksize, pInstData->DigitizationSource);
				break;
			}
			if ( ! res)
			{
				return FALSE;
			}
			Ascend(subchunk);
		}
		break;

	case mmioFOURCC('a', 'd', 't', 'l'):
		while(Descend(subchunk, & chunk))
		{
			BOOL res = TRUE;
			DWORD CueId;
			WaveMarker * pMarker;
			CString s;
			LtxtChunk ltxt;
			switch (subchunk.ckid)
			{
			case mmioFOURCC('l', 't', 'x', 't'):
				if (subchunk.cksize < sizeof ltxt
					|| sizeof ltxt != Read( & ltxt, sizeof ltxt))
				{
					return FALSE;
				}
				pMarker = GetCueItem(ltxt.NameId);
				pMarker->LengthSamples = ltxt.SampleLength;

				break;
			case mmioFOURCC('l', 'a', 'b', 'l'):
				if (subchunk.cksize < 5
					|| sizeof CueId != Read( & CueId, sizeof CueId))
				{
					return FALSE;
				}
				res = ReadChunkString(subchunk.cksize - 4,
									GetCueItem(CueId)->Name);
				break;
			case mmioFOURCC('n', 'o', 't', 'e'):
				if (subchunk.cksize < 5
					|| sizeof CueId != Read( & CueId, sizeof CueId))
				{
					return FALSE;
				}
				res = ReadChunkString(subchunk.cksize - 4,
									GetCueItem(CueId)->Comment);
				break;
			}

			if ( ! res)
			{
				return FALSE;
			}
			Ascend(subchunk);
		}
		break;
	}
	return TRUE;
}
// creates a file based on template format from pTemplateFile
BOOL CWaveFile::CreateWaveFile(CWaveFile * pTemplateFile, WAVEFORMATEX * pTemplateFormat,
								NUMBER_OF_CHANNELS Channels, WAV_FILE_SIZE SizeOrSamples, DWORD flags, LPCTSTR FileName)
{
	CString name;
	TCHAR NameBuf[512];
	CThisApp * pApp = GetApp();
	// if the name is empty, create a temp name
	DWORD OpenFlags = MmioFileOpenCreateAlways;
	// create new WAVEFORMATEX
	if (NULL == pTemplateFormat
		&& NULL != pTemplateFile
		&& pTemplateFile->IsOpen())
	{
		pTemplateFormat = pTemplateFile->GetWaveFormat();
	}
	if (flags & CreateWaveFileAllowMemoryFile)
	{
		// check file size
		int nSampleSize = 4;
		if (Channels != ALL_CHANNELS
			|| (pTemplateFormat != NULL && pTemplateFormat->nChannels < 2))
		{
			nSampleSize = 2;
		}
		LONGLONG size = SizeOrSamples * nSampleSize;
		if ( ! pApp->m_bUseMemoryFiles
			|| size > pApp->m_MaxMemoryFileSize * 1024)
		{
			flags &= ~CreateWaveFileAllowMemoryFile;
		}
	}

	if (flags & CreateWaveFileAllowMemoryFile)
	{
		OpenFlags |= MmioFileMemoryFile;
	}
	else if (NULL != FileName
			&& FileName[0] != 0
			&& 0 == (flags & CreateWaveFileTemp))
	{
		name = FileName;
	}
	else
	{
		CString dir;
		if (0 == (flags & CreateWaveFileTempDir))
		{
			// get directory name from template file or FileName
			LPCTSTR OriginalName = NULL;
			if ((flags & CreateWaveFileTemp)
				&& NULL != FileName
				&& FileName[0] != 0)
			{
				OriginalName = FileName;
			}
			else if (NULL != pTemplateFile
					&& pTemplateFile->IsOpen())
			{
				OriginalName = pTemplateFile->GetName();
			}
			LPTSTR pFilePart = NULL;
			if (NULL != OriginalName
				&& 0 != OriginalName[0]
				&& 0 != GetFullPathName(OriginalName,
										countof(NameBuf), NameBuf, & pFilePart)
				&& pFilePart != NULL)
			{
				*pFilePart = 0;
				dir = NameBuf;
			}
		}

		if (dir.IsEmpty())
		{
			dir = GetApp()->m_sTempDir;
			if (dir.IsEmpty())
			{
				if (GetTempPath(countof(NameBuf), NameBuf))
				{
					dir = NameBuf;
				}
			}
		}

		if ( ! dir.IsEmpty()
			&& dir[dir.GetLength() - 1] != '\\'
			&& dir[dir.GetLength() - 1] != '/')
		{
			dir += _T("\\");
		}
		if (GetTempFileName(dir, _T("wav"), 0, NameBuf))
		{
			name = NameBuf;
		}
		else
		{
			return FALSE;
		}
	}
	// create a file, RIFF list, fmt chunk, data chunk of specified size
	// temp file with this name may already be created
	if (flags & CreateWaveFileDeleteAfterClose)
	{
		OpenFlags |= MmioFileOpenDeleteAfterClose;
	}

	if (flags & CreateWaveFileDontInitStructure)
	{
		if (FALSE == Open(name, OpenFlags | MmioFileOpenDontCreateRiff))
		{
			return FALSE;
		}
		if (NULL == AllocateInstanceData<InstanceDataWav>())
		{
			Close();
			return FALSE;
		}

		if ((flags & CreateWaveFileSizeSpecified)
			&& 0 != SizeOrSamples)
		{
			return SetFileLength(SizeOrSamples);
		}
		return TRUE;
	}
	if (FALSE == Open(name, OpenFlags))
	{
		Close();
		return FALSE;
	}

	if (NULL == AllocateInstanceData<InstanceDataWav>())
	{
		Close();
		return FALSE;
	}

	WAVEFORMATEX * pWF = NULL;
	int FormatSize = sizeof (PCMWAVEFORMAT);
	if (NULL != pTemplateFormat)
	{
		if ((flags & CreateWaveFilePcmFormat)
			|| WAVE_FORMAT_PCM == pTemplateFormat->wFormatTag)
		{
			GetInstanceData()->wf.Allocate(0);
			pWF = GetWaveFormat();
			if (pWF)
			{
				pWF->nSamplesPerSec = pTemplateFormat->nSamplesPerSec;
				pWF->wFormatTag = WAVE_FORMAT_PCM;
				if (flags & CreateWaveFileTemp)
				{
					pWF->wBitsPerSample = 16;
				}
				else
				{
					pWF->wBitsPerSample = pTemplateFormat->wBitsPerSample;
				}
				if (ALL_CHANNELS == Channels)
				{
					pWF->nChannels = pTemplateFormat->nChannels;
				}
				else
				{
					pWF->nChannels = 1;
				}
				pWF->nBlockAlign = pWF->wBitsPerSample / 8 * pWF->nChannels;
				pWF->nAvgBytesPerSec = pWF->nBlockAlign * pWF->nSamplesPerSec;
			}
			else
			{
				Close();
				return FALSE;
			}
		}
		else
		{
			GetInstanceData()->wf = pTemplateFormat;
			FormatSize = sizeof (WAVEFORMATEX) + pTemplateFormat->cbSize;
			pWF = GetWaveFormat();
			if (pWF)
			{
				if (ALL_CHANNELS == Channels)
				{
					pWF->nChannels = pTemplateFormat->nChannels;
				}
				else
				{
					pWF->nChannels = 1;
					if (pTemplateFormat->nChannels != 1)
					{
						// it may not be correct, better query the compressor
						pWF->nBlockAlign /= 2;
						pWF->nAvgBytesPerSec /= 2;
					}
				}
			}
			else
			{
				Close();
				return FALSE;
			}
		}
	}
	else
	{
		// create default PCM descriptor
		GetInstanceData()->wf.InitCdAudioFormat();
		pWF = GetWaveFormat();

		if (pWF)
		{
			if (ALL_CHANNELS == Channels)
			{
				pWF->nChannels = 2;
			}
			else
			{
				pWF->nChannels = 1;
			}
			pWF->nBlockAlign = pWF->wBitsPerSample / 8 * pWF->nChannels;
			pWF->nAvgBytesPerSec = pWF->nBlockAlign * pWF->nSamplesPerSec;
		}
		else
		{
			Close();
			return FALSE;
		}
	}
	// RIFF created in Open()
	MMCKINFO * pfck = GetFmtChunk();
	pfck->ckid = mmioFOURCC('f', 'm', 't', ' ');
	CreateChunk(* pfck, 0);
	Write(pWF, FormatSize);
	Ascend(* pfck);

	// write fact chunk
	MMCKINFO * fact = GetFactChunk();
	if ((flags & CreateWaveFileCreateFact)
		&& NULL != fact)
	{
		memzero(*fact);
		fact->ckid = mmioFOURCC('f', 'a', 'c', 't');
		fact->cksize = sizeof (DWORD);

		if (CreateChunk(* fact, 0))
		{
			DWORD tmp = 0;
			Write(& tmp, sizeof tmp);
			Ascend( * fact);
		}
	}

	// create data chunk
	LPMMCKINFO pDatachunk = GetDataChunk();
	memzero(*pDatachunk);

	pDatachunk->ckid = mmioFOURCC('d', 'a', 't', 'a');
	CreateChunk( * pDatachunk, 0);
	if (SizeOrSamples)
	{
		if (flags & CreateWaveFileSizeSpecified)
		{
			SetFileLength(SizeOrSamples);
			Seek(SizeOrSamples);
		}
		else
		{
			size_t DataLength = SizeOrSamples * SampleSize();
			SetFileLength(pDatachunk->dwDataOffset + DataLength);
			Seek(DataLength, SEEK_CUR);
		}
	}
	Ascend( * pDatachunk);
	// and copy INFO
	// then update RIFF
	//Ascend( *GetRiffChunk());
	return TRUE;
}

BOOL CWaveFile::SaveMetadata()
{
	// TODO
	return FALSE;
}

DWORD CWaveFile::GetMetadataLength()
{
	// TODO
	return 0;
}

BOOL CWaveFile::SetDatachunkLength(DWORD Length)
{
	LPMMCKINFO pck = GetDataChunk();
	if (NULL == pck)
	{
		return FALSE;
	}

	if ( ! SetFileLength(Length + pck->dwDataOffset + GetMetadataLength()))
	{
		return FALSE;
	}

	if (pck->ckid != 0)
	{
		// update data chunk length
		pck->cksize = Length;
		pck->dwFlags |= MMIO_DIRTY;
	}
	CommitChanges();
	return TRUE;
}

void CWaveFile::SetFactNumberOfSamples(NUMBER_OF_SAMPLES samples)
{
	GetFactChunk()->dwFlags |= MMIO_DIRTY;
	// save number of samples
	m_FactSamples = samples;
}

BOOL CWaveFile::SetFileLengthSamples(NUMBER_OF_SAMPLES length)
{
	ASSERT(WAVE_FORMAT_PCM == GetWaveFormat()->wFormatTag);

	return SetDatachunkLength(length * SampleSize());
}

int CWaveFile::SampleSize() const
{
	InstanceDataWav * cd = GetInstanceData();
	if (NULL == cd)
	{
		return 0;
	}
	return cd->wf.NumChannels() * cd->wf.BitsPerSample() / 8;
}

NUMBER_OF_SAMPLES CWaveFile::NumberOfSamples() const
{
	InstanceDataWav * cd = GetInstanceData();
	if (NULL == cd
		|| 0 == cd->wf.NumChannels()
		|| 0 == cd->wf.BitsPerSample())
	{
		return 0;
	}
	return MulDiv(cd->datack.cksize, 8, cd->wf.NumChannels() * cd->wf.BitsPerSample());
}

WAVEFORMATEX * CWaveFile::GetWaveFormat() const
{
	InstanceDataWav * tmp = GetInstanceData();
	if (tmp)
	{
		return tmp->wf;
	}
	else
	{
		return NULL;
	}
}

MMCKINFO * CWaveFile::GetFmtChunk() const
{
	InstanceDataWav * tmp = GetInstanceData();
	if (tmp)
	{
		return & tmp->fmtck;
	}
	else
	{
		return NULL;
	}
}

MMCKINFO * CWaveFile::GetFactChunk() const
{
	InstanceDataWav * tmp = GetInstanceData();
	if (tmp)
	{
		return & tmp->factck;
	}
	else
	{
		return NULL;
	}
}

BOOL CMmioFile::CommitChanges()
{
	if ( ! IsOpen()
		|| IsReadOnly())
	{
		return FALSE;
	}
	// save RIFF header
	DWORD CurrentLength = (DWORD)GetLength();
	LPMMCKINFO riff = GetRiffChunk();

	if (NULL != riff && 0 != riff->ckid
		&& ((riff->dwFlags & MMIO_DIRTY) || riff->dwDataOffset + riff->cksize != CurrentLength))
	{
		Seek(CurrentLength);
		riff->dwFlags |= MMIO_DIRTY;

		Ascend( * riff);
	}
	Flush();
	return TRUE;
}

BOOL CWaveFile::CommitChanges()
{
	if ( ! IsOpen()
		|| IsReadOnly())
	{
		return FALSE;
	}
	// write new fmt chunk
	MMCKINFO * fmtck = GetFmtChunk();

	if (NULL != fmtck
		&& 0 != fmtck->dwDataOffset
		&& (fmtck->dwFlags & MMIO_DIRTY))
	{
		Seek(fmtck->dwDataOffset);

		Write(GetWaveFormat(), fmtck->cksize);
		fmtck->dwFlags &= ~MMIO_DIRTY;
	}

	MMCKINFO * factck = GetFactChunk();

	if (NULL != factck
		&& factck->dwDataOffset != 0
		&& (factck->dwFlags & MMIO_DIRTY))
	{
		Seek(factck->dwDataOffset);

		Write( & m_FactSamples, sizeof m_FactSamples);
		factck->dwFlags &= ~MMIO_DIRTY;
	}

	// update data chunk
	MMCKINFO * datack = GetDataChunk();
	if (NULL != datack
		&& 0 != datack->dwDataOffset
		&& datack->dwFlags & MMIO_DIRTY)
	{
		Seek(datack->dwDataOffset - sizeof datack->cksize);

		Write( & datack->cksize, sizeof datack->cksize);
		datack->dwFlags &= ~MMIO_DIRTY;
		GetRiffChunk()->dwFlags |= MMIO_DIRTY;
	}
	return CMmioFile::CommitChanges();
}

CWavePeaks::CWavePeaks(unsigned granularity)
	: m_pPeaks(NULL)
	, m_WavePeakSize(0)
	, m_AllocatedWavePeakSize(0),
	m_PeakDataGranularity(granularity)
{
}

CWavePeaks::~CWavePeaks()
{
	delete[] m_pPeaks;
}

WavePeak * CWavePeaks::AllocatePeakData(NUMBER_OF_SAMPLES NewNumberOfSamples,
										NUMBER_OF_CHANNELS NumberOfChannels)
{
	// change m_pPeaks size
	// need to synchronize with OnDraw
	unsigned NewWavePeakSize = NumberOfChannels *
								((NewNumberOfSamples + m_PeakDataGranularity - 1) / m_PeakDataGranularity);

	if (NULL == m_pPeaks
		|| NewWavePeakSize > m_AllocatedWavePeakSize)
	{
		unsigned NewAllocatedWavePeakSize = NewWavePeakSize + 1024;  // reserve more
		WavePeak * NewPeaks = new WavePeak[NewAllocatedWavePeakSize];
		if (NULL == NewPeaks)
		{
			return NULL;
		}
		if (NULL != m_pPeaks
			&& 0 != m_WavePeakSize)
		{
			memcpy(NewPeaks, m_pPeaks, m_WavePeakSize * sizeof (WavePeak));
		}
		else
		{
			m_WavePeakSize = 0;
		}
		for (unsigned i = m_WavePeakSize; i < NewWavePeakSize; i++)
		{
			NewPeaks[i].high = -0x8000;
			NewPeaks[i].low = 0x7FFF;
		}
		WavePeak * OldPeaks;
		{
			CSimpleCriticalSectionLock lock(m_PeakLock);
			OldPeaks = m_pPeaks;
			m_pPeaks = NewPeaks;
			m_WavePeakSize = NewWavePeakSize;
			m_AllocatedWavePeakSize = NewAllocatedWavePeakSize;
		}
		delete[] OldPeaks;
	}
	else
	{
		for (unsigned i = m_WavePeakSize; i < NewWavePeakSize; i++)
		{
			m_pPeaks[i].high = -0x8000;
			m_pPeaks[i].low = 0x7FFF;
		}
		m_WavePeakSize = NewWavePeakSize;
	}
	return m_pPeaks;
}

void CWaveFile::SetPeakData(PEAK_INDEX index, WAVE_SAMPLE low, WAVE_SAMPLE high)
{
	CWavePeaks * pPeaks = GetWavePeaks();
	if (NULL != pPeaks)
	{
		pPeaks->SetPeakData(index, low, high);
	}
}

BOOL CWaveFile::AllocatePeakData(NUMBER_OF_SAMPLES NewNumberOfSamples)
{
	CWavePeaks * pPeaks = GetWavePeaks();
	if (NULL != pPeaks)
	{
		return NULL != pPeaks->AllocatePeakData(NewNumberOfSamples, Channels());
	}
	return FALSE;
}

BOOL CWaveFile::SetSourceFile(CWaveFile * const pOriginalFile)
{
	if (BaseClass::SetSourceFile(pOriginalFile))
	{
		CWavePeaks * pPeaks2 = GetWavePeaks();
		CWavePeaks * pPeaks1 = pOriginalFile->GetWavePeaks();
		if (NULL != pPeaks1 && NULL != pPeaks2)
		{
			*pPeaks2 = *pPeaks1;
		}
		return TRUE;
	}
	return FALSE;
}

WavePeak CWaveFile::GetPeakMinMax(PEAK_INDEX from, PEAK_INDEX to, NUMBER_OF_CHANNELS stride)
{
	CWavePeaks * pPeaks = GetWavePeaks();
	if (NULL != pPeaks)
	{
		return pPeaks->GetPeakMinMax(from, to, stride);
	}
	return WavePeak(0, 0);
}

unsigned CWaveFile::GetPeakGranularity() const
{
	CWavePeaks const * pPeaks = GetWavePeaks();
	if (NULL != pPeaks)
	{
		return pPeaks->GetGranularity();
	}
	return 1024;
}

unsigned CWaveFile::GetPeaksSize() const
{
	CWavePeaks const * pPeaks = GetWavePeaks();
	if (NULL != pPeaks)
	{
		return pPeaks->GetPeaksSize();
	}
	return 0;
}

void CWaveFile::SetPeaks(PEAK_INDEX from, PEAK_INDEX to, NUMBER_OF_CHANNELS stride, WavePeak value)
{
	CWavePeaks * pPeaks = GetWavePeaks();
	if (NULL != pPeaks)
	{
		pPeaks->SetPeaks(from, to, stride, value);
	}
}

void CWaveFile::RescanPeaks(SAMPLE_INDEX begin, SAMPLE_INDEX end)
{
	// if called immediately after data modification, it will get
	// the data directly from the cache
	TRACE("RescanPeaks from %d to %d\n", begin, end);

	CWavePeaks * pPeaks = GetWavePeaks();
	unsigned Granularity = pPeaks->GetGranularity();

	int nSampleSize = SampleSize();
	LPMMCKINFO datack = GetDataChunk();
	DWORD dwDataChunkOffset = datack->dwDataOffset;

	unsigned GranuleSize = Channels() * Granularity * sizeof(WAVE_SAMPLE);

	DWORD Pos = nSampleSize * (begin & -int(Granularity)) + dwDataChunkOffset;

	DWORD EndPos = nSampleSize * ((end | (Granularity - 1)) + 1);
	if (EndPos > datack->cksize)
	{
		EndPos = datack->cksize;
	}
	EndPos += dwDataChunkOffset;

	while (Pos < EndPos)
	{
		DWORD SizeToRead = EndPos - Pos;
		void * pBuf;
		long lRead = GetDataBuffer( & pBuf, SizeToRead, Pos, 0);

		if (lRead > 0)
		{
			unsigned i;
			unsigned DataToProcess = lRead;
			WAVE_SAMPLE * pWaveData = (WAVE_SAMPLE *) pBuf;
			DWORD DataOffset = Pos - dwDataChunkOffset;
			unsigned DataForGranule = GranuleSize - DataOffset % GranuleSize;

			if (2 == Channels())
			{
				unsigned index = (DataOffset / GranuleSize) * 2;
				while (0 != DataToProcess)
				{
					int wpl_l;
					int wpl_h;
					int wpr_l;
					int wpr_h;
					if (0 == DataOffset % GranuleSize)
					{
						wpl_l = 0x7FFF;
						wpl_h = -0x8000;
						wpr_l = 0x7FFF;
						wpr_h = -0x8000;
					}
					else
					{
						wpl_l = pPeaks->GetPeakDataLow(index);
						wpl_h = pPeaks->GetPeakDataHigh(index);
						wpr_l = pPeaks->GetPeakDataLow(index + 1);
						wpr_h = pPeaks->GetPeakDataHigh(index + 1);
					}

					if (DataForGranule > DataToProcess)
					{
						DataForGranule = DataToProcess;
					}
					DataToProcess -= DataForGranule;

					if (DataOffset & 2)
					{
						if (pWaveData[0] < wpr_l)
						{
							wpr_l = pWaveData[0];
						}
						if (pWaveData[0] > wpr_h)
						{
							wpr_h = pWaveData[0];
						}
						pWaveData++;
						DataOffset += 2;
						DataForGranule -= 2;
					}

					DataOffset += DataForGranule;
					for (i = 0; i < DataForGranule / (sizeof(WAVE_SAMPLE) * 2); i++, pWaveData += 2)
					{
						if (pWaveData[0] < wpl_l)
						{
							wpl_l = pWaveData[0];
						}
						if (pWaveData[0] > wpl_h)
						{
							wpl_h = pWaveData[0];
						}
						if (pWaveData[1] < wpr_l)
						{
							wpr_l = pWaveData[1];
						}
						if (pWaveData[1] > wpr_h)
						{
							wpr_h = pWaveData[1];
						}
					}

					if (DataForGranule & 2)
					{
						if (pWaveData[0] < wpl_l)
						{
							wpl_l = pWaveData[0];
						}
						if (pWaveData[0] > wpl_h)
						{
							wpl_h = pWaveData[0];
						}
						pWaveData++;
					}

					pPeaks->SetPeakData(index, wpl_l, wpl_h);
					pPeaks->SetPeakData(index + 1, wpr_l, wpr_h);
					index += 2;

					DataForGranule = GranuleSize;
				}
			}
			else
			{
				unsigned index = DataOffset / GranuleSize;
				while (0 != DataToProcess)
				{
					int wp_l;
					int wp_h;
					if (0 == DataOffset % GranuleSize)
					{
						wp_l = 0x7FFF;
						wp_h = -0x8000;
					}
					else
					{
						wp_l = pPeaks->GetPeakDataLow(index);
						wp_h = pPeaks->GetPeakDataHigh(index);
					}

					if (DataForGranule > DataToProcess)
					{
						DataForGranule = DataToProcess;
					}
					DataToProcess -= DataForGranule;
					DataOffset += DataForGranule;

					for (i = 0; i < DataForGranule / sizeof(WAVE_SAMPLE); i++, pWaveData ++)
					{
						if (pWaveData[0] < wp_l)
						{
							wp_l = pWaveData[0];
						}
						if (pWaveData[0] > wp_h)
						{
							wp_h = pWaveData[0];
						}
					}

					pPeaks->SetPeakData(index, wp_l, wp_h);
					index++;

					DataForGranule = GranuleSize;
				}
			}

			Pos += lRead;

			ReturnDataBuffer(pBuf, lRead, 0);
		}
		else
		{
			break;
		}
	}
}

WavePeak CWavePeaks::GetPeakMinMax(PEAK_INDEX from, PEAK_INDEX to, NUMBER_OF_CHANNELS stride)
{

	WavePeak peak;
	peak.high = -0x8000;
	peak.low = 0x7FFF;

	CSimpleCriticalSectionLock lock(m_PeakLock);

	if (to > m_WavePeakSize)
	{
		to = m_WavePeakSize;
	}

	for (unsigned j = from; j < to; j += stride)
	{
		if (peak.low > m_pPeaks[j].low)
		{
			peak.low = m_pPeaks[j].low;
		}
		if (peak.high < m_pPeaks[j].high)
		{
			peak.high = m_pPeaks[j].high;
		}
	}
	return peak;
}

void CWavePeaks::SetPeaks(PEAK_INDEX from, PEAK_INDEX to, NUMBER_OF_CHANNELS stride, WavePeak value)
{
	CSimpleCriticalSectionLock lock(m_PeakLock);

	if (to > m_WavePeakSize)
	{
		to = m_WavePeakSize;
	}

	for (unsigned j = from; j < to; j += stride)
	{
		m_pPeaks[j] = value;
	}
}

CWavePeaks & CWavePeaks::operator =(CWavePeaks const & src)
{
	m_PeakDataGranularity = src.GetGranularity();
	WavePeak * pPeaks = AllocatePeakData(src.GetPeaksSize() * m_PeakDataGranularity);
	if (NULL != pPeaks)
	{
		memcpy(pPeaks, src.GetPeakArray(), GetPeaksSize() * sizeof (WavePeak));
	}
	else
	{
		AllocatePeakData(0);
	}
	return *this;
}

bool operator ==(FILETIME const & t1, FILETIME const & t2)
{
	return t1.dwHighDateTime == t2.dwHighDateTime
			&& t1.dwLowDateTime == t2.dwLowDateTime;
}

CPath CWaveFile::MakePeakFileName(LPCTSTR FileName)
{
	CPath path(FileName);
	if (0 == path.GetExtension().CompareNoCase(_T(".WAV")))
	{
		path.RenameExtension(_T(".wspk"));
	}
	else
	{
		static_cast<CString &>(path).Append(_T(".wspk"));
	}
	return path;
}

BOOL CWaveFile::CheckAndLoadPeakFile()
{
	// if peak file exists and the wav file length/date/time matches the stored
	// length/date/time, then use this peak file.
	// otherwise scan the wav file and build the new peak file
	CWavePeaks * pPeakInfo = GetWavePeaks();

	CFile PeakFile;
	PeakFileHeader pfh;

	CPath PeakFilename(MakePeakFileName(GetName()));

	if (PeakFile.Open(PeakFilename,
					CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary))
	{
		if (sizeof (PeakFileHeader)
			== PeakFile.Read( & pfh, sizeof (PeakFileHeader))
			&& PeakFileHeader::pfhSignature == pfh.dwSignature
			&& pfh.dwVersion == PeakFileHeader::pfhMaxVersion
			&& pfh.wSize == sizeof (PeakFileHeader)

			&& pfh.WaveFileTime == GetFileInformation().ftLastWriteTime
			&& pfh.dwWaveFileSize == GetFileInformation().nFileSizeLow

			&& 0 == memcmp(& pfh.wfFormat, GetWaveFormat(), sizeof pfh.wfFormat)
			&& pPeakInfo->GetGranularity() == pfh.Granularity
			&& pfh.PeakInfoSize == CalculatePeakInfoSize() * sizeof (WavePeak)
			)
		{
			// allocate data and read it
			WavePeak * pPeaks = pPeakInfo->AllocatePeakData(NumberOfSamples(), Channels());
			if (NULL == pPeaks)
			{
				TRACE("Unable to allocate peak info buffer\n");
				pPeakInfo->AllocatePeakData(0, 1);
				return FALSE;
			}

			if (pfh.PeakInfoSize <= pPeakInfo->GetPeaksSize() * sizeof (WavePeak)
				&& pfh.PeakInfoSize == PeakFile.Read(pPeaks, pfh.PeakInfoSize))
			{
				return TRUE;
			}
			TRACE("Unable to read peak data\n");
			// rebuild the info from the WAV file
		}
		else
		{
			TRACE("Peak Info modification time = 0x%08X%08X, open file time=0x%08X%08X\n",
				pfh.WaveFileTime.dwHighDateTime, pfh.WaveFileTime.dwLowDateTime,
				GetFileInformation().ftLastWriteTime.dwHighDateTime,
				GetFileInformation().ftLastWriteTime.dwLowDateTime);
		}
		PeakFile.Close();
	}
	return FALSE;
}

// the function is called to load peak info for a compressed file
// WaveFile argument - temporary wave file
// OriginalWaveFile - compressed file
BOOL CWaveFile::LoadPeaksForCompressedFile(CWaveFile & OriginalWaveFile,
											ULONG NumberOfSamples)
{

	// don't check peak file data size, just make sure source file parameters match
	// if peak file exists and the wav file length/date/time matches the stored
	// length/date/time, then use this peak file.
	// otherwise don't use it.
	// the peak info will be rebuilt in any case during file load
	CWavePeaks * pPeakInfo = GetWavePeaks();

	AllocatePeakData(NumberOfSamples);

	CFile PeakFile;
	PeakFileHeader pfh;
	CPath PeakFilename(MakePeakFileName(OriginalWaveFile.GetName()));

	if (PeakFile.Open(PeakFilename,
					CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary))
	{
		if (offsetof(PeakFileHeader, WaveFileTime)
			== PeakFile.Read( & pfh, offsetof(PeakFileHeader, WaveFileTime))
			&& PeakFileHeader::pfhSignature == pfh.dwSignature
			&& pfh.dwVersion == PeakFileHeader::pfhMaxVersion
			&& pfh.wSize == sizeof (PeakFileHeader)
			// read the rest of the header
			&& pfh.wSize - offsetof(PeakFileHeader, WaveFileTime)
			== PeakFile.Read( & pfh.WaveFileTime, pfh.wSize - offsetof(PeakFileHeader, WaveFileTime))
			// check date and time
			&& pfh.WaveFileTime == OriginalWaveFile.GetFileInformation().ftLastWriteTime
			// check source file size
			&& pfh.dwWaveFileSize == OriginalWaveFile.GetFileInformation().nFileSizeLow
			// check PCM number of channels and sampling rate
			&& 0 == memcmp(& pfh.wfFormat, GetWaveFormat(), sizeof pfh.wfFormat)
			&& pPeakInfo->GetGranularity() == pfh.Granularity
			)
		{
			// allocate data and read it
			WavePeak * pPeaks = pPeakInfo->AllocatePeakData(pfh.NumOfSamples, Channels());
			if (NULL == pPeaks)
			{
				TRACE("Unable to allocate peak info buffer\n");
				pPeakInfo->AllocatePeakData(0, 1);
				return FALSE;
			}

			if (pfh.PeakInfoSize <= pPeakInfo->GetPeaksSize() * sizeof (WavePeak)
				&& pfh.PeakInfoSize == PeakFile.Read(pPeaks, pfh.PeakInfoSize))
			{
				return TRUE;
			}
			TRACE("Unable to read peak data\n");
			// rebuild the info from the WAV file
		}
		else
		{
			TRACE("Peak Info modification time = 0x%08X%08X, open file time=0x%08X%08X\n",
				pfh.WaveFileTime.dwHighDateTime, pfh.WaveFileTime.dwLowDateTime,
				GetFileInformation().ftLastWriteTime.dwHighDateTime,
				GetFileInformation().ftLastWriteTime.dwLowDateTime);
		}
		PeakFile.Close();
	}
	return FALSE;
}

void CWaveFile::SavePeakInfo(CWaveFile & SavedWaveFile)
{
	CFile PeakFile;
	PeakFileHeader pfh;
	CPath PeakFilename(MakePeakFileName(SavedWaveFile.GetName()));

	if (PeakFile.Open(PeakFilename,
					CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive | CFile::typeBinary))
	{
		pfh.wSize = sizeof PeakFileHeader;
		pfh.dwSignature = PeakFileHeader::pfhSignature;
		pfh.dwVersion = PeakFileHeader::pfhMaxVersion;
		pfh.dwWaveFileSize = SavedWaveFile.GetFileSize(NULL);
		pfh.Granularity = GetPeakGranularity();
		pfh.PeakInfoSize = CalculatePeakInfoSize() * sizeof (WavePeak);
		pfh.WaveFileTime = SavedWaveFile.GetFileInformation().ftLastWriteTime;
		pfh.NumOfSamples = NumberOfSamples();
		pfh.wfFormat = * GetWaveFormat();

		PeakFile.Write( & pfh, sizeof pfh);
		PeakFile.Write(GetWavePeaks()->GetPeakArray(), pfh.PeakInfoSize);

		PeakFile.Close();
	}

}

SAMPLE_POSITION CWaveFile::SampleToPosition(SAMPLE_INDEX sample) const
{
	LPMMCKINFO datack = GetDataChunk();
	if (NULL == datack)
	{
		return 0;
	}
	if (LAST_SAMPLE == sample)
	{
		return datack->dwDataOffset + datack->cksize;
	}
	return datack->dwDataOffset + sample * SampleSize();
}

SAMPLE_INDEX CWaveFile::PositionToSample(SAMPLE_POSITION position) const
{
	LPMMCKINFO datack = GetDataChunk();
	if (NULL == datack)
	{
		return 0;
	}
	if (LAST_SAMPLE_POSITION == position)
	{
		return datack->cksize / SampleSize();
	}

	ASSERT(position >= datack->dwDataOffset);
	return (position - datack->dwDataOffset) / SampleSize();
}

using CWaveFile::InstanceDataWav;
void InstanceDataWav::CopyMetadata(InstanceDataWav const & src)
{
	Album = src.Album;
	Author = src.Author;
	Date = src.Date;
	Genre = src.Genre;
	Comment = src.Comment;
	Title = src.Title;
	DisplayTitle = src.DisplayTitle;

	Markers = src.Markers;

	Playlist = src.Playlist;
	m_PeakData = src.m_PeakData;
	InfoChanged = true;
}

InstanceDataWav & InstanceDataWav::operator =(InstanceDataWav const & src)
{
	if (this == & src)
	{
		return * this;
	}
	datack = src.datack;
	fmtck = src.fmtck;
	factck = src.factck;
	wf = src.wf;

	CopyMetadata(src);

	BaseInstanceClass::operator =(src);
	return *this;
}

void InstanceDataWav::ResetMetadata()
{
	Author.Empty();
	DisplayTitle.Empty();
	Album.Empty();
	Copyright.Empty();
	RecordingEngineer.Empty();

	Title.Empty();
	Date.Empty();
	Genre.Empty();
	Comment.Empty();
	Subject.Empty();
	Keywords.Empty();
	Medium.Empty();
	Source.Empty();
	Digitizer.Empty();
	DigitizationSource.Empty();

	Markers.clear();
	Playlist.clear();
	InfoChanged = true;

}

void CWaveFile::CopyMetadata(CWaveFile const & src)
{
	InstanceDataWav * pDst = GetInstanceData();
	InstanceDataWav const * pSrc = src.GetInstanceData();
	if (NULL != pDst
		&& NULL != pSrc)
	{
		pDst->CopyMetadata(*pSrc);
	}
}

