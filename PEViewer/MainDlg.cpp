#include "MainDlg.h"
#include "PEDlg.h"

INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);

    OPENFILENAMEW stOpenFile;

    switch (message) {
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_BUTTON_ABOUT: {

                    return TRUE;
                }
                case IDC_BUTTON_PE: {

                    WCHAR szPeFileExt[128] = L"*.exe;*.dll;*.scr;*.drv;*.sys";
                    WCHAR szFileName[256];
                    memset(szFileName, 0, 256 * 2);
                    memset(&stOpenFile, 0, sizeof(OPENFILENAME));
                    stOpenFile.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
                    stOpenFile.hwndOwner = hDlg;
                    stOpenFile.lpstrFilter = szPeFileExt;
                    stOpenFile.lpstrFile = szFileName;
                    stOpenFile.lStructSize = sizeof(OPENFILENAME);
                    stOpenFile.nMaxFile = MAX_PATH;

                    GetOpenFileNameW(&stOpenFile);
                    MessageBoxW(0, szFileName, 0, 0);
                    DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_PE), hDlg, PEDlgProc);

                    return TRUE;
                }
                case IDC_BUTTON_EXIT:
                case IDCANCEL: {
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR) TRUE;
                }
                default:
                    return FALSE;
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
    lv.cx = 260 ;
    lv.iSubItem = 1;
    SendMessage(hListModule, LVM_INSERTCOLUMN, 1, (LPARAM) &lv);
}

VOID EnumProcess(HWND hListProcess) {

    LV_ITEM vitem;
    memset(&vitem, 0, sizeof(LV_ITEM));
    vitem.mask = LVIF_TEXT;

    vitem.pszText = TEXT("示例进程.exe");
    vitem.iItem = 0;
    vitem.iSubItem = 0;
    SendMessage(hListProcess, LVM_INSERTITEM, 0, (LPARAM) &vitem);

    vitem.pszText = TEXT("448");
    vitem.iItem = 0;
    vitem.iSubItem = 1;
    ListView_SetItem(hListProcess, &vitem);

    vitem.pszText = TEXT("10000000");
    vitem.iItem = 0;
    vitem.iSubItem = 2;
    ListView_SetItem(hListProcess, &vitem);

    vitem.pszText = TEXT("00F00000");
    vitem.iItem = 0;
    vitem.iSubItem = 3;
    ListView_SetItem(hListProcess, &vitem);
}