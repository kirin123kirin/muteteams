/* muteteams | MIT Liscence | https://github.com/kirin123kirin/muteteams/blob/master/LICENSE */
#include <Windows.h>

#define BUFSIZE 512

/* 会議中のウィンドウ名検索ワード */
const wchar_t* meating_chk = L" | Microsoft Teams";
const wchar_t call_chk[] = L"Microsoft Teams 通話中";

/* キーボードを押下する信号を送信する */
void Send_Keys(int* keymap, unsigned int len) {
    INPUT inputs[len];

    /* キーを押下する */
    for(unsigned int i = 0; i < len; i++) {
        inputs[i].type = INPUT_KEYBOARD;
        inputs[i].ki.wVk = keymap[i];
        inputs[i].ki.dwFlags = 0;
        SendInput(1, &inputs[i], sizeof(INPUT));
    }

    /* 押下したキーを上げる */
    for(unsigned int i = 0; i < len; i++) {
        inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &inputs[i], sizeof(INPUT));
    }
}

/* 会議中のTeamsウィンドウタイトルを探しミュートコマンド発行する */
BOOL CALLBACK TeamsGlobalMute(HWND hwnd, LPARAM lpr) {
    wchar_t B[BUFSIZE] = {0};

    /* Windowタイトルを探す */
    if(GetWindowTextW(hwnd, B, BUFSIZE)) {
        /* 通話中のウィンドウがあったらウィンドウフォーカスを当てる */
        if(memcmp(B, call_chk, sizeof(call_chk) - sizeof(call_chk[0])) == 0)
            ShowWindow(hwnd, SW_RESTORE);

        /* 会議中のウィンドウを探す */
        if(wcsstr(B, meating_chk)) {
            /* 会議中のウィンドウが最小化されてたら元のサイズに戻す */
            if(IsIconic(hwnd))
                ShowWindow(hwnd, SW_RESTORE);

            /* 会議中のウィンドウを最前面に出す */
            SetForegroundWindow(hwnd);

            /* Teamsのミュートコマンド Ctrl+Shift+Mのキーマップを発行する */
            int kmap[] = {
                VK_CONTROL,  // Ctrl
                VK_SHIFT,    // Shift
                0x4D,        // ASCII M
            };
            Send_Keys(kmap, sizeof(kmap) / sizeof(kmap[0]));
            return FALSE;
        }
    }
    return TRUE;
}

int MuteHandler() {
    INPUT input;
    input.type = INPUT_KEYBOARD;

    int arry[256] = {0};
    int* p = arry;

    /* キー押下されてたら強制的に離す処理 */
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    for(int code = 0; code < 256; ++code) {
        if(GetAsyncKeyState(code) & 0x8000) {
            *p++ = code;
            input.ki.wVk = code;
            if(SendInput(1, &input, sizeof(INPUT)) == 0)
                break;
        }
    }

    /* 処理本体 */
    int ret = EnumWindows(TeamsGlobalMute, NULL) ? 0 : 1;

    /* 処理開始前に押下されていたキーを押されてた状態に復元する */
    input.ki.dwFlags = 0;
    p = arry;
    for(; *p; ++p) {
        input.ki.wVk = *p;
        if(SendInput(1, &input, sizeof(INPUT)) == 0)
            break;
    }
    return ret;
}

/*
    コマンドプロンプトウィンドウを表示させないためのおまじない
    https://stackoverflow.com/questions/48915216/link-error-when-compiling-win32-application-with-clion-cmake-msvc-2015
 */
int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // int main() {
    return MuteHandler();
}
