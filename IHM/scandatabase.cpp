#include "scandatabase.h"
#include <QDebug>


scanDataBase::scanDataBase(QObject *parent) :
    QThread(parent){

}

void scanDataBase::run(){
    int inteiro = 0;
    while (true){
        inteiro++;
        qDebug() << inteiro;
        QThread::msleep(1000);
    }

}
