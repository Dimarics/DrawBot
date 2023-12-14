#include "stackedwidget.h"

StackedWidget::StackedWidget(QWidget *parent) : QStackedWidget(parent){}

QSize StackedWidget::sizeHint() const
{
    return currentWidget()->sizeHint();
}

QSize StackedWidget::minimumSizeHint() const
{
    return currentWidget()->minimumSizeHint();
}
