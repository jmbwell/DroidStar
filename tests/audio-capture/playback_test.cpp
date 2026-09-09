#include "audioplaybackqueue.h"
#include <algorithm>
#include <iostream>

class LimitedWriter : public QIODevice {
public:
    LimitedWriter() { open(WriteOnly | Unbuffered); }
    qint64 limit = 3;
    QByteArray received;
    qint64 readData(char *, qint64) override { return -1; }
    qint64 writeData(const char *data, qint64 size) override {
        if(limit <= 0) return limit;
        const auto count = std::min(limit, size);
        received.append(data, count);
        return count;
    }
};

int main() {
    LimitedWriter device;
    const QByteArray original("0123456789abcdef");
    QByteArray pending = original;
    if(flushAudioPlayback(pending, device) != 3 || pending != original.mid(3)) return 1;
    for(qint64 limit : {0, -1}) {
        device.limit = limit;
        if(flushAudioPlayback(pending, device) != limit || pending != original.mid(3)) return 2;
    }
    device.limit = 3;
    while(!pending.isEmpty()) flushAudioPlayback(pending, device);
    if(device.received != original) return 3;
    std::cout << "Short, zero, and failed writes preserve PCM order and contents\n";
}
