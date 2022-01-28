#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "previewglwindow.h"
class PreviewGlWidget : public QWidget
{
    Q_OBJECT

public:
    PreviewGlWidget(QWidget *parent = nullptr);
    ~PreviewGlWidget();
    Engine::PreviewGlWindow* getWindowGl(){return glwindow;};

//    void enterEvent(QEvent *event) override;
//    void leaveEvent(QEvent *event) override;
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    //bool eventFilter(QObject *obj, QEvent *ev) override;
    QWidget *openglWidget;
    Engine::PreviewGlWindow *glwindow;
};

#endif // WIDGET_H
