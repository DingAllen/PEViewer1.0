#include "PEDlg.h"

HWND hPEDlg;
HWND hSectionDlg;
HWND hDirectoryDlg;
HWND hListSection;
PIMAGE_HEADER_POINTERS pImageHeaders;

LV_ITEMW vitem;

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
            
            return TRUE;
        }
    }
    return (INT_PTR) FALSE;
}

VOID SetItem(int index, int subIndex, DWORD data) {
    WCHAR lpString[256];
    memset(lpString, 0, 256);
    wsprintfW(lpString, L"%08lX", data);

    vitem.pszText = (LPWSTR)lpString;
    vitem.iItem = index;
    vitem.iSubItem = subIndex;
    ListView_SetItem(hListSection, &vitem);
}

VOID InsertItem(int index, BYTE *name) {

    WCHAR lpString[256];
    memset(lpString, 0, 256);
    wsprintfW(lpString, L"%S", (PSTR)name);

    vitem.pszText = (LPWSTR)lpString;
    vitem.iItem = index;
    vitem.iSubItem = 0;
    SendMessage(hListSection, LVM_INSERTITEM, 0, (LPARAM)&vitem);
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
    SendMessage(hListSection, LVM_INSERTCOLUMN, 0, (LPARAM)&lv);

    // 第二列
    lv.pszText = TEXT("文件偏移");
    lv.cx = 80;
    lv.iSubItem = 1;
    SendMessage(hListSection, LVM_INSERTCOLUMN, 1, (LPARAM)&lv);

    // 第三列
    lv.pszText = TEXT("文件大小");
    lv.cx = 80;
    lv.iSubItem = 2;
    SendMessage(hListSection, LVM_INSERTCOLUMN, 2, (LPARAM)&lv);

    // 第四列
    lv.pszText = TEXT("内存偏移");
    lv.cx = 80;
    lv.iSubItem = 3;
    SendMessage(hListSection, LVM_INSERTCOLUMN, 3, (LPARAM)&lv);

    // 第五列
    lv.pszText = TEXT("内存大小");
    lv.cx = 80;
    lv.iSubItem = 4;
    SendMessage(hListSection, LVM_INSERTCOLUMN, 4, (LPARAM)&lv);

    // 第六列
    lv.pszText = TEXT("节区属性");
    lv.cx = 80;
    lv.iSubItem = 5;
    SendMessage(hListSection, LVM_INSERTCOLUMN, 5, (LPARAM)&lv);

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

            LPVOID pFileBuffer = ReadPEFile(szFileName);
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