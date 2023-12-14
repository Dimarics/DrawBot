#include "widgets/window.h"
//#include "drawingmode.h"
#include "mainwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Window window(new MainWidget, "Draw Maker");
    window.showMaximum(true);
    window.show();

    return app.exec();
}
