#ifndef SKYPEDBB_H
#define SKYPEDBB_H

#include <QMap>
#include <QSet>
#include <QVector>
#include <QSharedPointer>
#include <QIODevice>
#include <QVariant>

namespace SkypeDbb {

    class ChatHistoryItem {
        QMap<QString, QVariant> m_fields;
        quint64 m_seq;

    public:
        const QString text() const { return m_fields["body_xml"].toString(); }
        int seq() const { return m_seq; }
        quint64 timestamp() const { return m_fields["timestamp"].toULongLong(); }
        const QString author() const { return m_fields["author"].toString(); }
        const QString chatname() const { return m_fields["chatname"].toString(); }
        quint64 pk_id() const { return m_fields["pk_id"].toULongLong(); }
        quint64 remote_id() const { return m_fields["remote_id"].toULongLong(); }
        QVariant operator[](const QString& key) const { return m_fields[key]; }

        static QSharedPointer<ChatHistoryItem> read(QIODevice& device, int record_size);
    };

    class Chat {
        QVector<QSharedPointer<ChatHistoryItem> > m_items;
        QSet<QString> m_participants;
    public:
        typedef QVector<QSharedPointer<ChatHistoryItem> >::const_iterator const_iterator;

        void addItem(QSharedPointer<ChatHistoryItem> item) {
            m_items.append(item);
        }
        void sort();
        QString name() { if (m_items.empty()) return "N/A"; else return m_items[0]->chatname(); }
        const QSet<QString>& participants() const { return m_participants; }
        quint64 startDate() const;

        const_iterator begin() { return m_items.constBegin(); }
        const_iterator end() { return m_items.constEnd(); }

        int count() const { return m_items.count(); }

        bool containsText(const QString& text);
        int findItemText(int startIdx, const QString& text);
        QSharedPointer<ChatHistoryItem> getItem(int index) { return (index < m_items.size()) ? m_items[index] : QSharedPointer<ChatHistoryItem>(); }
    };

    class Database {
        QMap<QString, QSharedPointer<Chat> > m_chats;
    public:
        typedef QMap<QString, QSharedPointer<Chat> >::const_iterator const_iterator;
        void loadItems();        
        const_iterator begin() { return m_chats.constBegin(); }
        const_iterator end() { return m_chats.constEnd(); }
        QStringList contacts();
    };
}
#endif // SKYPEDBB_H
