#pragma once
#include <Windows.h>
#include <vector>
#include <mutex>

#define OnButtonClicked	1
#define OnReportButtonClicked 2
#define OnSaveReportButtonClicked	4

#define TextBufferSize	65536

using std::vector;
//vars
vector<double> data;
vector<double> distribution;
vector<double> density;
HWND hEditControl;
OPENFILENAMEA ofn; 
char filename[300];
char buffer[TextBufferSize];
RECT rc1, rc2;
double min;
double max;
double delta;
double h;
double argument;

double M;	//Expected value
double D;	//Dispersion
double standartDeviation;
double median;
double mode;

std::mutex mutex;

//Prototypes
LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
WNDCLASS NewWindowClass(HBRUSH bgColor, HCURSOR cursor, HINSTANCE hInst, HICON icon, LPCWSTR name, WNDPROC procedure);
void RegisterReportWinClass();
void SetOpenFileParams(HWND hWnd);
void MainAddWidgets(HWND hWnd);
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