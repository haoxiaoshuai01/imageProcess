#include <iostream>
#include <QtWidgets/qapplication.h>
#include "QtWidgets/qwidget.h"

#include "QtGui/qsurfaceformat.h"
//#include "QtGui/qopenglwindow.h"
//#include "QtWidgets/qwidget.h"


int main(int argc, char *argv[])
{
	QSurfaceFormat qsf;
	qsf.setProfile(QSurfaceFormat::OpenGLContextProfile::CoreProfile);
	qsf.setRenderableType(QSurfaceFormat::RenderableType::OpenGL);
	qsf.setVersion(4, 3);
	qsf.setSwapInterval(0);
	QSurfaceFormat::setDefaultFormat(qsf);

	QApplication app(argc, argv);
	QWidget window;
	window.resize(100, 100);
	window.show();

	return app.exec();
}
