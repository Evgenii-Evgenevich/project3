#define XRootWindow getXRootWindow
#define Window HWindow
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#undef XRootWindow
#undef Window

#include "../WindowInput/Window.hpp"

#include <thread>
#include <vector>

#define MWM_HINTS_FUNCTIONS     (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)

#define MWM_DECOR_ALL           (1L << 0)
#define MWM_DECOR_BORDER        (1L << 1)
#define MWM_DECOR_RESIZEH       (1L << 2)
#define MWM_DECOR_TITLE         (1L << 3)
#define MWM_DECOR_MENU          (1L << 4)
#define MWM_DECOR_MINIMIZE      (1L << 5)
#define MWM_DECOR_MAXIMIZE      (1L << 6)

#define MWM_FUNC_ALL            (1L << 0)
#define MWM_FUNC_RESIZE         (1L << 1)
#define MWM_FUNC_MOVE           (1L << 2)
#define MWM_FUNC_MINIMIZE       (1L << 3)
#define MWM_FUNC_MAXIMIZE       (1L << 4)
#define MWM_FUNC_CLOSE          (1L << 5)

static XContext xUniqueContext() noexcept
{
    static XContext context = XUniqueContext();
    return context;
}

EXTERN_C XID xCreateWindow(
	int style,
    short width, short height,
	XID parentId,
	Screen* screen,
	void* pData = nullptr
) noexcept
{
	Display* display = DisplayOfScreen(screen);
    unsigned int border_width = style & WSTYLE_NOBORDER ? 0 : 1;

	XID xid = XCreateSimpleWindow(display, parentId, 0, 0, width, height, border_width, BlackPixelOfScreen(screen), WhitePixelOfScreen(screen));

    if (pData)
    {
        XSaveContext(display, xid, xUniqueContext(), static_cast<char*>(pData));
    }

    static Atom mwm_wm_hints = XInternAtom(display,"_MOTIF_WM_HINTS", False);
    if (mwm_wm_hints == 0) return xid;

    struct {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
    } hints = {MWM_HINTS_DECORATIONS | MWM_HINTS_FUNCTIONS, MWM_FUNC_MOVE, MWM_DECOR_MENU };
    if ((style & WSTYLE_NOBORDER) == 0)
    {
        hints.decorations |= MWM_DECOR_BORDER;
    }
    if ((style & WSTYLE_NODLGFRAME) == 0)
    {
        hints.decorations |= MWM_DECOR_TITLE;
    }
    if ((style & WSTYLE_NOTHICKFRAME) == 0)
    {
        hints.functions |= MWM_FUNC_RESIZE;
        hints.decorations |= MWM_DECOR_RESIZEH;
    }
    if ((style & WSTYLE_NOMINIMIZEBOX) == 0)
    {
        hints.functions |= MWM_FUNC_MINIMIZE;
        hints.decorations |= MWM_DECOR_MINIMIZE;
    }
    if ((style & WSTYLE_NOMAXIMIZEBOX) == 0)
    {
        hints.functions |= MWM_FUNC_MAXIMIZE;
        hints.decorations |= MWM_DECOR_MAXIMIZE;
    }
    if ((style & WSTYLE_NOCLOSEBOX) == 0)
    {
        hints.functions |= MWM_FUNC_CLOSE;
    }
    XChangeProperty(display, xid, mwm_wm_hints, mwm_wm_hints, 32, PropModeReplace, static_cast<unsigned char*>(static_cast<void*>(&hints)), 3);
    return xid;
}

static PWindow root_window{ nullptr };

struct XWindow final : IWindow
{
    Screen* m_screen = nullptr;
    XID m_handle = 0;

private:
	XWindow(char const* title, int style, short width, short height, XID parentId, Screen* screen)
	    : m_screen(screen)
		, m_handle(xCreateWindow(style, width, height, parentId, m_screen))
	{
        setTitle(title);
	}

	XWindow(wchar_t const* title, int style, short width, short height, XID parentId, Screen* screen)
            : m_screen(screen)
            , m_handle(xCreateWindow(style, width, height, parentId, m_screen))
	{
        setTitle(title);
	}

    static int predicate(Display*, XEvent* e, XPointer xid) noexcept
    {
        return e->xany.window == reinterpret_cast<XID>(xid);
    }

    static void loop(std::shared_ptr<XWindow> window) noexcept
    {
	    if (window == nullptr) return;

		Display* display = DisplayOfScreen(window->m_screen);
		XID xid = window->m_handle;

		static Atom WM_PROTOCOLS = XInternAtom(display, "WM_PROTOCOLS", False);
		static Atom WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display, xid, &WM_DELETE_WINDOW, 1);

        XSaveContext(display, xid, xUniqueContext(), static_cast<char*>(static_cast<void*>(&window)));

