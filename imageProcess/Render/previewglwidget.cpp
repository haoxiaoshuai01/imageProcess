#include "previewglwidget.h"
#include <QHBoxLayout>

#include <QKeyEvent>
#include "qdebug.h"
#include "QSpacerItem"

PreviewGlWidget::PreviewGlWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setLayout(new QHBoxLayout());
    glwindow = Engine::PreviewGlWindow::Instance();
    openglWidget = glwindow;
    this->layout()->setMargin(5);

    this->layout()->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Expanding));
    this->layout()->addWidget(openglWidget);
    this->layout()->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Expanding));
    //openglWidget->installEventFilter(this);
//    glwindow->installEventFilter(this);
}
//bool PreviewGlWidget::eventFilter(QObject *obj, QEvent *event)
// {
//     if (obj == glwindow) {
//         if(event->type() == QEvent::Type::Leave)
//         {
//            glwindow->cancelSele();
//         }
//         return QWidget::eventFilter(obj, event);
//     } else {
//         // pass the event on to the parent class
//         return QWidget::eventFilter(obj, event);
//     }
// }
PreviewGlWidget::~PreviewGlWidget()
{

}

//void PreviewGlWidget::enterEvent(QEvent *event)
//{
//    //qDebug()<<event;
//}

//void PreviewGlWidget::leaveEvent(QEvent *event)
//{
// //qDebug()<<event;
//}

void PreviewGlWidget::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    QSize openglwidgetSize ;
    float ratio = ((float)size.height()/(float)size.width());
    if( ratio<(16.0f/9.0f))
    {
        openglwidgetSize.setWidth((float)size.height()/(16.0f/9.0f));
        openglwidgetSize.setHeight(size.height());
    }else
    {
         openglwidgetSize.setWidth(size.width());
         openglwidgetSize.setHeight((float)size.width() * (16.0f/9.0f));
    }
    openglWidget->setMinimumSize(openglwidgetSize);
    openglWidget->setMaximumSize(openglwidgetSize);
    QWidget::resizeEvent(event);
}


