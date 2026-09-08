#include "audiocaptureconverter.h"
#include <iostream>
#include <stdexcept>
#include <limits>

static void require(bool result, const char *message)
{
    if(!result) throw std::runtime_error(message);
}

static QAudioFormat format(int rate, int channels, QAudioFormat::SampleFormat type)
{
    QAudioFormat f;
    f.setSampleRate(rate);
    f.setChannelCount(channels);
    f.setSampleFormat(type);
    return f;
}

static QByteArray tone(int rate, int seconds, double frequency, int channels = 1, bool cancel = false)
{
    QByteArray data;
    for(int i = 0; i < rate * seconds; ++i){
        float value = 0.5 * std::sin(2 * std::acos(-1.0) * frequency * i / rate);
        for(int channel = 0; channel < channels; ++channel){
            const float sample = cancel && channel == 1 ? -value : value;
            data.append(reinterpret_cast<const char *>(&sample), sizeof(sample));
        }
    }
    return data;
}

static double rms(const QVector<int16_t> &data)
{
    double power = 0;
    for(qsizetype i = 100; i < data.size(); ++i) power += double(data[i]) * data[i];
    return std::sqrt(power / (data.size() - 100));
}

int main()
{
    try {
        for(int rate : {8000, 44100, 48000, 96000}){
            AudioCaptureConverter whole, chunked;
            auto f = format(rate, 2, QAudioFormat::Float);
            whole.reset(f); chunked.reset(f);
            const auto input = tone(rate, 5, 1000, 2);
            const auto expected = whole.append(input);
            QVector<int16_t> actual;
            for(qsizetype offset = 0, n = 1; offset < input.size(); ++n){
                const auto part = input.mid(offset, qMin(qsizetype((n * 137) % 1001 + 1), input.size() - offset));
                actual += chunked.append(part);
                offset += part.size();
            }
            require(actual.size() == 40000, "Five seconds must produce exactly 40000 samples");
            require(actual == expected, "Callback boundaries must not change output");
            require(chunked.inputFrames() == quint64(rate * 5), "Native frame count incorrect");
            // Check that the output frequency remains 1 kHz after conversion.
            int crossings = 0;
            for(int i = 801; i < 39200; ++i)
                if(actual[i - 1] <= 0 && actual[i] > 0) ++crossings;
            require(std::abs(crossings - 4800) <= 2, "Resampling changed tone frequency");
            chunked.reset(f);
            require(chunked.append(input) == expected, "Reset leaked prior capture state");
            std::cout << rate << " Hz stereo: duration, frequency, chunks, reset PASS\n";
        }
        AudioCaptureConverter converter;
        auto f = format(48000, 2, QAudioFormat::Float);
        converter.reset(f);
        require(rms(converter.append(tone(48000, 1, 1000, 2, true))) == 0, "Stereo must be mixed to mono");
        f.setChannelCount(1);
        converter.reset(f);
        const auto speechBand = rms(converter.append(tone(48000, 1, 1000)));
        converter.reset(f);
        const auto aliasBand = rms(converter.append(tone(48000, 1, 6000)));
        require(aliasBand < speechBand * 0.02, "Insufficient filtering above output Nyquist frequency");
        converter.reset(format(8000, 1, QAudioFormat::Int16));
        QByteArray integerInput;
        for(int i = -30000; i <= 30000; ++i){
            const int16_t value = i;
            integerInput.append(reinterpret_cast<const char *>(&value), sizeof(value));
        }
        const auto integerOutput = converter.append(integerInput);
        require(integerOutput.size() == 60001, "Int16 sample count incorrect");
        for(int i = 0; i < integerOutput.size(); ++i)
            require(std::abs(int(integerOutput[i]) - (i - 30000)) <= 1, "Int16 conversion changed level");
        std::cout << "Downmix, anti-alias filtering, Int16 levels PASS\n";
    } catch(const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
