// PEViewer.cpp : 定义应用程序的入口点。
//

#include "PEViewer.h"

#define MAX_LOADSTRING 100

// 全局变量:

WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM MyRegisterClass(HINSTANCE hInstance);

BOOL InitInstance(HINSTANCE, int);

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HWND hPEDlg;
HWND hSectionDlg;
HWND hDirectoryDlg;
HWND hListSection;
LPVOID pFileBuffer;
PIMAGE_HEADER_POINTERS pImageHeaders;

CHAR info[0x1000000];

int flag = -1;
enum {
    FLAG_EXPORT,
    FLAG_IMPORT,
    FLAG_RESOURCE,
    FLAG_BASERELOC
};

LV_ITEMW vitem;

PSTR GetStrA(PWSTR wstr) {
    int len = WideCharToMultiByte(CP_ACP, 0, wstr, wcslen(wstr), NULL, 0, NULL, NULL);
    CHAR *m_char = new CHAR[len + 1];
    WideCharToMultiByte(CP_ACP, 0, wstr, wcslen(wstr), m_char, len, NULL, NULL);
    m_char[len] = '\0';
    return m_char;
}

VOID AppendInfo(CHAR *str) {
    strcat(info, str);
    strcat(info, "\r\n");
}

INT_PTR CALLBACK InfoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);

    hSectionDlg = hDlg;
    CHAR temp[128];
    memset(info, 0, 1024);

    switch (message) {
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDCANCEL: {
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR) TRUE;
                }
            }
            break;
        }
        case WM_INITDIALOG: {

            switch (flag) {
                case FLAG_EXPORT: {

                    PIMAGE_DATA_DIRECTORY DataDirectory = pImageHeaders->pOptionHeader->DataDirectory;

                    if (DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress == 0) {
                        AppendInfo("没有导出表，打个屁。");
                        SetDlgItemTextA(hDlg, IDC_EDIT_INFO, info);
                        break;
                    }

                    PIMAGE_EXPORT_DIRECTORY pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY) RVAToPTR(pFileBuffer,
                                                                                                       DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
                    DWORD numberOfNames = pImageExportDirectory->NumberOfNames;
                    PSTR *functionNames = (PSTR *) RVAToPTR(pFileBuffer, pImageExportDirectory->AddressOfNames);

                    AppendInfo("打印导出表内的函数名：");
                    for (DWORD i = 0; i < numberOfNames; i++) {
                        sprintf(temp, "%s", (PSTR) RVAToPTR(pFileBuffer, (DWORD) functionNames[i]));
                        AppendInfo(temp);
                    }
                    SetDlgItemTextA(hDlg, IDC_EDIT_INFO, info);
                    break;
                }
                case FLAG_IMPORT: {
                    PIMAGE_DATA_DIRECTORY dataDirectory = pImageHeaders->pOptionHeader->DataDirectory;
                    PIMAGE_IMPORT_DESCRIPTOR pImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR) RVAToPTR(pFileBuffer,
                                                                                                          dataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
                    AppendInfo("开始打印导入表：");

                    while (pImageImportDescriptor->OriginalFirstThunk != 0) {

                        PDWORD originalFirstThunk = (PDWORD) RVAToPTR(pFileBuffer,
                                                                      pImageImportDescriptor->OriginalFirstThunk);
                        PDWORD firstThunk = (PDWORD) RVAToPTR(pFileBuffer, pImageImportDescriptor->FirstThunk);
                        PSTR name = (PSTR) RVAToPTR(pFileBuffer, pImageImportDescriptor->Name);

                        AppendInfo("===========================================");
                        sprintf(temp, "%s", name);
                        AppendInfo(temp);

                        AppendInfo("开始遍历OriginalFirstThunk：");
                        while (*originalFirstThunk != 0) {
                            DWORD thunk = *originalFirstThunk;
                            DWORD flag = (thunk & 0x80000000) >> 31;
                            if (flag) {
                                sprintf(temp, "按序号导入：%08lX", thunk & 0x7FFFFFFF);
                                AppendInfo(temp);
                            } else {
                                PIMAGE_IMPORT_BY_NAME pImageImportByName = (PIMAGE_IMPORT_BY_NAME) RVAToPTR(pFileBuffer,
                                                                                                            thunk);
                                sprintf(temp, "按名字导入：{Hint: %04X    Name: %s}", pImageImportByName->Hint,
                                        pImageImportByName->Name);
                                AppendInfo(temp);
                            }
                            originalFirstThunk++;
                        }

                        AppendInfo("开始遍历FirstThunk：");
                        while (*firstThunk != 0) {
                            DWORD thunk = *firstThunk;
                            DWORD flag = (thunk & 0x80000000) >> 31;
                            if (flag) {
                                sprintf(temp, "按序号导入：%08lX", thunk & 0x7FFFFFFF);
                                AppendInfo(temp);
                            } else {
                                PIMAGE_IMPORT_BY_NAME pImageImportByName = (PIMAGE_IMPORT_BY_NAME) RVAToPTR(pFileBuffer,
                                                                                                            thunk);
                                sprintf(temp, "按名字导入：{Hint: %04X    Name: %s}", pImageImportByName->Hint,
                                        pImageImportByName->Name);
                                AppendInfo(temp);
                            }
                            firstThunk++;
                        }

                        AppendInfo("");

                        pImageImportDescriptor++;
                    }

                    SetDlgItemTextA(hDlg, IDC_EDIT_INFO, info);
                    break;
                }
                case FLAG_RESOURCE: {
                    AppendInfo("开始打印资源表");
                    AppendInfo("");

                    PIMAGE_DATA_DIRECTORY pImageDataDirectory = pImageHeaders->pOptionHeader->DataDirectory;
                    PIMAGE_RESOURCE_DIRECTORY pImageResourceDirectory1 = (PIMAGE_RESOURCE_DIRECTORY) RVAToPTR(
                            pFileBuffer,
                            pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
                    WORD numberOfEntries1 =
                            pImageResourceDirectory1->NumberOfIdEntries +
                            pImageResourceDirectory1->NumberOfNamedEntries;
                    PIMAGE_RESOURCE_DIRECTORY_ENTRY pImageResourceDirectoryEntry1 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY) (
                            pImageResourceDirectory1 + 1);

                    sprintf(temp, "================================第一层（类型）：共%d条================================",
                            numberOfEntries1);
                    AppendInfo(temp);

                    for (int i = 0; i < numberOfEntries1; i++) {
                        if (pImageResourceDirectoryEntry1[i].NameIsString) {
                            PIMAGE_RESOURCE_DIR_STRING_U dirStringU1 = (PIMAGE_RESOURCE_DIR_STRING_U) (
                                    (DWORD) pImageResourceDirectory1 +
                                    pImageResourceDirectoryEntry1[i].NameOffset);
                            PWSTR name1 = (PWSTR) malloc(dirStringU1->Length * 2 + 2);
                            memset(name1, 0, dirStringU1->Length * 2 + 2);
                            memcpy(name1, dirStringU1->NameString, dirStringU1->Length * 2);
                            sprintf(temp, "第一层（类型），第%d条--资源类型名称：%s", i + 1, GetStrA(name1));
                            AppendInfo(temp);
                        } else {
                            WORD id1 = pImageResourceDirectoryEntry1[i].Id;
                            sprintf(temp, "第一层（类型），第%d条--资源类型ID：%d", i + 1, id1);
                            AppendInfo(temp);
                        }
                        PIMAGE_RESOURCE_DIRECTORY pImageResourceDirectory2 = (PIMAGE_RESOURCE_DIRECTORY) (
                                (DWORD) pImageResourceDirectory1 + pImageResourceDirectoryEntry1[i].OffsetToDirectory);
                        WORD numberOfEntries2 =
                                pImageResourceDirectory2->NumberOfIdEntries +
                                pImageResourceDirectory2->NumberOfNamedEntries;
                        PIMAGE_RESOURCE_DIRECTORY_ENTRY pImageResourceDirectoryEntry2 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY) (
                                pImageResourceDirectory2 + 1);

                        sprintf(temp, "===========================第%d条的第二层（编号）：共%d条===========================", i + 1,
                                numberOfEntries2);
                        AppendInfo(temp);

                        for (int j = 0; j < numberOfEntries2; j++) {
                            if (pImageResourceDirectoryEntry2[j].NameIsString) {
                                PIMAGE_RESOURCE_DIR_STRING_U dirStringU2 = (PIMAGE_RESOURCE_DIR_STRING_U) (
                                        (DWORD) pImageResourceDirectory1 +
                                        pImageResourceDirectoryEntry2[j].NameOffset);
                                PWSTR name2 = (PWSTR) malloc(dirStringU2->Length * 2 + 2);
                                memset(name2, 0, dirStringU2->Length * 2 + 2);
                                memcpy(name2, dirStringU2->NameString, dirStringU2->Length * 2);
                                sprintf(temp, "第二层（编号），第%d条--名称：%s", j + 1, GetStrA(name2));
                                AppendInfo(temp);
                            } else {
                                WORD id2 = pImageResourceDirectoryEntry2[j].Id;
                                sprintf(temp, "第二层（编号），第%d条--ID：%d", j + 1, id2);
                                AppendInfo(temp);
                            }

                            PIMAGE_RESOURCE_DIRECTORY pImageResourceDirectory3 = (PIMAGE_RESOURCE_DIRECTORY) (
                                    (DWORD) pImageResourceDirectory1 +
                                    pImageResourceDirectoryEntry2[j].OffsetToDirectory);
                            WORD numberOfEntries3 =
                                    pImageResourceDirectory3->NumberOfIdEntries +
                                    pImageResourceDirectory3->NumberOfNamedEntries;
                            PIMAGE_RESOURCE_DIRECTORY_ENTRY pImageResourceDirectoryEntry3 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY) (
                                    pImageResourceDirectory3 + 1);

                            sprintf(temp, "================第%d条的第三层（代码页）：共%d条================", j + 1,
                                    numberOfEntries3);
                            AppendInfo(temp);

                            for (int k = 0; k < numberOfEntries3; k++) {
                                if (pImageResourceDirectoryEntry3[k].NameIsString) {
                                    PIMAGE_RESOURCE_DIR_STRING_U dirStringU3 = (PIMAGE_RESOURCE_DIR_STRING_U) (
                                            (DWORD) pImageResourceDirectory1 +
                                            pImageResourceDirectoryEntry3[k].NameOffset);
                                    PWSTR name3 = (PWSTR) malloc(dirStringU3->Length * 2 + 2);
                                    memset(name3, 0, dirStringU3->Length * 2 + 2);
                                    memcpy(name3, dirStringU3->NameString, dirStringU3->Length * 2);
                                    sprintf(temp, "第三层（代码页），第%d条--名称：%s", k + 1, GetStrA(name3));
                                    AppendInfo(temp);
                                } else {
                                    WORD id3 = pImageResourceDirectoryEntry3[k].Id;
                                    sprintf(temp, "第三层（代码页），第%d条--ID：%d", k + 1, id3);
                                    AppendInfo(temp);
                                }

                                PIMAGE_DATA_DIRECTORY dataEntryDirectory = (PIMAGE_DATA_DIRECTORY) (
                                        (DWORD) pImageResourceDirectory1 +
                                        pImageResourceDirectoryEntry3[k].OffsetToData);
                                sprintf(temp, "VirtualAddress:%08lX", dataEntryDirectory->VirtualAddress);
                                AppendInfo(temp);
                                sprintf(temp, "Size:%08lX", dataEntryDirectory->Size);
                                AppendInfo(temp);
                            }

                            AppendInfo("");
                        }
                        AppendInfo("");
                    }

                    SetDlgItemTextA(hDlg, IDC_EDIT_INFO, info);
                    break;
                }
                case FLAG_BASERELOC: {

                    PIMAGE_DATA_DIRECTORY DataDirectory = pImageHeaders->pOptionHeader->DataDirectory;

                    if (DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress == 0) {
                        AppendInfo("没有重定位表，打个屁。");
                        SetDlgItemTextA(hDlg, IDC_EDIT_INFO, info);
                        break;
                    }

                    PIMAGE_BASE_RELOCATION pImageBaseRelocation = (PIMAGE_BASE_RELOCATION) RVAToPTR(pFileBuffer,
                                                                                                    DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

                    AppendInfo("开始打印重定位表：");
                    while (pImageBaseRelocation->VirtualAddress != 0 && pImageBaseRelocation->SizeOfBlock != 0) {

                        DWORD numberOfItems = (pImageBaseRelocation->SizeOfBlock - 8) / 2;
                        WORD *items = (WORD *) ((DWORD) pImageBaseRelocation + 8);

                        AppendInfo("========================================");
                        sprintf(temp, "当前重定位表地址:0x%08lx", pImageBaseRelocation->VirtualAddress);
                        AppendInfo(temp);
                        sprintf(temp, "当前重定位表表项数:%lu", numberOfItems);
                        AppendInfo(temp);
                        AppendInfo("开始打印重定位表表项信息:");
                        for (DWORD i = 0; i < numberOfItems; i++) {
                            DWORD offset = items[i] & 0xFFF;
                            DWORD rva = pImageBaseRelocation->VirtualAddress + offset;
                            BYTE type = (BYTE) (items[i] >> 12);
                            sprintf(temp, "第%d个表项:\tRVA:0x%08x\t属性值:0x%x", i, rva, type);
                            AppendInfo(temp);
                        }
                        pImageBaseRelocation = (PIMAGE_BASE_RELOCATION) ((DWORD) pImageBaseRelocation +
                                                                         pImageBaseRelocation->SizeOfBlock);
                    }

                    SetDlgItemTextA(hDlg, IDC_EDIT_INFO, info);
                    break;
                }
                default: {
                    EndDialog(hDlg, 0);
                }
            }
            return TRUE;
        }
    }
    return (INT_PTR) FALSE;
}

