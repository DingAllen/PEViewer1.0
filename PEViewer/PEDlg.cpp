#include "PEDlg.h"

INT_PTR CALLBACK PEDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
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

            break;
        }
    }
    return (INT_PTR) FALSE;
}