#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QGraphicsView>
#include <QPainterPath>

class PaintWidget : public QGraphicsView
{
    Q_OBJECT

public:
    PaintWidget(QWidget* = nullptr);
    void scale(const qreal);
    QList<QList<QPointF>> points() const;

protected:
    void drawBackground(QPainter *painter, const QRectF&) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private:
    qreal m_canvasWidth;
    qreal m_canvasHeight;
    qreal m_scale;

public slots:
    void addImage(QList<QList<QPointF>>);
    void removeItem();
};

#endif // PAINTWIDGET_H
