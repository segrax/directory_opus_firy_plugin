// firyopus.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <clocale>
#include <locale>
#include <string>
#include <iostream>
#include <codecvt>
#include "images/adf.hpp"

DOpusPluginHelperFunction DOpus;

std::wstring s2ws(const std::string& str) {
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr) {
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

std::vector<std::wstring> tokenize(const std::wstring& in, const std::wstring& delim) {
	std::vector<std::wstring> tokens;

	std::wstring::size_type pos_begin, pos_end = 0;
	std::wstring input = in;

	while ((pos_begin = input.find_first_not_of(delim, pos_end)) != std::wstring::npos) {
		pos_end = input.find_first_of(delim, pos_begin);
		if (pos_end == std::wstring::npos) pos_end = input.length();

		tokens.push_back(input.substr(pos_begin, pos_end - pos_begin));
	}

	return tokens;
}

cFiryPluginData::cFiryPluginData() {

}

cFiryPluginData::~cFiryPluginData() {

}

FILETIME cFiryPluginData::GetFileTime(const firy::helpers::sDateTime& pDateTime) {
	SYSTEMTIME AmigaTime, TZAdjusted;
	AmigaTime.wDayOfWeek = 0;
	AmigaTime.wMilliseconds = 0;
	AmigaTime.wYear = pDateTime.year;
	AmigaTime.wMonth = pDateTime.month;
	AmigaTime.wDay = pDateTime.days;
	AmigaTime.wHour = pDateTime.hour;
	AmigaTime.wMinute = pDateTime.mins;
	AmigaTime.wSecond = pDateTime.secs;
	TzSpecificLocalTimeToSystemTime(NULL, &AmigaTime, &TZAdjusted);
	FILETIME ft;
	SystemTimeToFileTime(&TZAdjusted, &ft);
	return ft;
}

firy::helpers::sDateTime ToDateTime(FILETIME pFT) {
	SYSTEMTIME AmigaTime, TZAdjusted;
	FileTimeToSystemTime(&pFT, &AmigaTime);
	SystemTimeToTzSpecificLocalTime(NULL, &AmigaTime, &TZAdjusted);

	firy::helpers::sDateTime dt;
	dt.year = TZAdjusted.wYear - 1900;
	dt.month = TZAdjusted.wMonth;
	dt.days = TZAdjusted.wDay;
	dt.hour = TZAdjusted.wHour;
	dt.mins = TZAdjusted.wMinute;
	dt.secs = TZAdjusted.wSecond;
	return dt;
}

void cFiryPluginData::SetEntryTime(firy::spFile pFile, FILETIME pFT) {

	auto dt = ToDateTime(pFT);
	pFile->timeWriteSet(dt);
}

LPVFSFILEDATAHEADER cFiryPluginData::GetVFSforEntry(firy::spNode pEntry, HANDLE pHeap) {
	LPVFSFILEDATAHEADER lpFDH;

	if (lpFDH = (LPVFSFILEDATAHEADER)HeapAlloc(pHeap, 0, sizeof(VFSFILEDATAHEADER) + sizeof(VFSFILEDATA))) {
		LPVFSFILEDATA lpFD = (LPVFSFILEDATA)(lpFDH + 1);

		lpFDH->cbSize = sizeof(VFSFILEDATAHEADER);
		lpFDH->lpNext = 0;
		lpFDH->iNumItems = 1;
		lpFDH->cbFileDataSize = sizeof(VFSFILEDATA);

		lpFD->dwFlags = 0;
		lpFD->lpszComment = 0;
		lpFD->iNumColumns = 0;
		lpFD->lpvfsColumnData = 0;

		GetWfdForEntry(pEntry, &lpFD->wfdData);
	}

	return lpFDH;
}

void cFiryPluginData::GetWfdForEntry(firy::spNode pEntry, LPWIN32_FIND_DATA pData) {
	auto filename = s2ws(pEntry->nameGet());
	StringCchCopyW(pData->cFileName, MAX_PATH, filename.c_str());

	pData->nFileSizeHigh = 0;
	pData->nFileSizeLow = pEntry->sizeInBytesGet();

	if (pEntry->isDirectory())
		pData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	else
		pData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

	pData->dwReserved0 = 0;
	pData->dwReserved1 = 0;

	pData->ftCreationTime = GetFileTime(pEntry->timeCreateGet());
	pData->ftLastAccessTime = GetFileTime(pEntry->timeAccessGet());
	pData->ftLastWriteTime = GetFileTime(pEntry->timeWriteGet());
}

std::wstring cFiryPluginData::GetInsidePath(std::wstring pPath) {
	std::vector<std::wstring> Depth;

	if (pPath.find(mSourcePath) != std::wstring::npos) {
		pPath = pPath.replace(pPath.begin(), pPath.begin() + mSourcePath.length(), L"");
		std::replace(pPath.begin(), pPath.end(), '\\', '/');
		return pPath;
	}

	return L"";
}

bool cFiryPluginData::LoadFile(const std::wstring& pPath) {
	auto extensions = firy::cFiry::getKnownExtensions();

	std::wstring path = pPath;

	std::transform(path.begin(), path.end(), path.begin(), ::tolower);
	for (auto ext : extensions) {
		auto extw = s2ws(ext);

		size_t EndPos = path.find(extw);
		if (EndPos != std::wstring::npos) {

			mSourcePath = pPath.substr(0, EndPos + extw.size());

			mImage = firy::gFiry->openImage(ws2s(mSourcePath));
			return mImage != 0;
		}
	}

	mImage = 0;
	return false;
}

bool cFiryPluginData::ReadDirectory(LPVFSREADDIRDATAW lpRDD) {

	// Free directory if lister is closing (otherwise ignore free command)
	if (lpRDD->vfsReadOp == VFSREAD_FREEDIRCLOSE)
		return true;

	if (lpRDD->vfsReadOp == VFSREAD_FREEDIR)
		return true;

	// Do nothing if we have no path
	if (!lpRDD->lpszPath || !*lpRDD->lpszPath) {
		mLastError = ERROR_PATH_NOT_FOUND;
		return false;
	}

	if (lpRDD->vfsReadOp == VFSREAD_CHANGEDIR)
		return true;

	if (!LoadFile(lpRDD->lpszPath))
		return false;

	auto path = GetInsidePath(lpRDD->lpszPath);
	auto dirnode = mImage->filesystemPath(ws2s(path));
	LPVFSFILEDATAHEADER lpLastHeader = 0;

	for (auto& node : dirnode->mNodes) {

		LPVFSFILEDATAHEADER lpFDH = GetVFSforEntry(node, lpRDD->hMemHeap);
		if (lpFDH) {
			if (lpLastHeader)
				lpLastHeader->lpNext = lpFDH;
			else
				lpRDD->lpFileData = lpFDH;

			lpLastHeader = lpFDH;
		}
	}

	return true;
}

bool cFiryPluginData::ReadFile(cFiryFile* pFile, size_t pBytes, std::uint8_t* pBuffer, LPDWORD pReadSize) {

	if (!pFile)
		return false;

	*pReadSize = 0;

	if(!pFile->mBuffer)
		pFile->mBuffer = pFile->mFile->read();

	if (pFile->mBuffer) {
		auto buf = pFile->mBuffer->takeBytes(pBytes > pFile->mBuffer->size() ? pFile->mBuffer->size() : pBytes );
		*pReadSize = buf->size();
		memcpy(pBuffer, buf->data(), buf->size());
	}

	return (*pReadSize > 0);
}

cFiryFile* cFiryPluginData::OpenFile(std::wstring pPath) {

	if (!LoadFile(pPath))
		return false;

	auto path = GetInsidePath(pPath);
	auto dirnode = mImage->filesystemFile(ws2s(path));

	if (dirnode)
		return new cFiryFile(dirnode);

	return 0;
}

void cFiryPluginData::CloseFile(cFiryFile* pFile) {
	pFile->mFile = 0;
	pFile->mBuffer = 0;
	delete pFile;
}


int cFiryPluginData::ContextVerb(LPVFSCONTEXTVERBDATAW lpVerbData) {

	if (LoadFile(lpVerbData->lpszPath)) {

		auto path = GetInsidePath(lpVerbData->lpszPath);
		auto dirnode = mImage->filesystemNode(ws2s(path));
		if (dirnode) {
			if (dirnode->isDirectory())
				return VFSCVRES_DEFAULT;
			else
				return VFSCVRES_EXTRACT;
		}
	}

	return VFSCVRES_FAIL;
}

int cFiryPluginData::Delete(LPVFSBATCHDATAW lpBatchData, const std::wstring& pPath, const std::wstring& pFile, bool pAll) {
	int result = 0;
	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_STATUSTEXT, (DWORD_PTR)"Deleting");

	auto Depth = tokenize(pFile, L"\\\\");

	return 0;
}

