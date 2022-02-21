//
// Created by Administrator on 2022/2/2.
//

#ifndef PEHELPER_PETOOL_H
#define PEHELPER_PETOOL_H

#include <stdio.h>
#include <Windows.h>

#define PRINTLNF(format, ...) printf(format"\n", ##__VA_ARGS__)
#define WPRINTLNF(format, ...) wprintf(L""format"\n", ##__VA_ARGS__)
#define PRINTINT(value) PRINTLNF(#value": %d", value)
#define NOTICE(format, ...) PRINTLNF("("__FILE__":%d) %s: "format, __LINE__, __FUNCTION__, ##__VA_ARGS__)


typedef struct _IMAGE_HEADER_POINTERS {
    PIMAGE_DOS_HEADER pDosHeader;
    PIMAGE_NT_HEADERS pNTHeader;
    PIMAGE_FILE_HEADER pPEHeader;
    PIMAGE_OPTIONAL_HEADER32 pOptionHeader;
    PIMAGE_SECTION_HEADER pSectionHeaders;
} IMAGE_HEADER_POINTERS, *PIMAGE_HEADER_POINTERS;

// 重要逻辑
LPVOID ReadPEFile(LPSTR lpszFile);

PIMAGE_HEADER_POINTERS ReadHeaders(LPVOID pFileBuffer);

PIMAGE_HEADER_POINTERS ParsePEFile(LPSTR lpszFile);

BOOL MemoryTOFile(IN LPVOID pMemBuffer, IN size_t size, OUT LPSTR lpszFile);

BOOL MemoryToPEFile(IN LPVOID pMemBuffer, OUT LPSTR lpszFile);

DWORD RvaToFileOffset(IN LPVOID pFileBuffer, IN DWORD dwRva);

LPVOID RVAToPTR(IN LPVOID pFileBuffer, IN DWORD dwRva);

VOID UpperHeaders(LPVOID pFileBuffer);

LPVOID RaiseABufferWithANewSection(LPVOID pFileBuffer, char newSectionName[8]);

LPVOID RaiseABufferForDll(LPVOID pFileBuffer, char newSectionName[8]);

LPVOID RaiseABufferWithALargerLastSection(LPVOID pFileBuffer);

DWORD GetFunctionAddrByName(LPVOID pFileBuffer, PSTR fname);

DWORD GetFunctionAddrByOrdinals(LPVOID pFileBuffer, DWORD ordinal);

LPVOID RaiseABufferWithAMovedExportDirectory(LPVOID pFileBuffer, char newSectionName[8]);

LPVOID RaiseABufferWithAMovedRelocation(LPVOID pFileBuffer, char newSectionName[8]);

VOID changeImageBase(LPVOID pFileBuffer, DWORD newImageBase);

// 简单功能
DWORD CopyFileBufferToImageBuffer(IN LPVOID pFileBuffer, OUT LPVOID *ppImageBuffer);

DWORD CopyImageBufferToNewBuffer(IN LPVOID pImageBuffer, OUT LPVOID *ppNewBuffer);


// 算是示例代码吧
VOID GetRealHeaders(LPSTR lpszFile);

VOID FileToBufferToFile(LPSTR InitialFileName, LPSTR FinalFileName);

VOID GiveAMessageBoxToAFile1(LPSTR InitialFileName, LPSTR FinalFileName);

BOOL GiveAMessageBoxToAFile2(LPSTR InitialFileName, LPSTR FinalFileName);

BOOL GiveAMessageBoxToAFile3(LPSTR InitialFileName, LPSTR FinalFileName);

VOID PrintFunctionNames(LPVOID pFileBuffer);

VOID PrintBaseReloc(LPVOID pFileBuffer);

VOID PrintImportDescriptor(LPVOID pFileBuffer);

VOID TestIATInjectDll();

VOID PrintResourceDirectory(LPVOID pFileBuffer);

#endif // PEHELPER_PETOOL_H
