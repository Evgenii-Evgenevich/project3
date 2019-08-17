#include "Project3.hpp"
#include <chrono>
#include <thread>
#include "WindowInput/Window.hpp"

int main(int argc, char* argv[])
{
	Window root = getRootWindow();
	{
		PWindow window1 = root.create("window1", WSTYLE_POPUP);
		PWindow window2 = root.create("window2", WSTYLE_NOMINIMIZEBOX);

		window1->show();
		window2->show();
	}

	std::this_thread::sleep_for(std::chrono::seconds(25));

	return 0;
}
