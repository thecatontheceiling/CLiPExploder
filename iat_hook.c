#include <stdio.h>
#include "iat_hook.h"
#include "mdl_hook.h"
#include "utils.h"

PUINT_PTR GetFunctionAddressInIAT(HMODULE hModule, LPCSTR functionName) {
    // dos header pointer
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        wprintf(L"Invalid DOS signature for module %p", hModule);
        return NULL;
    }
    
    // nt header pointer (offset is specified by dos header apparently (e_elfanew))
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        wprintf(L"Invalid NT signature for module %p", hModule);
        return NULL;
    }
    
    // import directory
    IMAGE_DATA_DIRECTORY importDirectory = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)hModule + importDirectory.VirtualAddress);

    while (importDescriptor->Name != 0) {
        LPCSTR moduleName = (LPCSTR)((BYTE*)hModule + importDescriptor->Name);
        wprintf(L"[INFO] Imported module: %hs\n", moduleName);

        // "FirstThunk" is the IAT, function names are "OriginalFirstThunk"
        PIMAGE_THUNK_DATA pThunkData = (PIMAGE_THUNK_DATA)((BYTE*)hModule + importDescriptor->OriginalFirstThunk);
        PIMAGE_THUNK_DATA pFirstThunk = (PIMAGE_THUNK_DATA)((BYTE*)hModule + importDescriptor->FirstThunk);

        while (pThunkData->u1.AddressOfData != 0) {
            if (!(pThunkData->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
                PIMAGE_IMPORT_BY_NAME pImportByName = (PIMAGE_IMPORT_BY_NAME)((BYTE*)hModule + pThunkData->u1.AddressOfData);

                if (strcmp(pImportByName->Name, functionName) == 0) {
                    return (PUINT_PTR)&pFirstThunk->u1.Function;
                }
            }
            pThunkData++;
            pFirstThunk++;
        }

        importDescriptor++;
    }
    // crappy code not work
    wprintf(L"[ERROR] Function was not found\n");
    return NULL;
}

static inline BOOL SetInIAT(PUINT_PTR pIATEntry, PVOID pFunction) {
    if (!pIATEntry) {
        wprintf(L"[ERROR] Invalid IAT entry pointer\n");
        return FALSE;
    }

    *pIATEntry = (UINT_PTR)pFunction;
    return TRUE;
}

// https://0xrick.github.io/win-internals/pe6/
BOOL PatchIAT(HMODULE hClipSp) {
    PUINT_PTR pExAcquireFastMutex = GetFunctionAddressInIAT(hClipSp, "ExAcquireFastMutex");
    PUINT_PTR pExReleaseFastMutex = GetFunctionAddressInIAT(hClipSp, "ExReleaseFastMutex");
    PUINT_PTR pMmProbeAndLockPages = GetFunctionAddressInIAT(hClipSp, "MmProbeAndLockPages");
    PUINT_PTR pMmUnmapLockedPages = GetFunctionAddressInIAT(hClipSp, "MmUnmapLockedPages");
    PUINT_PTR pMmLockPagableDataSection = GetFunctionAddressInIAT(hClipSp, "MmLockPagableDataSection");

    PUINT_PTR pIoAllocateMdl = GetFunctionAddressInIAT(hClipSp, "IoAllocateMdl");
    PUINT_PTR pMmChangeImageProtection = GetFunctionAddressInIAT(hClipSp, "MmChangeImageProtection");
    PUINT_PTR pMmUnlockPages = GetFunctionAddressInIAT(hClipSp, "MmUnlockPages");
    PUINT_PTR pIoFreeMdl = GetFunctionAddressInIAT(hClipSp, "IoFreeMdl");

    if (!pExAcquireFastMutex || !pExReleaseFastMutex || !pIoAllocateMdl ||
        !pMmProbeAndLockPages || !pMmChangeImageProtection || !pMmUnlockPages ||
        !pIoFreeMdl || !pMmUnmapLockedPages || !pMmLockPagableDataSection)
    {
        wprintf(L"[ERROR] Failed to locate some function addresses in the IAT\n");
        return FALSE;
    }

    if (!SetClipIdataPageProtection(hClipSp, PAGE_READWRITE)) {
        wprintf(L"[ERROR] Failed to set IAT page protection to PAGE_READWWRITE\n");
        return FALSE;
    }

    if (!SetInIAT(pMmProbeAndLockPages, CrapFunction) ||
        !SetInIAT(pMmUnmapLockedPages, CrapFunction) ||
        !SetInIAT(pMmLockPagableDataSection, CrapFunction) ||
        
        !SetInIAT(pIoFreeMdl, HookIoFreeMdl) ||
        !SetInIAT(pIoAllocateMdl, HookIoAllocateMdl) ||
        !SetInIAT(pMmChangeImageProtection, HookMmChangeImageProtection) ||
        !SetInIAT(pMmUnlockPages, HookMmUnlockPages)) 
    {
        wprintf(L"[ERROR] Failed to patch some IAT entries\n");
        return FALSE;
    }
    
    // step 3: restore the original protection (read-only) on the IAT after modifying it
    if (!SetClipIdataPageProtection(hClipSp, PAGE_READONLY)) {
        wprintf(L"[ERROR] Failed to set IAT page protection to PAGE_READONLY\n");
        return FALSE;
    }

    wprintf(L"[INFO] Succesfully patched the IAT\n");
    return TRUE;
}