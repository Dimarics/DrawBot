#ifndef VOLUME_H
#define VOLUME_H

#include <QSpinBox>
#include <QSlider>

class Volume : public QWidget
{
    Q_OBJECT
public:
    Volume(QWidget* = nullptr);
    void setMinimum(const int);
    void setMaximum(const int);
    void setValue(const int, const bool = true);

    int value() const;
    int minimum() const;
    int maximum() const;

protected:
    bool eventFilter(QObject*, QEvent*) override;

private:
    QSpinBox *m_spinBox;
    QSlider *m_slider;

signals:
    void valueChanged(const int);
    void sendValue(const int);
};

#endif // VOLUME_H
