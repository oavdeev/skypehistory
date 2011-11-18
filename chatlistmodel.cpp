#include <QDateTime>
#include "chatlistmodel.h"

ChatListModel::ChatListModel(SkypeDbb::Database *database, QObject *parent) :
        QAbstractItemModel(parent)
{
    this->database = database;
}

int ChatListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (parent.parent().isValid())
            return 0;

        QSharedPointer<SearchResult> res = filtered[parent.row()];
        loadSnippets(res);
        return res->snippets.size();
    }
    else
        return filtered.size();
}

void ChatListModel::loadSnippets(QSharedPointer<SearchResult> result) const
{
    if (result->snippetsLoaded)
        return;

    QSharedPointer<SkypeDbb::Chat> chat = result->chat;

    Q_ASSERT(hasTextFilter());

    int i = chat->findItemText(-1, textFilter);
    while (i != -1)
    {
        QSharedPointer<SkypeDbb::ChatHistoryItem> item = chat->getItem(i);
        result->snippets.append(QSharedPointer<Snippet>(new Snippet(item, item->text())));
        i = chat->findItemText(i, textFilter);
    }
    result->snippetsLoaded = true;
}

int ChatListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 3;
    else
        return 2;
}

Qt::ItemFlags ChatListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ChatListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    int row = index.row();
    int column = index.column();
    QDateTime t;

    if (index.parent().isValid()) {
        /* snippet */
        if (index.parent().parent().isValid())
            return QVariant();

        QSharedPointer<SearchResult> res = filtered[index.parent().row()];
        loadSnippets(res);
        if (row >= res->snippets.size())
            return QVariant();

        QSharedPointer<Snippet> snippet = res->snippets[row];
        switch (column) {
        case 0:
            t.setTime_t(snippet->item->timestamp());
            return t.toString("hh:mm dd/MMM ") + snippet->item->author();
        case 1:
            return snippet->text;
        }
    } else {
        /* root item */
        if (row >= filtered.size())
            return QVariant();
        else {
            switch (column) {
            case 0:
                t.setTime_t(filtered[row]->chat->startDate());
                return t.toString("hh:mm dd/MMM/yy");
            case 1:
                return filtered[row]->chat->name();
            }
        }
    }
    return QVariant();
}

QSharedPointer<SkypeDbb::Chat> ChatListModel::getChat(const QModelIndex& index)
{
    int resIdx = (index.internalId() >> 16);
    if (resIdx >= filtered.size())
        return QSharedPointer<SkypeDbb::Chat>();
    return filtered[resIdx]->chat;
}

QSharedPointer<Snippet> ChatListModel::getSnippet(const QModelIndex& index)
{
    int resIdx = (index.internalId() >> 16);
    int snipIdx = (index.internalId() & 0x7fff);

    if (resIdx >= filtered.size())
        return QSharedPointer<Snippet>();

    if (snipIdx == 0 || snipIdx > filtered[resIdx]->snippets.size())
        return QSharedPointer<Snippet>();
    else
        return filtered[resIdx]->snippets[snipIdx - 1];

}

QModelIndex ChatListModel::index ( int row, int column, const QModelIndex & parent ) const
{
    if (!parent.isValid())
        return createIndex(row, column, (row << 16));
    else
        return createIndex(row, column, (quint32)(parent.internalId() + column + 1));
}

QModelIndex ChatListModel::parent ( const QModelIndex & index ) const
{
    if (!index.isValid())
        return QModelIndex();

    int resIdx = (index.internalId() >> 16);
    int snipIdx = (index.internalId() & 0x7fff);
    if (snipIdx == 0)
        return QModelIndex();
    else
        return createIndex(resIdx, 0, resIdx << 16);
}

QVariant ChatListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section) {
        case 0:
            return "Date";
        case 1:
            return "Name";
        case 2:
            return "Text";
        }
    }

    return QVariant();
}

bool chatDateCompare(const QSharedPointer<SearchResult>& lhs, const QSharedPointer<SearchResult>& rhs)
{
    return lhs->chat->startDate() > rhs->chat->startDate();
}

void ChatListModel::filter()
{
    emit layoutAboutToBeChanged();

    filtered.clear();
    SkypeDbb::Database::const_iterator i = database->begin();
    while (i != database->end())
    {
        QSharedPointer<SkypeDbb::Chat> chat = (*i);
        i++;

        bool filterOut = false;

        foreach (const QString& p, participantFilter)
        {
            if (!chat->participants().contains(p))
            {
                filterOut = true;
                break;
            }
        }

        if (hasTextFilter() && !chat->containsText(textFilter))
            filterOut = true;

        if (!filterOut)
        {
            QSharedPointer<SearchResult> res(new SearchResult(chat));
            if (!hasTextFilter())
                res->snippetsLoaded = true;
            filtered.append(res);
        }
    }

    qSort(filtered.begin(), filtered.end(), chatDateCompare);
    emit layoutChanged();
}

void ChatListModel::setParticipantFilter(QStringList participants)
{
    participantFilter.clear();
    foreach (const QString& p, participants)
        participantFilter.insert(p);
}
