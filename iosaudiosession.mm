#include "iosaudiosession.h"
#import <AVFoundation/AVFoundation.h>
#include <cmath>

int prepareIosAudioSession(QString &diagnostic)
{
    AVAudioSession *session = [AVAudioSession sharedInstance];
    NSError *error = nil;
    if (![session setCategory:AVAudioSessionCategoryPlayAndRecord
                 withOptions:AVAudioSessionCategoryOptionDefaultToSpeaker |
                             AVAudioSessionCategoryOptionAllowBluetooth
                       error:&error] || ![session setActive:YES error:&error]) {
        diagnostic = QString("iOS audio session failed: %1")
                         .arg(QString::fromNSString(error.localizedDescription));
        return 0;
    }
    diagnostic = QString("iOS audio session: %1 Hz, I/O buffer %2 ms, input %3, output %4")
        .arg(session.sampleRate).arg(session.IOBufferDuration * 1000)
        .arg(QString::fromNSString(session.currentRoute.inputs.firstObject.portName))
        .arg(QString::fromNSString(session.currentRoute.outputs.firstObject.portName));
    return int(std::lround(session.sampleRate));
}
