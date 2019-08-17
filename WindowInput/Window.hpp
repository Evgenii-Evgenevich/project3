#ifndef __WINDOW_HPP
#define __WINDOW_HPP 1

#include "../common.h"
#include <memory>
#include <string>

#define WSTYLE_DEFAULT 0x0000
#define WSTYLE_NOBORDER 0x0001
#define WSTYLE_NODLGFRAME 0x0002
#define WSTYLE_NOTHICKFRAME 0x0010
#define WSTYLE_NOMINIMIZEBOX 0x0100
#define WSTYLE_NOMAXIMIZEBOX 0x0200
#define WSTYLE_NOCLOSEBOX 0x0400

#define WSTYLE_POPUP (WSTYLE_NODLGFRAME | WSTYLE_NOTHICKFRAME | WSTYLE_NOMINIMIZEBOX | WSTYLE_NOMAXIMIZEBOX | WSTYLE_NOCLOSEBOX)

/**
 * Window Ref
 */
using Window = struct IWindow const&;
/**
 * Window Ptr
 */
using PWindow = std::shared_ptr<struct IWindow const>;

/**
 * Window Interface
 */
struct IWindow
{
	virtual ~IWindow() {}

	/**
	 * create a child window of a window
	 * 
	 * @param title[in] the window title in ASCII
	 * @param style[in] the window style @see WSTYLE_DEFAULT, WSTYLE_*
	 * @param width[in] the window width
	 * @param height[in] the window height
	 * @return a pointer of the created window
	 */
	virtual PWindow create(char const* title, int style = WSTYLE_DEFAULT, short width = 640, short height = 480) const = 0;

	/**
	 * create a child window of a window
	 *
	 * @param title[in] the window title in ASCII
	 * @param style[in] the window style @see WSTYLE_DEFAULT, WSTYLE_*
	 * @param width[in] the window width
	 * @param height[in] the window height
	 * @return a pointer of the created window
	 */
	PWindow create(std::string const& title, int style = WSTYLE_DEFAULT, short width = 640, short height = 480) const
	{
		return create(title.c_str(), style, width, height);
	}

	/**
	 * create a child window of a window
	 *
	 * @param title[in] the window title in Unicode
	 * @param style[in] the window style @see WSTYLE_DEFAULT, WSTYLE_*
	 * @param width[in] the window width
	 * @param height[in] the window height
	 * @return a pointer of the created window
	 */
	virtual PWindow create(wchar_t const* title, int style = WSTYLE_DEFAULT, short width = 640, short height = 480) const = 0;

	/**
	 * create a child window of a window
	 *
	 * @param title[in] the window title in Unicode
	 * @param style[in] the window style @see WSTYLE_DEFAULT, WSTYLE_*
	 * @param width[in] the window width
	 * @param height[in] the window height
	 * @return a pointer of the created window
	 */
	PWindow create(std::wstring const& title, int style = WSTYLE_DEFAULT, short width = 640, short height = 480) const
	{
		return create(title.c_str(), style, width, height);
	}

	/**
	 * create a child window of a window
	 *
	 * @param style[in] the window style @see WSTYLE_DEFAULT, WSTYLE_*
	 * @param width[in] the window width
	 * @param height[in] the window height
	 * @return a pointer of the created window
	 */
	virtual PWindow create(int style = WSTYLE_DEFAULT, short width = 640, short height = 480) const = 0;

	virtual void show() const noexcept = 0;

	virtual void minimize() const noexcept = 0;

	virtual void hide() const noexcept = 0;

	virtual void close() const noexcept = 0;

	virtual bool isClosed() const noexcept = 0;

	virtual bool isVisible() const noexcept = 0;

	virtual bool isHidden() const noexcept = 0;

	virtual bool isActive() const noexcept = 0;

	/**
	 * @param width[out] get the width of the client area of a window
	 * @param height[out] get the height of the client area of a window
	 */
	virtual void getClientSize(short& width, short& height) const noexcept = 0;

	/**
	 * @param x[out] get the x-coordinate of the client area cursor position
	 * @param y[out] get the y-coordinate of the client area cursor position
	 */
	virtual void getClientCursorPos(short& x, short& y) const noexcept = 0;

	struct Title
	{
		bool is_unicode;
		union { std::string ascii; std::wstring unicode; };
		Title() noexcept : is_unicode(false), ascii() {}
		Title(Title&& o) noexcept : is_unicode(o.is_unicode)
		{
			if (is_unicode) new(&unicode) std::wstring(static_cast<std::wstring&&>(o.unicode));
			else new(&ascii) std::string(static_cast<std::string&&>(o.ascii));
		}
		Title(std::string&& string) noexcept : is_unicode(false), ascii(static_cast<std::string&&>(string)) {}
		Title(std::wstring&& string) noexcept : is_unicode(true), unicode(static_cast<std::wstring&&>(string)) {}
		~Title() noexcept { if (is_unicode) unicode.~basic_string(); else ascii.~basic_string(); }
	};
	/**
	 * get the title of a window
	 * @return is_unicode ? unicode : ascii 
	 */
	virtual Title getTitle() const noexcept = 0;

	/**
	 * @param title[in] the new title, in ASCII, of a window
	 */
	virtual void setTitle(char const* title) const noexcept = 0;

	/**
	 * @param title[in] the new title, in ASCII, of a window
	 */
	void setTitle(std::string const& title) const noexcept { setTitle(title.c_str()); }

	/**
	 * @param title[in] the new title, in Unicode, of a window
	 */
	virtual void setTitle(wchar_t const* title) const noexcept = 0;

	/**
	 * @param title[in] the new title, in Unicode, of a window
	 */
	void setTitle(std::wstring const& title) const noexcept { setTitle(title.c_str()); }

	/**
	 * @return a pointer to the parent of a window
	 */
	virtual PWindow getParent() const = 0;

};

/**
 * @return the ref to a root window
 */
EXTERN_C Window getRootWindow();

#endif // !__WINDOW_HPP

