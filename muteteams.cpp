#include <Windows.h>

#define BUFSIZE 512

void Send_Ctrl_Shift_M() {
    int val[] = {
        VK_CONTROL,  // Ctrl
        VK_SHIFT,    // Shift
        0x4D,        // ASCII M
    };
    const unsigned int len = sizeof(val) / sizeof(val[0]);
    INPUT inputs[len];

    for(unsigned int i = 0; i < len; i++) {
        inputs[i].type = INPUT_KEYBOARD;
        inputs[i].ki.wVk = val[i];
        inputs[i].ki.dwFlags = 0;
        SendInput(1, &inputs[i], sizeof(INPUT));
    }

    for(unsigned int i = 0; i < len; i++) {
        inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &inputs[i], sizeof(INPUT));
    }
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lpr) {
    char B[BUFSIZE] = {0};
    char chk[23] = {'M',        'i',        'c',        'r',        'o',        's',        'o', 'f',
                    't',        ' ',        'T',        'e',        'a',        'm',        's', ' ',
                    (char)0x92, (char)0xCA, (char)0x98, (char)0x62, (char)0x92, (char)0x86, NULL};  // [Microsoft Teams
                                                                                                    // 通話中]を表すUNICODE文字列

    if(hwnd == NULL)
        return FALSE;

    GetWindowTextA(hwnd, B, BUFSIZE);

    if(B[0]) {
        if(memcmp(B, chk, 22) == 0)
            ShowWindow(hwnd, SW_RESTORE);

        if(strstr(B, " | Microsoft Teams")) {
            if(IsIconic(hwnd))
                ShowWindow(hwnd, SW_RESTORE);

            SetForegroundWindow(hwnd);
            Send_Ctrl_Shift_M();
            exit(0);
        }
    }

    return TRUE;
}

bool AnyKeyPressed() {
    for(int code = 0; code < 256; ++code) {
        if(GetAsyncKeyState(code) & 0x8000)
            return true;
    }
    return false;
}

/* https://stackoverflow.com/questions/48915216/link-error-when-compiling-win32-application-with-clion-cmake-msvc-2015
 */
int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // int main() {

    try {
        int timeout = 10;
        while(AnyKeyPressed() && --timeout)
            Sleep(100);

        EnumWindows(EnumWindowsProc, NULL);
    } catch(...) {
    }
    return 0;
}
