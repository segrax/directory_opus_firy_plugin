// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

// b3e2dc4a-3853-41cd-b7b4-c522bbcc5b07
static const GUID GUIDPlugin_ADF = { 0xb3e2dc4a, 0x3853, 0x41cd,{ 0xb7, 0xb4, 0xc5, 0x22, 0xbb, 0xcc,0x5b, 0x07 } };

HINSTANCE g_hModuleInstance = 0;


extern "C"
{
    __declspec(dllexport) bool VFS_IdentifyW(LPVFSPLUGININFOW lpVFSInfo);
    __declspec(dllexport) bool VFS_ReadDirectoryW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPVFSREADDIRDATAW lpRDD);
	__declspec(dllexport) bool VFS_CreateDirectoryW(HANDLE hVFSData, LPVFSFUNCDATA lpFuncData, LPWSTR lpszPath, DWORD dwFlags);

    __declspec(dllexport) HANDLE VFS_Create(LPGUID pGuid);
    __declspec(dllexport) void VFS_Destroy(HANDLE hData);

    __declspec(dllexport) HANDLE VFS_CreateFileW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPWSTR lpszPath, DWORD dwMode, DWORD dwFileAttr, DWORD dwFlags, LPFILETIME lpFT);
	__declspec(dllexport) bool VFS_DeleteFileW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPWSTR lpszPath, DWORD dwFlags, int iSecurePasses);
    __declspec(dllexport) bool VFS_ReadFile(HANDLE hData, LPVFSFUNCDATA lpVFSData, HANDLE hFile, LPVOID lpData, DWORD dwSize, LPDWORD lpdwReadSize);
    __declspec(dllexport) void VFS_CloseFile(HANDLE hData, LPVFSFUNCDATA lpVFSData, HANDLE hFile);

    __declspec(dllexport) int VFS_ContextVerbW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPVFSCONTEXTVERBDATAW lpVerbData);
    __declspec(dllexport) UINT VFS_BatchOperationW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPWSTR lpszPath, LPVFSBATCHDATAW lpBatchData);
    __declspec(dllexport) bool VFS_PropGetW(HANDLE hVFSData, vfsProperty propId, LPVOID lpPropData, LPVOID lpData1, LPVOID lpData2, LPVOID lpData3);

    __declspec(dllexport) bool VFS_GetFreeDiskSpaceW(HANDLE hData, LPVFSFUNCDATA lpFuncData, LPWSTR lpszPath, unsigned __int64* piFreeBytesAvailable, unsigned __int64* piTotalBytes, unsigned __int64* piTotalFreeBytes);

    __declspec(dllexport) HANDLE VFS_FindFirstFileW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPWSTR lpszPath, LPWIN32_FIND_DATA lpwfdData, HANDLE hAbortEvent);
    __declspec(dllexport) bool VFS_FindNextFileW(HANDLE hData, LPVFSFUNCDATA lpVFSData, HANDLE hFind, LPWIN32_FIND_DATA lpwfdData);
    __declspec(dllexport) void VFS_FindClose(HANDLE hData, HANDLE hFind);


    __declspec(dllexport) bool VFS_USBSafe(LPOPUSUSBSAFEDATA pUSBSafeData);
    __declspec(dllexport) bool VFS_Init(LPVFSINITDATA pInitData);
    __declspec(dllexport) void VFS_Uninit();
};


extern "C" int WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID) {
    g_hModuleInstance = hInstance;
	firy::gOptions = std::make_shared<cOpusOptions>();
    return 1;
}

// Initialise plugin
bool VFS_Init(LPVFSINITDATA pInitData) {

    return true;
}

void VFS_Uninit() {

}

bool VFS_IdentifyW(LPVFSPLUGININFOW lpVFSInfo) {
    // Initialise plugin information
    lpVFSInfo->idPlugin = GUIDPlugin_ADF;
    lpVFSInfo->dwFlags = VFSF_CANCONFIGURE | VFSF_NONREENTRANT;
    lpVFSInfo->dwCapabilities = VFSCAPABILITY_CASESENSITIVE | VFSCAPABILITY_POSTCOPYREREAD;

	std::string fin;
	auto extensions = firy::cFiry::getKnownExtensions();
	for (auto& ext : extensions) {
		if (fin.size())
			fin += ";";

		fin += "." + ext;
	}

    StringCchCopyW(lpVFSInfo->lpszHandleExts, lpVFSInfo->cchHandleExtsMax, s2ws(fin).data());
    StringCchCopyW(lpVFSInfo->lpszName, lpVFSInfo->cchNameMax, L"Firy disk image support");
    StringCchCopyW(lpVFSInfo->lpszDescription, lpVFSInfo->cchDescriptionMax, L"Disk image support");
    StringCchCopyW(lpVFSInfo->lpszCopyright, lpVFSInfo->cchCopyrightMax, L"(c) Copyright 2020 Robert Crossfield");
    StringCchCopyW(lpVFSInfo->lpszURL, lpVFSInfo->cchURLMax, L"github.com/segrax");

    return true;
}

