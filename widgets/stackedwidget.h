#ifndef STACKEDWIDGET_H
#define STACKEDWIDGET_H

#include <QStackedWidget>

class StackedWidget : public QStackedWidget
{
    Q_OBJECT
public:
    StackedWidget(QWidget* = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
};

#endif // STACKEDWIDGET_H
