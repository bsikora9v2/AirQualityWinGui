#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "ApiClient.h"
#include <nlohmann/json.hpp>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Comctl32.lib")

using namespace Gdiplus;
using json = nlohmann::json;

HINSTANCE hInst;
HWND hStationBox, hSensorBox, hInfoEdit, hGraphButton;
json currentStations, currentSensors, currentMeasurements;

/// Inicjalizacja GDI+
ULONG_PTR gdiplusToken;
VOID InitGDIPlus() {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

/// Rysowanie wykresu
VOID DrawGraph(HDC hdc) {
    Graphics graphics(hdc);
    Pen pen(Color(255, 0, 0, 255), 2);

    if (!currentMeasurements.contains("values")) return;

    const auto& values = currentMeasurements["values"];
    int count = 0;
    std::vector<double> data;
    for (auto& item : values) {
        if (!item["value"].is_null()) {
            data.push_back(item["value"]);
            if (++count >= 50) break;
        }
    }
    if (data.empty()) return;

    double minVal = *std::min_element(data.begin(), data.end());
    double maxVal = *std::max_element(data.begin(), data.end());

    int width = 500;
    int height = 200;
    int margin = 20;
    for (size_t i = 1; i < data.size(); ++i) {
        float x1 = margin + (i - 1) * (width - 2 * margin) / (data.size() - 1);
        float x2 = margin + i * (width - 2 * margin) / (data.size() - 1);
        float y1 = height - margin - ((data[i - 1] - minVal) / (maxVal - minVal)) * (height - 2 * margin);
        float y2 = height - margin - ((data[i] - minVal) / (maxVal - minVal)) * (height - 2 * margin);
        graphics.DrawLine(&pen, x1, y1, x2, y2);
    }
}

/// Obsługa komunikatów
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        CreateWindow("STATIC", "Stacje:", WS_VISIBLE | WS_CHILD, 10, 10, 60, 20, hWnd, nullptr, hInst, nullptr);
        hStationBox = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD | WS_VSCROLL, 80, 10, 300, 200, hWnd, (HMENU)1, hInst, nullptr);

        CreateWindow("STATIC", "Sensory:", WS_VISIBLE | WS_CHILD, 10, 40, 60, 20, hWnd, nullptr, hInst, nullptr);
        hSensorBox = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD | WS_VSCROLL, 80, 40, 300, 200, hWnd, (HMENU)2, hInst, nullptr);

        CreateWindow("BUTTON", "Pobierz stacje", WS_VISIBLE | WS_CHILD, 400, 10, 120, 25, hWnd, (HMENU)3, hInst, nullptr);
        CreateWindow("BUTTON", "Pobierz sensory", WS_VISIBLE | WS_CHILD, 400, 40, 120, 25, hWnd, (HMENU)4, hInst, nullptr);
        CreateWindow("BUTTON", "Pobierz dane", WS_VISIBLE | WS_CHILD, 400, 70, 120, 25, hWnd, (HMENU)5, hInst, nullptr);
        hGraphButton = CreateWindow("BUTTON", "Pokaż wykres", WS_VISIBLE | WS_CHILD, 400, 100, 120, 25, hWnd, (HMENU)6, hInst, nullptr);

        hInfoEdit = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 10, 130, 580, 100, hWnd, nullptr, hInst, nullptr);
        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == 3) { // Pobierz stacje
            try {
                currentStations = ApiClient::fetchStations();
                SendMessage(hStationBox, CB_RESETCONTENT, 0, 0);
                for (const auto& s : currentStations) {
                    std::string label = s["stationName"];
                    SendMessageA(hStationBox, CB_ADDSTRING, 0, (LPARAM)label.c_str());
                }
            } catch (...) {
                MessageBox(hWnd, "Błąd pobierania stacji.", "Błąd", MB_OK | MB_ICONERROR);
            }
        }
        else if (LOWORD(wParam) == 4) { // Pobierz sensory
            int index = SendMessage(hStationBox, CB_GETCURSEL, 0, 0);
            if (index < 0) return;
            int stationId = currentStations[index]["id"];
            try {
                currentSensors = ApiClient::fetchSensors(stationId);
                SendMessage(hSensorBox, CB_RESETCONTENT, 0, 0);
                for (const auto& s : currentSensors) {
                    std::string param = s["param"]["paramName"];
                    SendMessageA(hSensorBox, CB_ADDSTRING, 0, (LPARAM)param.c_str());
                }
            } catch (...) {
                MessageBox(hWnd, "Błąd pobierania sensorów.", "Błąd", MB_OK | MB_ICONERROR);
            }
        }
        else if (LOWORD(wParam) == 5) { // Pobierz dane
            int index = SendMessage(hSensorBox, CB_GETCURSEL, 0, 0);
            if (index < 0) return;
            int sensorId = currentSensors[index]["id"];
            try {
                currentMeasurements = ApiClient::fetchMeasurements(sensorId);
                ApiClient::saveToFile("last_measurements.json", currentMeasurements);
                std::ostringstream oss;
                for (auto& v : currentMeasurements["values"]) {
                    if (v["value"].is_null()) continue;
                    oss << v["date"] << " = " << v["value"] << "\r\n";
                }
                SetWindowTextA(hInfoEdit, oss.str().c_str());
            } catch (...) {
                MessageBox(hWnd, "Błąd pobierania pomiarów.", "Błąd", MB_OK | MB_ICONERROR);
            }
        }
        else if (LOWORD(wParam) == 6) { // Wykres
            InvalidateRect(hWnd, nullptr, TRUE);
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DrawGraph(hdc);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        GdiplusShutdown(gdiplusToken);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

/// Funkcja główna WinAPI
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    hInst = hInstance;
    InitGDIPlus();

    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "AirQualityWinGui";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc);

    HWND hWnd = CreateWindow("AirQualityWinGui", "Monitor Jakości Powietrza", WS_OVERLAPPEDWINDOW,
        100, 100, 640, 400, nullptr, nullptr, hInstance, nullptr);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

