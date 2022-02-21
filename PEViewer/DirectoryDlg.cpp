#include "DirectoryDlg.h"

HWND hDirectoryDlg;

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

            return TRUE;
        }
    }
    return (INT_PTR) FALSE;
}