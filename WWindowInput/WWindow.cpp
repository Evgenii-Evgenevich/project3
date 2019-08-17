#define WIN32_LEAN_MEAN 1
#include <windows.h>

#include "../WindowInput/Window.hpp"
#include <thread>

// Register the window class
bool registerWndClass(WNDCLASSEXA& wcex, HINSTANCE hInstance, char const* className, WNDPROC windowProc = DefWindowProcA)
{
	if (GetClassInfoExA(hInstance, className, &wcex))
	{
		return false;
	}
	memset(&wcex, 0, sizeof(WNDCLASSEXA));
	wcex.cbSize = sizeof(WNDCLASSEXA);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = windowProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursorA(nullptr, MAKEINTRESOURCEA(32512));
	wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOWFRAME);
	wcex.lpszClassName = className;
	return RegisterClassExA(&wcex) != 0;
}

// Register the window class
bool registerWndClass(WNDCLASSEXW& wcex, HINSTANCE hInstance, wchar_t const* className, WNDPROC windowProc = DefWindowProcW)
{
	if (GetClassInfoExW(hInstance, className, &wcex))
	{
		return false;
	}
	memset(&wcex, 0, sizeof(WNDCLASSEXW));
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = windowProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
	wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOWFRAME);
	wcex.lpszClassName = className;
	return RegisterClassExW(&wcex) != 0;
}

static LRESULT WINAPI WindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT WINAPI WindowProcW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

EXTERN_C HWND wCreateWindowA(
	char const* title,
	short width, short height,
	int style,
	HWND hParent,
	HMENU hMenu = nullptr,
	void* pData = nullptr
) noexcept
{
	static HINSTANCE hInstance = GetModuleHandleA(nullptr);
	HWND hWnd = nullptr;
	{
		char lpszClassName[] = "Window";
		{
			WNDCLASSEXA sWndClass;
			registerWndClass(sWndClass, hInstance, lpszClassName);
		}
		RECT windowRect = { 0, 0, width, height };
		DWORD windowStyle = WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		DWORD windowExtendedStyle;
		if (hParent)
		{
			windowStyle |= (style & WSTYLE_NODLGFRAME) ? WS_CHILD : (WS_DLGFRAME | WS_CHILD);
			windowExtendedStyle = 0;
		}
		else
		{
			windowStyle |= (style & WSTYLE_NODLGFRAME) ? WS_POPUP : WS_DLGFRAME;
			windowExtendedStyle = WS_EX_APPWINDOW;
		}
		if ((style & WSTYLE_NOBORDER) == 0)
		{
			windowStyle |= WS_BORDER;
		}
		if ((style & WSTYLE_NOTHICKFRAME) == 0)
		{
			windowStyle |= WS_THICKFRAME;
		}
		if ((style & WSTYLE_NOMINIMIZEBOX) == 0)
		{
			windowStyle |= WS_MINIMIZEBOX;
		}
		if ((style & WSTYLE_NOMAXIMIZEBOX) == 0)
		{
			windowStyle |= WS_MAXIMIZEBOX;
		}
		//if (!sWndClass.hIcon)
		{
			windowExtendedStyle |= WS_EX_DLGMODALFRAME;
		}

		AdjustWindowRectEx(&windowRect, windowStyle, hMenu ? 1 : 0, windowExtendedStyle);

		hWnd = CreateWindowExA(windowExtendedStyle, // window extended style 
			lpszClassName, // name of a window class 
			title, // title of a window 
			windowStyle, // window style 
			CW_USEDEFAULT, // x-position of a window 
			CW_USEDEFAULT, // y-position of a window 
			windowRect.right - windowRect.left, // width of a window 
			windowRect.bottom - windowRect.top, // height of a window 
			hParent, // a parent window 
			hMenu, // a window menu
			hInstance, // application handle 
			nullptr); // used with multiple windows 
	}
	if (style & WSTYLE_NOCLOSEBOX)
	{
		if (HMENU hMenu = GetSystemMenu(hWnd, 0))
		{
			EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
		}
	}
	SetWindowLongPtrA(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcA));
	if (pData)
	{
		SetWindowLongPtrA(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pData));
	}
	return hWnd;
}

