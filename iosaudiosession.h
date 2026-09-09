#pragma once
#include <QString>

// Activate the route before querying its hardware rate; Qt's iOS default is fixed.
int prepareIosAudioSession(QString &diagnostic);
