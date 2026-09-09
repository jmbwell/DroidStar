#pragma once
#include <QByteArray>
#include <QIODevice>

// A short write consumes only its accepted prefix; zero/error retains everything.
inline qint64 flushAudioPlayback(QByteArray &pending, QIODevice &device)
{
    if(pending.isEmpty()) return 0;
    const qint64 accepted = device.write(pending.constData(), pending.size());
    if(accepted > 0) pending.remove(0, accepted);
    return accepted;
}