EXTERN_C HWND wCreateWindowW(
	wchar_t const* title,
	short width, short height,
	int style,
	HWND hParent,
	HMENU hMenu = nullptr,
	void* pData = nullptr
) noexcept
{
	HWND hWnd = nullptr;
	{
		HINSTANCE hInstance = GetModuleHandleW(nullptr);
		wchar_t lpszClassName[] = L"Window";
		{
			WNDCLASSEXW sWndClass;
			registerWndClass(sWndClass, hInstance, lpszClassName);
		}
		RECT windowRect = { 0, 0, width, height };
		DWORD windowStyle = WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		DWORD windowExtendedStyle;
		if (hParent)
		{
			windowStyle |= (style & WSTYLE_NODLGFRAME) ? WS_CHILD : (WS_DLGFRAME | WS_CHILD);
			windowExtendedStyle = 0;
		}
		else
		{
			windowStyle |= (style & WSTYLE_NODLGFRAME) ? WS_POPUP : WS_DLGFRAME;
			windowExtendedStyle = 0;
		}
		if ((style & WSTYLE_NOBORDER) == 0)
		{
			windowStyle |= WS_BORDER;
		}
		if ((style & WSTYLE_NODLGFRAME) == 0)
		{
			windowStyle |= WS_DLGFRAME;
		}
		if ((style & WSTYLE_NOTHICKFRAME) == 0)
		{
			windowStyle |= WS_THICKFRAME;
		}
		if ((style & WSTYLE_NOMINIMIZEBOX) == 0)
		{
			windowStyle |= WS_MINIMIZEBOX;
		}
		if ((style & WSTYLE_NOMAXIMIZEBOX) == 0)
		{
			windowStyle |= WS_MAXIMIZEBOX;
		}
		//if (!sWndClass.hIcon)
		{
		//	windowExtendedStyle |= WS_EX_DLGMODALFRAME;
		}

		AdjustWindowRectEx(&windowRect, windowStyle, hMenu ? 1 : 0, windowExtendedStyle);

		hWnd = CreateWindowExW(windowExtendedStyle, // window extended style 
			lpszClassName, // name of a window class 
			title, // title of a window 
			windowStyle, // window style 
			CW_USEDEFAULT, // x-position of a window 
			CW_USEDEFAULT, // y-position of a window 
			windowRect.right - windowRect.left, // width of a window 
			windowRect.bottom - windowRect.top, // height of a window 
			hParent, // a parent window 
			hMenu, // a window menu
			hInstance, // application handle 
			nullptr); // used with multiple windows 
	}
	if (style & WSTYLE_NOCLOSEBOX)
	{
		if (HMENU hMenu = GetSystemMenu(hWnd, 0))
		{
			EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
		}
	}
	SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcW));
	if (pData)
	{
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pData));
	}
	return hWnd;
}

EXTERN_C HWND wCreateWindow(
	short width, short height,
	int style,
	HWND hParent,
	HMENU hMenu = nullptr,
	void* pData = nullptr
) noexcept
{
	return wCreateWindowW(nullptr, width, height, style, hParent, hMenu, pData);
}

static PWindow root_window{ nullptr };

struct WWindow final : IWindow
{
	HWND const m_handle = nullptr;

	// the low-order word specifies a width of the client area (LOWORD) 
	// the high-order word specifies a height of the client area (HIWORD) 
	unsigned int m_clientAreaSize;

	// the low-order word specifies a X-coordinat of the cursor (LOWORD) 
	// the high-order word specifies a Y-coordinate of the cursor (HIWORD) 
	unsigned int m_clientAreaCursor = 0;

private:
	WWindow(char const* title, short width, short height, int style, HWND hParent) noexcept
		: m_handle(wCreateWindowA(title, width, height, style, hParent))
		, m_clientAreaSize(height << 16 | width)
	{
	}

	WWindow(wchar_t const* title, short width, short height, int style, HWND hParent) noexcept
		: m_handle(wCreateWindowW(title, width, height, style, hParent))
		, m_clientAreaSize(height << 16 | width)
	{
	}

