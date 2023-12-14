#include "sectionwidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QStylePainter>
#include <QStyleOptionButton>

SectionWidget::SectionWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    //layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void SectionWidget::addTab(const QString &text, QWidget *widget)
{
    layout()->addWidget(new QPushButton(text, this));
    layout()->addWidget(widget);
}

// вкладка
SectionWidgetTab::SectionWidgetTab(const QString &text, QWidget *parent) : QPushButton(text, parent){}

void SectionWidgetTab::paintEvent(QPaintEvent*)
{
    const qreal left = 15, top = 2, right = 10, bottom = 0;
    const QRect rect(left, top, width() - left - right, height() - top - bottom);

    QStylePainter painter(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    painter.drawControl(QStyle::CE_PushButtonBevel, option);
    painter.drawPrimitive(QStyle::PE_FrameFocusRect, option);
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text());
    painter.drawItemPixmap(rect, Qt::AlignRight | Qt::AlignVCenter, icon().pixmap(iconSize()));
}
