/**
* Mystic Light LED Helper Tool
* Written by CptKakise after drinking too much coffee
* 
* This program should detect the LED zones and (probably) generate a boilerplate plugin for SignalRGB
* It should be ran as Administrator or you will face a beautiful ML_Initialize timeout loop
* 
* Copyright © 2022 CptKakise <https://github.com/Kakise>
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**/

#include <iostream>
#include <Windows.h>
#include <strsafe.h>
#include <atlsafe.h>

#include "MysticLight_SDK.h"

struct MysticLight { // Yoinked from somewhere because it's cleaner than what I did
    LPMLAPI_Initialize init;
    LPMLAPI_GetErrorMessage errorMsg;
    LPMLAPI_GetDeviceInfo getDevInfo;
    LPMLAPI_GetLedInfo getLedInfo;
    LPMLAPI_GetLedColor getColor;		
    LPMLAPI_GetLedStyle	getStyle;
    LPMLAPI_GetLedMaxBright getMaxBright;
    LPMLAPI_GetLedBright getBright;
    LPMLAPI_GetLedMaxSpeed getMaxSpeed;
    LPMLAPI_GetLedSpeed getSpeed;
    LPMLAPI_SetLedColor setColor;
    LPMLAPI_SetLedColors setColors;
    LPMLAPI_SetLedStyle setStyle;
    LPMLAPI_SetLedBright setBright;
    LPMLAPI_SetLedSpeed setSpeed;
} ml;

// Yoinked from MSDN, kinda ironic that the best documentation they have
// is for error codes
void ErrorExit(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}

void LoadMLSDK(HINSTANCE* MLinstance) {

    ml.init = (LPMLAPI_Initialize)GetProcAddress(*MLinstance, "MLAPI_Initialize");
    ml.errorMsg = (LPMLAPI_GetErrorMessage)GetProcAddress(*MLinstance, "MLAPI_GetErrorMessage");
    ml.getDevInfo = (LPMLAPI_GetDeviceInfo)GetProcAddress(*MLinstance, "MLAPI_GetDeviceInfo");
    ml.getLedInfo = (LPMLAPI_GetLedInfo)GetProcAddress(*MLinstance, "MLAPI_GetLedInfo");
    ml.getColor = (LPMLAPI_GetLedColor)GetProcAddress(*MLinstance, "MLAPI_GetLedColor");
    ml.getStyle = (LPMLAPI_GetLedStyle)GetProcAddress(*MLinstance, "MLAPI_GetLedStyle");
    ml.getMaxBright = (LPMLAPI_GetLedMaxBright)GetProcAddress(*MLinstance, "MLAPI_GetLedMaxBright");
    ml.getBright = (LPMLAPI_GetLedBright)GetProcAddress(*MLinstance, "MLAPI_GetLedBright");
    ml.getMaxSpeed = (LPMLAPI_GetLedMaxSpeed)GetProcAddress(*MLinstance, "MLAPI_GetLedMaxSpeed");
    ml.getSpeed = (LPMLAPI_GetLedSpeed)GetProcAddress(*MLinstance, "MLAPI_GetLedSpeed");
    ml.setColor = (LPMLAPI_SetLedColor)GetProcAddress(*MLinstance, "MLAPI_SetLedColor");
    ml.setColors = (LPMLAPI_SetLedColors)GetProcAddress(*MLinstance, "MLAPI_SetLedColors");
    ml.setStyle = (LPMLAPI_SetLedStyle)GetProcAddress(*MLinstance, "MLAPI_SetLedStyle");
    ml.setBright = (LPMLAPI_SetLedBright)GetProcAddress(*MLinstance, "MLAPI_SetLedBright");
    ml.setSpeed = (LPMLAPI_SetLedSpeed)GetProcAddress(*MLinstance, "MLAPI_SetLedSpeed");


    CComSafeArray<BSTR> devices;
    CComSafeArray<BSTR> ledCount;
    CComSafeArray<BSTR> ledName;
    CComSafeArray<BSTR> ledStyles;

    int initVal = ml.init();

    BSTR initMsg;
    ml.errorMsg(initVal, &initMsg);
    printf("MSI SDK Initialization: ");
    printf("%S\n", initMsg);

    // Hours spent before I figured that I needed to run the program as admin: 2h
    if (NULL != ml.getDevInfo)
    {
        int retVal = -1; 
        while (retVal != 0) {

            retVal = ml.getDevInfo(&(devices.m_psa), &(ledCount.m_psa));

            BSTR msg;
            ml.errorMsg(retVal, &msg);
            printf("Retreiving devices: ");
            printf("%S\n\n", msg);
        }
        for (LONG i = devices.GetLowerBound(); i <= devices.GetUpperBound(); i++) {
            printf("Device #%d: %S, Led count: %S\n", i, devices.GetAt(i), ledCount.GetAt(i));
        }
        // Yes, I mix C and C++
        std::cout << std::endl;
    }

    BSTR ledN;
    for (LONG i = devices.GetLowerBound(); i <= devices.GetUpperBound(); i++) {

        printf("Device %S detailed infos\n\n", devices.GetAt(i));

        // Now that we have device ids, let's get the LEDs too
        for (LONG k = 0; k < _wtoi(ledCount.GetAt(i)); k++) {
            int retLedVal = ml.getLedInfo(devices.GetAt(i), k, &ledN, &(ledStyles.m_psa));
            printf("LED #%d: %S\nSupported styles:\n", k, ledN);
            for (LONG j = ledStyles.GetLowerBound(); j <= ledStyles.GetUpperBound(); j++)
                printf("%S\n", ledStyles.GetAt(j));

            printf("---------------------------\n");
        }
    }

    // TEST TO SET THE COLOR AND STYLE TO ABSOLUTE
    int stRet = ml.setStyle(devices.GetAt(0), 3, ledStyles.GetAt(30));
    ml.errorMsg(stRet, &initMsg);
    printf("%S\n", initMsg);

    DWORD R, G, B;
    R = 110;
    G = 90;
    B = 30;

    // I guess MSI don't like documentation because how do I use this ????

    int rgbRet = ml.setColors(devices.GetAt(0), 3, &(ledName.m_psa), &R, &G, &B);
    
    //printf("%d\n", rgbRet);

}

int main()
{
    std::cout << "MSI Mystic Light LED Detect Tool" << std::endl;
    std::cout << "Made by CptKakise on a whim" << std::endl;
    std::cout << "Use with caution, it can brick your motherboard's light" << std::endl << std::endl;

    HINSTANCE dllHandle = NULL;

    dllHandle = LoadLibrary(TEXT("MysticLight_SDK_x64.dll"));
    
    if(dllHandle == NULL)
        ErrorExit(LPTSTR("LoadLibrary")); // Will print giberish, I hate Windows.h so much (the error will be aok tho)

    LoadMLSDK(&dllHandle);

    system("pause");
}