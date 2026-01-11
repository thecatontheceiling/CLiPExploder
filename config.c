#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#define MAX_LINE_LENGTH 256
#define MAX_SEGMENTS 64

static BOOL ParseHexValue(const char* str, DWORD* value) {
    while (*str == ' ' || *str == '\t') str++;

    if (str[0] != '0' || (str[1] != 'x' && str[1] != 'X')) {
        return FALSE;
    }

    char* endptr;
    unsigned long result = strtoul(str, &endptr, 16);

    if (endptr == str + 2) {
        return FALSE;
    }

    *value = (DWORD)result;
    return TRUE;
}

BOOL LoadConfig(const char* filePath, ConfigData* config) {
    FILE* file = NULL;
    char line[MAX_LINE_LENGTH];
    SegmentConfig* tempSegments = NULL;
    int count = 0;

    config->segments = NULL;
    config->count = 0;

    if (fopen_s(&file, filePath, "r") != 0 || !file) {
        wprintf(L"[ERROR] Failed to open config file: %hs\n", filePath);
        return FALSE;
    }

    tempSegments = malloc(sizeof(SegmentConfig) * MAX_SEGMENTS);
    if (!tempSegments) {
        wprintf(L"[ERROR] Failed to allocate memory for config");
        fclose(file);
        return FALSE;
    }

    int lineNumber = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        lineNumber++;

        char* trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        if (*trimmed == '\n' || *trimmed == '\r' || *trimmed == '\0' || *trimmed == '#') {
            continue;
        }

        if (count >= MAX_SEGMENTS) {
            wprintf(L"[ERROR] More than %d in config\n", MAX_SEGMENTS);
            free(tempSegments);
            fclose(file);
            return FALSE;
        }

        char* token1 = strtok(line, ",");
        char* token2 = strtok(NULL, ",");
        char* token3 = strtok(NULL, ",\n\r");

        if (!token1 || !token2 || !token3) {
            wprintf(L"[ERROR] Invalid config at line %d: expected 3 comma-separated values\n", lineNumber);
            free(tempSegments);
            fclose(file);
            return FALSE;
        }

        DWORD offset, pDataConst, pDataRW;

        if (!ParseHexValue(token1, &offset)) {
            wprintf(L"[ERROR] Line %d: First value must be a hex number\n", lineNumber);
            free(tempSegments);
            fclose(file);
            return FALSE;
        }

        if (!ParseHexValue(token2, &pDataConst)) {
            wprintf(L"[ERROR] Line %d: Second value must be a hex number\n", lineNumber);
            free(tempSegments);
            fclose(file);
            return FALSE;
        }
        
        if (!ParseHexValue(token3, &pDataRW)) {
            wprintf(L"[ERROR] Line %d: Third value must be a hex number\n", lineNumber);
            free(tempSegments);
            fclose(file);
            return FALSE;
        }

        tempSegments[count].offset = offset;
        tempSegments[count].pDataConst = pDataConst;
        tempSegments[count].pDataRW = pDataRW;
        count++;
    }

    fclose(file);

    if (count == 0) {
        wprintf(L"[ERROR] Config file is empty");
        free(tempSegments);
        return FALSE;
    }

    config->segments = realloc(tempSegments, sizeof(SegmentConfig) * count);
    if (!config->segments) {
        config->segments = tempSegments;
    }
    config->count = count;

    wprintf(L"[INFO] Loaded %d segment(s) from config file\n", count);
    return TRUE;
}

void FreeConfig(ConfigData* config) {
    if (config->segments) {
        free(config->segments);
        config->segments = NULL;
    }
    config->count = 0;
}