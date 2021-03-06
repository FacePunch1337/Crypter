// SysDialog.cpp : Определяет точку входа для приложения.
//
#define _CRT_SECURE_NO_WARNINGS
#define _WTL_NO_CSTRING
#include "framework.h"
#include "SysDialog.h"
#include <commdlg.h>
#include <stdio.h>
#include <Windowsx.h>
#include <Windows.h>
#include <CommCtrl.h>
#include <string>
#include <iostream>
#include <fstream>


#define MAX_LOADSTRING 100
#define CMD_OPEN_FIRST_FILE 1001
#define CMD_OPEN_SECOND_FILE 1002
#define CMD_SAVE_FILE  1003
#define CMD_SAVE_FILE2  1004
#define ID_FILE_SAVEAS  1005
#define CMD_CIPHOR_FILE  1006
#define CMD_DECIPHOR_FILE  1007
#define CMD_STOP_EDIT_FILE  1008
#define CMD_TRANSPORT_FILE_DATA 1009
#define CMD_TEST_DLL 1010
#define DLL_FILE_NAME "CipherDll.dll"

typedef char (*crypto_t)(char, char);


// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];
WCHAR hookbuff[MAX_LOADSTRING];
crypto_t cipher, decipher;
HMODULE dll;
HWND  fNameStatic;
HWND  fNameStatic2;
HWND  editor, crypter;
HWND  saver;
HWND  saver2;
HWND  transport;
HWND  editPass;
HWND progress;
HWND cipher_button;
HWND decipher_button;
HHOOK kbhook;
char f1content[1024] = "\0";
char f2content[1024] = "\0";

bool bsave1 = false;
bool bsave2 = false;
OPENFILENAMEW ofn;
HANDLE hFile1;
HANDLE hFile2;
LPCSTR path_to_file;

int HooKbuffCount = 0;


// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
DWORD   CALLBACK    OpenFileClick(LPVOID);
DWORD   CALLBACK    OpenFileClick2(LPVOID);
DWORD   CALLBACK    SaveFileClick(LPVOID);

DWORD   CALLBACK    TransportFileClick(LPVOID);
DWORD   CALLBACK    CipherDll(LPVOID);
DWORD   CALLBACK    CipherClick(LPVOID);
DWORD   CALLBACK    DecipherClick(LPVOID);
DWORD CALLBACK      StartKbHook(LPVOID);
LRESULT CALLBACK    KbHookProc(int, WPARAM, LPARAM);


