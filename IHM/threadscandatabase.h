#ifndef THREADSCANDATABASE_H
#define THREADSCANDATABASE_H

#include <QThread>
#include <QtSql>

class threadScanDatabase : public QThread
{
    Q_OBJECT
public:
    explicit threadScanDatabase(QObject *parent = nullptr);

    void run();

signals:
    void scanDatabase();

public slots:

};

#endif // THREADSCANDATABASE_H
