#pragma once
#include "QtWidgets\qwidget.h"
class ActionWidget :public QWidget
{
public:
	static ActionWidget *Instance() {
		if (ptr == nullptr)
			return ptr = new ActionWidget();
		else
			return ptr;
	};

private:
	static ActionWidget *ptr;
	explicit ActionWidget();
};

