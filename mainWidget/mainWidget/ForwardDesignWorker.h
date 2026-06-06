#ifndef FORWARD_DESIGN_WORKER_H
#define FORWARD_DESIGN_WORKER_H

#include <QObject>
#include <QString>
#include "ModelDataManager.h"

class ForwardDesignWorker : public QObject
{
	Q_OBJECT

public:
	explicit ForwardDesignWorker(QObject* parent = nullptr)
		: QObject(parent), m_interrupted(false)
	{
	}

public slots:
	void DoWork();
	void RequestInterruption();

signals:
	void ProgressUpdated(int progress);
	void StatusUpdated(const QString& status);
	void WorkFinished(bool success, const QString& msg);
	void FrameCalculated(int frameIndex);  // 新增：每帧计算完成信号

private:
	volatile bool m_interrupted;
};

#endif // FORWARD_DESIGN_WORKER_H