VOID SetRvaAndSize(int rvaItem, int sizeItem, DWORD rva, DWORD size) {

    CHAR lpString1[256];
    CHAR lpString2[256];
    memset(lpString1, 0, 256);
    memset(lpString2, 0, 256);
    sprintf(lpString1, "%08lX", rva);
    sprintf(lpString2, "%08lX", size);


    SetDlgItemTextA(hDirectoryDlg, rvaItem, lpString1);
    SetDlgItemTextA(hDirectoryDlg, sizeItem, lpString2);
}

INT_PTR CALLBACK DirectoryDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);

    hDirectoryDlg = hDlg;

    switch (message) {
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDCANCEL: {
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR) TRUE;
                }
                case IDC_BUTTON_EXPORT: {
                    flag = FLAG_EXPORT;
                    DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_INFO), hDlg, InfoDlgProc);
                    return TRUE;
                }
                case IDC_BUTTON_IMPORT: {
                    flag = FLAG_IMPORT;
                    DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_INFO), hDlg, InfoDlgProc);
                    return TRUE;
                }
                case IDC_BUTTON_RESOURCE: {
                    flag = FLAG_RESOURCE;
                    DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_INFO), hDlg, InfoDlgProc);
                    return TRUE;
                }
                case IDC_BUTTON_BASERELOC: {
                    flag = FLAG_BASERELOC;
                    DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_INFO), hDlg, InfoDlgProc);
                    return TRUE;
                }
            }
            break;
        }
        case WM_INITDIALOG: {
            PIMAGE_DATA_DIRECTORY pImageDataDirectory = pImageHeaders->pOptionHeader->DataDirectory;

            SetRvaAndSize(IDC_EDIT_RVAEXPORT, IDC_EDIT_SIZEEXPORT,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size);
            SetRvaAndSize(IDC_EDIT_RVAIMPORT, IDC_EDIT_SIZEIMPORT,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size);
            SetRvaAndSize(IDC_EDIT_RVARESOURCE, IDC_EDIT_SIZERESOURCE,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size);
            SetRvaAndSize(IDC_EDIT_RVAEXCEPTION, IDC_EDIT_SIZEEXCEPTION,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size);
            SetRvaAndSize(IDC_EDIT_RVASECURITY, IDC_EDIT_SIZESECURITY,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size);
            SetRvaAndSize(IDC_EDIT_RVARELOCATION, IDC_EDIT_SIZERELOCATION,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);
            SetRvaAndSize(IDC_EDIT_RVADEBUG, IDC_EDIT_SIZEDEBUG,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size);
            SetRvaAndSize(IDC_EDIT_RVAARCH, IDC_EDIT_SIZEARCH,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_ARCHITECTURE].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_ARCHITECTURE].Size);
            SetRvaAndSize(IDC_EDIT_RVAGLO, IDC_EDIT_SIZEGLO,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_GLOBALPTR].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_GLOBALPTR].Size);
            SetRvaAndSize(IDC_EDIT_RVATLS, IDC_EDIT_SIZETLS,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size);
            SetRvaAndSize(IDC_EDIT_RVACONFIG, IDC_EDIT_SIZECONFIG,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size);
            SetRvaAndSize(IDC_EDIT_RVABOUNDIMPORT, IDC_EDIT_SIZEBOUNDIMPORT,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size);
            SetRvaAndSize(IDC_EDIT_RVAIAT, IDC_EDIT_SIZEIAT,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size);
            SetRvaAndSize(IDC_EDIT_RVADELAYIMPORT, IDC_EDIT_SIZEDELAYIMPORT,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].Size);
            SetRvaAndSize(IDC_EDIT_RVACOM, IDC_EDIT_SIZECOM,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress,
                          pImageDataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size);
            SetRvaAndSize(IDC_EDIT_RVARESERVE, IDC_EDIT_SIZERESERVE,
                          pImageDataDirectory[15].VirtualAddress,
                          pImageDataDirectory[15].Size);

            return TRUE;
        }
    }
    return (INT_PTR) FALSE;
}

