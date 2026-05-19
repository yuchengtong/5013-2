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
        emit ProgressUpdated(5);

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }


        emit StatusUpdated("正在计算");
        emit ProgressUpdated(20);

        for (int i = 0; i < 1000; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            QThread::msleep(5);
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }

        

        emit StatusUpdated("正在计算");
        emit ProgressUpdated(50);

        for (int i = 0; i < 1000; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            QThread::msleep(5);
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }

      
       

        emit StatusUpdated("正在计算");
        emit ProgressUpdated(80);

        for (int i = 0; i < 1000; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            QThread::msleep(5);
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }

        

        emit StatusUpdated("正在计算");
        emit ProgressUpdated(90);

        for (int i = 0; i < 1000; ++i)
        {
            if (m_interrupted)
            {
                emit WorkFinished(false, "计算已取消");
                return;
            }
            QThread::msleep(5 );
        }

        if (m_interrupted)
        {
            emit WorkFinished(false, "计算已取消");
            return;
        }

        success = true;
        msg = "计算完成";
        emit ProgressUpdated(100);
    }
    catch (const Standard_Failure& e)
    {
        msg = QString("计算错误: %1").arg(e.GetMessageString());
        success = false;
    }
    catch (...)
    {
        msg = "计算时发生未知错误";
        success = false;
    }

    // 最终检查：如果在中途被取消，覆盖之前的成功状态
    if (m_interrupted)
    {
        emit WorkFinished(false, "计算已取消");
    }
    else
    {
        emit WorkFinished(success, msg);
    }
}


void ForwardDesignWorker::RequestInterruption()
{
    m_interrupted = true;
}


