#pragma once
#include "QtGui/qopenglwindow.h"
class OpenglWindow :public QOpenGLWindow
{
public:
	static OpenglWindow *Instance() {
		if (ptr == nullptr)
			return ptr = new OpenglWindow();
		else
			return ptr;
	};

private:
	static OpenglWindow *ptr;
	explicit OpenglWindow();
};

