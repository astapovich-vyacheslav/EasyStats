#include <Windows.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <tchar.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "Header.h"

#define GRID_SIZE 10

using std::vector;

//Prototypes
LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
WNDCLASS NewWindowClass(HBRUSH bgColor, HCURSOR cursor, HINSTANCE hInst, HICON icon, LPCWSTR name, WNDPROC procedure);
void LoadDoublesData(LPCSTR path, vector<double>* data);
void PaintGraphicsArea(HWND hWnd);
void DrawGrid(HDC hDC, RECT rc);
void DrawAxises(HDC hDC, RECT rc);
vector<double> GetDistribution();
void DrawDistribution(HWND hWnd);
//vars
bool canDrawGraphics;

//WinMain
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {

	WNDCLASS MainClass = NewWindowClass((HBRUSH)COLOR_WINDOW, LoadCursor(NULL, IDC_ARROW), hInst,
		LoadIcon(hInst, IDI_QUESTION), L"MainWndClass", SoftwareMainProcedure);

	if (!RegisterClassW(&MainClass)) { return -1; }
	MSG SoftwareMainMessage = { 0 };

	CreateWindow(L"MainWndClass", L"EasyStats", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 1200, 650, NULL, NULL, NULL, NULL);

	while (GetMessage(&SoftwareMainMessage, NULL, NULL, NULL)) {
		TranslateMessage(&SoftwareMainMessage);
		DispatchMessage(&SoftwareMainMessage);
	}
	return 0;
}


WNDCLASS NewWindowClass(HBRUSH bgColor, HCURSOR cursor, HINSTANCE hInst, HICON icon, LPCWSTR name, WNDPROC procedure) {

	WNDCLASS NWC = { 0 };

	NWC.hCursor = cursor;
	NWC.hIcon = icon;
	NWC.hInstance = hInst;
	NWC.lpszClassName = name;
	NWC.hbrBackground = bgColor;
	NWC.lpfnWndProc = procedure;

	return NWC;
}

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_COMMAND:
		switch (wp)
		{
		case OnButtonClicked:
			//MessageBoxA(hWnd, "Your message here!", "Button clicked!", MB_OK);
			if (GetOpenFileNameA(&ofn))
			{
				//open file
				try {
					LoadDoublesData(filename, &data);
				}
				catch (const char* e) {
					MessageBoxA(hWnd, "Не удалось открыть файл", "Ошибка", MB_OK);
					return DefWindowProc(hWnd, msg, wp, lp);
				}
				//do calculations for distribution and density
				std::sort(data.begin(), data.end());
				distribution = GetDistribution();
				canDrawGraphics = true;

				//redraw graphics

			}
			break;
		default:
			break;
		}
		break;
	case WM_CREATE:
		MainAddWidgets(hWnd);
		SetOpenFileParams(hWnd);
		canDrawGraphics = false;
		break;
	case WM_PAINT:
		PaintGraphicsArea(hWnd);
		if (canDrawGraphics)
			DrawDistribution(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default: return DefWindowProc(hWnd, msg, wp, lp);
	}
}

void MainAddWidgets(HWND hWnd) {
	//Label
	//CreateWindowA("static", "This is LABEL!", WS_VISIBLE | WS_CHILD, 5, 5, 490, 20, hWnd, NULL, NULL, NULL);

	//Edit
	//hEditControl = CreateWindowA("edit", "This is EDIT!", WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL, 5, 30, 490, 200, hWnd, NULL, NULL, NULL);

	//Edit
	CreateWindowA("button", "Открыть файл с данными", WS_VISIBLE | WS_CHILD | ES_CENTER, 5, 500, 200, 60, hWnd, (HMENU)OnButtonClicked, NULL, NULL);
}


void SetOpenFileParams(HWND hWnd) {
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = "*.txt"; // TODO: to be able to read .csv
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}

