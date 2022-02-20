#include "MainDlg.h"

INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_BUTTON_ABOUT: {

                    return TRUE;
                }
                case IDC_BUTTON_PE: {

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
    }
    return (INT_PTR) FALSE;
}

VOID InitProcessListView(HWND hDlg) {

    LV_COLUMN lv;
    HWND hListProcess;

    memset(&lv, 0, sizeof(LV_COLUMN));
    hListProcess = GetDlgItem(hDlg, IDC_LIST_PROCESS);
    SendMessage(hListProcess, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    // ��һ��
    lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lv.pszText = TEXT("����");
    lv.cx = 260;
    lv.iSubItem = 0;
    SendMessage(hListProcess, LVM_INSERTCOLUMN, 0, (LPARAM) &lv);

    // �ڶ���
    lv.pszText = TEXT("PID");
    lv.cx = 60;
    lv.iSubItem = 1;
    SendMessage(hListProcess, LVM_INSERTCOLUMN, 1, (LPARAM) &lv);

    // ������
    lv.pszText = TEXT("�����ַ");
    lv.cx = 100;
    lv.iSubItem = 2;
    SendMessage(hListProcess, LVM_INSERTCOLUMN, 2, (LPARAM) &lv);

    // ������
    lv.pszText = TEXT("�����С");
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

    // ��һ��
    lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lv.pszText = TEXT("ģ������");
    lv.cx = 260;
    lv.iSubItem = 0;
    SendMessage(hListModule, LVM_INSERTCOLUMN, 0, (LPARAM) &lv);

    // �ڶ���
    lv.pszText = TEXT("ģ��λ��");
    lv.cx = 260 ;
    lv.iSubItem = 1;
    SendMessage(hListModule, LVM_INSERTCOLUMN, 1, (LPARAM) &lv);
}

VOID EnumProcess(HWND hListProcess) {

    LV_ITEM vitem;
    memset(&vitem, 0, sizeof(LV_ITEM));
    vitem.mask = LVIF_TEXT;

    vitem.pszText = TEXT("csrss.exe");
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