VOID SetItem(int index, int subIndex, DWORD data) {
    WCHAR lpString[256];
    memset(lpString, 0, 256);
    wsprintfW(lpString, L"%08lX", data);

    vitem.pszText = (LPWSTR) lpString;
    vitem.iItem = index;
    vitem.iSubItem = subIndex;
    ListView_SetItem(hListSection, &vitem);
}

VOID InsertItem(int index, BYTE *name) {

    WCHAR lpString[256];
    memset(lpString, 0, 256);
    wsprintfW(lpString, L"%S", (PSTR) name);

    vitem.pszText = (LPWSTR) lpString;
    vitem.iItem = index;
    vitem.iSubItem = 0;
    SendMessage(hListSection, LVM_INSERTITEM, 0, (LPARAM) &vitem);
}

VOID EnumSection() {

    memset(&vitem, 0, sizeof(LV_ITEM));
    vitem.mask = LVIF_TEXT;

    PIMAGE_SECTION_HEADER pSectionHeaders = pImageHeaders->pSectionHeaders;
    WORD numberOfSections = pImageHeaders->pPEHeader->NumberOfSections;

    for (WORD i = 0; i < numberOfSections; i++) {

        InsertItem(i, pSectionHeaders[i].Name);
        SetItem(i, 1, pSectionHeaders[i].PointerToRawData);
        SetItem(i, 2, pSectionHeaders[i].SizeOfRawData);
        SetItem(i, 3, pSectionHeaders[i].VirtualAddress);
        SetItem(i, 4, pSectionHeaders[i].Misc.VirtualSize);
        SetItem(i, 5, pSectionHeaders[i].Characteristics);
    }
}

