/*
	Copyright (c) 2020-2021 Robert Crossfield

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "stdafx.h"
#include <string>
#include <iostream>
#include <codecvt>
#include "images/adf.hpp"
#include <OleIdl.h>

DOpusPluginHelperFunction DOpus;
DOpusPluginHelperUtil DOpusUtil;

cSafeContainer<std::pair<std::wstring, firy::wpImage>, std::map<std::wstring, firy::wpImage>> gOpenFiles;
std::mutex gOpenLock;

/**
 * String to WideString
 */
std::wstring s2ws(const std::string& str) {
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	try {
		return converterX.from_bytes(str);
	}
	catch (...) {
		return L"";
	}
}

/**
 * WideString to String
 */
std::string ws2s(const std::wstring& wstr) {
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	try {
		return converterX.to_bytes(wstr);
	}
	catch (...) {
		return "";
	}
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
	dt.year = TZAdjusted.wYear;
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

/**
 *
 */
void cFiryPluginData::GetWfdForEntry(firy::spNode pEntry, LPWIN32_FIND_DATA pData) {
	auto filename = s2ws(pEntry->nameGet());
	StringCchCopyW(pData->cFileName, MAX_PATH, filename.c_str());

	pData->nFileSizeHigh = (DWORD) (pEntry->sizeInBytesGet() >> 32);
	pData->nFileSizeLow = (DWORD)  pEntry->sizeInBytesGet();

	pData->dwFileAttributes = (pEntry->isDirectory() == true) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;

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
	tLockGuard mLock(gOpenLock);

	if (mImage)
		return true;

	auto extensions = firy::cFiry::getKnownExtensions();
	std::wstring path = pPath;

	std::transform(path.begin(), path.end(), path.begin(), ::tolower);
	for (auto ext : extensions) {
		auto extw = s2ws(ext);

		size_t EndPos = path.find(extw);
		if (EndPos != std::wstring::npos) {

			mSourcePath = pPath.substr(0, EndPos + extw.size());

			auto& files = gOpenFiles.lockContainer();
			for (auto fileIT = files.begin(); fileIT != files.end(); ) {

				if (fileIT->second.expired()) {
					fileIT = files.erase(fileIT);
					continue;
				}
				++fileIT;
			}
			gOpenFiles.unlock();

			auto file = gOpenFiles.find_if([this](auto pImage) -> bool {
				if (pImage.second.expired())
					return false;

				if (pImage.first == mSourcePath) {
					return true;
				}
				return false;
			});
			
			if (!file.second.expired()) {
				mImage = file.second.lock();
				return true;
			}

			mImage = firy::gFiry->openImage(ws2s(mSourcePath));
			gOpenFiles.push({ mSourcePath, firy::wpImage(mImage) });

			return mImage != 0;
		}
	}

	mImage = 0;
	return false;
}

bool cFiryPluginData::CreateDir(const std::wstring& pPath) {
	if (!LoadFile(pPath))
		return false;

	auto path = GetInsidePath(pPath);

	auto depth = tokenize(path, L"/");
	auto name = depth[depth.size() - 1];
	path = path.substr(0, path.size() - name.size());

	auto node = mImage->filesystemPath(ws2s(path));
	if (!node)
		return false;

	auto Dir = mImage->filesystemDirectoryCreate(ws2s(name));
	return node->add(Dir);
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
	if (!dirnode)
		return false;

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
		*pReadSize = (DWORD)buf->size();
		memcpy(pBuffer, buf->data(), buf->size());
	}

	return (*pReadSize > 0);
}

cFiryFile* cFiryPluginData::OpenFile(std::wstring pPath) {

	if (!LoadFile(pPath))
		return 0;

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
	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_STATUSTEXT, (DWORD_PTR)"Deleting");
	auto depth = tokenize(pFile, L"\\\\");
	if (LoadFile(pPath)) {

		DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_SETFILENAME, (DWORD_PTR)depth[depth.size() - 1].c_str());

		auto path = GetInsidePath(pFile);
		auto dirnode = mImage->filesystemNode(ws2s(path));

		auto res = this->DeleteFile(pFile);

		DOpus.AddFunctionFileChange(lpBatchData->lpFuncData, true, OPUSFILECHANGE_REMDIR, pFile.c_str());
		return res == true ? 0 : 1;
	}

	return 1;
}

bool cFiryPluginData::DeleteFile(const std::wstring& pPath) {
	if (!LoadFile(pPath))
		return false;

	auto path = GetInsidePath(pPath);
	auto node = mImage->filesystemNode(ws2s(path));
	if (!node)
		return false;

	return node->remove();
}

cFiryFindData* cFiryPluginData::FindFirstFile(LPTSTR lpszPath, LPWIN32_FIND_DATA lpwfdData, HANDLE hAbortEvent) {
	ZeroMemory(lpwfdData, sizeof(WIN32_FIND_DATA));
	cFiryFindData* findData = new cFiryFindData();

	if (LoadFile(lpszPath)) {

		auto path = GetInsidePath(lpszPath);

		auto depth = tokenize(lpszPath, L"\\\\");
		auto filemask = depth[depth.size() - 1];
		auto Filepath = ws2s(filemask);
		path = path.substr(0, path.size() - filemask.size());

		findData->mFindMask = std::regex("(." + Filepath + ")");
		findData->mDirectory = mImage->filesystemPath(ws2s(path));
		if (!findData->mDirectory)
			return findData;

		for (findData->mIndex = 0; findData->mIndex < findData->mDirectory->mNodes.size(); ++findData->mIndex) {
			auto node = findData->mDirectory->mNodes[findData->mIndex];

			if (std::regex_match(node->nameGet(), findData->mFindMask)) {

				GetWfdForEntry(node, lpwfdData);
				++findData->mIndex;
				return findData;
			}
		}
	}

	return findData;
}

