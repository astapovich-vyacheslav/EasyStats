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



using std::vector;

//Prototypes
LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK ReportWinProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
WNDCLASS NewWindowClass(HBRUSH bgColor, HCURSOR cursor, HINSTANCE hInst, HICON icon, LPCWSTR name, WNDPROC procedure);
void InitializeRects(HWND hWnd);
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
BOOL WINAPI GetExcess();
BOOL WINAPI GetAssymetry();
BOOL WINAPI GetVarCoeff();
void RegisterReportWinClass();
void CreateReportWin(HWND hWnd);
void FillEdit();
void SaveData(LPCSTR path);
char* StdStrToCharArr(std::string str);
void DrawNumsOnPlane(HWND hWnd, RECT rc, double topValue);
int XToCoord(double x, RECT rc, double transformation);
int YToCoord(double y, RECT rc, int magnification);
//vars
bool canDrawGraphics;
bool canDrawCoordPlane;
bool dataIsClear;
HWND hWndR;
HINSTANCE hInstance;

//WinMain
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {

	WNDCLASS MainClass = NewWindowClass((HBRUSH)COLOR_WINDOW, LoadCursor(NULL, IDC_ARROW), hInst,
		LoadIcon(hInst, IDI_QUESTION), L"MainWndClass", SoftwareMainProcedure);

	if (!RegisterClassW(&MainClass)) { return -1; }
	MSG SoftwareMainMessage = { 0 };

	CreateWindow(L"MainWndClass", L"EasyStats", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 50, 50, 1400, 700, NULL, NULL, NULL, NULL);
	canDrawGraphics = false;
	canDrawCoordPlane = true;
	dataIsClear = true;

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
				//clear data
				if (!dataIsClear) {
					data.clear();
					distribution.clear();
					density.clear();
					M = 0;
					D = 0;
					standartDeviation = 0;
					median = 0;
					medianLeft = 0;
					medianRight = 0;
					mode = 0;
					modeLeft = 0;
					modeRight = 0;
					assymetry = 0;
					excess = 0;
					variationCoeff = 0;
				}

				//open file
				try {
					LoadDoublesData(filename, &data);
				}
				catch (...) {
					MessageBoxA(hWnd, "�� ������� ������� ����", "������", MB_OK);
					data.clear();
					canDrawGraphics = false;
					dataIsClear = true;
					InvalidateRect(hWnd, 0, true);
					return DefWindowProc(hWnd, msg, wp, lp);
				}
				//do calculations for distribution and density
				std::sort(data.begin(), data.end());
				distribution = GetDistribution();
				if (distribution.size() == 0) {
					MessageBoxA(hWnd, "� ������� ������ ���� ��������", "������", MB_OK);
					data.clear();
					distribution.clear();
					return DefWindowProc(hWnd, msg, wp, lp);
				}
				density = GetDensity();
				canDrawGraphics = true;
				dataIsClear = false;
				//redraw graphics
				InvalidateRect(hWnd, 0, false);
				//data is not clear
				
			}
			break;
		case OnReportButtonClicked: {
			if (dataIsClear) {
				MessageBoxA(hWnd, "�� ������� ��������� ������", "������", MB_OK);
				return DefWindowProc(hWnd, msg, wp, lp);
			}
				std::thread getM(GetExpectedValue);
				std::thread getD(GetDispersion);
				std::thread getMedian(GetMedian);
				std::thread getMode(GetMode);
				getM.join();
				getD.join();
				getMode.join();
				std::thread getAssymetry(GetAssymetry);
				std::thread getExcess(GetExcess);
				std::thread getVarCoeff(GetVarCoeff);
				getMedian.join();
				getAssymetry.join();
				getExcess.join();
				getVarCoeff.join();
				CreateReportWin(hWnd);
			}
			break;
		default:
			break;
		}
		break;
	case WM_CREATE:
		InitializeRects(hWnd);
		MainAddWidgets(hWnd);
		SetOpenFileParams(hWnd);
		break;
	case WM_PAINT:
		InitializeRects(hWnd);
		DestroyWindow(hOpenButton);
		DestroyWindow(hReportButton);
		MainAddWidgets(hWnd);
		if (canDrawGraphics) {
			PaintGraphicsArea(hWnd);
			InvalidateRect(hWnd, 0, false);
			DrawDistribution(hWnd);
			InvalidateRect(hWnd, 0, false);
			DrawDensity(hWnd);
			InvalidateRect(hWnd, 0, false);
			DrawNumsOnPlane(hWnd, rc1, 1);
			InvalidateRect(hWnd, 0, false);
			DrawNumsOnPlane(hWnd, rc2, maxDensityValue);
			//canDrawGraphics = false;
		}
		if (canDrawCoordPlane) {
			PaintGraphicsArea(hWnd);
			//canDrawCoordPlane = false;
		}
		break;
	case WM_SIZE:
		InvalidateRect(hWnd, 0, true);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default: return DefWindowProc(hWnd, msg, wp, lp);
	}
}

