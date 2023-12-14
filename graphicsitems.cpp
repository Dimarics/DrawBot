#include "graphicsitems.h"
#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>

ContourImage::ContourImage(const QList<QList<QPointF>> &contours, qreal &scale) :
    m_block(false),
    m_originalWidth(0),
    m_originalHeight(0),
    m_scale(scale),
    m_originalContours(contours),
    m_leftTopField(new ResizeField(ResizeField::LeftTop, this)),
    m_topField(new ResizeField(ResizeField::Top, this)),
    m_rightTopField(new ResizeField(ResizeField::RightTop, this)),
    m_rightField(new ResizeField(ResizeField::Right, this)),
    m_rightBottomField(new ResizeField(ResizeField::RightBottom, this)),
    m_bottomField(new ResizeField(ResizeField::Bottom, this)),
    m_leftBottomField(new ResizeField(ResizeField::LeftBottom, this)),
    m_leftField(new ResizeField(ResizeField::Left, this))
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    m_leftTopField->setCursor(QCursor(Qt::SizeFDiagCursor));
    m_topField->setCursor(QCursor(Qt::SizeVerCursor));
    m_rightTopField->setCursor(QCursor(Qt::SizeBDiagCursor));
    m_rightField->setCursor(QCursor(Qt::SizeHorCursor));
    m_rightBottomField->setCursor(QCursor(Qt::SizeFDiagCursor));
    m_bottomField->setCursor(QCursor(Qt::SizeVerCursor));
    m_leftBottomField->setCursor(QCursor(Qt::SizeBDiagCursor));
    m_leftField->setCursor(QCursor(Qt::SizeHorCursor));

    qreal min_x = contours.at(0).at(0).x();
    qreal max_x = contours.at(0).at(0).x();
    qreal min_y = contours.at(0).at(0).y();
    qreal max_y = contours.at(0).at(0).y();
    for (QList<QPointF> &contour : m_originalContours)
    {
        for (QPointF &point : contour)
        {
            if (point.x() < min_x) min_x = point.x();
            if (point.x() > max_x) max_x = point.x();
            if (point.y() < min_y) min_y = point.y();
            if (point.y() > max_y) max_y = point.y();
        }
    }

    const qreal width = max_x - min_x;
    const qreal height = max_y - min_y;
    const qreal padding = 0;//(width > height ? width : height) * 0.03;
    m_originalWidth = width + padding * 2;
    m_originalHeight += height + padding * 2;

    for (QList<QPointF> &contour : m_originalContours)
    {
        for (QPointF &point : contour)
        {
            point.setX(point.x() - min_x + padding);
            point.setY(point.y() - min_y + padding);
        }
    }
    resize(m_originalWidth, m_originalHeight);
}

void ContourImage::resize(const qreal width, const qreal height)
{
    setRect(0.0, 0.0, width, height);
    m_contours.clear();
    for (QList<QPointF> &contour : m_originalContours)
    {
        m_contours << QList<QPointF>();
        for (QPointF &point : contour)
        {
            m_contours.last() << QPointF(point.x() * width / m_originalWidth,
                                         point.y() * height / m_originalHeight);
        }
    }
}

void ContourImage::scale(const qreal scale)
{
    resize(rect().width() * scale, rect().height() * scale);
}

void ContourImage::setBlocked(const bool block)
{
    m_block = block;
    update();
}

bool ContourImage::isBlocked() const
{
    return m_block;
}

QList<QList<QPointF>> ContourImage::contours() const
{
    return m_contours;
}

void ContourImage::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    const qreal width = rect().width();
    const qreal height = rect().height();
    m_leftTopField->setPlace(0, 0);
    m_topField->setPlace(width / 2.f, 0);
    m_rightTopField->setPlace(width, 0);
    m_rightField->setPlace(width, height / 2.f);
    m_rightBottomField->setPlace(width, height);
    m_bottomField->setPlace(width / 2.f, height);
    m_leftBottomField->setPlace(0, height);
    m_leftField->setPlace(0, height / 2.f);
    for (QGraphicsItem *item : childItems())
        if (item->type() == UserType + 1)
            item->setScale(1.f / m_scale);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(isBlocked() ? Qt::red : QColor(0, 170, 230), 0));

    for (QList<QPointF> &spline : m_contours)
    {
        painter->drawPolyline(spline);
    }

    if (isSelected())
    {
        QPen pen(QColor(0, 120, 215), 0);
        pen.setDashPattern(QList<qreal>{4.0, 4.0});
        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->drawRect(rect());
        for (QGraphicsItem *&field : childItems()) field->show();
    }
    else for (QGraphicsItem *&field : childItems()) field->hide();
}

int ContourImage::type() const
{
    return UserType;
}

//-----------------------------------------------
ResizeField::ResizeField(const quint8 type, ContourImage *parent) : QGraphicsRectItem(parent), m_type(type), m_parent(parent)
{
    const qreal side = 12.f;
    setRect(0, 0, side, side);
    setFlag(QGraphicsItem::ItemIsFocusable);
}

