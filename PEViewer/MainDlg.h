#pragma once

#include "PEViewer.h"

INT_PTR CALLBACK MainDlgProc(HWND, UINT, WPARAM, LPARAM);

VOID InitProcessListView(HWND);
VOID InitModuleListView(HWND);
VOID EnumProcess(HWND);
VOID EnumModule(HWND);