bool VFS_USBSafe(LPOPUSUSBSAFEDATA pUSBSafeData) {
    return true;
}

HANDLE VFS_Create(LPGUID pGuid) {
    return (HANDLE)new cFiryPluginData();
}

void VFS_Destroy(HANDLE hData) {
    delete (cFiryPluginData*)hData;
}

bool VFS_DeleteFileW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPWSTR lpszPath, DWORD dwFlags, int iSecurePasses) {
	return (hData) ? ((cFiryPluginData*)hData)->DeleteFile(lpszPath) : false;
}

bool VFS_ReadFile(HANDLE hData, LPVFSFUNCDATA lpVFSData, HANDLE hFile, LPVOID lpData, DWORD dwSize, LPDWORD lpdwReadSize) {
    return (hData) ? ((cFiryPluginData*)hData)->ReadFile((cFiryFile*)hFile, dwSize, (std::uint8_t*) lpData, lpdwReadSize) : false;
    
}

void VFS_CloseFile(HANDLE hData, LPVFSFUNCDATA lpVFSData, HANDLE hFile) {
    ((cFiryPluginData*)hData)->CloseFile((cFiryFile*)hFile);
}

HANDLE VFS_CreateFileW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPWSTR lpszPath, DWORD dwMode, DWORD dwFileAttr, DWORD dwFlags, LPFILETIME lpFT) {
    return (hData) ? ((cFiryPluginData*)hData)->OpenFile(lpszPath) : 0;

}

bool VFS_ReadDirectoryW(HANDLE hData, LPVFSFUNCDATA lpFuncData, LPVFSREADDIRDATAW lpRDD) {
    return (hData) ? ((cFiryPluginData*)hData)->ReadDirectory(lpRDD) : false;
}

bool VFS_CreateDirectoryW(HANDLE hData, LPVFSFUNCDATA lpFuncData, LPWSTR lpszPath, DWORD dwFlags) {
	return (hData) ? ((cFiryPluginData*)hData)->CreateDir(lpszPath) : false;
}

int VFS_ContextVerbW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPVFSCONTEXTVERBDATAW lpVerbData) {

    return (hData) ? ((cFiryPluginData*)hData)->ContextVerb(lpVerbData) : VFSCVRES_FAIL;
}

UINT VFS_BatchOperationW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPWSTR lpszPath, LPVFSBATCHDATAW lpBatchData) {
    return (hData) ? ((cFiryPluginData*)hData)->BatchOperation(lpszPath, lpBatchData) : VFSCVRES_FAIL;
}

bool VFS_PropGetW(HANDLE hData, vfsProperty propId, LPVOID lpPropData, LPVOID lpData1, LPVOID lpData2, LPVOID lpData3) {

    return (hData) ? ((cFiryPluginData*)hData)->PropGet(propId, lpPropData, lpData1, lpData2, lpData3) : VFSCVRES_FAIL;
}

bool VFS_GetFreeDiskSpaceW(HANDLE hData, LPVFSFUNCDATA lpFuncData, LPWSTR lpszPath, unsigned __int64* piFreeBytesAvailable, unsigned __int64* piTotalBytes, unsigned __int64* piTotalFreeBytes) {

    if (!hData)
        return false;

    if(piFreeBytesAvailable)
        *piFreeBytesAvailable = ((cFiryPluginData*)hData)->TotalFreeBlocks(lpszPath);

    if(piTotalFreeBytes)
        *piTotalFreeBytes = ((cFiryPluginData*)hData)->TotalFreeBlocks(lpszPath);

    if(piTotalBytes)
        *piTotalBytes = ((cFiryPluginData*)hData)->TotalDiskBlocks(lpszPath);
    
    return true;
}

HANDLE VFS_FindFirstFileW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPWSTR lpszPath, LPWIN32_FIND_DATA lpwfdData, HANDLE hAbortEvent) {

    return (hData) ? (HANDLE)((cFiryPluginData*)hData)->FindFirstFile(lpszPath, lpwfdData, hAbortEvent) : INVALID_HANDLE_VALUE;
}

bool VFS_FindNextFileW(HANDLE hData, LPVFSFUNCDATA lpVFSData, HANDLE hFind, LPWIN32_FIND_DATA lpwfdData) {
    return (hData && hFind) ? ((cFiryPluginData*)hData)->FindNextFile((cFiryFindData*)hFind, lpwfdData) : false;
}

void VFS_FindClose(HANDLE hData, HANDLE hFind) {
    if (hData && hFind) ((cFiryPluginData*)hData)->FindClose((cFiryFindData*)hFind);
}