VOID InitSectionListView(HWND hDlg) {

    LV_COLUMN lv;

    memset(&lv, 0, sizeof(LV_COLUMN));
    hListSection = GetDlgItem(hDlg, IDC_LIST_SECTION);
    SendMessage(hListSection, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    // 第一列
    lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lv.pszText = TEXT("节名");
    lv.cx = 100;
    lv.iSubItem = 0;
    SendMessage(hListSection, LVM_INSERTCOLUMN, 0, (LPARAM) &lv);

    // 第二列
    lv.pszText = TEXT("文件偏移");
    lv.cx = 80;
    lv.iSubItem = 1;
    SendMessage(hListSection, LVM_INSERTCOLUMN, 1, (LPARAM) &lv);

    // 第三列
    lv.pszText = TEXT("文件大小");
    lv.cx = 80;
    lv.iSubItem = 2;
    SendMessage(hListSection, LVM_INSERTCOLUMN, 2, (LPARAM) &lv);

    // 第四列
    lv.pszText = TEXT("内存偏移");
    lv.cx = 80;
    lv.iSubItem = 3;
    SendMessage(hListSection, LVM_INSERTCOLUMN, 3, (LPARAM) &lv);

    // 第五列
    lv.pszText = TEXT("内存大小");
    lv.cx = 80;
    lv.iSubItem = 4;
    SendMessage(hListSection, LVM_INSERTCOLUMN, 4, (LPARAM) &lv);

    // 第六列
    lv.pszText = TEXT("节区属性");
    lv.cx = 80;
    lv.iSubItem = 5;
    SendMessage(hListSection, LVM_INSERTCOLUMN, 5, (LPARAM) &lv);

    EnumSection();
}

INT_PTR CALLBACK SectionDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);

    hSectionDlg = hDlg;

    switch (message) {
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDCANCEL: {
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR) TRUE;
                }
            }
            break;
        }
        case WM_INITDIALOG: {
            InitSectionListView(hDlg);
            return TRUE;
        }
    }
    return (INT_PTR) FALSE;
}

