#ifndef CHATLISTMODEL_H
#define CHATLISTMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QStringList>
#include "skypedbb.h"

struct Snippet
{
    QString text;
    QSharedPointer<SkypeDbb::ChatHistoryItem> item;

    Snippet(QSharedPointer<SkypeDbb::ChatHistoryItem> i, const QString& t) : item(i), text(t) {}
};

struct SearchResult
{
    SearchResult(QSharedPointer<SkypeDbb::Chat> c) : chat(c), snippetsLoaded(false) {}

    bool snippetsLoaded;
    QSharedPointer<SkypeDbb::Chat> chat;
    QList<QSharedPointer<Snippet> > snippets;
};

class ChatListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ChatListModel(SkypeDbb::Database *database, QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent ( const QModelIndex & index ) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags ( const QModelIndex & index ) const;

    void setParticipantFilter(QStringList participants);
    void setTextFilter(const QString& filter) { textFilter = filter; }
    bool hasTextFilter() const { return !textFilter.isEmpty() && !textFilter.isNull(); }
    void filter();

    QSharedPointer<SkypeDbb::Chat> getChat(const QModelIndex& index);
    QSharedPointer<Snippet> getSnippet(const QModelIndex& index);
private:
    void loadSnippets(QSharedPointer<SearchResult> result) const;
    SkypeDbb::Database *database;
    QSet<QString> participantFilter;
    QString textFilter;
    QList<QSharedPointer<SearchResult> > filtered;
};

#endif // CHATLISTMODEL_H
