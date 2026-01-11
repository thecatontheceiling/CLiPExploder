#include <stdio.h>
#include <windows.h>
#include "config.h"
#include "mdl_hook.h"
#include "patch.h"
#include "utils.h"

#define CONFIG_FILE "offsets.txt"

// About half of this codebase was written about 2 years ago, when I was 15 and was getting started with C programming.
// Please excuse my weird code and weird comments.
int wmain (int argc, wchar_t *argv[]) {
    const wchar_t* clipSpPath;
    const wchar_t* outputPath;

    if (argc > 1) {
        clipSpPath = argv[1];
    } else {
        clipSpPath = L"clipsp.sys";
    }

    if (argc > 2) {
        outputPath = argv[2];
    } else {
        outputPath = L"clipsp_decrypted.sys";
    }

    ConfigData config;
    if (!LoadConfig(CONFIG_FILE, &config)) {
        wprintf(L"[ERROR] Failed to load config.\n");
        return 1;
    }

    // returns handle to loaded driver
    HMODULE hClipSp = InitializeClipSp(clipSpPath);

    if (!hClipSp) {
        FreeConfig(&config);
        return 1;
    }

    DecryptSegments(hClipSp, &config);

    wprintf(L"\n");
    if (!ApplyPatchesToFile(clipSpPath, outputPath, hClipSp)) {
        wprintf(L"[ERROR] Failed to apply patches to file.\n");
        CleanupPatches();
        FreeLibrary(hClipSp);
        FreeConfig(&config);
        return 1;
    }

    CleanupPatches();
    FreeLibrary(hClipSp);
    FreeConfig(&config);

    return 0;
}