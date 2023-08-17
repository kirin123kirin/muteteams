/* muteteams | MIT Liscence | https://github.com/kirin123kirin/muteteams/blob/master/LICENSE */
#include <Windows.h>

#define BUFSIZE 512

/* キーボードを押下する信号を送信する */
int Send_Keys(int* keymap, unsigned int len) {
    INPUT inputs[len];
    int ret = 0;

    /* キーを押下する */
    for(unsigned int i = 0; i < len; i++) {
        inputs[i].type = INPUT_KEYBOARD;
        inputs[i].ki.wVk = keymap[i];
        inputs[i].ki.dwFlags = 0;
        if((SendInput(1, &inputs[i], sizeof(INPUT))))
            inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
        else
            ret += 1;
    }

    /* 押下したキーを上げる */
    for(unsigned int i = 0; i < len; i++) {
        if(inputs[i].ki.dwFlags == KEYEVENTF_KEYUP)
            ret += SendInput(1, &inputs[i], sizeof(INPUT)) ? 0 : 1;
    }
    return ret;
}

/* 会議中のTeamsウィンドウタイトルを探しミュートコマンド発行する */
BOOL CALLBACK TeamsGlobalMute(HWND hwnd, LPARAM lpr) {
    char B[BUFSIZE] = {0};

    /* Windowタイトルを探す */
    if(GetWindowTextA(hwnd, B, BUFSIZE)) {
        if(strstr(B, " | Microsoft Teams")) {
            /* 会議中のウィンドウを探す */
            SwitchToThisWindow(hwnd, true);
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
                return 1;
        }
    }

    /* 処理本体 */
    int ret = EnumWindows(TeamsGlobalMute, NULL) ? 0 : 1;

    /* 処理開始前に押下されていたキーを押されてた状態に復元する */
    input.ki.dwFlags = 0;
    p = arry;
    for(; *p; ++p) {
        input.ki.wVk = *p;
        ret += SendInput(1, &input, sizeof(INPUT)) ? 0 : 1;
    }
    return ret;
}

/*
    コマンドプロンプトウィンドウを表示させないためのおまじない
    https://stackoverflow.com/questions/48915216/link-error-when-compiling-win32-application-with-clion-cmake-msvc-2015
 */
int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return MuteHandler();
}
