#include "PETool.h"

/*
 * lpszFile: �ļ���
 * ����ֵ��ָ���ȡ�����ڴ������׵�ַ��ָ��
 * */
LPVOID ReadPEFile(LPSTR lpszFile) {
    FILE *pFile = NULL;
    DWORD fileSize = 0;
    LPVOID pFileBuffer = NULL;

    //���ļ�
    pFile = fopen(lpszFile, "rb");
    if (!pFile) {
        return NULL;
    }
    //��ȡ�ļ���С
    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    //���仺����
    pFileBuffer = malloc(fileSize);

    if (!pFileBuffer) {
        fclose(pFile);
        return NULL;
    }
    //���ļ����ݶ�ȡ��������
    size_t n = fread(pFileBuffer, fileSize, 1, pFile);
    if (!n) {
        free(pFileBuffer);
        fclose(pFile);
        return NULL;
    }
    //�ر��ļ�
    fclose(pFile);
    return pFileBuffer;
}

/*
 * pFileBuffer: ָ���ȡ�����ڴ������׵�ַ��ָ��
 * �ú�����ɶ�PE�ļ���ͷ�Ķ�ȡ���ŵ�PETool.h��������ȫ�ֱ�����
 * */
PIMAGE_HEADER_POINTERS ReadHeaders(LPVOID pFileBuffer) {

    if (!pFileBuffer) {
        return NULL;
    }

    //�ж��Ƿ�����Ч��MZ��־
    if (*((PWORD) pFileBuffer) != IMAGE_DOS_SIGNATURE) {
        return NULL;
    }
    PIMAGE_HEADER_POINTERS pImageHeaders = (PIMAGE_HEADER_POINTERS) malloc(sizeof(IMAGE_HEADER_POINTERS));
    pImageHeaders->pDosHeader = (PIMAGE_DOS_HEADER) pFileBuffer;
    //�ж��Ƿ�����Ч��PE��־
    if (*((PDWORD) ((DWORD) pFileBuffer + pImageHeaders->pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE) {
        printf("������Ч��PE��־\n");
        return NULL;
    }
    pImageHeaders->pNTHeader = (PIMAGE_NT_HEADERS) ((DWORD) pFileBuffer + pImageHeaders->pDosHeader->e_lfanew);
    pImageHeaders->pPEHeader = (PIMAGE_FILE_HEADER) (((DWORD) pImageHeaders->pNTHeader) + 4);
    pImageHeaders->pOptionHeader = (PIMAGE_OPTIONAL_HEADER32) ((DWORD) pImageHeaders->pPEHeader +
                                                               IMAGE_SIZEOF_FILE_HEADER);

    // ������һ���ṹ��ָ������һ��SectionHeader�Ľṹ������
//    pImageHeaders->pSectionHeaders = malloc(pImageHeaders->NumberOfSections * sizeof(PIMAGE_SECTION_HEADER));
//    for (int i = 0; i < pImageHeaders->NumberOfSections; i++) {
//        pImageHeaders->pSectionHeaders[i] = ((PIMAGE_SECTION_HEADER) ((DWORD) pImageHeaders->pOptionHeader + pImageHeaders->pPEHeader->SizeOfOptionalHeader)) + i;
//    }

    pImageHeaders->pSectionHeaders = (PIMAGE_SECTION_HEADER) ((DWORD) pImageHeaders->pOptionHeader +
                                                              pImageHeaders->pPEHeader->SizeOfOptionalHeader);

    return pImageHeaders;
}

//**************************************************************************
// RvaToFileOffset:���ڴ�ƫ��ת��Ϊ�ļ�ƫ��
// ����˵����
// pFileBuffer FileBufferָ��
// dwRva RVA��ֵ
// ����ֵ˵����
// ����ת�����FOA��ֵ  ���ʧ�ܷ���-1
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

// ���ڴ�ƫ��ת��Ϊ��ǰ�����ڿ�ֱ�Ӷ�д��ָ�룬��RvaToFileOffset�Ļ���������һ��С��װ
LPVOID RVAToPTR(IN LPVOID pFileBuffer, IN DWORD dwRva) {
    return (LPVOID) ((DWORD) pFileBuffer + RvaToFileOffset(pFileBuffer, dwRva));
}