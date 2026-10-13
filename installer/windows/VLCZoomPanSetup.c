#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define IDR_PLUGIN 101
#define IDR_SCRIPT 102

static int write_resource_to_file(int id, const wchar_t *path)
{
    HRSRC hrsrc = FindResourceW(NULL, MAKEINTRESOURCEW(id), RT_RCDATA);
    if (!hrsrc)
        return 0;

    HGLOBAL hglob = LoadResource(NULL, hrsrc);
    if (!hglob)
        return 0;

    DWORD size = SizeofResource(NULL, hrsrc);
    const void *data = LockResource(hglob);
    if (!data || size == 0)
        return 0;

    HANDLE h = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return 0;

    DWORD written = 0;
    BOOL ok = WriteFile(h, data, size, &written, NULL);
    CloseHandle(h);
    return ok && written == size;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrev, PWSTR cmdLine, int show)
{
    (void)hInstance;
    (void)hPrev;
    (void)cmdLine;
    (void)show;

    wchar_t temp[MAX_PATH];
    if (!GetTempPathW(MAX_PATH, temp)) {
        MessageBoxW(NULL, L"Nao foi possivel localizar a pasta temporaria.",
                    L"VLC ZoomPan Setup", MB_OK | MB_ICONERROR);
        return 1;
    }

    DWORD pid = GetCurrentProcessId();
    wchar_t pluginPath[MAX_PATH];
    wchar_t scriptPath[MAX_PATH];

    swprintf(pluginPath, MAX_PATH, L"%sVLCZoomPan-%lu.dll", temp, (unsigned long)pid);
    swprintf(scriptPath, MAX_PATH, L"%sVLCZoomPan-%lu.ps1", temp, (unsigned long)pid);

    if (!write_resource_to_file(IDR_PLUGIN, pluginPath) ||
        !write_resource_to_file(IDR_SCRIPT, scriptPath)) {
        DeleteFileW(pluginPath);
        DeleteFileW(scriptPath);
        MessageBoxW(NULL, L"Falha ao extrair os arquivos internos do instalador.",
                    L"VLC ZoomPan Setup", MB_OK | MB_ICONERROR);
        return 2;
    }

    wchar_t sysdir[MAX_PATH];
    if (!GetSystemDirectoryW(sysdir, MAX_PATH)) {
        DeleteFileW(pluginPath);
        DeleteFileW(scriptPath);
        return 3;
    }

    wchar_t powershell[MAX_PATH];
    swprintf(powershell, MAX_PATH,
             L"%s\\WindowsPowerShell\\v1.0\\powershell.exe", sysdir);

    wchar_t command[4096];
    swprintf(command, 4096,
             L"\"%s\" -NoProfile -STA -ExecutionPolicy Bypass -WindowStyle Hidden -File \"%s\" -PluginPath \"%s\"",
             powershell, scriptPath, pluginPath);

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    BOOL ok = CreateProcessW(NULL, command, NULL, NULL, FALSE,
                             CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (!ok) {
        DeleteFileW(pluginPath);
        DeleteFileW(scriptPath);
        MessageBoxW(NULL, L"Nao foi possivel iniciar o instalador interno.",
                    L"VLC ZoomPan Setup", MB_OK | MB_ICONERROR);
        return 4;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    DeleteFileW(pluginPath);
    DeleteFileW(scriptPath);

    return (int)exitCode;
}