void LoadDoublesData(LPCSTR path, vector<double>* data) {
	HANDLE FileToLoad = CreateFileA(
		path,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	DWORD bytesIterated;
	ReadFile(FileToLoad, buffer, TextBufferSize, &bytesIterated, NULL);

	for (int i = 0; i <= bytesIterated; i++) {
		int j = i;
		char numInStr[100] = { 0 };
		for (; i <= bytesIterated && buffer[i] != '\r'; i++) {
			numInStr[i - j] = buffer[i];
		}
		numInStr[i - j] = '\0';
		i++;
		if (numInStr[0] != '\0')
		{
			double num = std::stod(numInStr);
			data->push_back(num);
		}
	}

	CloseHandle(FileToLoad);
}

void DrawGrid(HDC hDC, RECT rc) {
	//verticals
	for (int i = 0; i < GRID_SIZE; i++) {
		MoveToEx(hDC, rc.right + (rc.left - rc.right) * i / GRID_SIZE, rc.top + 5, NULL);
		LineTo(hDC, rc.right + (rc.left - rc.right) * i / GRID_SIZE, rc.bottom - 5);
	}
	//horizontals
	for (int i = 1; i < GRID_SIZE; i++) {
		MoveToEx(hDC, rc.left + 5, rc.top + (rc.bottom - rc.top) * i / GRID_SIZE, NULL);
		LineTo(hDC, rc.right - 5, rc.top + (rc.bottom - rc.top) * i / GRID_SIZE);
	}
}

void DrawAxises(HDC hDC, RECT rc) {
	//vertical
	MoveToEx(hDC, (rc.right + rc.left) / 2, rc.top, NULL);
	LineTo(hDC, (rc.right + rc.left) / 2, rc.bottom);
	//horizontal
	MoveToEx(hDC, rc.left, rc.top + 8 * (rc.bottom - rc.top) / GRID_SIZE, NULL);
	LineTo(hDC, rc.right, rc.top + 8 * (rc.bottom - rc.top) / GRID_SIZE);
}

void PaintGraphicsArea(HWND hWnd)
{
	// Set up the device context for drawing.
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);
	HPEN hpenOld = static_cast<HPEN>(SelectObject(hDC, GetStockObject(DC_PEN)));
	HBRUSH hbrushOld = static_cast<HBRUSH>(SelectObject(hDC, GetStockObject(NULL_BRUSH)));
	HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
	HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));

	// Calculate the dimensions of the 4 equal rectangles.
	RECT rcWindow;
	GetClientRect(hWnd, &rcWindow);

	rc1 = rc2 = rcWindow;

	rc1.right -= (rcWindow.right - rcWindow.left) / 2;
	rc1.bottom -= (rcWindow.bottom - rcWindow.top) / 2;

	rc2.left = rc1.right;
	rc2.bottom = rc1.bottom;

	// Optionally, deflate each of the rectangles by an arbitrary amount so that
	// they don't butt up right next to each other and we can distinguish them.
	InflateRect(&rc1, -5, -5);
	InflateRect(&rc2, -5, -5);

	// Draw (differently-colored) borders around these rectangles.
	SetDCPenColor(hDC, RGB(0, 0, 0));    // black
	FillRect(hDC, &rc1, brush);	//white bg
	Rectangle(hDC, rc1.left, rc1.top, rc1.right, rc1.bottom);
	FillRect(hDC, &rc2, brush);	//white bg
	Rectangle(hDC, rc2.left, rc2.top, rc2.right, rc2.bottom);

	//Draw grid
	SelectObject(hDC, gridPen);
	DrawGrid(hDC, rc1);
	DrawGrid(hDC, rc2);

	//Draw the axises
	SelectObject(hDC, borderPen);
	DrawAxises(hDC, rc1);
	DrawAxises(hDC, rc2);

	
	//// Draw the text into the center of each of the rectangles.
	//SetBkMode(hDC, TRANSPARENT);
	//SetBkColor(hDC, RGB(255, 255, 255));   // white
	//// TODO: Optionally, set a nicer font than the default.
	//DrawText(hDC, TEXT("Hello World!"), -1, &rc1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	//DrawText(hDC, TEXT("Hello World!"), -1, &rc2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	// Clean up after ourselves.
	DeleteObject(borderPen);
	DeleteObject(brush);
	SelectObject(hDC, hpenOld);
	SelectObject(hDC, hbrushOld);
	EndPaint(hWnd, &ps);
}

vector<double> GetDistribution() {
	int size = data.size();
	vector<double>* result = new vector<double>(0);
	min = data[0];
	max = data[data.size() - 1];
	delta = max - min;
	h = delta / data.size();
	argument = min;
	for (int i = 0; i < size && argument <= max; argument += h)
	{
		while (data[i] < argument)
			i++;
		double value = (double)i / size;
		result->push_back(value);
	}
	return *result;
}

int XToCoord(double x, RECT rc) {
	//min, delta - globals
	//range, left - from form
	int range = (rc.right - rc.left) * (GRID_SIZE - 2) / GRID_SIZE;
	int left = rc.left + (rc.right - rc.left) / GRID_SIZE;
	return left + range * (x - min) / delta;
	//return 0;
}

int YToCoord(double y, RECT rc) {
	int result = rc.bottom - 2 * (rc.bottom - rc.top) / GRID_SIZE;
	result += y * 5 * (rc.bottom - rc.top) / GRID_SIZE;
	return result;
}

void DrawDistribution(HWND hWnd) {
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);
	HPEN hpenOld = static_cast<HPEN>(SelectObject(hDC, GetStockObject(DC_PEN)));
	HBRUSH hbrushOld = static_cast<HBRUSH>(SelectObject(hDC, GetStockObject(NULL_BRUSH)));
	HPEN graphPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

	double extremeValue = max > abs(min) ? max : abs(min);
	double range = 2 * extremeValue;
	double growthPoint = max - abs(min);
	MoveToEx(hDC, XToCoord(min, rc1), YToCoord(0, rc1), NULL);
	MoveToEx(hDC, 0, 0, NULL);
	LineTo(hDC, 500, 500);
	for (int i = 0; i < distribution.size(); i++) {
		LineTo(hDC, XToCoord(min + h * i, rc1), YToCoord(distribution[i], rc1));
	}
}