bool cFiryPluginData::FindNextFile(cFiryFindData* pFindData, LPWIN32_FIND_DATA lpwfdData) {
	ZeroMemory(lpwfdData, sizeof(WIN32_FIND_DATA));
	if (!pFindData->mDirectory)
		return false;

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

int cFiryPluginData::Import(LPVFSBATCHDATAW lpBatchData, const std::wstring& pFile, const std::wstring& pPath) {

	if (lpBatchData->hAbortEvent && WaitForSingleObject(lpBatchData->hAbortEvent, 0) == WAIT_OBJECT_0) {
		return 1;
	}

	if (!LoadFile(pFile))
		return false;

	auto path = GetInsidePath(pFile);
	auto node = mImage->filesystemPath(ws2s(path));
	return ImportNode(lpBatchData, node, pPath);
}

int cFiryPluginData::ImportNode(LPVFSBATCHDATAW lpBatchData, firy::spDirectory pDest, std::wstring pPath) {
	auto Attribs = GetFileAttributes(pPath.c_str());

	if (Attribs & FILE_ATTRIBUTE_DIRECTORY)
		return ImportPath(lpBatchData, pDest, pPath);

	return ImportFile(lpBatchData, pDest, pPath);
}

int cFiryPluginData::ImportFile(LPVFSBATCHDATAW lpBatchData, firy::spDirectory pDest, const std::wstring& pPath) {

	auto Depth = tokenize(pPath, L"\\\\");

	auto content = firy::gResources->FileRead(ws2s(pPath));

	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_SETFILENAME, (DWORD_PTR)Depth[Depth.size() - 1].c_str());
	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_SETFILESIZE, (DWORD_PTR)content->size());

	FILETIME ft;
	HANDLE filename = CreateFile(pPath.c_str(), FILE_READ_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	::GetFileTime(filename, 0, 0, &ft);
	CloseHandle(filename);

	std::string name = ws2s(Depth[Depth.size() - 1]);
	auto File = mImage->filesystemFileCreate(name);
	File->mContent = content;

	SetEntryTime(File, ft);
	auto result = pDest->add(File);

	DOpus.AddFunctionFileChange(lpBatchData->lpFuncData, true, OPUSFILECHANGE_CREATE, Depth[Depth.size() - 1].c_str());
	return result == true ? 0 : 1;
}

int cFiryPluginData::ImportPath(LPVFSBATCHDATAW lpBatchData, firy::spDirectory pDest, const std::wstring& pPath) {

	std::wstring FinalPath = pPath;
	if (FinalPath[FinalPath.size() - 1] != L'\\')
		FinalPath += L"\\";

	auto Depth = tokenize(pPath, L"\\\\");

	FILETIME ft;
	HANDLE filename = CreateFile(pPath.c_str(), FILE_READ_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	::GetFileTime(filename, 0, 0, &ft);
	CloseHandle(filename);

	DOpus.AddFunctionFileChange(lpBatchData->lpFuncData, true, OPUSFILECHANGE_MAKEDIR, FinalPath.c_str());

	std::string name = ws2s(Depth[Depth.size() - 1]);
	auto dir = mImage->filesystemDirectoryCreate(name);
	auto t = ToDateTime(ft);
	dir->timeWriteSet(t);

	pDest->add(dir);

	int result = 0;
	auto contents = directoryList(FinalPath + L"*.*");
	for (auto& File : contents) {
		if (File == L"." || File == L"..")
			continue;

		result |= ImportNode(lpBatchData, dir, FinalPath + File);
	}

	return result;
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

	if (lpBatchData->hAbortEvent && WaitForSingleObject(lpBatchData->hAbortEvent, 0) == WAIT_OBJECT_0) {
		return 1;
	}

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
		auto time = GetFileTime(pPath->timeWriteGet());

		SetFileTime(filename, 0, 0, &time);
		CloseHandle(filename);
	}

	return result;
}

int cFiryPluginData::ExtractFile(LPVFSBATCHDATAW lpBatchData, firy::spFile pEntry, const std::wstring& pDest) {
	std::string buffer(pEntry->sizeInBytesGet(), 0);

	auto content = pEntry->read();
	if (!content)
		return 1;

	std::ofstream ofile(pDest, std::ios::binary);
	ofile.write((const char*)content->data(), content->size());
	ofile.close();

	DOpus.UpdateFunctionProgressBar(lpBatchData->lpFuncData, PROGRESSACTION_STEPBYTES, (DWORD_PTR)pEntry->sizeInBytesGet());

	HANDLE filename = CreateFile(pDest.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	auto time = GetFileTime(pEntry->timeWriteGet());
	SetFileTime(filename, 0, 0, &time);
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

		*Data &= ~(VFSFUNCAVAIL_RENAME | VFSFUNCAVAIL_SETATTR | VFSFUNCAVAIL_CLIPCUT | VFSFUNCAVAIL_CLIPPASTE | VFSFUNCAVAIL_CLIPPASTESHORTCUT | VFSFUNCAVAIL_DUPLICATE);
		return true;
	}

	if (propId == VFSPROP_DRAGEFFECTS) {
		unsigned __int64* Data = (unsigned __int64*)lpPropData;

		*Data = DROPEFFECT_COPY;

		return true;
	}
	if (propId == VFSPROP_CANSHOWSUBFOLDERS)
		return false;

	return false;
}