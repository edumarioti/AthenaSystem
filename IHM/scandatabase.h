#ifndef SCANDATABASE_H
#define SCANDATABASE_H

#include <QThread>

class scanDataBase : public QThread{

    Q_OBJECT

public:
    explicit scanDataBase(QObject *parent = nullptr);

    void run();

signals:
    void verifyDataBase();

};

#endif // SCANDATABASE_H
