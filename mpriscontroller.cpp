#include "mpriscontroller.h"
#include "QCoreApplication"
#include "QDBusConnection"
#include "musicplayer.h"
#include <QDBusMessage>
#include <QMetaMethod>

MprisController::MprisController(QObject *parent)
    : QObject{parent}
{
    int proc = QCoreApplication::applicationPid();
    QDBusConnection con = QDBusConnection::connectToBus(QDBusConnection::SessionBus, QString("org.mpris.MediaPlayer.RhinoMusic.pid%1").arg(proc));

    if (!con.isConnected() ) {DBusUnreachable = true; return; }
    //new DBusMPRISAdaptor(this);
    mprisInterface = new MprisDBusInterface(this);
    playerInterface = new MprisDBusPlayerInterface(this);

    playerInterface->setCanControl(true);

    QObject::connect(&mprisInterface->signaler, &MprisDBusInterfaceSignaler::canRaiseChanged,            this, [=](bool value) { propertiesChanged("org.mpris.MediaPlayer2", "CanRaise",            QVariant(value));});
    QObject::connect(&mprisInterface->signaler, &MprisDBusInterfaceSignaler::canQuitChanged,             this, [=](bool value) { propertiesChanged("org.mpris.MediaPlayer2", "CanQuit",             QVariant(value));});
    QObject::connect(&mprisInterface->signaler, &MprisDBusInterfaceSignaler::fullscreenChanged,          this, [=](bool value) { propertiesChanged("org.mpris.MediaPlayer2", "Fullscreen",          QVariant(value));});
    QObject::connect(&mprisInterface->signaler, &MprisDBusInterfaceSignaler::canSetFullscreenChanged,    this, [=](bool value) { propertiesChanged("org.mpris.MediaPlayer2", "CanSetFullscreen",    QVariant(value));});
    QObject::connect(&mprisInterface->signaler, &MprisDBusInterfaceSignaler::hasTracklistChanged,        this, [=](bool value) { propertiesChanged("org.mpris.MediaPlayer2", "HasTrackList",        QVariant(value));});
    QObject::connect(&mprisInterface->signaler, &MprisDBusInterfaceSignaler::supportedMimeTypesChanged,  this, [=](QStringList value) { propertiesChanged("org.mpris.MediaPlayer2", "SupportedMimeTypes",  QVariant(value));});
    QObject::connect(&mprisInterface->signaler, &MprisDBusInterfaceSignaler::supportedURISchemesChanged, this, [=](QStringList value) { propertiesChanged("org.mpris.MediaPlayer2", "SupportedURISchemes", QVariant(value));});

    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::playbackStatusChanged, this, [=](QString                 value) { propertiesChanged("org.mpris.MediaPlayer2.Player", "PlaybackStatus", QVariant(value));});
    // QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::loopStatusChanged,     this, [=](QString                 value) { propertiesChanged("org.mpris.MediaPlayer2.Player", "LoopStatus",     QVariant(value));});
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::playbackRateChanged,   this, [=](qlonglong               value) { if (value != 1.0) playerInterface->setPlaybackRate(1.0); });
    // QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::shuffleChanged,        this, [=](bool                    value) { propertiesChanged("org.mpris.MediaPlayer2.Player", "Shuffle",        QVariant(value));});
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::metadataChanged,       this, [=](QMap<QString, QVariant> value) { propertiesChanged("org.mpris.MediaPlayer2.Player", "Metadata",       QVariant(value));});
    // QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::volumeChanged,         this, [=](double                  value) { propertiesChanged("org.mpris.MediaPlayer2.Player", "Volume",         QVariant(value));});
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::minimumRateChanged,    this, [=](double                  value) { propertiesChanged("org.mpris.MediaPlayer2.Player", "MinimumRate",    QVariant(value));});
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::maximumRateChanged,    this, [=](double                  value) { propertiesChanged("org.mpris.MediaPlayer2.Player", "MaximumRate",    QVariant(value));});

    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::canGoNextChanged,     this, [=](bool value) { propertiesChanged("org.mpris.MediaPlayer2.Player", "CanGoNext",     QVariant(value));});
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::canGoPreviousChanged, this, [=](bool value) { propertiesChanged("org.mpris.MediaPlayer2.Player", "CanGoPrevious", QVariant(value));});
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::canPlayChanged,       this, [=](bool value) { propertiesChanged("org.mpris.MediaPlayer2.Player", "CanPlay",       QVariant(value));});
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::canPauseChanged,      this, [=](bool value) { propertiesChanged("org.mpris.MediaPlayer2.Player", "CanPause",      QVariant(value));});
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::canSeekChanged,       this, [=](bool value) { propertiesChanged("org.mpris.MediaPlayer2.Player", "CanSeek",       QVariant(value));});

    mprisInterface->setIdentity("RhinoMusic");
    playerInterface->setCanPlay(true);
    playerInterface->setCanPause(true);
    playerInterface->setCanGoNext(true);
    playerInterface->setCanGoPrevious(true);
    playerInterface->setCanSeek(true);

    //QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::PlayPause, this, [=]() {emit MprisController::playPause();});
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::PlayPause,         this, &MprisController::playPause     );
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::Play,              this, &MprisController::play          );
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::Pause,             this, &MprisController::pause         );
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::Stop,              this, &MprisController::stop          );
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::Next,              this, &MprisController::next          );
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::Previous,          this, &MprisController::prev          );
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::Seek    ,          this, &MprisController::m_seek        );
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::SetPosition,       this, &MprisController::m_setPosition );

    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::loopStatusChanged, this, &MprisController::m_setLoop     );
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::shuffleChanged,    this, &MprisController::m_setShuffle  );
    QObject::connect(&playerInterface->signaler, &MprisDBusPlayerSignaler::volumeChanged,     this, &MprisController::m_setVolume   );

    con.registerObject("/org/mpris/MediaPlayer2", this);
    con.registerService(QString("org.mpris.MediaPlayer2.RhinoMusic.pid%1").arg(proc));

    DBusUnreachable = false;
}

