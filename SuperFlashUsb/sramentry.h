#ifndef SRAMENTRY_H
#define SRAMENTRY_H

#include "utility.h"
#include <QtWidgets>
#include <QTextEdit>

class SramEntry : public QTextEdit
{
    Q_OBJECT
public:
    explicit SramEntry(QWidget *parent = 0);

signals:

public slots:

protected:
    //bool	canInsertFromMimeData(const QMimeData * source) const;
    //void	insertFromMimeData(const QMimeData * source);
    void dragEnterEvent(QDragEnterEvent * event);
    void dropEvent(QDropEvent * event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent * event);
    void contextMenuEvent(QContextMenuEvent * event);


public:
    SRAM sram;

};

#endif // SRAMENTRY_H