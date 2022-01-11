#pragma once
#include "glad/glad.h"
#include "QtGui/qopenglwindow.h"
#include <iostream>
class OpenglWindow :public QOpenGLWindow
{
public:
	static OpenglWindow *Instance() {
		if (ptr == nullptr)
			return ptr = new OpenglWindow();
		else
			return ptr;
	};
protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;
private:
	static OpenglWindow *ptr;
	explicit OpenglWindow();
	long long lastFrameTime = 0;
	long long frameDelta = 0;
};

