#ifndef AUDIOCAPTURECONVERTER_H
#define AUDIOCAPTURECONVERTER_H

#include <QAudioFormat>
#include <QByteArray>
#include <QVector>
#include <array>
#include <algorithm>
#include <cmath>

// Streaming native PCM -> mono 8 kHz PCM. All state survives callback boundaries.
class AudioCaptureConverter
{
public:
    void reset(const QAudioFormat &format)
    {
        m_format = format;
        m_pending.clear();
        m_history.fill(0);
        m_position = 0;
        m_phase = 0;
        m_previous = 0;
        m_frames = 0;
        const double pi = std::acos(-1.0);
        const double cutoff = std::min(3400.0 / std::max(1, format.sampleRate()), 0.45);
        double sum = 0;
        for(int i = 0; i < taps; ++i){
            const int x = i - (taps - 1) / 2;
            const double sinc = x == 0 ? 2 * cutoff : std::sin(2 * pi * cutoff * x) / (pi * x);
            m_filter[i] = sinc * (0.54 - 0.46 * std::cos(2 * pi * i / (taps - 1)));
            sum += m_filter[i];
        }
        for(auto &value : m_filter) value /= sum;
    }

    quint64 inputFrames() const { return m_frames; }

    QVector<int16_t> append(const QByteArray &data)
    {
        QVector<int16_t> output;
        if(!m_format.isValid()) return output;
        m_pending.append(data);
        const int frameBytes = m_format.bytesPerFrame();
        const qsizetype complete = m_pending.size() - m_pending.size() % frameBytes;
        for(qsizetype offset = 0; offset < complete; offset += frameBytes){
            double sample = 0;
            for(int channel = 0; channel < m_format.channelCount(); ++channel)
                sample += m_format.normalizedSampleValue(m_pending.constData() + offset + channel * m_format.bytesPerSample());
            sample /= m_format.channelCount();
            if(!std::isfinite(sample)) sample = 0;
            ++m_frames;
            double filtered = sample;
            if(m_format.sampleRate() != 8000){
                m_history[m_position] = sample;
                filtered = 0;
                for(int i = 0; i < taps; ++i)
                    filtered += m_filter[i] * m_history[(m_position + taps - i) % taps];
                m_position = (m_position + 1) % taps;
            }
            m_phase += 8000;
            while(m_phase >= m_format.sampleRate()){
                m_phase -= m_format.sampleRate();
                const double fraction = 1.0 - double(m_phase) / 8000;
                const double value = m_previous + fraction * (filtered - m_previous);
                output.append(int16_t(std::lround(std::clamp(value, -1.0, 1.0) * 32767)));
            }
            m_previous = filtered;
        }
        m_pending.remove(0, complete);
        return output;
    }

private:
    static constexpr int taps = 127;
    QAudioFormat m_format;
    QByteArray m_pending;
    std::array<double, taps> m_filter{};
    std::array<double, taps> m_history{};
    int m_position = 0;
    qint64 m_phase = 0;
    double m_previous = 0;
    quint64 m_frames = 0;
};
#endif
