#include "stdafx.h"
#include "WindowMain.h"
#include "Network.h"

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SOCKET_NOTIFY:
		Network::GetInstance().ProcessWSA(wParam, lParam);
		return 0;
	case WM_MOUSEMOVE: 
		WindowMain::GetInstance().HandleMouseMove(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_LBUTTONUP:
		WindowMain::GetInstance().HandleLButtonUp();
		return 0;
	case WM_LBUTTONDOWN:
		WindowMain::GetInstance().HandleLButtonDown(LOWORD(lParam), HIWORD(lParam));
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

inline void WindowMain::HandleMouseMove(int x, int y) noexcept {
	if (!_isDrawing) return;
	Network::GetInstance().SendDrawPacket(_ptPrev.x, _ptPrev.y, x, y);
	_ptPrev = { x, y };
}

void WindowMain::CreateGDIObjects() noexcept {
	_pens[static_cast<size_t>(PEN_TYPE::BLACK)] = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	_pens[static_cast<size_t>(PEN_TYPE::WHITE)] = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	_pens[static_cast<size_t>(PEN_TYPE::GRAY)] = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
	_pens[static_cast<size_t>(PEN_TYPE::RED)] = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	_pens[static_cast<size_t>(PEN_TYPE::GREEN)] = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	_pens[static_cast<size_t>(PEN_TYPE::BLUE)] = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	_pens[static_cast<size_t>(PEN_TYPE::CYAN)] = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
	_pens[static_cast<size_t>(PEN_TYPE::MAGENTA)] = CreatePen(PS_SOLID, 1, RGB(255, 0, 255));
	_pens[static_cast<size_t>(PEN_TYPE::YELLOW)] = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
	
	_brushes[static_cast<size_t>(BRUSH_TYPE::BLACK)] = CreateSolidBrush(RGB(0, 0, 0));
	_brushes[static_cast<size_t>(BRUSH_TYPE::WHITE)] = CreateSolidBrush(RGB(255, 255, 255));
	_brushes[static_cast<size_t>(BRUSH_TYPE::GRAY)] = CreateSolidBrush(RGB(128, 128, 128));
	_brushes[static_cast<size_t>(BRUSH_TYPE::RED)] = CreateSolidBrush(RGB(255, 0, 0));
	_brushes[static_cast<size_t>(BRUSH_TYPE::GREEN)] = CreateSolidBrush(RGB(0, 255, 0));
	_brushes[static_cast<size_t>(BRUSH_TYPE::BLUE)] = CreateSolidBrush(RGB(0, 0, 255));
	_brushes[static_cast<size_t>(BRUSH_TYPE::CYAN)] = CreateSolidBrush(RGB(0, 255, 255));
	_brushes[static_cast<size_t>(BRUSH_TYPE::MAGENTA)] = CreateSolidBrush(RGB(255, 0, 255));
	_brushes[static_cast<size_t>(BRUSH_TYPE::YELLOW)] = CreateSolidBrush(RGB(255, 255, 0));
	_brushes[static_cast<size_t>(BRUSH_TYPE::HOLLOW)] = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
}

bool WindowMain::Initialize() noexcept {

	_wcex.cbSize = sizeof(WNDCLASSEX);
	_wcex.style = CS_HREDRAW | CS_VREDRAW;
	_wcex.lpfnWndProc = WndProc; // DefWindowProc;
	_wcex.cbClsExtra = 0;
	_wcex.cbWndExtra = 0;
	_wcex.hInstance = GetModuleHandle(NULL);
	_wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	_wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	_wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	_wcex.lpszMenuName = NULL;
	_wcex.lpszClassName = L"DrawClientWindowClass";
	_wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&_wcex);

	_hWindow = CreateWindow(
		L"DrawClientWindowClass",
		L"Draw Client",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 600,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL
	);

	if (!_hWindow) {
		fprintf_s(stderr, "Failed to create main window.\n");
		return false;
	}

	_hMainDC = GetDC(_hWindow);
	_hBackBufferDC = CreateCompatibleDC(_hMainDC);

	RECT clientRect;
	GetClientRect(_hWindow, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
	_hBackBufferBitmap = CreateCompatibleBitmap(_hMainDC, width, height);
	SelectObject(_hBackBufferDC, _hBackBufferBitmap);

	CreateGDIObjects();
	ShowWindow(_hWindow, SW_SHOW);

	return true;
}

void WindowMain::Shutdown() noexcept {
	// Delete GDI Objects 
	for (size_t i = 0; i < static_cast<size_t>(PEN_TYPE::END); ++i) {
		if (_pens[i] == NULL) continue;
		DeleteObject(_pens[i]);
		_pens[i] = NULL;
	}
	for (size_t i = 0; i < static_cast<size_t>(BRUSH_TYPE::END); ++i) {
		if (_brushes[i] == NULL) continue; 
		DeleteObject(_brushes[i]);
		_brushes[i] = NULL;
	}

	// Delete DCs
	if (_hBackBufferDC) {
		DeleteDC(_hBackBufferDC);
		_hBackBufferDC = NULL;
	}
	if (_hMainDC) {
		ReleaseDC(_hWindow, _hMainDC);
		_hMainDC = NULL;
	}
	if (_hBackBufferBitmap) {
		DeleteObject(_hBackBufferBitmap); 
		_hBackBufferBitmap = NULL;
	}
	if (_hWindow) {
		DestroyWindow(_hWindow);
		_hWindow = NULL;
	}

	UnregisterClass(L"DrawClientWindowClass", GetModuleHandle(NULL));
}

void WindowMain::DrawRecvPacket() noexcept {
	while (!_line2Draw.empty()) {
		Line line = _line2Draw.front();
		_line2Draw.pop();
		MoveToEx(_hBackBufferDC, line.ax, line.ay, NULL);
		LineTo(_hBackBufferDC, line.bx, line.by);
	}
}

void WindowMain::Present() const noexcept {
	RECT clientRect;
	GetClientRect(_hWindow, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
	BitBlt(_hMainDC, 0, 0, width, height, _hBackBufferDC, 0, 0, SRCCOPY);
}


WindowMain::WindowMain() = default;

WindowMain::~WindowMain() = default;
