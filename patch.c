#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "patch.h"

static HMODULE g_hModule = NULL;
static PatchInfo g_patches[MAX_PATCHES];
static int g_patchCount = 0;

void InitPatchCollection(HMODULE hModule) {
    g_hModule = hModule;
    g_patchCount = 0;
    memset(g_patches, 0, sizeof(g_patches));
}

static DWORD RvaToFileOffset(HMODULE hModule, DWORD rva, DWORD* pRemainingSize) {
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
    
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);
    WORD numSections = ntHeaders->FileHeader.NumberOfSections;

    for (WORD i = 0; i < numSections; i++) {
        DWORD sectionStart = section[i].VirtualAddress;
        DWORD sectionSize = section[i].Misc.VirtualSize;
        if (sectionSize == 0) {
            sectionSize = section[i].SizeOfRawData;
        }
        DWORD sectionEnd = sectionStart + sectionSize;

        if (rva >= sectionStart && rva < sectionEnd) {
            if (pRemainingSize) {
                *pRemainingSize = sectionEnd - rva;
            }
            return rva - sectionStart + section[i].PointerToRawData;
        }
    }

    // rva might equal file offset (headers)
    return rva;
}

void AddPatch(PVOID virtualAddress, DWORD size) {
    if (g_patchCount >= MAX_PATCHES || !g_hModule || !virtualAddress || size == 0) {
        return;
    }

    DWORD rva = (DWORD)((BYTE*)virtualAddress - (BYTE*)g_hModule);

    DWORD remainingInSection = 0;
    RvaToFileOffset(g_hModule, rva, &remainingInSection);

    DWORD actualSize = size;

    if (remainingInSection == 0) {
        wprintf(L"[WARNING] Remaining section size for patch %d is 0\n", g_patchCount);
    }

    if (remainingInSection < size) {
        wprintf(L"[INFO] Clamping patch size from %u to %u\n", size, remainingInSection);
        actualSize = remainingInSection;
    }

    g_patches[g_patchCount].rva = rva;
    g_patches[g_patchCount].size = actualSize;
    g_patches[g_patchCount].data = malloc(actualSize);

    if (g_patches[g_patchCount].data) {
        memcpy(g_patches[g_patchCount].data, virtualAddress, actualSize);
        wprintf(L"[INFO] Stored patch at RVA 0x%08X, size %u\n", rva, actualSize);
        g_patchCount++;
    } else {
        wprintf(L"[ERROR] Failed to allocate memory for patch data.\n");
    }
}

BOOL ApplyPatchesToFile(const wchar_t* inputPath, const wchar_t* outputPath, HMODULE hModule) {
    FILE* inFile = NULL;
    FILE* outFile = NULL;
    BYTE* fileBuffer = NULL;
    BOOL success = FALSE;

    if (g_patchCount == 0) {
        wprintf(L"[WARNING] No patches to apply\n");
        return FALSE;
    }

    if (_wfopen_s(&inFile, inputPath, L"rb") != 0 || !inFile) {
        wprintf(L"[ERROR] Failed to open input file %ls\n", inputPath);
        goto cleanup;
    }

    fseek(inFile, 0, SEEK_END);
    long fileSize = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);

    if (fileSize <= 0) {
        wprintf(L"[ERROR] Invalid file size\n");
        goto cleanup;
    }

    fileBuffer = malloc(fileSize);
    if (!fileBuffer) {
        wprintf(L"[ERROR] Failed to allocate buffer for file\n");
        goto cleanup;
    }

    if (fread(fileBuffer, 1, fileSize, inFile) != (size_t)fileSize) {
        wprintf(L"[ERROR] Failed to read input\n");
        goto cleanup;
    }

    fclose(inFile);
    inFile = NULL;

    wprintf(L"[INFO] Applying %d patches...\n", g_patchCount);

    for (int i = 0; i < g_patchCount; i++) {
        DWORD fileOffset = RvaToFileOffset(hModule, g_patches[i].rva, NULL);

        if (fileOffset + g_patches[i].size > (DWORD)fileSize) {
            wprintf(L"[WARNING] Patch at RVA 0x%08X (file offset 0x%08X) exceeds file size\n", g_patches[i].rva, fileOffset);
            continue;
        }

        memcpy(fileBuffer + fileOffset, g_patches[i].data, g_patches[i].size);
        wprintf(L"[INFO] Applied patch %d: file offset 0x%08X (RVA 0x%08X), size %u bytes\n", i + 1, fileOffset, g_patches[i].rva, g_patches[i].size);
    }

    if (_wfopen_s(&outFile, outputPath, L"wb") != 0 || !outFile) {
        wprintf(L"[ERROR] Failed to create output file: %ls\n", outputPath);
        goto cleanup;
    }

    if (fwrite(fileBuffer, 1, fileSize, outFile) != (size_t)fileSize) {
        wprintf(L"[ERROR] Failed to write output file.");
        goto cleanup;
    }

    success = TRUE;
    wprintf(L"[SUCCESS] File written to %ls\n", outputPath);

cleanup:
    if (inFile) fclose(inFile);
    if (outFile) fclose(outFile);
    if (fileBuffer) free(fileBuffer);

    return success;
}

void CleanupPatches(void) {
    for (int i = 0; i < g_patchCount; i++) {
        if (g_patches[i].data) {
            free(g_patches[i].data);
            g_patches[i].data = NULL;
        }
    }
    g_patchCount = 0;
    g_hModule = NULL;
}