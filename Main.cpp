#include <Windows.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <tchar.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <thread>
#include "Header.h"

#define GRID_SIZE 10

using std::vector;

//Prototypes
LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK ReportWinProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
WNDCLASS NewWindowClass(HBRUSH bgColor, HCURSOR cursor, HINSTANCE hInst, HICON icon, LPCWSTR name, WNDPROC procedure);
void LoadDoublesData(LPCSTR path, vector<double>* data);
void PaintGraphicsArea(HWND hWnd);
void DrawGrid(HDC hDC, RECT rc);
void DrawAxises(HDC hDC, RECT rc);
vector<double> GetDistribution();
void DrawDistribution(HWND hWnd);
vector<double> GetDensity();
void DrawDensity(HWND hWnd);
BOOL WINAPI GetExpectedValue();
BOOL WINAPI GetDispersion();
BOOL WINAPI GetMedian();
BOOL WINAPI GetMode();
void CreateReportWin();
void FillEdit();
void SaveData(LPCSTR path);
//vars
bool canDrawGraphics;
bool canDrawCoordPlane;
HWND hWndR;
HINSTANCE hInstance;

//WinMain
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {

	WNDCLASS MainClass = NewWindowClass((HBRUSH)COLOR_WINDOW, LoadCursor(NULL, IDC_ARROW), hInst,
		LoadIcon(hInst, IDI_QUESTION), L"MainWndClass", SoftwareMainProcedure);

	if (!RegisterClassW(&MainClass)) { return -1; }
	MSG SoftwareMainMessage = { 0 };

	CreateWindow(L"MainWndClass", L"EasyStats", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 1200, 650, NULL, NULL, NULL, NULL);
	canDrawGraphics = false;
	canDrawCoordPlane = true;
	RegisterReportWinClass();
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
				density = GetDensity();
				canDrawGraphics = true;

				InvalidateRect(hWnd, 0, false);
				//redraw graphics

			}
			break;
		case OnReportButtonClicked: {
				std::thread getM(GetExpectedValue);
				std::thread getD(GetDispersion);
				std::thread getMedian(GetMedian);
				std::thread getMode(GetMode);
				getM.join();
				getD.join();
				getMedian.join();
				getMode.join();
				CreateReportWin();
			}
			break;
		default:
			break;
		}
		break;
	case WM_CREATE:
		MainAddWidgets(hWnd);
		SetOpenFileParams(hWnd);
		break;
	case WM_PAINT:
		
		if (canDrawGraphics) {
			//PaintGraphicsArea(hWnd);
			DrawDistribution(hWnd);
			InvalidateRect(hWnd, 0, false);
			DrawDensity(hWnd);
			//canDrawGraphics = false;
		}
		if (canDrawCoordPlane) {
			PaintGraphicsArea(hWnd);
			//canDrawCoordPlane = false;
		}
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
	CreateWindowA("button", "Открыть файл с данными", WS_VISIBLE | WS_CHILD | ES_CENTER, 5, 500, 200, 60, hWnd, (HMENU)OnButtonClicked, NULL, NULL);

	CreateWindowA("button", "Отчет о характеристиках", WS_VISIBLE | WS_CHILD | ES_CENTER, 300, 500, 200, 60, hWnd, (HMENU)OnReportButtonClicked, NULL, NULL);
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

	// Calculate the dimensions of the 2 equal rectangles.
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

	rc1.top += 100;
	rc2.top += 100;

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
	SetBkMode(hDC, TRANSPARENT);
	SetBkColor(hDC, RGB(255, 255, 255));   // white
	// TODO: Optionally, set a nicer font than the default.
	RECT rcDistributionText = rc1;
	rcDistributionText.top = rcWindow.top;
	rcDistributionText.bottom = rc1.top;
	RECT rcDensityText = rc2;
	rcDensityText.top = rcWindow.top;
	rcDensityText.bottom = rc2.top;
	DrawText(hDC, TEXT("Функция распределения:"), -1, &rcDistributionText, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	DrawText(hDC, TEXT("Функция плотности:"), -1, &rcDensityText, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	// Clean up after ourselves.
	SelectObject(hDC, hpenOld);
	SelectObject(hDC, hbrushOld);
	DeleteObject(borderPen);
	DeleteObject(brush);
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

int XToCoord(double x, RECT rc, double transformation) {
	//min, delta - globals
	//range, left - from form
	int range = (rc.right - rc.left) * (GRID_SIZE - 2) / GRID_SIZE;
	int left = rc.left + (rc.right - rc.left) / GRID_SIZE;
	int offset = range * transformation;
	return left + (range - offset) * (x - min) / delta + offset;
	//return 0;
}

int YToCoord(double y, RECT rc, int magnification) {
	int result = rc.bottom - 2 * (rc.bottom - rc.top) / GRID_SIZE;
	result -= y * 5 * (rc.bottom - rc.top) * magnification / GRID_SIZE;
	return result;
}

void DrawDistribution(HWND hWnd) {
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);
	HPEN hpenOld = static_cast<HPEN>(SelectObject(hDC, GetStockObject(DC_PEN)));
	HBRUSH hbrushOld = static_cast<HBRUSH>(SelectObject(hDC, GetStockObject(NULL_BRUSH)));
	HPEN graphPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	SelectObject(hDC, graphPen);

	double extremeValue = max > abs(min) ? max : abs(min);
	double xRange = 2 * extremeValue;
	double transformation = 1 - delta / xRange;

	//Draw from -inf
	MoveToEx(hDC, rc1.left, YToCoord(0, rc1, 1), NULL);
	LineTo(hDC, XToCoord(min, rc1, transformation), YToCoord(0, rc1, 1));
	//Draw middle
	for (int i = 1; i < distribution.size(); i++) {
		LineTo(hDC, XToCoord(min + h * i, rc1, transformation), YToCoord(distribution[i - 1], rc1, 1));
		LineTo(hDC, XToCoord(min + h * i, rc1, transformation), YToCoord(distribution[i], rc1, 1));
	}
	//Draw to +inf
	LineTo(hDC, rc1.right, YToCoord(1, rc1, 1));

	SelectObject(hDC, hpenOld);
	SelectObject(hDC, hbrushOld);
	DeleteObject(graphPen);
	EndPaint(hWnd, &ps);
}

vector<double> GetDensity() {
	int size = data.size();
	vector<double>* result = new vector<double>(0);
	min = data[0];
	max = data[data.size() - 1];
	delta = max - min;
	h = delta / data.size();
	argument = min + h;
	for (int i = 1; i < size && argument <= max; argument += h)
	{
		int j = 0;
		while (data[i] < argument) {
			i++;
			j++;
		}
		double value = (double)j / size;
		result->push_back(value);
	}
	return *result;
}


//TODO: calculate magnification
void DrawDensity(HWND hWnd) {
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);
	HPEN hpenOld = static_cast<HPEN>(SelectObject(hDC, GetStockObject(DC_PEN)));
	HBRUSH hbrushOld = static_cast<HBRUSH>(SelectObject(hDC, GetStockObject(NULL_BRUSH)));
	HPEN graphPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	SelectObject(hDC, graphPen);

	double extremeValue = max > abs(min) ? max : abs(min);
	double xRange = 2 * extremeValue;
	double transformation = 1 - delta / xRange;

	//Draw from -inf
	MoveToEx(hDC, rc2.left, YToCoord(0, rc2, 20), NULL);
	LineTo(hDC, XToCoord(min, rc2, transformation), YToCoord(0, rc2, 20));
	//Draw middle
	for (int i = 1; i < density.size(); i++) {
		LineTo(hDC, XToCoord(min + h * i, rc2, transformation), YToCoord(density[i - 1], rc2, 20));
		LineTo(hDC, XToCoord(min + h * i, rc2, transformation), YToCoord(density[i], rc2, 20));
	}
	//Draw to +inf
	LineTo(hDC, XToCoord(min + h * density.size(), rc2, transformation), YToCoord(0, rc2, 20));
	LineTo(hDC, rc2.right, YToCoord(0, rc2, 20));

	SelectObject(hDC, hpenOld);
	SelectObject(hDC, hbrushOld);
	DeleteObject(graphPen);
	EndPaint(hWnd, &ps);
}