bool FileExists(LPCTSTR fname);
bool bchoicer = false;
bool bchoicel = false;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SYSDIALOG, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SYSDIALOG));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SYSDIALOG));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SYSDIALOG);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE: {
        CipherDll(&hWnd);
        CreateWindowExW(0, L"Button", L"...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            420, 10, 37, 23, hWnd, (HMENU)CMD_OPEN_FIRST_FILE, hInst, NULL);
        CreateWindowExW(0, L"Button", L"...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            420, 40, 37, 23, hWnd, (HMENU)CMD_OPEN_SECOND_FILE, hInst, NULL);

        fNameStatic = CreateWindowExW(0, L"Edit", L"Source file",
            WS_CHILD | WS_VISIBLE  | ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL,
            10, 10, 400, 23, hWnd, 0, hInst, NULL);
        fNameStatic2 = CreateWindowExW(0, L"Edit", L"Destination file",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL,
            10, 40, 400, 23, hWnd, 0, hInst, NULL);
        CreateWindowExW(0, L"Static", L"cipher PIN",
            WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
            440, 70, 100, 23, hWnd, 0, hInst, NULL);
        editPass = CreateWindowExW(0, L"Edit", L"",
            WS_CHILD |WS_BORDER | WS_VISIBLE | ES_CENTER |ES_PASSWORD ,
           530, 70, 32, 23, hWnd, 0, hInst, NULL);
        cipher_button = CreateWindowExW(0, L"Button", L"Ciphor",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            420, 100, 75, 23, hWnd, (HMENU)CMD_CIPHOR_FILE, hInst, NULL);
        decipher_button = CreateWindowExW(0, L"Button", L"Deciphor",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            505, 100, 75, 23, hWnd, (HMENU)CMD_DECIPHOR_FILE, hInst, NULL);
        editor = CreateWindowExW(0, L"Edit", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
            10, 90, 300, 300, hWnd, 0, hInst, NULL);
        
        saver = CreateWindowExW(0, L"Button", L"Save",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            150, 400, 75, 23, hWnd, (HMENU)CMD_SAVE_FILE, hInst, NULL);
        saver2 = CreateWindowExW(0, L"Button", L"Save",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            800, 400, 75, 23, hWnd, (HMENU)CMD_SAVE_FILE2, hInst, NULL);
        transport = CreateWindowExW(0, L"Button", L"Transport",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            235, 400, 75, 23, hWnd, (HMENU)CMD_TRANSPORT_FILE_DATA, hInst, NULL);

        progress = CreateWindowW(PROGRESS_CLASSW, L"",
            WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
            420, 130, 160, 23, hWnd, 0, hInst, NULL);
        SendMessageW(progress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
        SendMessageW(progress, PBM_SETBARCOLOR, 0, RGB(100, 200, 250));
        SendMessageW(progress, PBM_DELTAPOS, 0, 0);
        CreateWindowExW(0, L"Button", L"STOP",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            465, 160, 75, 23, hWnd, (HMENU)CMD_STOP_EDIT_FILE, hInst, NULL);

        CreateWindowExW(0, L"Button", L"TEST DLL",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            450, 190, 100, 23, hWnd, (HMENU)CMD_TEST_DLL, hInst, NULL);

       

        crypter = CreateWindowExW(0, L"Edit", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
            690, 90, 300, 300, hWnd, 0, hInst, NULL);

        
            Button_Enable(cipher_button, false);
            Button_Enable(decipher_button, false);
       
            StartKbHook(NULL);
            

        dll = (HMODULE) 0;

        break;
    }
    case WM_COMMAND: {
        int notifId = HIWORD(wParam);
        // Parse notifications:

        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case CMD_OPEN_FIRST_FILE:
            CreateThread(NULL, 0,
                OpenFileClick, &hWnd,
                0, NULL);
            break;
        case CMD_OPEN_SECOND_FILE:
            CreateThread(NULL, 0,
                OpenFileClick2, &hWnd,
                0, NULL);
            break;
            case  CMD_TRANSPORT_FILE_DATA:
                CreateThread(NULL, 0,
                    TransportFileClick, &hWnd,
                    0, NULL);
                SendMessageW(transport, WM_KILLFOCUS, 0, 0);
                break;
        case CMD_SAVE_FILE:
            bsave1 = true;
            SaveFileClick(&hWnd);
            SendMessageW(saver, WM_KILLFOCUS, 0, 0);
            break;
        case CMD_SAVE_FILE2:
            bsave2 = true;
            SaveFileClick(&hWnd);
            SendMessageW(saver, WM_KILLFOCUS, 0, 0);
            break;
        case ID_FILE_SAVEAS:

            SaveFileClick(&hWnd);
            break;

        case CMD_TEST_DLL:
            CipherDll(&hWnd);
            break;
        case CMD_CIPHOR_FILE:
            CreateThread(NULL, 0,
                CipherClick, &hWnd,
                0, NULL);
            SendMessageW(cipher_button, WM_KILLFOCUS, 0, 0);
            break;

        case CMD_DECIPHOR_FILE:
            CreateThread(NULL, 0,
                DecipherClick, &hWnd,
                0, NULL);
            SendMessageW(decipher_button, WM_KILLFOCUS, 0, 0);
            break;

        

       
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
                   break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

DWORD CALLBACK OpenFileClick(LPVOID  params) {



    HWND hWnd = *((HWND*)params);

    WCHAR fname[512] = L"Hello.txt\0";
    
    OPENFILENAMEW ofn;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.hInstance = hInst;
    ofn.lpstrFile = fname;
    ofn.nMaxFile = 512;
    ofn.lpstrFilter = L"All files\0*.*\0Text files\0*.txt\0C++ code file\0*.cpp;*.c\0\0";

    if (GetOpenFileNameW(&ofn)) {

     

        SendMessageW(fNameStatic, WM_SETTEXT, 0,
            (LPARAM)ofn.lpstrFile);
        // read file to editor
        hFile1 = CreateFileW(fname, GENERIC_READ, 0, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile1 == 0) {
            SendMessageW(editor, WM_SETTEXT, 0,
                (LPARAM)L"File open error");
        }
        else {
            DWORD fSize;
            fSize = GetFileSize(hFile1, NULL);
            if (fSize > 0) {
                char* f1content = new char[fSize + 1];
                DWORD read;
                if (ReadFile(hFile1, f1content, fSize, &read, NULL)) {
                    f1content[fSize] = '\0';
                    SendMessageA(editor, WM_SETTEXT, 0,
                        (LPARAM)f1content);

                    delete[] f1content;
                }
                else {
                    SendMessageW(editor, WM_SETTEXT, 0,
                        (LPARAM)L"File read error");
                }
            }
            else {
                SendMessageW(editor, WM_SETTEXT, 0,
                    (LPARAM)L"File is empty");
            }
            CloseHandle(hFile1);
        }
    }
    else {
        SendMessageW(fNameStatic, WM_SETTEXT, 0,
            (LPARAM)L"Selection cancelled");
    }

    return 0;
}


DWORD CALLBACK OpenFileClick2(LPVOID  params) {



    HWND hWnd = *((HWND*)params);

    WCHAR fname[512] = L"0";

    OPENFILENAMEW ofn;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.hInstance = hInst;
    ofn.lpstrFile = fname;
    ofn.nMaxFile = 512;
    ofn.lpstrFilter = L"All files\0*.*\0Text files\0*.txt\0C++ code file\0*.cpp;*.c\0\0";

    if (GetOpenFileNameW(&ofn)) {



        SendMessageW(fNameStatic2, WM_SETTEXT, 0,
            (LPARAM)ofn.lpstrFile);
        // read file to editor
        hFile2 = CreateFileW(fname, GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile2 == 0) {
            SendMessageW(editor, WM_SETTEXT, 0,
                (LPARAM)L"File open error");
        }
        else {
            DWORD fSize;
            fSize = GetFileSize(hFile2, NULL);
            if (fSize > 0) {
                char* f2content = new char[fSize + 1];
                DWORD read;
                if (ReadFile(hFile2, f2content, fSize, &read, NULL)) {
                    f2content[fSize] = '\0';
                    SendMessageA(editor, WM_SETTEXT, 0,
                        (LPARAM)f2content);

                    delete[] f2content;
                }
                else {
                    SendMessageW(editor, WM_SETTEXT, 0,
                        (LPARAM)L"File read error");
                }
            }
           
            CloseHandle(hFile2);
           
        }
    }
    else {
        SendMessageW(fNameStatic2, WM_SETTEXT, 0,
            (LPARAM)L"Selection cancelled");
    }

    return 0;
}

char srcName[512] = "\0";
char destName[512] = "\0";

DWORD CALLBACK SaveFileClick(LPVOID params) {
    HWND hWnd = *((HWND*)params);

    if (bsave1 == true) {
        SendMessageA(editor, WM_GETTEXT, 1024, (LPARAM)f1content);

       

        OPENFILENAMEW ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWnd;
        ofn.hInstance = hInst;
        ofn.lpstrFile = (LPWSTR)srcName;
        ofn.nMaxFile = 512;
        ofn.lpstrFilter = L"All files\0*.*\0Text files\0*.txt\0C++ code file\0*.cpp;*.c\0\0";

        if (GetSaveFileNameW(&ofn)) {
            
            hFile1 = CreateFileW(
                (LPCWSTR)srcName, GENERIC_WRITE, 0, NULL,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile1 == 0) {
                SendMessageW(editor, WM_SETTEXT, 0,
                    (LPARAM)L"File open error");
                return -1;
            }
            else {
                DWORD write;
                if (!WriteFile(hFile1, f1content, strnlen_s(f1content, 1024), &write, NULL)) {

                    MessageBoxA(hWnd, "write error", "write error", MB_OK | MB_ICONWARNING);


                    SendMessageW(editor, WM_SETTEXT, 0,
                        (LPARAM)L"write error");
                }

                CloseHandle(hFile1);
            }
            
        }
        bsave1 = false;
    }
  
    if(bsave2 == true) {
        SendMessageA(crypter, WM_GETTEXT, 1024, (LPARAM)f2content);

        

        OPENFILENAMEW ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWnd;
        ofn.hInstance = hInst;
        ofn.lpstrFile = (LPWSTR)destName;
        ofn.nMaxFile = 512;
        ofn.lpstrFilter = L"All files\0*.*\0Text files\0*.txt\0C++ code file\0*.cpp;*.c\0\0";

        if (GetSaveFileNameW(&ofn)) {

            hFile2 = CreateFileW(
                (LPWSTR)destName, GENERIC_WRITE, 0, NULL,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile2 == 0) {
                SendMessageW(crypter, WM_SETTEXT, 0,
                    (LPARAM)L"File open error");
                return -1;
            }
            else {
                DWORD write;
                if (!WriteFile(hFile2, f2content, strnlen_s(f2content, 1024), &write, NULL)) {

                    MessageBoxA(hWnd, "write error", "write error", MB_OK | MB_ICONWARNING);


                    SendMessageW(crypter, WM_SETTEXT, 0,
                        (LPARAM)L"write error");
                }

                CloseHandle(hFile2);
            }
        }
        bsave2 = false;
    }
    

 

    return 0;
}



//char srcName[512] = "0";
DWORD CALLBACK TransportFileClick(LPVOID params) {
    HWND hWnd = *((HWND*)params);
    WCHAR str[1024];
    SendMessageW(editor, WM_GETTEXT, 0, (LPARAM)str);
    
   /* OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = hWnd;
    ofn.hInstance = hInst;
    ofn.lpstrFile = srcName;
    ofn.nMaxFile = 512;
        
    if (GetOpenFileNameA(&ofn)) {

        

    }*/


    return 0;
}

DWORD CALLBACK CipherDll(LPVOID params) {
    HWND hWnd = *((HWND*)params);
    dll = LoadLibraryA(DLL_FILE_NAME);
    if (dll != 0){
       
        cipher = (crypto_t)GetProcAddress(dll, "Cipher");
        if (cipher == NULL) {
            SendMessageW(editor, WM_SETTEXT, 0, (LPARAM)"Cipher not located");
            CloseHandle(dll);
            dll = (HMODULE)0;
            return -2;
        }
        decipher = (crypto_t)GetProcAddress(dll, "Decipher");
        if (decipher == NULL) {
            SendMessageW(editor, WM_SETTEXT, 0, (LPARAM)"Decipher not located");
            CloseHandle(dll);
            dll = (HMODULE)0;
            return -3;
        }
    
        char c = 'c', p = 'p', d, s;
        s = cipher(c, p);
        d = decipher(s, p);

        WCHAR wc, wp, wd, ws;
        mbstowcs(&wc, &c, 1);
        mbstowcs(&wp, &p, 1);
        mbstowcs(&wd, &d, 1);
        mbstowcs(&ws, &s, 1);
        WCHAR str[100];
        _snwprintf_s(str, 100, L"%c ^ %c -> %c; %c ^ %c -> %c",
            wc, wp, ws, ws, wp, wd);
        SendMessageW(editor, WM_SETTEXT, 0, (LPARAM)str);
    }
    if (dll == 0) {

        if (IDYES == MessageBoxW(hWnd, L"Dll not foundet, are you need to include?", L"DLL error", MB_YESNO)) {


            WCHAR fname[512] = L"\0";
       
            OPENFILENAMEW ofn;
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.hInstance = hInst;
            ofn.lpstrFile = fname;
            ofn.nMaxFile = 512;
            ofn.lpstrFilter = L"All files\0*.*\0DLL\0*.dll\0C++ code file\0*.cpp;*.c\0\0";
       
             if (GetOpenFileName(&ofn)) {
                dll = LoadLibraryA(DLL_FILE_NAME);

            }
            
            
        }
        else{
            exit(0);
        }
    }
       
        
        return 0;
        
    
}

DWORD CALLBACK CipherClick(LPVOID params) {
    
    HWND hWnd = *((HWND*)params);
    /*if (GetWindowTextLengthA(editPass) < 4) {
        MessageBoxA(NULL, "Minimum 4 characters", "PIN cipher short", MB_ICONERROR | MB_OK);
    }*/
    //else {

        if (cipher == NULL) {
            MessageBoxW(hWnd, L"Dll not foundet, are you need to include?", L"DLL error", MB_YESNO);
            return -1;
        }
        else{
            FILE* fsrc;
            fsrc = fopen(srcName, "rb");

            FILE* fdest;
            fdest = fopen(destName, "wb");
            
           
           

          
            SendMessageW(progress, PBM_STEPIT, 10, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 20, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 30, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 40, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 50, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 60, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 70, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 80, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 90, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 100, 0);
            Sleep(1000);
            int pos = SendMessageW(progress, PBM_GETPOS, 100, 0);
            if (pos == 100) {
                char pass[5];
                
                size_t len_pass = SendMessageA(editPass, WM_GETTEXT, 1024, (LPARAM)pass);
                //size_t len_pass = strlen(pass);
                if (len_pass == 4) {
                    Button_Enable(cipher_button, true);
                }
              

                    char* pin = new char[len_pass + 1];
                    for (size_t i = 0; i < len_pass; i++) {
                        pin[i] = pass[i];
                    }

                    char txt[1024];
                    SendMessageA(editor, WM_GETTEXT, 1024, (LPARAM)txt);
                    size_t len = strlen(txt);

                    

                    char* cod = new char[len + 1];
                    for (size_t i = 0; i < len; i++) {
                        cod[i] = cipher(txt[i], pass[i % 5]);
                    }
                    cod[len] = '\0';
                    SendMessageA(crypter, WM_SETTEXT, 0, (LPARAM)cod);
                    SendMessageW(progress, PBM_SETPOS, 0, 0);
                }
            
          
            
     }
    
        
        

    return 0;
}

DWORD CALLBACK DecipherClick(LPVOID params) {
    HWND hWnd = *((HWND*)params);
    if (GetWindowTextLengthA(editPass) > 4) {
        MessageBoxA(NULL, "max 4 characters", "PIN cipher long", MB_ICONERROR | MB_OK);
    }
    else {
        if (cipher == NULL) {
            MessageBoxW(hWnd, L"Dll not foundet", L"DLL error", NULL);
            return -1;
        }
        else {
            SendMessageW(progress, PBM_STEPIT, 10, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 20, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 30, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 40, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 50, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 60, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 70, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 80, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 90, 0);
            Sleep(30);
            SendMessageW(progress, PBM_STEPIT, 100, 0);
            Sleep(1000);
            int pos = SendMessageW(progress, PBM_GETPOS, 100, 0);
            if (pos == 100) {
            char pass[5];
            SendMessageA(editPass, WM_GETTEXT, 1024, (LPARAM)pass);
            size_t len_pass = strlen(pass);
            char* pin = new char[len_pass + 1];
            for (size_t i = 0; i < len_pass; i++) {
                pin[i] = pass[i];
            }
    
          char txt[1024];
             SendMessageA(crypter, WM_GETTEXT, 1024, (LPARAM)txt);
             size_t len = strlen(txt);

            char* cod = new char[len + 1];
             for (size_t i = 0; i < len; i++) {
                cod[i] = decipher(txt[i], pass[i % 5]);
             }
                 cod[len] = '\0';
                SendMessageA(editor, WM_SETTEXT, 0, (LPARAM)cod);
                SendMessageW(progress, PBM_SETPOS, 0, 0);
            }
          
        }
        
    }
    return 0;
}

char text[100];

DWORD CALLBACK StartKbHook(LPVOID params) {
    kbhook = SetWindowsHookExW(WH_KEYBOARD, KbHookProc, (HINSTANCE)NULL, GetCurrentThreadId());
    return 0;
}

LRESULT CALLBACK KbHookProc(int nCode, WPARAM wParam, LPARAM lParam) {


    if (nCode == HC_ACTION) {
        {
             HooKbuffCount = SendMessageW(editPass, WM_GETTEXT, 100, (LPARAM)hookbuff);
            if (HooKbuffCount >= 4 ) {
                Button_Enable(cipher_button, true);
                Button_Enable(decipher_button, true);
            }
            else {
                Button_Enable(cipher_button, false);
                Button_Enable(decipher_button, false);
            }
        }
    }
    return CallNextHookEx(kbhook, nCode, wParam, lParam);

}