void MainAddWidgets(HWND hWnd) {
	hOpenButton = CreateWindowA("button", "������� ���� � �������",
		WS_VISIBLE | WS_CHILD | ES_CENTER,
		rc1.left, rc1.bottom + 20, rc1.right - rc1.left, rcWindow.bottom - rc1.bottom - 40,
		hWnd, (HMENU)OnButtonClicked, NULL, NULL);
	hReportButton = CreateWindowA("button", "����� � ���������������",
		WS_VISIBLE | WS_CHILD | ES_CENTER,
		rc2.left, rc2.bottom + 20, rc2.right - rc2.left, rcWindow.bottom - rc2.bottom - 40,
		hWnd, (HMENU)OnReportButtonClicked, NULL, NULL);
}

void InitializeRects(HWND hWnd) {
	GetClientRect(hWnd, &rcWindow);

	rc1 = rc2 = rcWindow;

	rc1.right -= (rcWindow.right - rcWindow.left) / 2;
	rc1.bottom = 2 * rcWindow.bottom / 3;

	rc2.left = rc1.right;
	rc2.bottom = rc1.bottom;
	// Optionally, deflate each of the rectangles by an arbitrary amount so that
	// they don't butt up right next to each other and we can distinguish them.
	InflateRect(&rc1, -5, -5);
	InflateRect(&rc2, -5, -5);


	rc1.top += 100;
	rc2.top += 100;
}