cFiryFindData* cFiryPluginData::FindFirstFile(LPTSTR lpszPath, LPWIN32_FIND_DATA lpwfdData, HANDLE hAbortEvent) {

	cFiryFindData* findData = new cFiryFindData();

	if (LoadFile(lpszPath)) {

		auto path = GetInsidePath(lpszPath);

		auto depth = tokenize(lpszPath, L"\\\\");
		auto filemask = depth[depth.size() - 1];
		auto Filepath = ws2s(filemask);
		path = path.substr(0, path.size() - filemask.size());

		findData->mFindMask = std::regex("(." + Filepath + ")");
		findData->mDirectory = mImage->filesystemPath(ws2s(path));
		
		for (findData->mIndex = 0; findData->mIndex < findData->mDirectory->mNodes.size(); ++findData->mIndex) {
			auto node = findData->mDirectory->mNodes[findData->mIndex];

			if (std::regex_match(node->nameGet(), findData->mFindMask)) {

				GetWfdForEntry(node, lpwfdData);
				++findData->mIndex;
				break;
			}
		}
	}

	return findData;
}

bool cFiryPluginData::FindNextFile(cFiryFindData* pFindData, LPWIN32_FIND_DATA lpwfdData) {

	for (; pFindData->mIndex < pFindData->mDirectory->mNodes.size(); ++pFindData->mIndex) {
		auto node = pFindData->mDirectory->mNodes[pFindData->mIndex];

		if (std::regex_match(node->nameGet(), pFindData->mFindMask)) {

			GetWfdForEntry(node, lpwfdData);
			++pFindData->mIndex;
			return true;
		}
	}

	return false;
}

