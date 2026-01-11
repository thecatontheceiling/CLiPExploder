#include <stdio.h>
#include "iat_hook.h"
#include "patch.h"
#include "utils.h"


void CrapFunction(void) {
    wprintf(L"[INFO] CrapFunction called\n");
}

// `hClipSp` is the base address where the driver was loaded, and `dwProtection` is the protection type
BOOL SetClipIdataPageProtection(HMODULE hClipSp, DWORD dwProtection) {
    DWORD dwOldProtect = 0;

    // `VirtualProtect` changes the memory protection of a specific memory region
    return VirtualProtect(
        (PBYTE)hClipSp + CLIPSP_IDATA_OFFSET,  // starting address (IAT offset within the driver)
        CLIPSP_IDATA_LENGTH,            // length of the memory region to change protection for
        dwProtection,
        &dwOldProtect                   // unused
    );
}

void DecryptSegments(HMODULE hClipSp, const ConfigData* config) {
    for (int i = 0; i < config->count; i++) {
        wprintf(L"\n[INFO] Decrypting section %d\n", i + 1);

        DWORD result = WarbirdDecrypt(hClipSp, config->segments[i].pDataConst, config->segments[i].pDataRW, config->segments[i].offset);

        wprintf(L"[INFO] Result: 0x%08lx\n", result);
    }
}

// load & patch
HMODULE InitializeClipSp(const wchar_t* path) {
    // https://devblogs.microsoft.com/oldnewthing/20050214-00/?p=36463 but who cares
    HMODULE hLib = LoadLibraryExW(path, NULL, DONT_RESOLVE_DLL_REFERENCES);

    if (!hLib) {
        wprintf(L"[ERROR] Failed to load ClipSp from path: %ls\n", path);
        return NULL;
    }

    wprintf(L"[INFO] Library loaded from %ls at 0x%p\n", path, hLib);

    InitPatchCollection(hLib);

    // patch the IAT
    if (!PatchIAT(hLib)) {
        wprintf(L"[ERROR] Failed to patch IAT\n");
        FreeLibrary(hLib);
        return NULL;
    }

    return hLib;
}