void MprisController::propertiesChanged(QString interface, QString name, QVariant value)
{
    if (DBusUnreachable) { return; }
    int proc = QCoreApplication::applicationPid();
    QDBusConnection con = QDBusConnection::connectToBus(QDBusConnection::SessionBus, QString("org.mpris.MediaPlayer.RhinoMusic.pid%1").arg(proc));
    QDBusMessage signal = QDBusMessage::createSignal("/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged");
    QMap<QString, QVariant> propmap;
    propmap[name] = value;
    signal.setArguments({interface, propmap, QStringList()});
    qDebug() << signal.service() << signal.member();
    qDebug() << con.send(signal);
}

void MprisController::mediaLoaded(const Song &song, int dynPlstIdx) {
    if (DBusUnreachable) { return; }
    QMap<QString, QVariant> newMetadata;

    newMetadata["mpris:trackid"]     = QVariant(QDBusObjectPath(QString("/com/RhinoMusic/track/%1").arg(QByteArray::fromStdString(song.title.toStdString() + "-" + song.file.toStdString()).toHex())));
    newMetadata["mpris:artUrl"]      = QVariant(QUrl::fromLocalFile(song.image).toString());
    newMetadata["mpris:length"]      = (qlonglong)song.duration * 1000;
    newMetadata["xesam:title"]       = song.title;
    newMetadata["xesam:album"]       = song.album;
    newMetadata["xesam:albumArtist"] = QList<QString> (song.artist);
    newMetadata["xesam:artist"]      = QList<QString> (song.artist);

    playerInterface->setMetadata(newMetadata);
}

void MprisController::noMedia() {
    if (DBusUnreachable) { return; }
    QMap<QString, QVariant> newMetadata;

    newMetadata["mpris:trackid"]     = QVariant(QDBusObjectPath("/org/mpris/MediaPlayer2/TrackList/NoTrack"));

    playerInterface->setMetadata(newMetadata);
}

void MprisController::positionChanged(const qlonglong position, const qlonglong duration) {
    if (DBusUnreachable) { return; }
    playerInterface->setPosition(position * 1000);
}
void MprisController::playbackStateChanged(QMediaPlayer::PlaybackState state)
{
    if (DBusUnreachable) { return; }
    switch (state){
    case QMediaPlayer::PausedState:
        playerInterface->setPlaybackStatus("Paused");
        break;
    case QMediaPlayer::PlayingState:
        playerInterface->setPlaybackStatus("Playing");
        break;
    case QMediaPlayer::StoppedState:
        playerInterface->setPlaybackStatus("Stopped");
        break;
    }
}

bool MprisController::unreachable() {return DBusUnreachable;}

void MprisController::seeked(qint64 position)
{
    if (DBusUnreachable) return;
    emit playerInterface->Seeked(position * 1000);
}

void MprisController::m_seek(qlonglong offset)
{
    if (DBusUnreachable) { return; }
    emit relSeek(offset / 1000);
    qDebug() << offset / 1000;
}

void MprisController::m_setPosition(QDBusObjectPath path, qlonglong position)
{
    if (DBusUnreachable) { return; }
    if (path == playerInterface->getMetadata().value("mpris:trackid").value<QDBusObjectPath>())
    {
        emit seek(position / 1000);
        qDebug() << position / 1000;
    }
}

void MprisController::m_setLoop(QString value)
{
    if (DBusUnreachable) { return; }
    if (value == "Track")
    {
        emit setLoop(RepeatMode::RepeatSong);
        return;
    }

    if (value == "Playlist")
    {
        emit setLoop(RepeatMode::RepeatPlaylist);
        return;
    }

    emit setLoop(RepeatMode::RepeatOff);
    return;
}

void MprisController::m_setVolume(double value)
{
    if (DBusUnreachable) { return; }
    if (value < 0 || value > 1) return;
    emit setVolume(value * 100);
}

void MprisController::m_setShuffle(bool value)
{
    if (DBusUnreachable) { return; }
    emit setShuffle(value);
}

void MprisController::shuffleSet(bool shuffle)
{
    if (DBusUnreachable) { return; }
    playerInterface->setShuffle(shuffle, false);
    propertiesChanged("org.mpris.MediaPlayer2.Player", "Shuffle", shuffle);
}

void MprisController::loopSet(int loop)
{
    if (DBusUnreachable) { return; }
    switch (loop)
    {
    case RepeatMode::RepeatPlaylist:
    case RepeatMode::RepeatShuffle:
        playerInterface->setLoopStatus("Playlist", false);
        propertiesChanged("org.mpris.MediaPlayer2.Player", "LoopStatus", "Playlist");
        break;
    case RepeatMode::RepeatSong:
        playerInterface->setLoopStatus("Track", false);
        propertiesChanged("org.mpris.MediaPlayer2.Player", "LoopStatus", "Track");
        break;
    default:
        playerInterface->setLoopStatus("None", false);
        propertiesChanged("org.mpris.MediaPlayer2.Player", "LoopStatus", "None");
        break;
    }
}

void MprisController::volumeSet(int volume)
{
    if (DBusUnreachable) { return; }
    playerInterface->setVolume(double(volume / 100.0), false);
    propertiesChanged("org.mpris.MediaPlayer2.Player", "Shuffle", double(volume / 100.0));
}