//Calculating characteristics
BOOL WINAPI GetExpectedValue() {
	mutex.lock();
	int count = data.size();
	double total = 0;
	for (int i = 0; i < count; i++) {
		total += data[i];
	}
	mutex.unlock();
	M = total / count;
	return true;
}

BOOL WINAPI GetDispersion() {
	mutex.lock();
	int count = data.size();
	double total1 = 0;
	double total2 = 0;
	for (int i = 0; i < count; i++) {
		total1 += data[i] * data[i];
		total2 += data[i];
	}
	mutex.unlock();
	D = total1 / count - (total2 * total2 / (count * count));
	standartDeviation = sqrt(D);
	return true;
}

BOOL WINAPI GetMedian() {
	mutex.lock();
	int count = data.size();
	double diff1 = 0.5;
	double diff2 = 0.5;
	for (int i = 1; i < count; i++) {
		diff1 = abs(0.5 - distribution[i]);
		if (diff1 > diff2) {
			median = data[i - 1];
			break;
		}
		diff2 = diff1;
	}
	mutex.unlock();
	return true;
}

BOOL WINAPI GetMode() {
	mutex.lock();
	int count = data.size();
	double maxValue = 0;
	int ind = 0;
	for (int i = 1; i < count; i++) {
		if (density[i - 1] > maxValue) {
			maxValue = density[i - 1];
			ind = i;
		}
	}
	mutex.unlock();
	mode = data[ind];
	return true;
}

