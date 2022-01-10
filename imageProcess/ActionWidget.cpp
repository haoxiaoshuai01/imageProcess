#include "ActionWidget.h"
#include "QtWidgets/QVBoxLayout"
#include "QtWidgets/QHBoxLayout"
#include "QtWidgets/QSlider"
#include "QtWidgets/QLabel"
ActionWidget *ActionWidget::ptr = nullptr;
ActionWidget::ActionWidget()
{
	auto mainlayout = new QVBoxLayout;
	this->setLayout(mainlayout);

	auto layout1 = new QHBoxLayout;
	auto slider1 = new QSlider(Qt::Orientation::Horizontal);
	layout1->addWidget(new QLabel("Contrast"));
	layout1->addWidget(slider1);
	auto layout2 = new QHBoxLayout;
	auto slider2 = new QSlider(Qt::Orientation::Horizontal);
	layout2->addWidget(new QLabel("brightness"));
	layout2->addWidget(slider2);
	auto layout3 = new QHBoxLayout;
	auto slider3 = new QSlider(Qt::Orientation::Horizontal);
	layout3->addWidget(new QLabel("Color Enhance"));
	layout3->addWidget(slider3);
	
	mainlayout->addLayout(layout1);
	mainlayout->addLayout(layout2);
	mainlayout->addLayout(layout3);
	mainlayout->addSpacerItem(new QSpacerItem(1,1,QSizePolicy::Expanding, QSizePolicy::Expanding));
}
