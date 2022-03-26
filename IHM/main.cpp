#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(QPixmap(":/imagens/logo-texto-branco-com-borda-1000px-350px.png"));
    splash->show();

    MainWindow window;

    QTimer::singleShot(3000, splash, SLOT(close()));
    QTimer::singleShot(3000, &window, SLOT(show()));


    return app.exec();
}
