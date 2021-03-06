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

class cFiryFindData {

public:
	firy::spDirectory mDirectory;
	std::regex mFindMask;
	size_t mIndex;

	cFiryFindData() {
		mIndex = 0;
	}
};

class cFiryFile {
public:
	firy::spFile mFile;
	firy::spBuffer mBuffer;

	cFiryFile(firy::spFile pFile) { mFile = pFile;}
};

class cFiryPluginData {

	size_t                  mLastError;
	std::wstring             mSourcePath;

	firy::spImage			 mImage;

protected:

	LPVFSFILEDATAHEADER GetVFSforEntry(firy::spNode pEntry, HANDLE pHeap);
	void GetWfdForEntry(firy::spNode pEntry, LPWIN32_FIND_DATA pData);

	FILETIME            GetFileTime(const firy::helpers::sDateTime& pDateTime);
	void                SetEntryTime(firy::spFile pFile, FILETIME pFT);

	bool LoadFile(const std::wstring& pPath);
	std::wstring GetInsidePath(std::wstring pPath);

public:
	cFiryPluginData();
	~cFiryPluginData();

	bool CreateDir(const std::wstring& pPath);
	bool ReadDirectory(LPVFSREADDIRDATAW lpRDD);
	bool ReadFile(cFiryFile* pFile, size_t pBytes, std::uint8_t* pBuffer, LPDWORD pReadSize);
	bool DeleteFile(const std::wstring& pPath);

	cFiryFile* OpenFile(std::wstring pPath);
	void CloseFile(cFiryFile* pFile);

	size_t TotalFreeBlocks(const std::wstring& pFile);
	size_t TotalDiskBlocks(const std::wstring& pFile);

	int Delete(LPVFSBATCHDATAW lpBatchData, const std::wstring& pPath, const std::wstring& pFile, bool pAll = false);

	cFiryFindData* FindFirstFile(LPTSTR lpszPath, LPWIN32_FIND_DATA lpwfdData, HANDLE hAbortEvent);
	bool FindNextFile(cFiryFindData* lpRAF, LPWIN32_FIND_DATA lpwfdData);
	void FindClose(cFiryFindData* lpRAF);

	int Import(LPVFSBATCHDATAW lpBatchData, const std::wstring& pDest, const std::wstring& pPath);
	int ImportNode(LPVFSBATCHDATAW lpBatchData, firy::spDirectory pDest, std::wstring pPath);
	int ImportFile(LPVFSBATCHDATAW lpBatchData, firy::spDirectory pDest, const std::wstring& pPath);
	int ImportPath(LPVFSBATCHDATAW lpBatchData, firy::spDirectory pDest, const std::wstring& pPath);

	int Extract(LPVFSBATCHDATAW lpBatchData, const std::wstring& pFile, const std::wstring& pDest);
	int ExtractNode(LPVFSBATCHDATAW lpBatchData, firy::spNode pNode, std::wstring pDest);
	int ExtractFile(LPVFSBATCHDATAW lpBatchData, firy::spFile pEntry, const std::wstring& pDest);
	int ExtractPath(LPVFSBATCHDATAW lpBatchData, firy::spDirectory pPath, const std::wstring& pDest);

	int ContextVerb(LPVFSCONTEXTVERBDATAW lpVerbData);
	UINT BatchOperation(LPTSTR lpszPath, LPVFSBATCHDATAW lpBatchData);
	bool PropGet(vfsProperty propId, LPVOID lpPropData, LPVOID lpData1, LPVOID lpData2, LPVOID lpData3);

};

std::wstring s2ws(const std::string& str);