        XEvent e;
        bool looping = true;
        while (looping)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (XCheckIfEvent(display, &e, predicate, reinterpret_cast<XPointer>(xid)))
            {
                switch (e.type)
                {
                    case DestroyNotify:
                        looping = false;
						break;

                    case ClientMessage:
                        if (e.xclient.message_type == WM_PROTOCOLS)
                        {
                            if (static_cast<Atom>(e.xclient.data.l[0]) == WM_DELETE_WINDOW)
                            {
                                looping = false;
                                break;
                            }
                        }
                        break;
                }
            }
        }
        XDeleteContext(display, xid, xUniqueContext());
        XDestroyWindow(display, xid);
        window->m_handle = 0;
        XFlush(display);
    }

public:
	static std::shared_ptr<XWindow> create(char const* title, int style, short width, short height, XID parentId, Screen* screen)
    {
        std::shared_ptr<XWindow> shared_ptr{ nullptr };
        std::thread([title, style, width, height, parentId, screen, &shared_ptr]() {
            shared_ptr = std::shared_ptr<XWindow>(new XWindow(title, style, width, height, parentId, screen));
            loop(shared_ptr);
        }).detach();
        while (!shared_ptr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        return shared_ptr;
    }

    static std::shared_ptr<XWindow> create(wchar_t const* title, int style, short width, short height, XID parentId, Screen* screen)
    {
        std::shared_ptr<XWindow> shared_ptr{ nullptr };
        std::thread([title, style, width, height, parentId, screen, &shared_ptr]() {
            shared_ptr = std::shared_ptr<XWindow>(new XWindow(title, style, width, height, parentId, screen));
            loop(shared_ptr);
        }).detach();
        while (!shared_ptr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        return shared_ptr;
    }

    ~XWindow() noexcept override
    {
        if (m_handle != 0)
        {
            Display* display = DisplayOfScreen(m_screen);
            XDeleteContext(display, m_handle, xUniqueContext());
            XDestroyWindow(display, m_handle);
            m_handle = 0;
            XFlush(display);
        }
    }

    PWindow create(char const* title, int style, short width, short height) const override
    {
        return PWindow(create(title, style, width, height, m_handle, m_screen));
    }

    PWindow create(wchar_t const* title, int style, short width, short height) const override
    {
        return PWindow(create(title, style, width, height, m_handle, m_screen));
    }

    PWindow create(int style, short width, short height) const override
    {
        return PWindow(create(static_cast<char const*>(nullptr), style, width, height, m_handle, m_screen));
    }

	void show() const noexcept override
	{
		XMapWindow(DisplayOfScreen(m_screen), m_handle);
	}

    void minimize() const noexcept override
    {
	    Display* display = DisplayOfScreen(m_screen);
		XIconifyWindow(display, m_handle, DefaultScreen(display));
    }

    void hide() const noexcept override
	{
		XUnmapWindow(DisplayOfScreen(m_screen), m_handle);
	}

    void close() const noexcept override
    {
        Display* display = DisplayOfScreen(m_screen);
		XEvent event;
		event.xclient.type = ClientMessage;
		event.xclient.serial = 0;
		event.xclient.send_event = True;
		event.xclient.window = m_handle;
		static Atom _NET_CLOSE_WINDOW = XInternAtom(display, "_NET_CLOSE_WINDOW", False);
        event.xclient.message_type = _NET_CLOSE_WINDOW;
		event.xclient.format = 32;
		event.xclient.data = { 0L, 0L, 0L, 0L, 0L };
		XSendEvent(display, RootWindowOfScreen(m_screen), False, SubstructureNotifyMask | SubstructureRedirectMask, &event);
		XSync(display, False);
    }

    bool isClosed() const noexcept override
    {
        return m_handle == 0;
    }

    static unsigned int state(Display* display, XID xid) noexcept
    {
        static Atom WM_STATE = XInternAtom(display, "WM_STATE", False);
        Atom actual_type = 0;
        int actual_format;
        unsigned long nitems = 0, bytes_after;
        struct {
            unsigned int state;
            XID icon;
        }* property = nullptr;
        XGetWindowProperty(display, xid, WM_STATE, 0, ~0L, False, WM_STATE, &actual_type, &actual_format, &nitems, &bytes_after, static_cast<unsigned char**>(static_cast<void*>(&property)));
        if (property == nullptr || nitems == 0 || actual_type != WM_STATE)
        {
            return WithdrawnState;
        }
        unsigned int state = property->state;
        XFree(property);
        return state;
    }

    bool isVisible() const noexcept override
    {
        unsigned int state = XWindow::state(DisplayOfScreen(m_screen), m_handle);
        return state != WithdrawnState && state != IconicState;
    }

    bool isHidden() const noexcept override
    {
        XWindowAttributes attributes{};
        XGetWindowAttributes(DisplayOfScreen(m_screen), m_handle, &attributes);
        return attributes.map_state != IsViewable;
    }

    bool isActive() const noexcept override
    {
		XID focus = 0;
		int revert_to;
		XGetInputFocus(DisplayOfScreen(m_screen), &focus, &revert_to);
        return focus == m_handle;
    }

    void getClientSize(short& width, short& height) const noexcept override
    {
		XWindowAttributes attributes{};
		XGetWindowAttributes(DisplayOfScreen(m_screen), m_handle, &attributes);
		width = attributes.width;
		height = attributes.height;
    }

    void getClientCursorPos(short& x, short& y) const noexcept override
    {
        XID root, child;
        int root_x, root_y, win_x, win_y;
        unsigned int mask;
        XQueryPointer(DisplayOfScreen(m_screen), m_handle, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);
        x = win_x;
        y = win_y;
    }

    Title getTitle() const noexcept override
    {
        XTextProperty property{};
        XGetWMName(DisplayOfScreen(m_screen), m_handle, &property);
        switch (property.format)
        {
            case 8:
                return std::string(static_cast<char const*>(static_cast<void const*>(property.value)));
            case 16:
                return std::wstring(static_cast<wchar_t const*>(static_cast<void const*>(property.value)));
            default:
                return {};
        }
    }

    void setTitle(char const* title) const noexcept override
    {
        Display* display = DisplayOfScreen(m_screen);
        char* list[] { const_cast<char*>(title) };
        XTextProperty property;
        XmbTextListToTextProperty(display, list, 1, XStdICCTextStyle, &property);
        XSetWMName(display, m_handle, &property);
        XSetWMIconName(display, m_handle, &property);
        XFree(property.value);
        XFlush(display);
    }

    void setTitle(wchar_t const* title) const noexcept override
    {
        Display* display = DisplayOfScreen(m_screen);
        wchar_t* list[] { const_cast<wchar_t*>(title) };
		XTextProperty property;
		XwcTextListToTextProperty(display, list, 1, XStdICCTextStyle, &property);
		XSetWMName(display, m_handle, &property);
		XSetWMIconName(display, m_handle, &property);
		XFree(property.value);
		XFlush(display);
    }

	PWindow getParent() const override {
        Display *display = DisplayOfScreen(m_screen);
        XID root;
        XID parent;
        XID *children;
        unsigned int nchildren;
        XQueryTree(display, m_handle, &root, &parent, &children, &nchildren);
        XFree(children);

        if (parent == root) return PWindow(root_window);

        PWindow const *result = nullptr;
        XFindContext(display, parent, xUniqueContext(), (XPointer *) &result);

        return result == nullptr ? nullptr : PWindow(*result);
    }
};

typedef struct XRootWindow final : IWindow
{
    Screen* m_screen;

public:
    XRootWindow() noexcept
    {
        XInitThreads();
        Display* display = XOpenDisplay(nullptr);
        int screenId = DefaultScreen(display);
        m_screen = ScreenOfDisplay(display, screenId);
    }

    ~XRootWindow() override
    {
		XCloseDisplay(DisplayOfScreen(m_screen));
    }

    PWindow create(char const* title, int style, short width, short height) const override
    {
        return PWindow(XWindow::create(title, style, width, height, RootWindowOfScreen(m_screen), m_screen));
    }

    PWindow create(wchar_t const* title, int style, short width, short height) const override
    {
        return PWindow(XWindow::create(title, style, width, height, RootWindowOfScreen(m_screen), m_screen));
    }

    PWindow create(int style, short width, short height) const override
    {
        return PWindow(XWindow::create(static_cast<char const*>(nullptr), style, width, height, RootWindowOfScreen(m_screen), m_screen));
    }

	void show() const noexcept override {}

    void minimize() const noexcept override {}

    void hide() const noexcept override {}

    void close() const noexcept override {}

	bool isClosed() const noexcept override { return false; }

	bool isVisible() const noexcept override { return false; }

	bool isHidden() const noexcept override { return false; }

    bool isActive() const noexcept override
    {
        XID focus = 0;
        int revert_to;
        XGetInputFocus(DisplayOfScreen(m_screen), &focus, &revert_to);
        return focus == RootWindowOfScreen(m_screen);
    }

	void getClientSize(short& width, short& height) const noexcept override
	{
		width = m_screen->width;
		height = m_screen->height;
	}

    void getClientCursorPos(short& x, short& y) const noexcept override
    {
        XID root, child;
        int root_x, root_y, win_x, win_y;
        unsigned int mask;
        XQueryPointer(DisplayOfScreen(m_screen), RootWindowOfScreen(m_screen), &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);
		x = root_x;
		y = root_y;
    }

    Title getTitle() const noexcept override
    {
        return {};
    }

    void setTitle(char const* title) const noexcept override {}

    void setTitle(wchar_t const* title) const noexcept override {}

	PWindow getParent() const override { return root_window; }
} XRootWindow__;

EXTERN_C Window getRootWindow()
{
    if (root_window == nullptr)
    {
        root_window = std::make_shared<XRootWindow__>();
    }
    return *root_window;
}
