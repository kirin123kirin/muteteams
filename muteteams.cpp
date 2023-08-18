/* muteteams | MIT Liscence | https://github.com/kirin123kirin/muteteams/blob/master/LICENSE */
#include <Windows.h>

#define BUFSIZE 512

/* キーボードを押下する信号を送信する */
int Send_Keys(int* keymap, unsigned int len) {
    int ret = 0;
    if (keymap == NULL)
        return 1;
    
    INPUT* p = (INPUT*)malloc(sizeof(INPUT) * (len + 1));
    if (p == NULL)
        return 1;

    /* キーを押下する */
    for(unsigned int i = 0; i < len; ++i, ++p) {
        p->type = INPUT_KEYBOARD;
        p->ki.wVk = keymap[i];
        p->ki.dwFlags = 0;
        if((SendInput(1, p, sizeof(INPUT))))
            p->ki.dwFlags = KEYEVENTF_KEYUP;
        else
            ret += 1;
    }

    p -= len;
    /* 押下したキーを上げる */
    for(unsigned int i = 0; i < len; ++i, ++p) {
        if(p->type == INPUT_KEYBOARD && p->ki.dwFlags == KEYEVENTF_KEYUP && p->ki.wVk == keymap[i])
            ret += SendInput(1, p, sizeof(INPUT)) ? 0 : 1;
    }

    free(p);
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
    INPUT inputs[256];

    int arry[256] = {0};

    /* キー押下されてたら強制的に離す処理 */
    for(unsigned int i = 0; i < 256; ++i) {
        auto& p = inputs[i];
        p.type = INPUT_KEYBOARD;
        p.ki.dwFlags = KEYEVENTF_KEYUP;
        if(GetAsyncKeyState(i) & 0x8000) {
            p.ki.wVk = i;
            if(SendInput(1, &p, sizeof(INPUT))) {
                p.ki.dwFlags = 0;
                p.ki.wVk = i;
            } else {
                return 1;
            }
        }
    }

    /* 処理本体 */
    int ret = EnumWindows(TeamsGlobalMute, NULL) ? 0 : 1;

    /* 処理開始前に押下されていたキーを押されてた状態に復元する */
    for(unsigned int i = 0; i < 256; ++i) {
        auto& p = inputs[i];
        if(p.type == INPUT_KEYBOARD && p.ki.dwFlags == 0 && p.ki.wVk == i)
            ret += SendInput(1, &p, sizeof(INPUT)) ? 0 : 1;
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
