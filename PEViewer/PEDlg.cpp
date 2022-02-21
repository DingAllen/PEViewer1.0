#include "PEDlg.h"

HWND hPEDlg;

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

            GetOpenFileNameA(&stOpenFile);

            LPVOID pFileBuffer = ReadPEFile(szFileName);
            PIMAGE_HEADER_POINTERS pImageHeaders = ReadHeaders(pFileBuffer);

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