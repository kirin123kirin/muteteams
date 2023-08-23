/* muteteams | MIT Liscence | https://github.com/kirin123kirin/muteteams/blob/master/LICENSE */

#include <Windows.h>

#define BUFSIZE 512

NOTIFYICONDATA g_nid;
HMENU g_contextMenu = NULL;
HHOOK g_hook = NULL;
HANDLE hLogoutEvent = NULL;

/* キーボードを押下する信号を送信する */
int Send_Keys(int* keymap, unsigned int len) {
    int ret = 0;
    if(keymap == NULL)
        return 1;

    INPUT* heap = (INPUT*)malloc(sizeof(INPUT) * (len + 1));
    if(heap == NULL)
        return 1;
    INPUT* p = heap;
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

    p = heap;
    /* 押下したキーを上げる */
    for(unsigned int i = 0; i < len; ++i, ++p) {
        if(p->type == INPUT_KEYBOARD && p->ki.dwFlags == KEYEVENTF_KEYUP && p->ki.wVk == keymap[i])
            ret += SendInput(1, p, sizeof(INPUT)) ? 0 : 1;
    }

    free(heap);
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
    INPUT p[256];

    /* キー押下されてたら強制的に離す処理 */
    for(unsigned int i = 0; i < 256; ++i) {
        p[i].type = INPUT_KEYBOARD;
        p[i].ki.dwFlags = KEYEVENTF_KEYUP;
        if(GetAsyncKeyState(i) & 0x8000) {
            p[i].ki.wVk = i;
            if(SendInput(1, &p[i], sizeof(INPUT))) {
                p[i].ki.dwFlags = 0;
                p[i].ki.wVk = i;
            } else {
                return 1;
            }
        }
    }

    /* 処理本体 */
    int ret = EnumWindows(TeamsGlobalMute, NULL) ? 0 : 1;

    /* 処理開始前に押下されていたキーを押されてた状態に復元する */
    for(unsigned int i = 0; i < 256; ++i) {
        if(p[i].type == INPUT_KEYBOARD && p[i].ki.dwFlags == 0 && p[i].ki.wVk == i)
            ret += SendInput(1, &p[i], sizeof(INPUT)) ? 0 : 1;
    }
    return ret;
}

/* キー監視 */
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if(nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        if(kbStruct->vkCode == VK_NONCONVERT || kbStruct->vkCode == VK_PAUSE) {
            MuteHandler();
            UnhookWindowsHookEx(g_hook);
            PostQuitMessage(0);
        }
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_APP + 1:
            switch(lParam) {
                case WM_RBUTTONUP: {
                    POINT cursorPos;
                    GetCursorPos(&cursorPos);
                    SetForegroundWindow(hwnd);
                    TrackPopupMenu(g_contextMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, cursorPos.x, cursorPos.y, 0, hwnd,
                                   NULL);
                    PostMessage(hwnd, WM_NULL, 0, 0);
                    break;
                }
            }
            break;
        case WM_COMMAND:
            if(LOWORD(wParam) == 1) {
                // 終了メニューが選択されたらプログラム終了
                if(g_hook)
                    UnhookWindowsHookEx(g_hook);
                DestroyWindow(hwnd);
                ExitProcess(0);
            }
            break;
        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &g_nid);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_LOGOFF_EVENT) {
        // ログオフイベントが発生した場合、プログラムを終了
        if(g_hook)
            UnhookWindowsHookEx(g_hook);
        ExitProcess(0);
    }
    return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc = {sizeof(WNDCLASSEX),    CS_CLASSDC, WindowProc, 0L,   0L,
                     GetModuleHandle(NULL), NULL,       NULL,       NULL, NULL,
                     TEXT("MyWindowClass"), NULL};
    if(RegisterClassEx(&wc) == 0)
        return 1;

    HWND hwnd = CreateWindow(wc.lpszClassName, NULL, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
    if(hwnd == NULL)
        return 1;

    // ログオフイベントを監視するイベントハンドルを作成
    if((hLogoutEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
        return 1;

    // ログオフイベントを監視
    if((SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE)) == 0)
        return 1;

    int RETCODE = 0;
    try {
        ShowWindow(hwnd, SW_HIDE);

        g_nid.cbSize = sizeof(NOTIFYICONDATA);
        g_nid.hWnd = hwnd;
        g_nid.uID = 1;
        g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        g_nid.uCallbackMessage = WM_APP + 1;

        // カスタムアイコンを読み込む
        g_nid.hIcon =
            (HICON)LoadImage(NULL, TEXT("icon.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
        if(!g_nid.hIcon) {
            goto finalize;
        }

        // タスクトレイアイコンの設定
        if(!(Shell_NotifyIcon(NIM_ADD, &g_nid)))
            goto finalize;

        // コンテキストメニューを作成
        if((g_contextMenu = CreatePopupMenu()) == NULL)
            goto finalize;

        if((AppendMenu(g_contextMenu, MF_STRING, 1, TEXT("exit"))) == 0)
            goto finalize;

        while(1) {
            // キーボードフックを設定
            if((g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0)) == NULL)
                goto finalize;

            // メッセージループ
            MSG msg;
            BOOL chk;
            while((chk = GetMessage(&msg, NULL, 0, 0)) != 0) {
                if(chk == -1)
                    goto finalize;
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            RETCODE += msg.wParam;
            if((UnhookWindowsHookEx(g_hook)) == 0)
                goto finalize;
            // Sleep(10);
        }

    } catch(...) {
        RETCODE += 1;
    }

finalize:
    if(hwnd)
        DestroyWindow(hwnd);

    return RETCODE;
}
