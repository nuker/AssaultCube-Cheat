// gcc main.c -o main -lpsapi
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <Psapi.h>
#include <TlHelp32.h>

#define BASE_ADDR 0x0040000
#define OFF_LOCALPLAYER 0x10F4F4
#define OFF_HEALTH 0xF8
#define OFF_SPEED 0x0080
#define OFF_AMMO1 0x150
#define OFF_AMMO2 0x13c
#define OFF_RECOIL 0x4EE444


DWORD get_pid(const char* name) {

    DWORD pid;
    PROCESSENTRY32 proc;
    proc.dwSize = sizeof(PROCESSENTRY32);

    HANDLE plist = CreateToolhelp32Snapshot(0x00000002 | 0x00000004, 0);

    if (Process32First(plist, &proc) == TRUE) {
        do
        {
            if (strcmp(proc.szExeFile, name) == 0) {
                pid = proc.th32ProcessID;
                break;
            }
        } while (Process32Next(plist, &proc) == TRUE);
    }

    CloseHandle(plist);
    return pid;
}

unsigned int GetBase(HANDLE hGame) {

    HMODULE mods[1024];
    unsigned int *base = NULL;
    DWORD Needed;
    EnumProcessModules(hGame, mods, sizeof(mods), &Needed);

    HMODULE hMainMod = mods[0];
    TCHAR szName[MAX_PATH];

    if (GetModuleBaseNameA(hGame, hMainMod, szName, sizeof(szName) / sizeof(TCHAR))) {
        base = (unsigned int *)hMainMod; // Don't need to search for all modules because everything is in the first
    }

    return (int)base;
}

void val_change(HANDLE game, int value, unsigned int base, int offset) {

    BOOL bcheating = FALSE;

    unsigned int Player;
    unsigned int LocalPlayer = (base + OFF_LOCALPLAYER);
    ReadProcessMemory(game, (LPCVOID)LocalPlayer, &Player, 4, NULL);
    //WriteProcessMemory(game, (LPVOID)(Player + offset), &value, sizeof(value), NULL);

    // Write to addr
    
    while (TRUE) {

        if ((GetAsyncKeyState(0xA1) & 0x8000)) {

            bcheating = !bcheating;

            if (bcheating) {

                WriteProcessMemory(game, (LPVOID)(Player + offset), &value, sizeof(value), NULL);

            } else {

                continue;

            }
        }
    }
}

int cheat(HANDLE game, unsigned int base) {

    int val;
    BOOL bcheating = FALSE;

    unsigned int Player;
    unsigned int LocalPlayer = (base + OFF_LOCALPLAYER);
    ReadProcessMemory(game, (LPCVOID)LocalPlayer, &Player, 4, NULL);
    //WriteProcessMemory(game, (LPVOID)(Player + offset), &value, sizeof(value), NULL);

    // Write to addr
    
    while (TRUE) {

        // shift = Health
        if ((GetAsyncKeyState(0xA1) & 0x8000)) {
            bcheating = !bcheating;
            if (bcheating) {
                val = 7331;
                WriteProcessMemory(game, (LPVOID)(Player + OFF_HEALTH), &val, sizeof(val), NULL);
            } else {
                continue;
            }
        }

        // backspace = exit
        if ((GetAsyncKeyState(0x08) & 0x8000)) {
            return 0;
        } else {
            continue;
        }
    }
}

int main () {

    // Get PID of game
    const char *name = "ac_client.exe";
    DWORD game = get_pid(name);

    if (game == 0) {
        printf("Could not find correct process.\n");
        exit(0);
    }
    printf("Target process: [%s] - [%d]\n", name, game);

    // Get Handle to process
    HANDLE hGame = OpenProcess(PROCESS_ALL_ACCESS, FALSE, game);
    unsigned int base = GetBase(hGame);
    printf("AssCube base : 0x%p\n", base);

    //val_change(hGame, 222, base, OFF_AMMO2);

    cheat(hGame, base);
    
    CloseHandle(hGame);
    return 0;
}