void cFiryPluginData::FindClose(cFiryFindData* pFindData) {
	delete pFindData;
}

int cFiryPluginData::ImportFile(LPVFSBATCHDATAW lpBatchData, const std::wstring& pFile, const std::wstring& pPath) {

	auto Depth = tokenize(pPath, L"\\\\");
	/*
	std::ifstream t(pPath, std::ios::binary);
	std::wstring FileData((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_SETFILENAME, (DWORD_PTR)Depth[Depth.size() - 1].c_str());
	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_SETFILESIZE, (DWORD_PTR)FileData.size());


	FILETIME ft;
	HANDLE filename = CreateFile(pPath.c_str(), FILE_READ_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	::GetFileTime(filename, 0, 0, &ft);
	CloseHandle(filename);

	auto File = adfOpenFile(mAdfVolume.get(), (char*)Depth[Depth.size() - 1].c_str(), (char*) "w");
	if (!File)
		return 1;

	SetEntryTime(File, ft);
	adfWriteFile(File, (int32_t)FileData.size(), reinterpret_cast<uint8_t*>(&FileData[0]));
	adfCloseFile(File);

	std::wstring Final = pFile;
	if (Final[Final.size() - 1] != L'\\')
		Final += L"\\";
	Final += Depth[Depth.size() - 1];

	DOpus.AddFunctionFileChange(lpBatchData->lpFuncData, true, OPUSFILECHANGE_CREATE, Final.c_str());
	*/
	return 0;
}

