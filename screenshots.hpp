/*
    Author: @TheFlash2k
    Github: https://github.com/theflash2k/
*/

#ifndef SCREENSHOT_HPP
#define SCREENSHOT_HPP

#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>

namespace Screenshot {

    // I like naming my private namespaces with __internal to hide the functions that the user might not need.
    // More like abstraction but, not really. If only private namespaces were a thing.
    namespace __internal {
        typedef struct {
            int sWidth;
            int sHeight;
            HDC hDC;
            HDC hMemoryDC;
            HBITMAP hBitmap;
            HBITMAP hOldBitmap;
            BITMAPINFOHEADER bi;
            BITMAPFILEHEADER bf;
            DWORD dwBmpSize;
            BYTE* bmpData;
        }SCREENSHOT, * PSCREENSHOT;

        // This method initializes the necessary components for taking screenshots
        void init_screenshot(PSCREENSHOT ss) {
            ss->sWidth = GetSystemMetrics(SM_CXSCREEN);
            ss->sHeight = GetSystemMetrics(SM_CYSCREEN);
            ss->hDC = GetDC(NULL);
            ss->hMemoryDC = CreateCompatibleDC(ss->hDC);
            ss->hBitmap = CreateCompatibleBitmap(ss->hDC, ss->sWidth, ss->sHeight);
            ss->hOldBitmap = (HBITMAP)SelectObject(ss->hMemoryDC, ss->hBitmap);
            BitBlt(ss->hMemoryDC, 0, 0, ss->sWidth, ss->sHeight, ss->hDC, 0, 0, SRCCOPY);
            ss->bi.biSize = sizeof(BITMAPINFOHEADER);
            ss->bi.biWidth = ss->sWidth;
            ss->bi.biHeight = -ss->sHeight;
            ss->bi.biPlanes = 1;
            ss->bi.biBitCount = 24;
            ss->bi.biCompression = BI_RGB;
            ss->bi.biSizeImage = 0;
            ss->dwBmpSize = ((ss->sWidth * ss->bi.biBitCount + 31) / 32) * 4 * ss->sHeight;
            ss->bmpData = new BYTE[ss->dwBmpSize];
            GetDIBits(ss->hMemoryDC, ss->hBitmap, 0, ss->sHeight, ss->bmpData, (BITMAPINFO*)&ss->bi, DIB_RGB_COLORS);
            ss->bf.bfType = 0x4D42;
            ss->bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + ss->dwBmpSize;
            ss->bf.bfReserved1 = 0;
            ss->bf.bfReserved2 = 0;
            ss->bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        }

        void delete_screenshot(PSCREENSHOT ss) {
            delete[] ss->bmpData;
            SelectObject(ss->hMemoryDC, ss->hOldBitmap);
            DeleteObject(ss->hBitmap);
            DeleteDC(ss->hMemoryDC);
            ReleaseDC(NULL, ss->hDC);
        }

    }

    std::vector<BYTE> GetCurrentState() {
        using namespace __internal;
        SCREENSHOT ss;
        init_screenshot(&ss);
        BYTE* bfData = reinterpret_cast<BYTE*>(&ss.bf);
        BYTE* biData = reinterpret_cast<BYTE*>(&ss.bi);
        std::vector<BYTE> byteVector;
        byteVector.reserve(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + ss.dwBmpSize);
        byteVector.insert(byteVector.end(), bfData, bfData + sizeof(BITMAPFILEHEADER));
        byteVector.insert(byteVector.end(), biData, biData + sizeof(BITMAPINFOHEADER));
        byteVector.insert(byteVector.end(), ss.bmpData, ss.bmpData + ss.dwBmpSize);
        delete_screenshot(&ss);
        return byteVector;
    }

    // Another method to store the screenshot vector to a file.
    bool ToFile(std::vector<BYTE> ssVec, std::string fileName = "output.bmp") {
		using namespace __internal;
        if (fileName.substr(fileName.size() - 4, 4) != ".bmp") {
            fileName += ".bmp";
        }
        std::ofstream file(fileName.c_str(), std::ios::out | std::ios::binary);
        if (file) {
			file.write(reinterpret_cast<const char*>(&ssVec[0]), ssVec.size());
			file.close();
            return true;
		}
        return false;
    }

    // Capturing a screenshot, and storing the output to a file.
    bool ToFile(std::string fileName="output.bmp") {
        using namespace __internal;
        if (fileName.substr(fileName.size() - 4, 4) != ".bmp") {
			fileName += ".bmp";
		}
        SCREENSHOT ss;
        init_screenshot(&ss);
        std::ofstream file(fileName.c_str(), std::ios::out | std::ios::binary);
        if (file) {
            file.write(reinterpret_cast<const char*>(&ss.bf), sizeof(BITMAPFILEHEADER));
            file.write(reinterpret_cast<const char*>(&ss.bi), sizeof(BITMAPINFOHEADER));
            file.write(reinterpret_cast<const char*>(ss.bmpData), ss.dwBmpSize);
            file.close();
            return true;
        }
        delete_screenshot(&ss);
        return false;
    }
}
#endif // !SCREENSHOT_HPP

/*
Usage:

int main(int argc, char* argv[]) {
    // Storing to a file
    Screenshot::ToFile("test.bmp");

    // Storing to a vector
    std::vector<BYTE> ssVec = Screenshot::GetCurrentState();

    // Storing the vector to a file
    Screenshot::ToFile(ssVec, "test_vec.bmp");

    return 0;
}
*/
