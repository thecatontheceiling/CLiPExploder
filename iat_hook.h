#pragma once

#include <windows.h>

BOOL PatchIAT(HMODULE hClipSp);
PUINT_PTR GetFunctionAddressInIAT(HMODULE hModule, LPCSTR functionName);