	static void loop(std::shared_ptr<WWindow> window)
	{
		HWND const hWnd = window ? window->m_handle : nullptr;
		if (hWnd == nullptr) return;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&window));
		MSG msg{};
		while (msg.message != WM_QUIT && IsWindow(hWnd))
		{
			Sleep(1);
			if (PeekMessageA(&msg, hWnd, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageA(&msg);
			}
		}
		DestroyWindow(hWnd);
	}

public:
	~WWindow() override
	{
	}

	static std::shared_ptr<WWindow> create(char const* title, int style, short width, short height, HWND hParent)
	{
		std::shared_ptr<WWindow> shared_ptr{ nullptr };
		std::thread([title, style, width, height, hParent, &shared_ptr]() {
			shared_ptr = std::shared_ptr<WWindow>(new WWindow(title, width, height, style, hParent));
			loop(shared_ptr);
		}).detach();
		while (!shared_ptr)
		{
			Sleep(20);
		}
		return shared_ptr;
	}

	static std::shared_ptr<WWindow> create(wchar_t const* title, int style, short width, short height, HWND hParent)
	{
		std::shared_ptr<WWindow> shared_ptr{ nullptr };
		std::thread([title, style, width, height, hParent, &shared_ptr]() {
			shared_ptr = std::shared_ptr<WWindow>(new WWindow(title, width, height, style, hParent));
			loop(shared_ptr);
		}).detach();
		while (!shared_ptr)
		{
			Sleep(20);
		}
		return shared_ptr;
	}

	PWindow create(char const* title, int style, short width, short height) const override
	{
		return PWindow(create(title, style, width, height, m_handle));
	}

	PWindow create(wchar_t const* title, int style, short width, short height) const override
	{
		return PWindow(create(title, style, width, height, m_handle));
	}

	PWindow create(int style, short width, short height) const override
	{
		return PWindow(create(static_cast<wchar_t const*>(nullptr), style, width, height, m_handle));
	}

	void show() const noexcept override
	{
		ShowWindow(m_handle, SW_RESTORE);
		SetForegroundWindow(m_handle);
	}

	void minimize() const noexcept override
	{
		ShowWindow(m_handle, SW_MINIMIZE);
	}

	void hide() const noexcept override
	{
		ShowWindow(m_handle, SW_HIDE);
	}

	void close() const noexcept override
	{
		PostMessageA(m_handle, WM_CLOSE, 0, 0);
	}

	bool isClosed() const noexcept override
	{
		return IsWindow(m_handle) == 0;
	}

	bool isVisible() const noexcept override
	{
		return IsIconic(m_handle) == 0 && IsWindowVisible(m_handle) != 0;
	}

	bool isHidden() const noexcept override
	{
		return IsWindowVisible(m_handle);
	}

	bool isActive() const noexcept override
	{
		return GetForegroundWindow() == m_handle;
	}

	void getClientSize(short& width, short& height) const noexcept override
	{
		short const* pPoint = static_cast<short const*>(static_cast<void const*>(&m_clientAreaSize));
		width = pPoint[0];
		height = pPoint[1];
	}

	void getClientCursorPos(short& x, short& y) const noexcept override
	{
		short const* pPoint = static_cast<short const*>(static_cast<void const*>(&m_clientAreaCursor));
		x = pPoint[0];
		y = pPoint[1];
	}

	Title getTitle() const noexcept override
	{
		if (IsWindowUnicode(m_handle) == 0)
		{
			std::string title(GetWindowTextLengthA(m_handle) + 1, '\0');
			GetWindowTextA(m_handle, const_cast<LPSTR>(title.data()), title.capacity());
			return title;
		}
		std::wstring title(GetWindowTextLengthW(m_handle) + 1, L'\0');
		GetWindowTextW(m_handle, const_cast<LPWSTR>(title.data()), title.capacity());
		return title;
	}

	void setTitle(char const* title) const noexcept override
	{
		SetWindowTextA(m_handle, title);
	}

	void setTitle(wchar_t const* title) const noexcept override
	{
		SetWindowTextW(m_handle, title);
	}

	PWindow getParent() const override
	{
		HWND hParent = GetParent(m_handle);
		return hParent
			? *reinterpret_cast<std::shared_ptr<WWindow> const*>(GetWindowLongPtrA(hParent, GWLP_USERDATA))
			: root_window;
	}

	void resize() noexcept
	{
		RECT rect;
		if (GetClientRect(m_handle, &rect))
		{
			short* point = reinterpret_cast<short*>(&m_clientAreaSize);
			point[0] = static_cast<short>(rect.right - rect.left);
			point[1] = static_cast<short>(rect.bottom - rect.top);
		}
	}
};