std::vector<std::wstring> directoryList(const std::wstring pPath) {
	WIN32_FIND_DATA fdata;
	HANDLE dhandle;
	std::vector<std::wstring> results;

	if ((dhandle = ::FindFirstFile(pPath.c_str(), &fdata)) == INVALID_HANDLE_VALUE)
		return results;

	results.push_back(std::wstring(fdata.cFileName));

	while (1) {
		if (::FindNextFile(dhandle, &fdata)) {
			results.push_back(std::wstring(fdata.cFileName));
		}
		else {
			if (GetLastError() == ERROR_NO_MORE_FILES) {
				break;
			}
			else {
				FindClose(dhandle);
				return results;
			}
		}
	}

	FindClose(dhandle);
	return results;
}

int cFiryPluginData::ImportPath(LPVFSBATCHDATAW lpBatchData, const std::wstring& pFile, const std::wstring& pPath) {

	std::wstring FinalPath = pPath;
	if (FinalPath[FinalPath.size() - 1] != L'\\')
		FinalPath += L"\\";

	auto Depth = tokenize(pPath, L"\\\\");
	std::wstring Final = pFile;
	if (Final[Final.size() - 1] != L'\\')
		Final += L"\\";
	Final += Depth[Depth.size() - 1];

	FILETIME ft;
	HANDLE filename = CreateFile(pPath.c_str(), FILE_READ_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	::GetFileTime(filename, 0, 0, &ft);
	CloseHandle(filename);
	/*
	DateTime dt = ToDateTime(ft);

	adfCreateDir(mAdfVolume.get(), mAdfVolume->curDirPtr, (char*)Depth[Depth.size() - 1].c_str(), dt);
	DOpus.AddFunctionFileChange(lpBatchData->lpFuncData, true, OPUSFILECHANGE_MAKEDIR, Final.c_str());

	auto head = GetCurrentDirectoryList();
	List* cell = head.get();

	while (cell) {
		struct Entry* entry = (Entry*)cell->content;
		auto Filename = s2ws(entry->name);

		if (!Depth[Depth.size() - 1].compare(Filename)) {

			auto contents = directoryList(FinalPath + L"*.*");

			for (auto& File : contents) {
				if (File == L"." || File == L"..")
					continue;

				Import(lpBatchData, pFile + L"\\" + Depth[Depth.size() - 1] + L"\\", FinalPath + File);
			}

		}

		cell = cell->next;
	}
	*/

	return 0;
}

int cFiryPluginData::Import(LPVFSBATCHDATAW lpBatchData, const std::wstring& pFile, const std::wstring& pPath) {
	/*
	if (AdfChangeToPath(pFile, false)) {

		if (lpBatchData->hAbortEvent && WaitForSingleObject(lpBatchData->hAbortEvent, 0) == WAIT_OBJECT_0) {
			return 1;
		}

		// is pPath a directory?
		auto Attribs = GetFileAttributes(pPath.c_str());

		if (Attribs & FILE_ATTRIBUTE_DIRECTORY)
			ImportPath(lpBatchData, pFile, pPath);
		else {
			ImportFile(lpBatchData, pFile, pPath);
		}
	}*/

	return 0;
}

int cFiryPluginData::Extract(LPVFSBATCHDATAW lpBatchData, const std::wstring& pFile, const std::wstring& pDest) {
	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_STATUSTEXT, (DWORD_PTR)"Copying");

	int result = 1;

	if (!LoadFile(pFile))
		return false;

	auto path = GetInsidePath(pFile);
	auto node = mImage->filesystemNode(ws2s(path));

	result = ExtractNode(lpBatchData, node, pDest);

	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_NEXTFILE, 0);
	return result;
}

int cFiryPluginData::ExtractNode(LPVFSBATCHDATAW lpBatchData, firy::spNode pNode, std::wstring pDest) {
	int result = 1;

	if (pDest[pDest.size() - 1] != '\\') {
		pDest += '\\';
	}
	auto Filename = s2ws(pNode->nameGet());
	 pDest += Filename;

	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_SETFILENAME, (DWORD_PTR)pNode->nameGet().c_str());
	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_SETFILESIZE, (DWORD_PTR)pNode->sizeInBytesGet());

	if (pNode->isDirectory())
		return ExtractPath(lpBatchData, std::dynamic_pointer_cast<firy::filesystem::sDirectory>(pNode), pDest);

	return ExtractFile(lpBatchData, std::dynamic_pointer_cast<firy::filesystem::sFile>(pNode), pDest);
}

