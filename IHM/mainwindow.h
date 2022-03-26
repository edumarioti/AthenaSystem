#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include "threadscandatabase.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    threadScanDatabase * mThread;

    float positionXInPx = 0;
    float positionYInPx = 0;


private slots:

    void SQLInsertComandFromHMI(QString comand, QString value);

    void changeStateManualAuto(bool newState);

    void messageInBottomFrame(bool isWarning, QString message);

    void on_buttonSystemInManual_clicked();

    void on_buttonSystemInManual_2_clicked();

    void on_buttonSystemInAuto_2_clicked();

    void on_buttonSystemInAuto_clicked();

    void on_buttonUpLeftManual_pressed();

    void on_buttonUpLeftManual_released();

    void on_buttonUpManual_pressed();

    void on_buttonUpManual_released();

    void on_buttonUpRightManual_pressed();

    void on_buttonUpRightManual_released();

    void on_buttonRightManual_pressed();

    void on_buttonRightManual_released();

    void on_buttonDownRightManual_pressed();

    void on_buttonDownRightManual_released();

    void on_buttonDownManual_pressed();

    void on_buttonDownManual_released();

    void on_buttonDownLeftManual_pressed();

    void on_buttonDownLeftManual_released();

    void on_buttonLeftManual_pressed();

    void on_buttonLeftManual_released();

    void on_buttonIncrementCoordinate_clicked();

    void on_buttonGoToPosition_clicked();

    void on_buttonConfirmEntry_clicked();

    void on_buttonCloseWindow_clicked();

    void on_buttonMinimizeWindow_clicked();

    void on_buttonConfirmManualDistance_clicked();

    void updateCurrentProductsBatabase();

    void updateProductsHistoryBatabase();

    void on_buttonPickUpAndDrop_clicked(bool checked);

    void on_buttonUpZManual_clicked();

    void on_buttonDownZManual_clicked();

    void on_buttonHomePosition_clicked();

    void on_buttonResetMachine_clicked();

    void on_buttonConfirmIDExit_clicked();

    void on_buttonConfirmOrderExit_clicked();

    void updateStatePoints(QStringList points);

public slots:

    void onScanDatabase();

private:
    Ui::MainWindow *ui;

    QSqlDatabase dataBase=QSqlDatabase::addDatabase("QMYSQL");

    void SQLScanFromDataBase();

};

#endif // MAINWINDOW_H
