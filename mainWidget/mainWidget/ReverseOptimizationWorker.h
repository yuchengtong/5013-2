#ifndef REVERSE_OPTIMIZATION_WORKER_H
#define REVERSE_OPTIMIZATION_WORKER_H

#include <QObject>
#include <QString>
#include "ModelDataManager.h"

class ReverseOptimizationWorker : public QObject
{
    Q_OBJECT

public:
    explicit ReverseOptimizationWorker(QObject* parent = nullptr)
        : QObject(parent), m_interrupted(false) {}

public slots:
    void DoWork();

    void RequestInterruption();

signals:
    void ProgressUpdated(int progress);
    void StatusUpdated(const QString& status);
    void WorkFinished(bool success, const QString& msg);

private:
    volatile bool m_interrupted;

};

#endif // REVERSE_OPTIMIZATION_WORKER_H

