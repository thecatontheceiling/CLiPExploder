#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "mdl_hook.h"
#include "patch.h"

BOOL HookMmChangeImageProtection(
    PMDL MemoryDescriptorList,
    PVOID VirtualAddress __attribute__((unused)),
    ULONG Size __attribute__((unused)),
    ULONG Flags __attribute__((unused))
) {
    wprintf(L"[INFO] MmChangeImageProtection called for 0x%p (MappedSystemVa: 0x%p)\n",
            MemoryDescriptorList, MemoryDescriptorList->MappedSystemVa);
    return TRUE;
}

void HookMmUnlockPages(PMDL MemoryDescriptorList) {
    if (!MemoryDescriptorList) {
        wprintf(L"[WARNING] NULL MDL passed to MmUnlockPages.\n");
        return;
    }
    if (MemoryDescriptorList->MappedSystemVa) {
        wprintf(L"[INFO] MmUnlockPages called for 0x%p\n", MemoryDescriptorList->MappedSystemVa);
    } else {
        wprintf(L"[WARNING] MappedSystemVa is null\n");
    }

    MemoryDescriptorList->MdlFlags = 0;
}

PMDL HookIoAllocateMdl(
    PVOID VirtualAddress,
    ULONG Length,
    BOOLEAN SecondaryBuffer __attribute__((unused)),
    BOOLEAN ChargeQuota __attribute__((unused)),
    PVOID Irp __attribute__((unused))
) {
    PMDL pMDL = malloc(sizeof(MDL));
    if (!pMDL) {
        wprintf(L"[ERROR] Failed to allocate MDL.\n");
        return NULL;
    }

    DWORD dwOldProtect = 0;

    wprintf(L"[INFO] IoAllocateMdl called for 0x%p, Size = %lu\n", VirtualAddress, Length);

    if(!VirtualProtect(VirtualAddress, Length, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
        wprintf(L"[ERROR] VirtualProtect failed. Error code: %lu\n", GetLastError());
        free(pMDL);
        return NULL;
    }

    pMDL->Next = NULL;
    pMDL->Size = (SHORT)Length;
    pMDL->MdlFlags = 1;
    pMDL->Process = NULL;
    pMDL->MappedSystemVa = VirtualAddress;
    pMDL->StartVa = VirtualAddress;
    pMDL->ByteCount = Length;
    pMDL->ByteOffset = 0;

    return pMDL;
}

void HookIoFreeMdl(PMDL Mdl) {
    if (Mdl == NULL) {
        return;
    }

    if (Mdl->MappedSystemVa == NULL || Mdl->Size == 0) {
        wprintf(L"[ERROR] MDL is invalid\n");
        free(Mdl);
        return;
    }

    wprintf(L"[INFO] IoFreeMdl called for 0x%p, Size = %d\n", Mdl->MappedSystemVa, Mdl->Size);

    AddPatch(Mdl->MappedSystemVa, Mdl->ByteCount);

    free(Mdl);
}