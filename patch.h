#pragma once

#include <windows.h>

typedef struct {
    DWORD rva;
    DWORD size;
    BYTE* data;
} PatchInfo;

#define MAX_PATCHES 16

void InitPatchCollection(HMODULE hModule);
void AddPatch(PVOID virtualAddress, DWORD size);
BOOL ApplyPatchesToFile(const wchar_t* inputPath, const wchar_t* outputPath, HMODULE hModule);
void CleanupPatches(void);