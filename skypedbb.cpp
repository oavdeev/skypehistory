#include <QtGlobal>
#include <QtDebug>
#include <QDir>
#include <QRegExp>

#include "skypedbb.h"

namespace SkypeDbb {

    qint32 read_u32(QIODevice& device)
    {
        qint32 result = 0;
        device.read((char *)&result, sizeof(result));
        return result;
    }

    int read_byte(QIODevice& device)
    {
        unsigned char result = 0;
        device.read((char *)&result, sizeof(result));
        return result;
    }

    qint64 read_skype_int(QIODevice& device)
    {
        int b = read_byte(device);
        qint64 result = b;
        int shift = 0;
        while (b >= 0x80) {
            b = read_byte(device);
            shift += 7;
            result |= ((b & 0x7f) << shift);
        }
        return result;
    }

    QString read_string(QIODevice& device)
    {
        char c;

        device.read(&c, 1);
        QByteArray arr;
        while (c != 0) {
            arr.append(c);
            device.read(&c, 1);
        }
        return QString::fromUtf8(arr.data(), arr.size());
    }

    const char *getFieldName(int code, int type)
    {
        switch (code) {
        case 3: return "pk_id";
        case 7: return "crc";
        case 11: return "remote_id";
        case 480: return "chatname";
        case 485: return "timestamp";
        case 488: return "author";
        case 492: return "from_dispname";
        case 497: return "chatmsg_type";
        case 500: return "identities";
        case 505: return "leavereason";
        case 508: return "body_xml";
        case 513: return "chatmsg_status";
        case 517: return "body_is_rawxml";
        case 641: return "origin"; // # 2 for: return "me"; 4 for everyone else
        case 818: return "edited_by";
        case 893: return "edited_timestamp";
        case 3160: return "dialog_partner";
        case 3170: return "guid";
        case 3298: return "magic_guid";
        default: return 0;
        }
    }

    QSharedPointer<ChatHistoryItem> ChatHistoryItem::read(QIODevice& device, int record_size)
    {
        qint64 p = device.pos();
        QSharedPointer<ChatHistoryItem> result(new ChatHistoryItem());

        if (read_u32(device) != 'l33l')
        {
            qWarning() << "Invalid item magic value at " << p;
            return QSharedPointer<ChatHistoryItem>();
        }
        qint32 len = read_u32(device);
        result->m_seq = read_u32(device);
        device.read(5);

        qint32 end_pos = p + len + 8;
        int blob_len = 0;

        while (device.pos() < end_pos) {
            /* read record */
            int type = read_byte(device);
            int code = read_skype_int(device);

            QVariant val;
            switch (type) {
            case 0:
                val = QVariant(read_skype_int(device));
                break;
            case 3:
                val = QVariant(read_string(device));
                break;
            case 4:
                blob_len = read_skype_int(device);
                val = QVariant(device.read(blob_len));
                break;
            default:
                qWarning() << "Unknown record type " << type << " record at " << device.pos() << " item at " << p;
                device.seek(p + record_size + 8);
                return QSharedPointer<ChatHistoryItem>();
            }

            const char *fieldName = getFieldName(code, type);
            if (fieldName)
                result->m_fields[fieldName] = val;
            else
                result->m_fields[QString("(%1,%2)").arg(code).arg(type)] = val;
        }

        device.seek(p + record_size + 8);
        return result;
    }

    bool itemTimeOrder(QSharedPointer<ChatHistoryItem> lhs, QSharedPointer<ChatHistoryItem> rhs)
    {
        quint64 t1 = lhs->timestamp();
        quint64 t2 = rhs->timestamp();
        if (t1 == t2)
            return lhs->pk_id() < rhs->pk_id();
        else
            return t1 < t2;
    }

    void Chat::sort()
    {
        qSort(m_items.begin(), m_items.end(), itemTimeOrder);

        Q_FOREACH(QSharedPointer<ChatHistoryItem> item, m_items)
            m_participants.insert(item->author());
    }

    quint64 Chat::startDate() const
    {
        if (m_items.size() == 0)
            return 0;
        else
            return m_items[0]->timestamp();
    }

    int Chat::findItemText(int startIdx, const QString& text)
    {
        for (int i = startIdx+1; i < m_items.size(); i++)
            if (m_items[i]->text().contains(text, Qt::CaseInsensitive))
                return i;
        return -1;
    }

    bool Chat::containsText(const QString& text)
    {
        return findItemText(-1, text) != -1;
    }

    void Database::loadItems()
    {
        QDir skypeDir = QDir::home();
        skypeDir.cd(".Skype");
        Q_FOREACH(const QString& userDir, skypeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        {
            skypeDir.cd(userDir);
            qDebug() << "User dir " << skypeDir.absoluteFilePath(userDir);
            Q_FOREACH(const QString& dbbFileName, skypeDir.entryList(QDir::Files | QDir::NoDotAndDotDot))
            {
                qDebug() << "File " << skypeDir.absoluteFilePath(dbbFileName);
                QRegExp r1("chatmsg(\\d+)\\.dbb$");
                QRegExp r2("chat(\\d+)\\.dbb");

                int recordSize = 0;
                if (r1.exactMatch(dbbFileName))
                    recordSize = r1.cap(1).toInt();
//                else if (r2.exactMatch(dbbFileName))
//                    recordSize = r2.cap(1).toInt();
                else
                    continue;

                qDebug() << "Processing " << skypeDir.absoluteFilePath(dbbFileName);
                QFile dbbFile(skypeDir.absoluteFilePath(dbbFileName));
                dbbFile.open(QFile::ReadOnly);

                while (!dbbFile.atEnd())
                {
                    QSharedPointer<ChatHistoryItem> item = ChatHistoryItem::read(dbbFile, recordSize);
                    if (!item.isNull())
                    {
                        QString chatName = item->chatname();
                        if (!m_chats.contains(chatName))
                        {
                            QSharedPointer<Chat> c(new Chat());
                            m_chats.insert(chatName, c);
                        }
                        m_chats[chatName]->addItem(item);
                    }
                }
            }
            skypeDir.cdUp();
        }

        foreach (QSharedPointer<Chat> chat, m_chats)
            chat->sort();
    }

    QStringList Database::contacts()
    {
        QSet<QString> contacts;
        foreach (QSharedPointer<Chat> chat, m_chats)
        {
            contacts.unite(chat->participants());
        }

        return QStringList(contacts.toList());
    }
}
