/**
 * Copyright (c) 2015      Los Alamos National Security, LLC
 *                         All rights reserved.
 *
 * This file is part of the Gladius project. See the LICENSE.txt file at the
 * top-level directory of this distribution.
 */

#ifndef TIMELINE_PROC_TIMELINE_H_INCLUDED
#define TIMELINE_PROC_TIMELINE_H_INCLUDED

#include "common.h"
#include "info-types.h"

#include <QList>
#include <QGraphicsItem>
#include <QPainter>

QT_BEGIN_NAMESPACE
class QRectF;
class QGraphicsView;
QT_END_NAMESPACE

////////////////////////////////////////////////////////////////////////////////
class TaskWidget : public QGraphicsItem {
public:
    TaskWidget(
        const TaskInfo &info
    ) : mInfo(info)
      , mWidth((mInfo.uStopTime - mInfo.uStartTime) / sMicroSecPerPixel) {
        // TODO Add Cache
        QString toolTip = "Start:" + QString::number(mInfo.uStartTime)
                        + " End: " + QString::number(mInfo.uStopTime);
        this->setToolTip(toolTip);
    }
    //
    QRectF boundingRect(void) const Q_DECL_OVERRIDE {
        return QRectF(0, 0, mWidth, sHeight);
    }
    //
    void
    paint(
        QPainter *painter,
        const QStyleOptionGraphicsItem * /* option */,
        QWidget * /* widget */) Q_DECL_OVERRIDE
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::black);
        painter->drawRect(boundingRect());
    }

    ustime_t
    getCreateTime(void) {
        return mInfo.uCreateTime;
    }

    ustime_t
    getReadyTime(void) {
        return mInfo.uReadyTime;
    }

    ustime_t
    getStartTime(void) {
        return mInfo.uStartTime;
    }

    ustime_t
    getStopTime(void) {
        return mInfo.uStopTime;
    }

private:
    //
    static constexpr qreal sHeight = 30;
    //
    TaskInfo mInfo;
    //
    qreal mWidth = 0.0;
};

////////////////////////////////////////////////////////////////////////////////
class ProcTimeline : public QGraphicsItem {
public:
    //
    ProcTimeline(
        ProcType procType,
        QGraphicsView *parent
    );
    //
    QRectF boundingRect(void) const Q_DECL_OVERRIDE;
    //
    void
    paint(
        QPainter *painter,
        const QStyleOptionGraphicsItem *option,
        QWidget *widget
    ) Q_DECL_OVERRIDE;
    //
    void
    addTask(const TaskInfo &info);

private:
    ProcType mProcType = ProcType::UNKNOWN;
    //
    QGraphicsView *mView = nullptr;
    //
    QRectF mBoundingRect;
    //
    QList<TaskWidget *> mTaskWidgets;
};

#endif // TIMELINE_PROC_TIMELINE_H_INCLUDED
