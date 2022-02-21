#include "PETool.h"

/*
 * lpszFile: 文件名
 * 返回值是指向读取到的内存区域首地址的指针
 * */
LPVOID ReadPEFile(LPSTR lpszFile) {
    FILE *pFile = NULL;
    DWORD fileSize = 0;
    LPVOID pFileBuffer = NULL;

    //打开文件
    pFile = fopen(lpszFile, "rb");
    if (!pFile) {
        return NULL;
    }
    //读取文件大小
    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    //分配缓冲区
    pFileBuffer = malloc(fileSize);

    if (!pFileBuffer) {
        fclose(pFile);
        return NULL;
    }
    //将文件数据读取到缓冲区
    size_t n = fread(pFileBuffer, fileSize, 1, pFile);
    if (!n) {
        free(pFileBuffer);
        fclose(pFile);
        return NULL;
    }
    //关闭文件
    fclose(pFile);
    return pFileBuffer;
}

/*
 * pFileBuffer: 指向读取到的内存区域首地址的指针
 * 该函数完成对PE文件的头的读取，放到PETool.h里声明的全局变量中
 * */
PIMAGE_HEADER_POINTERS ReadHeaders(LPVOID pFileBuffer) {

    if (!pFileBuffer) {
        return NULL;
    }

    //判断是否是有效的MZ标志
    if (*((PWORD) pFileBuffer) != IMAGE_DOS_SIGNATURE) {
        return NULL;
    }
    PIMAGE_HEADER_POINTERS pImageHeaders = (PIMAGE_HEADER_POINTERS) malloc(sizeof(IMAGE_HEADER_POINTERS));
    pImageHeaders->pDosHeader = (PIMAGE_DOS_HEADER) pFileBuffer;
    //判断是否是有效的PE标志
    if (*((PDWORD) ((DWORD) pFileBuffer + pImageHeaders->pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE) {
        printf("不是有效的PE标志\n");
        return NULL;
    }
    pImageHeaders->pNTHeader = (PIMAGE_NT_HEADERS) ((DWORD) pFileBuffer + pImageHeaders->pDosHeader->e_lfanew);
    pImageHeaders->pPEHeader = (PIMAGE_FILE_HEADER) (((DWORD) pImageHeaders->pNTHeader) + 4);
    pImageHeaders->pOptionHeader = (PIMAGE_OPTIONAL_HEADER32) ((DWORD) pImageHeaders->pPEHeader +
                                                               IMAGE_SIZEOF_FILE_HEADER);

    // 这里拿一个结构体指针来接一个SectionHeader的结构体数组
//    pImageHeaders->pSectionHeaders = malloc(pImageHeaders->NumberOfSections * sizeof(PIMAGE_SECTION_HEADER));
//    for (int i = 0; i < pImageHeaders->NumberOfSections; i++) {
//        pImageHeaders->pSectionHeaders[i] = ((PIMAGE_SECTION_HEADER) ((DWORD) pImageHeaders->pOptionHeader + pImageHeaders->pPEHeader->SizeOfOptionalHeader)) + i;
//    }

    pImageHeaders->pSectionHeaders = (PIMAGE_SECTION_HEADER) ((DWORD) pImageHeaders->pOptionHeader +
                                                              pImageHeaders->pPEHeader->SizeOfOptionalHeader);

    return pImageHeaders;
}

//**************************************************************************
// RvaToFileOffset:将内存偏移转换为文件偏移
// 参数说明：
// pFileBuffer FileBuffer指针
// dwRva RVA的值
// 返回值说明：
// 返回转换后的FOA的值  如果失败返回-1
//**************************************************************************
DWORD RvaToFileOffset(IN LPVOID pFileBuffer, IN DWORD dwRva) {
    PIMAGE_HEADER_POINTERS pImageHeaders = ReadHeaders(pFileBuffer);
    DWORD sizeOfHeaders = pImageHeaders->pOptionHeader->SizeOfHeaders;
    DWORD sizeOfImage = pImageHeaders->pOptionHeader->SizeOfImage;
    WORD numberOfSections = pImageHeaders->pPEHeader->NumberOfSections;
    PIMAGE_SECTION_HEADER pSectionHeaders = pImageHeaders->pSectionHeaders;

    if (dwRva > sizeOfImage) return -1;
    if (dwRva < sizeOfHeaders) return dwRva;
    for (int i = numberOfSections - 1; i >= 0; i--) {
        if (dwRva >= pSectionHeaders[i].VirtualAddress) {
            return dwRva - (pSectionHeaders[i].VirtualAddress - pSectionHeaders[i].PointerToRawData);
        }
    }
    return -1;
}

// 将内存偏移转换为当前程序内可直接读写的指针，在RvaToFileOffset的基础上做了一层小封装
LPVOID RVAToPTR(IN LPVOID pFileBuffer, IN DWORD dwRva) {
    return (LPVOID) ((DWORD) pFileBuffer + RvaToFileOffset(pFileBuffer, dwRva));
}