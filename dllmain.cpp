// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>
#include <Windows.h>

namespace fs = std::filesystem;

HMODULE g_dinput = 0;

extern "C" {
    __declspec(dllexport) HRESULT WINAPI direct_input8_create(HINSTANCE hinst, DWORD dw_version, const IID& riidltf, LPVOID* ppv_out, LPUNKNOWN punk_outer) {
#pragma comment(linker, "/EXPORT:DirectInput8Create=direct_input8_create")
        return ((decltype(DirectInput8Create)*)GetProcAddress(g_dinput, "DirectInput8Create"))(hinst, dw_version, riidltf, ppv_out, punk_outer);
    }
}

void failed() {
    ExitProcess(0);
}

void file_List(const fs::path& fld, const std::string& ext, std::vector<fs::path>& flst)
{
    if (!fs::exists(fld) || !fs::is_directory(fld)) {
        fs::create_directory(fld);
    }

    fs::recursive_directory_iterator it(fld);
    fs::recursive_directory_iterator endit;

    while (it != endit)
    {
        if (fs::is_regular_file(*it) && it->path().extension() == ext) {
            flst.push_back(it->path());
        }
        ++it;
    }
}

void startup_thread() {
    std::cout << "Mod loader by Darkness.\n";

    wchar_t buffer[MAX_PATH]{ 0 };
    if (GetSystemDirectoryW(buffer, MAX_PATH) != 0) {
        // Org dinput8.dll
        if ((g_dinput = LoadLibraryW((std::wstring{ buffer } +L"\\dinput8.dll").c_str())) == NULL) {
            failed();
        }

        std::vector<fs::path> dll_list;
        file_List("Mods", ".dll", dll_list);
        for (auto& dll : dll_list) {
            if (LoadLibraryW(dll.wstring().c_str()) == NULL) {
                std::cout << "Failed to load " << dll.string() << std::endl;
            }
        }
    }
    else {
        failed();
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE mainThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)startup_thread, hModule, 0, nullptr);
        if (mainThread) CloseHandle(mainThread);
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