int cFiryPluginData::ExtractPath(LPVFSBATCHDATAW lpBatchData, firy::spDirectory pPath, const std::wstring& pDest) {
	int result = 0;

	CreateDirectory(pDest.c_str(), 0);
	DOpus.AddFunctionFileChange(lpBatchData->lpFuncData, false, OPUSFILECHANGE_CREATE, lpBatchData->pszDestPath);

	for (auto& node : pPath->mNodes) {
		result |= ExtractNode(lpBatchData, node, pDest);
	}

	if (!result) {
		HANDLE filename = CreateFile(pDest.c_str(), FILE_WRITE_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		SetFileTime(filename, 0, 0, &GetFileTime(pPath->timeWriteGet()));
		CloseHandle(filename);
	}

	return result;
}

int cFiryPluginData::ExtractFile(LPVFSBATCHDATAW lpBatchData, firy::spFile pEntry, const std::wstring& pDest) {
	std::string buffer(pEntry->sizeInBytesGet(), 0);

	auto content = pEntry->read();

	std::ofstream ofile(pDest, std::ios::binary);
	ofile.write((const char*)content->data(), content->size());
	ofile.close();

	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_STEPBYTES, (DWORD_PTR)pEntry->sizeInBytesGet());

	HANDLE filename = CreateFile(pDest.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	SetFileTime(filename, 0, 0, &GetFileTime(pEntry->timeWriteGet()));
	CloseHandle(filename);

	DOpus.AddFunctionFileChange(lpBatchData->lpFuncData, false, OPUSFILECHANGE_CREATE, lpBatchData->pszDestPath);
	return 0;
}

size_t cFiryPluginData::TotalFreeBlocks(const std::wstring& pFile) {
	if (!LoadFile(pFile))
		return false;

	return mImage->filesystemTotalBytesFree();
}

size_t cFiryPluginData::TotalDiskBlocks(const std::wstring& pFile) {
	if (!LoadFile(pFile))
		return false;

	return mImage->filesystemTotalBytesMax();
}

UINT cFiryPluginData::BatchOperation(LPTSTR lpszPath, LPVFSBATCHDATAW lpBatchData) {
	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_SETPERCENT, (DWORD_PTR)0);

	auto result = VFSBATCHRES_COMPLETE;

	auto File = lpBatchData->pszFiles;

	for (int i = 0; i < lpBatchData->iNumFiles; ++i) {

		if (lpBatchData->uiOperation == VFSBATCHOP_EXTRACT) {
			lpBatchData->piResults[i] = Extract(lpBatchData, File, lpBatchData->pszDestPath);
		}

		if (lpBatchData->uiOperation == VFSBATCHOP_ADD) {
			lpBatchData->piResults[i] = Import(lpBatchData, lpszPath, File);
		}

		if (lpBatchData->uiOperation == VFSBATCHOP_DELETE) {
			lpBatchData->piResults[i] = Delete(lpBatchData, lpszPath, File);
		}

		if (lpBatchData->piResults[i]) {
			result = VFSBATCHRES_ABORT;
			break;
		}

		File += wcslen(File) + 1;
	}

	return result;
}

bool cFiryPluginData::PropGet(vfsProperty propId, LPVOID lpPropData, LPVOID lpData1, LPVOID lpData2, LPVOID lpData3) {

	if (propId == VFSPROP_SHOWTHUMBNAILS) {

		*(bool*)lpPropData = true;

		return true;
	}

	if (propId == VFSPROP_FUNCAVAILABILITY) {
		unsigned __int64* Data = (unsigned __int64*)lpPropData;

		*Data &= ~(VFSFUNCAVAIL_DELETE | VFSFUNCAVAIL_MAKEDIR | VFSFUNCAVAIL_RENAME | VFSFUNCAVAIL_SETATTR | VFSFUNCAVAIL_CLIPCUT | VFSFUNCAVAIL_CLIPPASTE | VFSFUNCAVAIL_CLIPPASTESHORTCUT | VFSFUNCAVAIL_DUPLICATE);
		return true;
	}

	return false;
}