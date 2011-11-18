#include <QDateTime>

#include "chatlogview.h"

int ChatItemModel::rowCount ( const QModelIndex & parent ) const
{
    return chat->count();
}

int ChatItemModel::columnCount ( const QModelIndex & parent ) const
{
    return 3;
}

QVariant ChatItemModel::data ( const QModelIndex & index, int role ) const
{
    QSharedPointer<SkypeDbb::ChatHistoryItem> item = chat->getItem(index.row());

    if ( index.isValid() && role == Qt::BackgroundRole )
    {
        if ((*item)["origin"].toInt() == 2)
            return QVariant( QColor( "#EEEEEE") );
        else
            return  QVariant( QColor( Qt::white ) );
    }


    if (role != Qt::DisplayRole)
        return QVariant();

    if (item.isNull())
        return QVariant();

    QDateTime t;

    switch (index.column())
    {
    case 0:
        t.setTime_t(item->timestamp());
        return t.toString("hh:mm");
    case 1:
        return item->author();
    case 2:
        return item->text();
    }
}

QVariant ChatItemModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    switch (section) {
    case 0:
        return "time";
    case 1:
        return "message";
    case 2:
        return "message";
    }
}

ChatLogView::ChatLogView(QWidget *parent, QSharedPointer<SkypeDbb::Chat> chat) :
        QTreeView(parent)
{
    this->setModel(new ChatItemModel(this, chat));
    this->chat = chat;    
    this->setUniformRowHeights(false);
    this->setWordWrap(true);
    this->setHeaderHidden(false);
}

void ChatLogView::setPosition(QSharedPointer<SkypeDbb::ChatHistoryItem>& item)
{
    int row = 0;

    for (SkypeDbb::Chat::const_iterator i = chat->begin(); i != chat->end(); i++)
        if (*i == item)
            break;
        else
            row++;

    this->scrollTo(model()->index(row, 0, QModelIndex()));
}

void ChatLogView::setHighlight(const QString& highlightText)
{
}