void SetOpenFileParams(HWND hWnd) {
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = "*.txt";
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

	for (int i = 0; i < bytesIterated; i++) {
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
	//if (data->size() <= 1) {
	//	throw "������ �������";
	//}
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
	DrawText(hDC, TEXT("������� �������������:"), -1, &rcDistributionText, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	DrawText(hDC, TEXT("������� ���������:"), -1, &rcDensityText, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	// Clean up after ourselves.
	SelectObject(hDC, hpenOld);
	SelectObject(hDC, hbrushOld);
	DeleteObject(borderPen);
	DeleteObject(brush);
	EndPaint(hWnd, &ps);
}

void DrawNumsOnPlane(HWND hWnd, RECT rc, double topValue) {
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);
	HPEN hpenOld = static_cast<HPEN>(SelectObject(hDC, GetStockObject(DC_PEN)));
	HBRUSH hbrushOld = static_cast<HBRUSH>(SelectObject(hDC, GetStockObject(NULL_BRUSH)));
	HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
	HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	SetBkMode(hDC, TRANSPARENT);

	int hDelta = rc.right - rc.left;
	int vDelta = rc.bottom - rc.top;
	std::string tempStr;
	char tempCharArr[500];
	//use GRID_SIZE
	//Drawing top value
	RECT rcTemp = rc;
	rcTemp.left += 4 * hDelta / GRID_SIZE;
	rcTemp.top += 3 * vDelta / GRID_SIZE;
	rcTemp.right -= 5 * hDelta / GRID_SIZE;
	rcTemp.bottom -= 6 * vDelta / GRID_SIZE;
	tempStr = std::to_string(topValue);
	for (int i = 0; i <= tempStr.length(); i++) // Use <= to copy \0
		tempCharArr[i] = tempStr[i];
	DrawTextA(hDC, tempCharArr, lstrlenA(tempCharArr), &rcTemp, DT_CENTER | DT_EDITCONTROL | DT_NOCLIP);

	//Drawing max value
	rcTemp = rc;
	rcTemp.left += 17 * hDelta / (2 * GRID_SIZE);
	rcTemp.top += 8 * vDelta / GRID_SIZE;
	rcTemp.right -= 1 * hDelta / (2 * GRID_SIZE);
	rcTemp.bottom -= 1 * vDelta / GRID_SIZE;
	tempStr = std::to_string(max);
	for (int i = 0; i <= tempStr.length(); i++) // Use <= to copy \0
		tempCharArr[i] = tempStr[i];
	DrawTextA(hDC, tempCharArr, lstrlenA(tempCharArr), &rcTemp, DT_CENTER | DT_EDITCONTROL | DT_NOCLIP);

	//Drawing min value
	double extremeValue = max > abs(min) ? max : abs(min);
	double xRange = 2 * extremeValue;
	double transformation = 1 - delta / xRange;

	rcTemp = rc;
	rcTemp.left = XToCoord(min, rc, transformation) - hDelta / (2 * GRID_SIZE);
	rcTemp.top += 8 * vDelta / GRID_SIZE;
	rcTemp.right = rcTemp.left + hDelta / (2 * GRID_SIZE);
	rcTemp.bottom -= 1 * vDelta / GRID_SIZE;

	tempStr = std::to_string(min);
	for (int i = 0; i <= tempStr.length(); i++) // Use <= to copy \0
		tempCharArr[i] = tempStr[i];
	DrawTextA(hDC, tempCharArr, lstrlenA(tempCharArr), &rcTemp, DT_CENTER | DT_EDITCONTROL | DT_NOCLIP);



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
	/*if (delta > 10 * data.size())
		h = 1 / h;*/
	argument = min;
	/*if (delta == 0) {
		result->push_back(1);
		return *result;
	}*/
	for (int i = 0; i < size && argument < max; argument += h)
	{
		while (i < size && (data[i] < argument || h == 0))
			i++;
		double value = (double)i / size;
		result->push_back(value);
	}
	//result->push_back(1);
	
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
	//LineTo(hDC, XToCoord(min, rc1, transformation), YToCoord(distribution[0], rc1, 1));
	//Draw middle
	for (int i = 0; i < distribution.size() - 1; i++) {
		LineTo(hDC, XToCoord(min + h * i, rc1, transformation), YToCoord(distribution[i + 1], rc1, 1));
		LineTo(hDC, XToCoord(min + h * (i + 1), rc1, transformation), YToCoord(distribution[i + 1], rc1, 1));
	}
	//Draw to +inf
	LineTo(hDC, XToCoord(max, rc1, transformation), YToCoord(distribution[distribution.size() - 1], rc1, 1));
	LineTo(hDC, XToCoord(max, rc1, transformation), YToCoord(1, rc1, 1));
	LineTo(hDC, rc1.right, YToCoord(1, rc1, 1));

	/*RECT numsRect = rc1;
	numsRect.left += (rc1.bottom - rc1.top) / GRID_SIZE;
	numsRect.right -= (rc1.bottom - rc1.top) / GRID_SIZE;
	std::string leftStr = std::to_string(-extremeValue);
	char temp[50] = { 0 };
	for (int i = 0; i++ < leftStr.length(); i++)
		temp[i] = leftStr[i];
	DrawText(hDC, TEXT(temp), -1, &numsRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);*/

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
	/*if (delta > 10 * data.size())
		h = 1 / h;*/
	argument = min + h;
	/*if (delta == 0) {
		result->push_back(1);
		return *result;
	}*/
	maxDensityValue = 0;
	for (int i = 0; i < size && argument <= max + 0.0001; argument += h)
	{
		int j = 0;
		while (i < size && (data[i] <= argument + 0.0001 || h == 0)) {
			i++;
			j++;
		}
		double value = (double)j / size;
		if (value > maxDensityValue)
			maxDensityValue = value;
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

	double max = 0;
	for (int i = 0; i < density.size(); i++) {
		if (density[i] > max)
			max = density[i];
	}
	int magnification = 1;
	if (max != 0)
		magnification = 1 / max;



	//Draw from -inf
	MoveToEx(hDC, rc2.left, YToCoord(0, rc2, magnification), NULL);
	LineTo(hDC, XToCoord(min, rc2, transformation), YToCoord(0, rc2, magnification));
	LineTo(hDC, XToCoord(min, rc2, transformation), YToCoord(density[0], rc2, magnification));
	//Draw middle
	for (int i = 0; i < density.size() - 1; i++) {
		LineTo(hDC, XToCoord(min + h * (i + 1), rc2, transformation), YToCoord(density[i], rc2, magnification));
		LineTo(hDC, XToCoord(min + h * (i + 1), rc2, transformation), YToCoord(density[i + 1], rc2, magnification));
	}
	//Draw to +inf
	LineTo(hDC, XToCoord(min + h * density.size(), rc2, transformation), YToCoord(density[density.size() - 1], rc2, magnification));
	LineTo(hDC, XToCoord(min + h * density.size(), rc2, transformation), YToCoord(0, rc2, magnification));
	LineTo(hDC, rc2.right, YToCoord(0, rc2, magnification));

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
	int count = distribution.size();
	double minDiff = 0.5;
	int minDiffInd = 0;
	for (int i = 0; i < count; i++) {
		if (abs(0.5 - distribution[i]) < minDiff) {
			minDiff = abs(0.5 - distribution[i]);
			minDiffInd = i;
		}
	}
	mutex.unlock();
	median = data[minDiffInd];
	medianRight = min;
	for (; median > medianRight; medianRight += h);
	medianLeft = medianRight - h;
	return true;
}

BOOL WINAPI GetMode() {
	mutex.lock();
	int count = density.size();
	double maxValue = 0;
	int ind = 0;
	for (int i = 1; i < count - 1; i++) {
		if (density[i - 1] > maxValue) {
			maxValue = density[i - 1];
			ind = i;
		}
	}
	mutex.unlock();
	maxDensityValue = maxValue;
	mode = data[ind];
	modeRight = min;
	for (; mode > modeRight; modeRight += h);
	modeLeft = modeRight - h;
	return true;
}

BOOL WINAPI GetExcess() {
	mutex.lock();
	int count = data.size();
	double total = 0;
	for (int i = 0; i < count; i++) {
		double term = pow(data[i] - M, 4); //fourth central moment
		total += term;
	}
	mutex.unlock();
	excess = total / (count * pow(standartDeviation, 4)) - 3;
	return true;
}

BOOL WINAPI GetAssymetry() {
	assymetry = (M - mode) / standartDeviation;
	return true;
}
BOOL WINAPI GetVarCoeff() {
	variationCoeff = standartDeviation / M;
	return true;
}

void CreateReportWin(HWND hWnd) {
	hWndR = CreateWindowEx(0, L"ReportWindowClass", L"����� � ���������������", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME | WS_BORDER | WS_POPUP | WS_VISIBLE | WS_CHILD,
		200, 100, 1000, 700, hWnd, NULL, NULL, NULL);

	EnableWindow(hWndR, TRUE);
	ShowWindow(hWndR, SW_SHOWDEFAULT);
	UpdateWindow(hWndR);
}

void RegisterReportWinClass() { //���� ����������/������������

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
		hEditControl = CreateWindowA("edit", "", WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL, 20, 5, 950, 550, hWnd, NULL, NULL, NULL);
		//Buttton
		CreateWindowA("button", "��������� �����", WS_VISIBLE | WS_CHILD | ES_CENTER, 20, 570, 950, 80, hWnd, (HMENU)OnSaveReportButtonClicked, NULL, NULL);

		FillEdit();
		break;
	case WM_PAINT:
		break;
	case WM_DESTROY:
		//PostQuitMessage(0);
		break;
	default: return DefWindowProc(hWnd, msg, wp, lp);
	}
}

void FillEdit() {
	std::string str;
	str.append("�����������:\r\n");
	std::string MStr = std::to_string(M);
	str.append(MStr);
	str.append("\r\n\r\n���������:\r\n");
	std::string DStr = std::to_string(D);
	str.append(DStr);
	str.append("\r\n\r\n�������������������� ����������:\r\n");
	std::string SDStr = std::to_string(standartDeviation);
	str.append(SDStr);
	str.append("\r\n\r\n�������:\r\n");
	std::string medianStr = std::to_string(median);
	str.append(medianStr);
	str.append("\r\n\r\n��������� ��������:\r\n");
	str.append("[");
	str.append(std::to_string(medianLeft));
	str.append("; ");
	str.append(std::to_string(medianRight));
	str.append(")");
	str.append("\r\n\r\n����:\r\n");
	std::string modeStr = std::to_string(mode);
	str.append(modeStr);
	str.append("\r\n\r\n��������� ��������:\r\n");
	str.append("[");
	str.append(std::to_string(modeLeft));
	str.append("; ");
	str.append(std::to_string(modeRight));
	str.append(")");
	str.append("\r\n\r\n������ ��������:\r\n");
	std::string deltaStr = std::to_string(delta);
	str.append(deltaStr);
	str.append("\r\n\r\n����������:\r\n");
	std::string assymetryStr = std::to_string(assymetry);
	str.append(assymetryStr);
	str.append("\r\n\r\n�������:\r\n");
	std::string excessStr = std::to_string(excess);
	str.append(excessStr);
	str.append("\r\n\r\n����������� ��������:\r\n");
	std::string varCStr = std::to_string(variationCoeff);
	str.append(varCStr);
	char output[500] = { 0 };
	for (int i = 0; i < str.length(); i++)
		output[i] = str[i];
	//HFONT hFont = CreateFontA(10, 5, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	//SendMessageA(hEditControl, WM_SETFONT,(WPARAM) hFont, 0);
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

char* StdStrToCharArr(std::string str) {
	char result[500] = { 0 };
	for (int i = 0; i < str.length(); i++)
		result[i] = str[i];
	return result;
}