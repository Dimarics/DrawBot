#include "paintwidget.h"
#include "graphicsitems.h"

#include <QKeyEvent>

PaintWidget::PaintWidget(QWidget *parent) : QGraphicsView(parent), m_scale(1)
{
    m_canvasWidth = 210;
    m_canvasHeight = 297;
    setScene(new QGraphicsScene);
    setSceneRect(0, 0, m_canvasWidth, m_canvasHeight);
    //setBackgroundBrush(QPixmap(":/images/background.jpg").scaledToHeight(1500));
}

void PaintWidget::scale(const qreal scale)
{
    m_scale *= scale;
    QGraphicsView::scale(scale, scale);
}

void PaintWidget::addImage(QList<QList<QPointF>> contours)
{
    ContourImage *image = new ContourImage(contours, m_scale);
    const qreal width = image->rect().width();
    const qreal height = image->rect().height();
    const qreal k = (m_canvasWidth > m_canvasHeight ? m_canvasHeight : m_canvasWidth) /
                    (width > height ? width : height) * 0.8;
    image->setPos((m_canvasWidth - width * k) / 2.f, (m_canvasHeight - height * k) / 2.f);
    image->scale(k);
    scene()->addItem(image);
}

void PaintWidget::removeItem()
{
    QGraphicsItem *item = scene()->focusItem();
    if (item)
    {
        switch (item->type())
        {
        case QGraphicsItem::UserType:
            scene()->removeItem(item);
            delete item;
            break;
        case QGraphicsItem::UserType + 1:
        {
            QGraphicsItem *deleteItem = item->parentItem();
            scene()->removeItem(deleteItem);
            delete deleteItem;
            break;
        }
        default:
            break;
        }
    }
}

void PaintWidget::drawBackground(QPainter *painter, const QRectF&)
{
    //QGraphicsView::drawBackground(painter, rect);
    painter->fillRect(sceneRect(), Qt::white);
}

void PaintWidget::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsItem *item = scene()->mouseGrabberItem();
    if (item)
    {
        switch (item->type())
        {
        case QGraphicsItem::UserType:
            static_cast<ContourImage*>(item)->setBlocked(!sceneRect().contains(item->boundingRect().translated(item->scenePos())));
            break;
        case QGraphicsItem::UserType + 1:
        {
            ContourImage *image = static_cast<ContourImage*>(item->parentItem());
            image->setBlocked(!sceneRect().contains(image->boundingRect().translated(image->scenePos())));
        }
            //static_cast<ContourImage*>(item->parentItem())->setBlocked(m_workingArea.contains(item->parentItem()->shape().translated(item->parentItem()->pos())) ? false : true);
            break;
        default:
            break;
        }
    }
    QGraphicsView::mouseMoveEvent(event);
}

void PaintWidget::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
}

void PaintWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
        removeItem();
    QGraphicsView::keyPressEvent(event);
}

QList<QList<QPointF>> PaintWidget::points() const
{
    QList<QList<QPointF>> lines;
    for (QGraphicsItem *&item : scene()->items())
    {
        if (item && item->type() == QGraphicsItem::UserType && !static_cast<ContourImage*>(item)->isBlocked())
        {
            QList<QList<QPointF>> contours = static_cast<ContourImage*>(item)->contours();
            for (QList<QPointF> &contour : contours)
            {
                if (contour.size() > 1)
                {
                    lines << QList<QPointF>();
                    for (QPointF &point : contour)
                    {
                        point += item->pos();
                        //if (lines.last().isEmpty() || (!lines.last().isEmpty() && sqrt(pow(lines.last().last().x() - point.x(), 2) + pow(lines.last().last().y() - point.y(), 2)) > 0.2))
                        if (lines.last().isEmpty() || (!lines.last().isEmpty() && (point - lines.last().last()).manhattanLength() >= 0.2))
                        {
                            lines.last() << point;
                        }
                    }
                    if (lines.last().size() < 2) lines.removeLast();
                }
            }
            //lines += contours;
        }
    }
    return lines;
}
