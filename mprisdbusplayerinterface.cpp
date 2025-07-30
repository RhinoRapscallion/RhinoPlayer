#include "mprisdbusplayerinterface.h"

MprisDBusPlayerSignaler::MprisDBusPlayerSignaler(QObject *parent) : QObject(parent) {}

MprisDBusPlayerInterface::MprisDBusPlayerInterface(QObject *parent)
    : QDBusAbstractAdaptor{parent}
{}

QString                 MprisDBusPlayerInterface::getPlaybackStatus () { return playbackStatus ;}
QString                 MprisDBusPlayerInterface::getLoopStatus     () { return loopStatus     ;}
double                  MprisDBusPlayerInterface::getPlaybackRate   () { return playbackRate   ;}
bool                    MprisDBusPlayerInterface::getShuffle        () { return shuffle        ;}
QMap<QString, QVariant> MprisDBusPlayerInterface::getMetadata       () { return metadata       ;}
double                  MprisDBusPlayerInterface::getVolume         () { return volume         ;}
qlonglong               MprisDBusPlayerInterface::getPosition       () { return position       ;}
double                  MprisDBusPlayerInterface::getMinimumRate    () { return minimumRate    ;}
double                  MprisDBusPlayerInterface::getMaximumRate    () { return maximumRate    ;}

bool MprisDBusPlayerInterface::getCanGoNext     () { return canGoNext     ;}
bool MprisDBusPlayerInterface::getCanGoPrevious () { return canGoPrevious ;}
bool MprisDBusPlayerInterface::getCanPlay       () { return canPlay       ;}
bool MprisDBusPlayerInterface::getCanPause      () { return canPause      ;}
bool MprisDBusPlayerInterface::getCanSeek       () { return canSeek       ;}
bool MprisDBusPlayerInterface::getCanControl    () { return canControl    ;}


void MprisDBusPlayerInterface::setLoopStatus     (QString                 value, bool emitChange ) { loopStatus     = value; if (emitChange) emit signaler.loopStatusChanged(value)     ;}
void MprisDBusPlayerInterface::setPlaybackRate   (double                  value, bool emitChange ) { playbackRate   = value; if (emitChange) emit signaler.playbackRateChanged(value)   ;}
void MprisDBusPlayerInterface::setShuffle        (bool                    value, bool emitChange ) { shuffle        = value; if (emitChange) emit signaler.shuffleChanged(value)        ;}
void MprisDBusPlayerInterface::setVolume         (double                  value, bool emitChange ) { volume         = value; if (emitChange) emit signaler.volumeChanged(value)         ;}

void MprisDBusPlayerInterface::setPlaybackStatus (QString                 value ) { playbackStatus = value; emit signaler.playbackStatusChanged(value) ;}
void MprisDBusPlayerInterface::setMetadata       (QMap<QString, QVariant> value ) { metadata       = value; emit signaler.metadataChanged(value)       ;}
void MprisDBusPlayerInterface::setPosition       (qlonglong               value ) { position       = value; emit signaler.positionChanged(value)       ;}
void MprisDBusPlayerInterface::setMinimumRate    (double                  value ) { minimumRate    = value; emit signaler.minimumRateChanged(value)    ;}
void MprisDBusPlayerInterface::setMaximumRate    (double                  value ) { maximumRate    = value; emit signaler.maximumRateChanged(value)    ;}

void MprisDBusPlayerInterface::setCanGoNext      (bool value) { canGoNext     = value;}
void MprisDBusPlayerInterface::setCanGoPrevious  (bool value) { canGoPrevious = value;}
void MprisDBusPlayerInterface::setCanPlay        (bool value) { canPlay       = value;}
void MprisDBusPlayerInterface::setCanPause       (bool value) { canPause      = value;}
void MprisDBusPlayerInterface::setCanSeek        (bool value) { canSeek       = value;}
void MprisDBusPlayerInterface::setCanControl     (bool value) { canControl    = value;}

void MprisDBusPlayerInterface::Next() { emit signaler.Next();}
void MprisDBusPlayerInterface::Previous() { emit signaler.Previous();}
void MprisDBusPlayerInterface::Pause() { emit signaler.Pause();}
void MprisDBusPlayerInterface::PlayPause() {emit signaler.PlayPause();}
void MprisDBusPlayerInterface::Stop() {emit signaler.Stop();}
void MprisDBusPlayerInterface::Play() {emit signaler.Play();}
void MprisDBusPlayerInterface::Seek(qlonglong Offset) {emit signaler.Seek(Offset);}
void MprisDBusPlayerInterface::SetPosition(QDBusObjectPath TrackId, qlonglong Offset) {emit signaler.SetPosition(TrackId, Offset);}
void MprisDBusPlayerInterface::OpenURI(QString Uri) {emit signaler.OpenURI(Uri);}
