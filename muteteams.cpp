/* muteteams | MIT Liscence | https://github.com/kirin123kirin/muteteams/blob/master/LICENSE */
#include <Windows.h>

#define BUFSIZE 512

const wchar_t* meating_chk = L" | Microsoft Teams";
const wchar_t call_chk[] = L"Microsoft Teams 通話中";

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
    wchar_t B[BUFSIZE] = {0};
    if(GetWindowTextW(hwnd, B, BUFSIZE)) {
        if(memcmp(B, call_chk, sizeof(call_chk) - sizeof(call_chk[0])) == 0)
            ShowWindow(hwnd, SW_RESTORE);

        if(wcsstr(B, meating_chk)) {
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
        int timeout = 100;
        while(AnyKeyPressed() && --timeout)
            Sleep(10);

        EnumWindows(EnumWindowsProc, NULL);
    } catch(...) {
    }
    return 0;
}