void ResizeField::setPlace(const qreal x, const qreal y)
{
    setPos(x - rect().width() * scale() / 2.f, y - rect().height() * scale() / 2.f);
}

void ResizeField::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    const qreal side = 4.f;
    painter->setPen(QPen(QColor(85, 85, 85), 0));
    painter->setBrush(Qt::white);
    painter->drawRect((rect().width() - side) / 2.f, (rect().width() - side) / 2.f, side, side);
}

void ResizeField::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_mousePos = event->scenePos();
    switch (m_type)
    {
    case LeftTop:
        m_mousePos += QPointF(m_parent->rect().width(), m_parent->rect().height());
        break;
    case Top:
        m_mousePos.setY(m_mousePos.y() + m_parent->rect().height());
        break;
    case RightTop:
        m_mousePos += QPointF(-m_parent->rect().width(), m_parent->rect().height());
        break;
    case Right:
        m_mousePos.setX(m_mousePos.x() - m_parent->rect().width());
        break;
    case RightBottom:
        m_mousePos -= QPointF(m_parent->rect().width(), m_parent->rect().height());
        break;
    case Bottom:
        m_mousePos.setY(m_mousePos.y() - m_parent->rect().height());
        break;
    case LeftBottom:
        m_mousePos += QPointF(m_parent->rect().width(), -m_parent->rect().height());
        break;
    case Left:
        m_mousePos.setX(m_mousePos.x() + m_parent->rect().width());
        break;
    default:
        break;
    }
    parentItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
    parentItem()->setSelected(true);
    //QGraphicsRectItem::mousePressEvent(event);
}

void ResizeField::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    qreal current_width = m_parent->rect().width();
    qreal current_height = m_parent->rect().height();
    qreal r1 = current_width / current_height;
    switch (m_type)
    {
    case LeftTop:
    {
        qreal r2 = (-event->scenePos().x() + m_mousePos.x()) / (-event->scenePos().y() + m_mousePos.y());
        qreal k = r1 > r2 ? (-event->scenePos().x() + m_mousePos.x()) / current_width :
                            (-event->scenePos().y() + m_mousePos.y()) / current_height;
        m_parent->setPos(m_parent->pos() - QPointF(current_width * (k - 1), current_height * (k - 1)));
        m_parent->scale(k);
        break;
    }
    case Top:
    {
        qreal k = (-event->scenePos().y() + m_mousePos.y()) / current_height;
        m_parent->setPos(m_parent->pos().x(), m_parent->pos().y() - current_height * (k - 1));
        m_parent->resize(current_width, current_height * k);
        break;
    }
    case RightTop:
    {
        qreal r2 = (event->scenePos().x() - m_mousePos.x()) / (-event->scenePos().y() + m_mousePos.y());
        qreal k = r1 > r2 ? (event->scenePos().x() - m_mousePos.x()) / current_width :
                            (-event->scenePos().y() + m_mousePos.y()) / current_height;
        m_parent->setPos(m_parent->pos().x(), m_parent->pos().y() - current_height * (k - 1));
        m_parent->scale(k);
        break;
    }
    case Right:
    {
        qreal k = (event->scenePos().x() - m_mousePos.x()) / current_width;
        m_parent->resize(current_width * k, current_height);
        //m_parent->setPos(m_mousePos.x(), m_parent->pos().y());
        break;
    }
    case RightBottom:
    {
        qreal r2 = (event->scenePos().x() - m_mousePos.x()) / (event->scenePos().y() - m_mousePos.y());
        qreal k = r1 > r2 ? (event->scenePos().x() - m_mousePos.x()) / current_width :
                            (event->scenePos().y() - m_mousePos.y()) / current_height;
        m_parent->scale(k);
        break;
    }
    case Bottom:
    {
        qreal k = (event->scenePos().y() - m_mousePos.y()) / current_height;
        m_parent->resize(current_width, current_height * k);
        break;
    }
    case LeftBottom:
    {
        qreal r2 = (-event->scenePos().x() + m_mousePos.x()) / (event->scenePos().y() - m_mousePos.y());
        qreal k = r1 > r2 ? (-event->scenePos().x() + m_mousePos.x()) / current_width :
                            (event->scenePos().y() - m_mousePos.y()) / current_height;
        m_parent->setPos(m_parent->pos().x() - current_width * (k - 1), m_parent->pos().y());
        m_parent->scale(k);
        break;
    }
    case Left:
    {
        qreal k = (-event->scenePos().x() + m_mousePos.x()) / current_width;
        m_parent->setPos(m_parent->pos().x() - current_width * (k - 1), m_parent->pos().y());
        m_parent->resize(current_width * k, current_height);
        break;
    }
    default:
        break;
    }
    //static_cast<GraphicsItem*>(parentItem())->transform(event->scenePos() - m_mousePos, this);
    QGraphicsRectItem::mouseMoveEvent(event);
}

void ResizeField::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
    parentItem()->setFlag(QGraphicsItem::ItemIsMovable);
}

int ResizeField::type() const
{
    return UserType + 1;
}
