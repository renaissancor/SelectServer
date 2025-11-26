#pragma once

// WindowMain.h 

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); 

class WindowMain {
private:

	HINSTANCE _hInstance = NULL; 
	HWND _hWindow = NULL;
	WNDCLASSEX _wcex = { 0 };

	HDC _hMainDC = NULL;
	HDC _hBackBufferDC = NULL;
	HBITMAP _hBackBufferBitmap = NULL; 

	HPEN _pens[static_cast<size_t>(PEN_TYPE::END)] = { NULL };
	HBRUSH _brushes[static_cast<size_t>(BRUSH_TYPE::END)] = { NULL };

	POINT _ptPrev;
	bool _isDrawing = false;

	struct Line { int ax, ay, bx, by; };
	std::queue<Line> _line2Draw; 

private:
	WindowMain();
	~WindowMain();
	WindowMain(const WindowMain&) = delete;
	WindowMain& operator=(const WindowMain&) = delete;

public:
	inline static WindowMain& GetInstance() noexcept {
		static WindowMain instance;
		return instance;
	}

	inline HDC GetMainDC() const noexcept { return _hMainDC; }
	inline HDC GetBackBufferDC() const noexcept { return _hBackBufferDC; }
	inline HWND GetWindowHandle() const noexcept { return _hWindow; }
	inline HPEN GetPen(PEN_TYPE type) const noexcept 
	{ return _pens[static_cast<size_t>(type)]; }
	inline HBRUSH GetBrush(BRUSH_TYPE type) const noexcept 
	{ return _brushes[static_cast<size_t>(type)]; }
	inline void EnqueueLineToDraw(int ax, int ay, int bx, int by) noexcept {
		_line2Draw.push({ ax, ay, bx, by });
	}
	
	inline void HandleLButtonDown(int x, int y) noexcept {
		_isDrawing = true;
		_ptPrev.x = x;
		_ptPrev.y = y;
	}

	inline void HandleLButtonUp() noexcept { _isDrawing = false; }

	inline void HandleMouseMove(int x, int y) noexcept; 

	void CreateGDIObjects() noexcept; 
	
	bool Initialize() noexcept;
	void Shutdown() noexcept;

	void DrawRecvPacket() noexcept; 

	void Present() const noexcept; 

};