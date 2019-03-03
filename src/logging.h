#ifndef LOGGING_H
#define LOGGING_H

// To be included only by globals.cpp and maintab_log.cpp.
// This object is needed to send log messages between threads (since the UI can be modified only by its own thread)

#include <QObject>
#include <string>


extern class LogEventEmitter : public QObject
{
    Q_OBJECT
signals:
    void requestAppend(QString) const;
public:
    void append(const std::string& text) const {
        emit requestAppend(QString::fromStdString(text));
    }
} g_logEventEmitter;


#endif // LOGGING_H
