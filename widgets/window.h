#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

namespace Ui { class Window; }

class Window : public QWidget
{
    Q_OBJECT
public:
    Window(QWidget*, const QString& = QString(), const bool = false);
    ~Window();
    void setTitle(const QString&);
    void setIcon();
    void setSize(const QSize&);
    void setSize(const int, const int);
    void showMaximum(const bool);

private:
    Ui::Window *ui;
    QWidget* m_widget;
    QRect m_normalGeometry;
    bool eventFilter(QObject*, QEvent*);

signals:
    void finished();
};

#endif // WINDOW_H