void CreateReportWin() {
	hWndR = CreateWindowEx(0, L"ReportWindowClass", L"Отчет о характеристиках", WS_OVERLAPPEDWINDOW | WS_BORDER | WS_POPUP | WS_VISIBLE | WS_CHILD,
		200, 300, 500, 500, NULL, NULL, hInstance, NULL);

	EnableWindow(hWndR, TRUE);
	ShowWindow(hWndR, SW_SHOWDEFAULT);
	UpdateWindow(hWndR);
}

void RegisterReportWinClass() { //окно шифрования/дешифрования

	WNDCLASSEX wcexed;
	memset(&wcexed, 0, sizeof(wcexed));
	wcexed.cbSize = sizeof(WNDCLASSEX);
	wcexed.style = CS_HREDRAW | CS_VREDRAW;
	wcexed.lpfnWndProc = ReportWinProc;
	//wcexed.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KURSACH1));
	wcexed.hInstance = hInstance;
	//wcexed.hbrBackground = CreatePatternBrush(hBitmapED);
	wcexed.lpszClassName = L"ReportWindowClass";
	RegisterClassEx(&wcexed);
}

LRESULT CALLBACK ReportWinProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_COMMAND:
		switch (wp)
		{
		case OnSaveReportButtonClicked:
			if (GetSaveFileNameA(&ofn))
				SaveData(filename);
			break;
		default:
			break;
		}
		break;
	case WM_CREATE:
		//Edit
		hEditControl = CreateWindowA("edit", "", WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL, 5, 30, 490, 200, hWnd, NULL, NULL, NULL);
		//Buttton
		CreateWindowA("button", "Сохранить отчет", WS_VISIBLE | WS_CHILD | ES_CENTER, 5, 300, 200, 60, hWnd, (HMENU)OnSaveReportButtonClicked, NULL, NULL);

		FillEdit();
		break;
	case WM_PAINT:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default: return DefWindowProc(hWnd, msg, wp, lp);
	}
}

void FillEdit() {
	std::string str;
	str.append("Матожидание:\r\n");
	std::string MStr = std::to_string(M);
	str.append(MStr);
	str.append("\r\n\r\nДисперсия:\r\n");
	std::string DStr = std::to_string(D);
	str.append(DStr);
	str.append("\r\n\r\nСреднеквадратическое отклонение:\r\n");
	std::string SDStr = std::to_string(standartDeviation);
	str.append(SDStr);
	str.append("\r\n\r\nМедиана:\r\n");
	std::string MedianStr = std::to_string(median);
	str.append(MedianStr);
	str.append("\r\n\r\nМода:\r\n");
	std::string ModeStr = std::to_string(mode);
	str.append(ModeStr);
	char output[300] = { 0 };
	for (int i = 0; i <= str.length(); i++)	//Use <= to add '\0'
		output[i] = str[i];
	SetWindowTextA(hEditControl, output);
}

void SaveData(LPCSTR path) {
	HANDLE FileToSave = CreateFileA(
		path,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	int saveLenth = GetWindowTextLength(hEditControl) + 1;
	char* data = new char[saveLenth];

	saveLenth = GetWindowTextA(hEditControl, data, saveLenth);

	DWORD bytesIterated;
	WriteFile(FileToSave, data, saveLenth, &bytesIterated, NULL);

	CloseHandle(FileToSave);
	delete[] data;
}