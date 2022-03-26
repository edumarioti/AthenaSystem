#include "threadscandatabase.h"
#include <QtCore/QtCore>

threadScanDatabase::threadScanDatabase(QObject *parent) :
    QThread (parent){

}

void threadScanDatabase::run(){

    while(true){

        this->msleep(50);
        emit scanDatabase();

    }
}
