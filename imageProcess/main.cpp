#include <iostream>
#include <QtWidgets/qapplication.h>
#include "QtWidgets/qwidget.h"
#include "QtGui/qsurfaceformat.h"
#include "QtWidgets/QHBoxLayout"
#include "OpenglWindow.h"
#include "ActionWidget.h"



int main(int argc, char *argv[])
{
	QSurfaceFormat qsf;
	qsf.setProfile(QSurfaceFormat::OpenGLContextProfile::CoreProfile);
	qsf.setRenderableType(QSurfaceFormat::RenderableType::OpenGL);
	qsf.setVersion(4, 3);
	qsf.setSwapInterval(0);
	QSurfaceFormat::setDefaultFormat(qsf);

	QApplication app(argc, argv);
	QWidget mainWidget;
	QHBoxLayout layout;
	auto window = OpenglWindow::Instance();
	layout.addWidget(QWidget::createWindowContainer(window),3);
	layout.addWidget(ActionWidget::Instance(),1);

	mainWidget.setLayout(&layout);
	mainWidget.resize(1000, 800);
	mainWidget.show();

	return app.exec();
}
