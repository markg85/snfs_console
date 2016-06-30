#ifndef BENCHMARKJOB_H
#define BENCHMARKJOB_H

#include <QObject>

class BenchmarkJob : public QObject
{
    Q_OBJECT
public:
    explicit BenchmarkJob(QObject *parent = 0);

signals:

public slots:
};

#endif // BENCHMARKJOB_H
