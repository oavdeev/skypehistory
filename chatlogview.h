#ifndef CHATLOGVIEW_H
#define CHATLOGVIEW_H

#include <QTreeView>
#include "skypedbb.h"

class ChatItemModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ChatItemModel(QObject *parent, QSharedPointer<SkypeDbb::Chat> ch) : QAbstractListModel(parent), chat(ch) {}
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
private:
    QSharedPointer<SkypeDbb::Chat> chat;
};

class ChatLogView : public QTreeView
{
    Q_OBJECT
public:
    ChatLogView(QWidget *parent, QSharedPointer<SkypeDbb::Chat> chat);

    void setPosition(QSharedPointer<SkypeDbb::ChatHistoryItem>& setPosition);
    void setHighlight(const QString& highlightText);
private:
    QSharedPointer<SkypeDbb::Chat> chat;

};

#endif // CHATLOGVIEW_H
