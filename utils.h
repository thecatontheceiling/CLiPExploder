#pragma once

#include <windows.h>
#include "config.h"

#define CLIPSP_IDATA_OFFSET 0xB1000
#define CLIPSP_IDATA_LENGTH 0x1000

typedef DWORD (*WarbirdDecrypt_type)(PVOID pDataConst, PVOID pDataRW, BOOL bEncrypt);
#define WarbirdDecrypt(pAddr, pDataConst, pDataRW, offset) \
    ((WarbirdDecrypt_type)((PBYTE)pAddr + (offset)))((PBYTE)pAddr + pDataConst, (PBYTE)pAddr + pDataRW, FALSE)

void CrapFunction(void);
BOOL SetClipIdataPageProtection(HMODULE hClipSp, DWORD dwProtection);
void DecryptSegments(HMODULE hClipSp, const ConfigData* config);
HMODULE InitializeClipSp(const wchar_t* path);