#pragma execution_character_set("utf-8")
#include "ForwardDesignWorker.h"

#include <QThread>
#include <cmath>

void ForwardDesignWorker::DoWork()
{
	bool success = false;
	QString msg;

	try
	{
		emit StatusUpdated("开始计算...");
		emit ProgressUpdated(0);

		if (m_interrupted)
		{
			emit WorkFinished(false, "计算已取消");
			return;
		}

		const int totalFrames = 30;

		for (int i = 1; i <= totalFrames; ++i)
		{
			if (m_interrupted)
			{
				emit WorkFinished(false, "计算已取消");
				return;
			}

			// 模拟每帧计算耗时（如有真实计算可替换）
			QThread::msleep(500);

			emit StatusUpdated(QString("正在计算第 %1 / %2 帧...").arg(i).arg(totalFrames));
			emit ProgressUpdated(i * 100 / totalFrames);
			emit FrameCalculated(i);  // 通知主线程第 i 帧已完成
		}

		if (m_interrupted)
		{
			emit WorkFinished(false, "计算已取消");
			return;
		}

		success = true;
		msg = QString("%1").arg(totalFrames);  // 返回总帧数
	}
	catch (...)
	{
		msg = "计算时发生未知错误";
		success = false;
	}

	if (!m_interrupted)
	{
		emit WorkFinished(success, msg);
	}
	else
	{
		emit WorkFinished(false, "计算已取消");
	}
}

void ForwardDesignWorker::RequestInterruption()
{
	m_interrupted = true;
}