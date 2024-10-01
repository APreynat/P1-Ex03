#include "framework.h"
#include "P1-Ex03.h"
#include <iostream>
#include <windows.h>
#include <wingdi.h>
#include <stdio.h>
#include <string>
#include <winuser.h>

#define MAX_LOADSTRING 100
#define IDM_MODIFY_BMP 1001  // Custom command ID for the button

#define WM_TIMER 0x0113

UINT Time1;
UINT Time2;

// Global Variables:
HINSTANCE hInst;  // Current instance
WCHAR szTitle[MAX_LOADSTRING];  // Title bar text
WCHAR szWindowClass[MAX_LOADSTRING];  // Main window class name
HWND hButton;  // Handle to the button
HWND hButtonDraw;  // Handle to the button
HBITMAP hBitmap;  // Handle to the bitmap

// Declarations:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

FILE* tempFile;
FILE* outputFile;
BITMAPFILEHEADER fileHeader;
BITMAPINFOHEADER infoHeader;

HDC bm_hdc;

// Function to read BMP into memory
BYTE* readBMP(const char* filename, int& size) {
    if (fopen_s(&tempFile, filename, "rb") != 0) {
        std::cout << "Error: Unable to open file " << filename << std::endl;
        return nullptr;
    }

    fseek(tempFile, 0, SEEK_END);
    size = ftell(tempFile);
    fseek(tempFile, 0, SEEK_SET);

    BYTE* buffer = new BYTE[size];
    fread(buffer, sizeof(BYTE), size, tempFile);
    fclose(tempFile);

    memcpy(&fileHeader, buffer, sizeof(BITMAPFILEHEADER));
    memcpy(&infoHeader, buffer + sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));

    return buffer;
}

// Function to write BMP to disk
void writeBMP(const char* filename, BYTE* buffer, int size) {
    if (fopen_s(&outputFile, filename, "wb") != 0) {
        std::cout << "Error: Unable to open file for writing " << filename << std::endl;
        return;
    }

    fwrite(buffer, sizeof(BYTE), size, outputFile);
    fclose(outputFile);
}

// Calculate row size (padded)
int calculateRowSize(int width, int bitCount) {
    return ((bitCount * width + 31) / 32) * 4;
}

// Modify a pixel in the BMP
void modifyPixel(BYTE* pixelArray, int x, int y, int width, int height, int rowSize, BYTE red, BYTE green, BYTE blue) {
    int pixelIndex = (height - y - 1) * rowSize + x * 3;
    pixelArray[pixelIndex] = blue;
    pixelArray[pixelIndex + 1] = green;
    pixelArray[pixelIndex + 2] = red;
}

// Main Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDM_MODIFY_BMP: {  // Handle button click
            int size;
            BYTE* bmpDataBuffer = readBMP("C:/Users/apreynat/Pictures/tableau.bmp", size);
            if (!bmpDataBuffer) {
                MessageBox(hWnd, L"Failed to open BMP file", L"Error", MB_OK | MB_ICONERROR);
                break;
            }

            // Calculate row size and modify a pixel (e.g., make a pixel red)
            int rowSize = calculateRowSize(infoHeader.biWidth, infoHeader.biBitCount);
            modifyPixel(bmpDataBuffer + fileHeader.bfOffBits, 10, 10, infoHeader.biWidth, infoHeader.biHeight, rowSize, 255, 0, 0);  // Modify pixel to red

            // Write modified BMP to a new file
            //writeBMP("C:/Users/apreynat/Pictures/tableau-modified.bmp", bmpDataBuffer, size);

            BYTE* pixelArray = bmpDataBuffer + fileHeader.bfOffBits;

            HDC hdc = GetDC(hWnd);

            hBitmap = CreateDIBitmap(
                hdc,
                &infoHeader,
                CBM_INIT,
                pixelArray,
                (BITMAPINFO*)&infoHeader,
                DIB_RGB_COLORS
            );
            bm_hdc = CreateCompatibleDC(hdc);
            ReleaseDC(hWnd, hdc);

            SelectObject(bm_hdc, hBitmap);

            if (hBitmap) {
                MessageBox(hWnd, L"BMP modified and HBITMAP created!", L"Success", MB_OK);
            }
            else {
                MessageBox(hWnd, L"Failed to create HBITMAP", L"Error", MB_OK | MB_ICONERROR);
            }

            delete[] bmpDataBuffer;
            break;
        }
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case 1:
           
            RedrawWindow(hWnd, 0, 0, RDW_INVALIDATE | RDW_NOCHILDREN);

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_CREATE: {
        hButton = CreateWindowW(
            L"BUTTON",  
            L"Modify BMP", 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 10, 120, 30,  
            hWnd, 
            (HMENU)IDM_MODIFY_BMP,  
            hInst,  
            nullptr);

        hButtonDraw = CreateWindowW(
            L"BUTTON",
            L"Draw BMP",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            300, 10,
            120, 30,
            hWnd,
            (HMENU)1, 
            hInst,
            nullptr);
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        BitBlt(hdc, 150, 150, infoHeader.biWidth, infoHeader.biHeight, bm_hdc, 0, 0, SRCCOPY);
        
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// About dialog box handler
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Register window class
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_P1EX03));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_P1EX03);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// Initialize application instance
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_P1EX03, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_P1EX03));
    MSG msg;

    // Main message loop
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}
