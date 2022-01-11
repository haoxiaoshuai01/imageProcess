#include "OpenglWindow.h"
#include <sys/timeb.h>
long long systemtime()
{
	timeb t;
	ftime(&t);
	return t.time * 1000 + t.millitm;
}
OpenglWindow *OpenglWindow::ptr = nullptr;
void OpenglWindow::initializeGL()
{
	makeCurrent();
	if (!gladLoadGL())
	{
		std::cout << "init failed" << std::endl;
		return;
	}
	lastFrameTime = systemtime();
}
void OpenglWindow::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}
void OpenglWindow::paintGL()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	frameDelta = systemtime() - lastFrameTime;
	lastFrameTime = systemtime();
}
OpenglWindow::OpenglWindow()
{

}
