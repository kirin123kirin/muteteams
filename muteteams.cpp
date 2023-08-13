#include <Windows.h>

void Send_Ctrl_Shift_M() {
    int val[] = {
        VK_CONTROL, // Ctrl
        VK_SHIFT,   // Shift
        0x4D,       // ASCII M
    };

    INPUT inputs[sizeof(val)];

    for(int i = 0; i < sizeof(val); i++) {
        inputs[i].type = INPUT_KEYBOARD;
        inputs[i].ki.wVk = val[i];
        inputs[i].ki.dwFlags = 0;
        SendInput(1, &inputs[i], sizeof(INPUT));
    }

    for(int i = 0; i < sizeof(val); i++) {
        inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &inputs[i], sizeof(INPUT));
    }
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lpr) {
    char buffer[512] = {0};
    GetWindowTextA(hwnd, buffer, sizeof(buffer) / sizeof(buffer[0]));

    if(buffer[0] && strstr(buffer, " | Microsoft Teams")) {
        if(IsIconic(hwnd))
            ShowWindow(hwnd, SW_RESTORE);

        SetForegroundWindow(hwnd);
        Send_Ctrl_Shift_M();
        exit(0);
    }

    return TRUE;
}

int main() {
    EnumWindows(EnumWindowsProc, NULL);
    return 0;
}
