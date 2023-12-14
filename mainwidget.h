#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QMainWindow>
#include <QQueue>

namespace Ui {
class MainWidget;
}

class MainWidget : public QMainWindow
{
    Q_OBJECT
public:
    enum Tool
    {
        Pen,
        Laser
    };
    MainWidget(QWidget* = nullptr);
    void setTool(Tool);

private:
    Ui::MainWidget *ui;
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

#endif // MAINWIDGET_H
