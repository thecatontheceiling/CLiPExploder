#pragma once

#include <windows.h>

typedef struct {
    DWORD offset;
    DWORD pDataConst;
    DWORD pDataRW;
} SegmentConfig;

typedef struct {
    SegmentConfig* segments;
    int count;
} ConfigData;

BOOL LoadConfig(const char* filePath, ConfigData* config);
void FreeConfig(ConfigData* config);