void menuEvent(int wParam, HMENU lParam, Window window) {}

static int window_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpUserData)
{
	WWindow* p_window = lpUserData ? *reinterpret_cast<WWindow* const*>(lpUserData) : nullptr;
	switch (Msg)
	{
	case WM_CREATE:
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		if (p_window)
		{
			p_window->m_clientAreaSize = lParam & 0xffffffff;
		}
		return 0;

	case WM_ACTIVATE:
		return 0;

	case WM_PAINT:
		return 0;

	case WM_MENUCOMMAND:
		if (p_window)
		{
			menuEvent(wParam, reinterpret_cast<HMENU>(lParam), *p_window);
		}
		return 0;

	case WM_MOUSEMOVE:
		if (p_window)
		{
			p_window->m_clientAreaCursor = lParam & 0xffffffff;
		}
		return 0;

	default:
		break;
	}
	return -1;
}

static LRESULT WINAPI WindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = window_proc(hWnd, Msg, wParam, lParam, GetWindowLongPtrA(hWnd, GWLP_USERDATA));
	return result == -1
		? DefWindowProcA(hWnd, Msg, wParam, lParam)
		: result;
}

static LRESULT WINAPI WindowProcW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = window_proc(hWnd, Msg, wParam, lParam, GetWindowLongPtrW(hWnd, GWLP_USERDATA));
	return result == -1
		? DefWindowProcW(hWnd, Msg, wParam, lParam)
		: result;
}

struct WRootWindow final : IWindow
{
	constexpr WRootWindow() noexcept = default;

	~WRootWindow() override
	{
	}

	PWindow create(char const* title, int style, short width, short height) const override
	{
		return PWindow(WWindow::create(title, style, width, height, HWND_DESKTOP));
	}

	PWindow create(wchar_t const* title, int style, short width, short height) const override
	{
		return PWindow(WWindow::create(title, style, width, height, HWND_DESKTOP));
	}

	PWindow create(int style, short width, short height) const override
	{
		return PWindow(WWindow::create(static_cast<wchar_t const*>(nullptr), style, width, height, HWND_DESKTOP));
	}

	void show() const noexcept override
	{
		SendMessageA(FindWindowA("Shell_TrayWnd", nullptr), WM_COMMAND, 419, 0);
	}

	void minimize() const noexcept override
	{
		SendMessageA(FindWindowA("Shell_TrayWnd", nullptr), WM_COMMAND, 416, 0);
	}

	void hide() const noexcept override {}

	void close() const noexcept override {}

	bool isClosed() const noexcept override { return false; }

	bool isVisible() const noexcept override { return false; }

	bool isHidden() const noexcept override { return false; }

	bool isActive() const noexcept override { return GetForegroundWindow() == nullptr; }

	void getClientSize(short& width, short& height) const noexcept override
	{
		width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}

	void getClientCursorPos(short& x, short& y) const noexcept override
	{
		POINT point;
		GetCursorPos(&point);
		x = point.x;
		y = point.y;
	}

	Title getTitle() const noexcept override
	{
		std::string title = std::string(16, '\0');
		unsigned long n;
		GetComputerNameA(const_cast<LPSTR>(title.data()), &n);
		return title;
	}

	void setTitle(char const*) const noexcept override {}
	void setTitle(wchar_t const*) const noexcept override {}

	PWindow getParent() const noexcept override { return root_window; }
};

EXTERN_C Window getRootWindow()
{
	if (root_window == nullptr)
	{
		root_window = std::make_shared<WRootWindow>();
	}
	return *root_window;
}
