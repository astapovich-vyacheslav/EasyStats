#pragma once
#include <Windows.h>
#include <vector>

#define OnButtonClicked	1

#define TextBufferSize	65536

using std::vector;
//vars
vector<double> data;
vector<double> distribution;
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

//Prototypes
LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
WNDCLASS NewWindowClass(HBRUSH bgColor, HCURSOR cursor, HINSTANCE hInst, HICON icon, LPCWSTR name, WNDPROC procedure);
void SetOpenFileParams(HWND hWnd);
void MainAddWidgets(HWND hWnd);
void LoadDoublesData(LPCSTR path, vector<double>* data);
void PaintGraphicsArea(HWND hWnd);
void DrawGrid(HDC hDC, RECT rc);
void DrawAxises(HDC hDC, RECT rc);
vector<double> GetDistribution();
void DrawDistribution(HWND hWnd);