#ifndef DRAWINGMODE_H
#define DRAWINGMODE_H

#include <QWidget>
#include <QQueue>

QT_BEGIN_NAMESPACE
namespace Ui { class DrawingMode; }
QT_END_NAMESPACE

class DrawingMode : public QWidget
{
    Q_OBJECT
public:
    enum Tool
    {
        Pen,
        Laser
    };
    DrawingMode(QWidget* = nullptr);
    void setTool(Tool);

private:
    Ui::DrawingMode *ui;
    bool laser_active;
    QString m_pointsPath;
    QQueue<QString> m_commands;
    QString m_currentCommand;
    QFont m_font;
    Tool m_tool;

    void move();
    QList<QPointF> circle(const qreal = 1, const QPointF& = QPointF(), const quint16 = 500) const;

public slots:
    void readPort();
    void stop();
};
#endif // DRAWINGMODE_H