BOOL SetText(int nIDDlgItem, DWORD data) {

    CHAR lpString[256];
    memset(lpString, 0, 256);
    sprintf(lpString, "%08lX", data);

    return SetDlgItemTextA(hPEDlg, nIDDlgItem, lpString);
}

BOOL SetText(int nIDDlgItem, WORD data) {

    CHAR lpString[256];
    memset(lpString, 0, 256);
    sprintf(lpString, "%04lX", data);

    return SetDlgItemTextA(hPEDlg, nIDDlgItem, lpString);
}

INT_PTR CALLBACK PEDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);

    hPEDlg = hDlg;
    OPENFILENAMEA stOpenFile;

    switch (message) {
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDCANCEL: {
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR) TRUE;
                }
                case IDC_BUTTON_SECTION: {
                    DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_SECTION), hDlg, SectionDlgProc);
                    return TRUE;
                }
                case IDC_BUTTON_DIRECTORY: {
                    DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_DIRECTORY), hDlg, DirectoryDlgProc);
                }
            }
            break;
        }
        case WM_INITDIALOG: {

            CHAR szPeFileExt[128] = "*.exe;*.dll;*.scr;*.drv;*.sys";
            CHAR szFileName[256];
            memset(szFileName, 0, 256);
            memset(&stOpenFile, 0, sizeof(OPENFILENAME));
            stOpenFile.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            stOpenFile.hwndOwner = hDlg;
            stOpenFile.lpstrFilter = szPeFileExt;
            stOpenFile.lpstrFile = szFileName;
            stOpenFile.lStructSize = sizeof(OPENFILENAME);
            stOpenFile.nMaxFile = MAX_PATH;

            if (!GetOpenFileNameA(&stOpenFile)) {
                EndDialog(hDlg, 0);
                return FALSE;
            }

            pFileBuffer = ReadPEFile(szFileName);
            pImageHeaders = ReadHeaders(pFileBuffer);

            SetText(IDC_EDIT_ENTRYPOINT, pImageHeaders->pOptionHeader->AddressOfEntryPoint);
            SetText(IDC_EDIT_IMAGEBASE, pImageHeaders->pOptionHeader->ImageBase);
            SetText(IDC_EDIT_SIZEOFIMAGE, pImageHeaders->pOptionHeader->SizeOfImage);
            SetText(IDC_EDIT_BASEOFCODE, pImageHeaders->pOptionHeader->BaseOfCode);
            SetText(IDC_EDIT_BASEOFDATA, pImageHeaders->pOptionHeader->BaseOfData);
            SetText(IDC_EDIT_SECTIONALIGN, pImageHeaders->pOptionHeader->SectionAlignment);
            SetText(IDC_EDIT_FILEALIGN, pImageHeaders->pOptionHeader->FileAlignment);
            SetText(IDC_EDIT_SIGNATURE, pImageHeaders->pNTHeader->Signature);
            SetText(IDC_EDIT_SUBSYSTEM, pImageHeaders->pPEHeader->TimeDateStamp);
            SetText(IDC_EDIT_NUMBEROFSECTIONS, pImageHeaders->pPEHeader->NumberOfSections);
            SetText(IDC_EDIT_TIMEDATESTAMP, pImageHeaders->pPEHeader->TimeDateStamp);
            SetText(IDC_EDIT_SIZEOFHEADERS, pImageHeaders->pOptionHeader->SizeOfHeaders);
            SetText(IDC_EDIT_CHARACTERISTICS, pImageHeaders->pPEHeader->Characteristics);
            SetText(IDC_EDIT_CHECKSUM, pImageHeaders->pOptionHeader->CheckSum);
            SetText(IDC_EDIT_MACHINE, pImageHeaders->pPEHeader->Machine);
            SetText(IDC_EDIT_NUMBEROFRVAANDSIZE, pImageHeaders->pOptionHeader->NumberOfRvaAndSizes);

            return TRUE;
        }
    }
    return (INT_PTR) FALSE;
}

