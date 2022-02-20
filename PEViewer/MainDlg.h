#pragma once

#include "framework.h"
#include "PEViewer.h"
#include <CommCtrl.h>

INT_PTR CALLBACK MainDlgProc(HWND, UINT, WPARAM, LPARAM);

VOID InitProcessListView(HWND);
VOID InitModuleListView(HWND);
VOID EnumProcess(HWND);
VOID EnumModule(HWND);