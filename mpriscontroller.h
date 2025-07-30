#ifndef MPRISCONTROLLER_H
#define MPRISCONTROLLER_H

#include <QObject>
#include <QMediaPlayer>
#include "mprisdbusinterface.h"
#include "mprisdbusplayerinterface.h"
#include "song.h"

class MprisController : public QObject
{
    Q_OBJECT
public:
    explicit MprisController(QObject *parent = nullptr);

    bool unreachable();

public slots:
    void mediaLoaded(const Song &song, int dynPlstIdx);
    void noMedia();
    void positionChanged(const qint64 position, const qint64 duration);
    void playbackStateChanged(QMediaPlayer::PlaybackState state);
    void seeked(qint64 position);
    void shuffleSet(bool shuffle);
    void loopSet(int loop);
    void volumeSet(int volume);


private slots:
    void m_setPosition(QDBusObjectPath TrackId, qlonglong Offset);
    void m_seek(qlonglong position);
    void m_setLoop(QString value);
    void m_setVolume(double value);
    void m_setShuffle(bool value);

signals:
    void playPause();
    void play();
    void pause();
    void stop();
    void playSong(int plstIdx);
    void next();
    void prev();
    void setShuffle(bool value);
    void seek(qint64 position);
    void relSeek(qint64 offset);
    void setVolume(int newVolume);
    void setLoop(int loop);

private:
    MprisDBusPlayerInterface* playerInterface;
    MprisDBusInterface*       mprisInterface;

    bool DBusUnreachable = true;

    void propertiesChanged(QString interface, QString name, QVariant value);
};

#endif // MPRISCONTROLLER_H