VOID EnumProcess(HWND hListProcess) {

    LV_ITEM vitem;
    memset(&vitem, 0, sizeof(LV_ITEM));
    vitem.mask = LVIF_TEXT;

    vitem.pszText = TEXT("现在能用的只有PE查看器，这边等后面写线程注入的时候再写");
    vitem.iItem = 0;
    vitem.iSubItem = 0;
    SendMessage(hListProcess, LVM_INSERTITEM, 0, (LPARAM) &vitem);

    vitem.pszText = TEXT("448");
    vitem.iItem = 0;
    vitem.iSubItem = 1;
    ListView_SetItem(hListProcess, &vitem);

    vitem.pszText = TEXT("448");
    vitem.iItem = 0;
    vitem.iSubItem = 2;
    ListView_SetItem(hListProcess, &vitem);

    vitem.pszText = TEXT("448");
    vitem.iItem = 0;
    vitem.iSubItem = 3;
    ListView_SetItem(hListProcess, &vitem);
}

VOID InitProcessListView(HWND hDlg) {

    LV_COLUMN lv;
    HWND hListProcess;

    memset(&lv, 0, sizeof(LV_COLUMN));
    hListProcess = GetDlgItem(hDlg, IDC_LIST_PROCESS);
    SendMessage(hListProcess, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    // 第一列
    lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lv.pszText = TEXT("进程");
    lv.cx = 260;
    lv.iSubItem = 0;
    SendMessage(hListProcess, LVM_INSERTCOLUMN, 0, (LPARAM) &lv);

    // 第二列
    lv.pszText = TEXT("PID");
    lv.cx = 60;
    lv.iSubItem = 1;
    SendMessage(hListProcess, LVM_INSERTCOLUMN, 1, (LPARAM) &lv);

    // 第三列
    lv.pszText = TEXT("镜像基址");
    lv.cx = 100;
    lv.iSubItem = 2;
    SendMessage(hListProcess, LVM_INSERTCOLUMN, 2, (LPARAM) &lv);

    // 第四列
    lv.pszText = TEXT("镜像大小");
    lv.cx = 100;
    lv.iSubItem = 3;
    SendMessage(hListProcess, LVM_INSERTCOLUMN, 3, (LPARAM) &lv);

    EnumProcess(hListProcess);
}

VOID InitModuleListView(HWND hDlg) {

    LV_COLUMN lv;
    HWND hListModule;

    memset(&lv, 0, sizeof(LV_COLUMN));
    hListModule = GetDlgItem(hDlg, IDC_LIST_MODULE);
    SendMessage(hListModule, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    // 第一列
    lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lv.pszText = TEXT("模块名称");
    lv.cx = 260;
    lv.iSubItem = 0;
    SendMessage(hListModule, LVM_INSERTCOLUMN, 0, (LPARAM) &lv);

    // 第二列
    lv.pszText = TEXT("模块位置");
    lv.cx = 260;
    lv.iSubItem = 1;
    SendMessage(hListModule, LVM_INSERTCOLUMN, 1, (LPARAM) &lv);
}

INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);

    switch (message) {
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_BUTTON_ABOUT: {

                    DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);

                    return TRUE;
                }
                case IDC_BUTTON_PE: {

                    DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_PE), hDlg, PEDlgProc);

                    return TRUE;
                }
                case IDC_BUTTON_EXIT:
                case IDCANCEL: {
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR) TRUE;
                }
            }
            break;
        }
        case WM_INITDIALOG: {
            InitProcessListView(hDlg);
            InitModuleListView(hDlg);
            break;
        }
        default:
            return FALSE;
    }
    return (INT_PTR) FALSE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。
    hAppInstance = hInstance;

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PEVIEWER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, MainDlgProc);

    return 0;
}


//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PEVIEWER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PEVIEWER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId) {
                case IDM_ABOUT:
                    DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG:
            return (INT_PTR) TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR) TRUE;
            }
            break;
    }
    return (INT_PTR) FALSE;
}