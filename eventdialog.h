#pragma once

#include <QDialog>
#include <QListWidget>

class EventDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EventDialog(QWidget* parent=nullptr);
    QString eventId() const { return m_eventId; }
    void setEventId(const QString& id) { m_eventId = id; }
    void addEventPiece(const QString& name, const QString& pixResPath, int count);
private:
    QString m_eventId;
    QListWidget* list = nullptr;
};
