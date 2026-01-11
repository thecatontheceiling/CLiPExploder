#pragma once

#include <windows.h>

typedef struct _MDL {
    struct _MDL *Next;
    SHORT Size;
    SHORT MdlFlags;
    struct _EPROCESS *Process;
    PVOID MappedSystemVa;
    PVOID StartVa;
    ULONG ByteCount;
    ULONG ByteOffset;
} MDL, *PMDL;

BOOL HookMmChangeImageProtection(PMDL MemoryDescriptorList, PVOID VirtualAddress, ULONG Size, ULONG Flags);
void HookMmUnlockPages(PMDL MemoryDescriptorList);
PMDL HookIoAllocateMdl(PVOID VirtualAddress, ULONG Length, BOOLEAN SecondaryBuffer, BOOLEAN ChargeQuota, PVOID Irp);
void HookIoFreeMdl(PMDL Mdl);