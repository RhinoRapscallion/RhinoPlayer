#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QObject>
#include <QList>
#include <QStack>
#include <QStringListModel>
#include <QMediaPlayer>
#include "song.h"
#include "songqueuemodel.h"
#include "mpriscontroller.h"

struct RepeatMode
{
    static const int RepeatOff = 0;
    static const int RepeatSong = 1;
    static const int RepeatPlaylist = 2;
    static const int RepeatShuffle = 3;
};

// This Class is responsible for controlling the internal media player as well as managing the queue
class MusicPlayer : public QObject
{
    Q_OBJECT
public:
    explicit MusicPlayer(QObject *parent = nullptr);
    ~MusicPlayer();
    bool addSong(Song song, bool play);
    bool addSongs(QList<Song> songs);
    bool insertNext(Song song);
    bool isQueueEmpty();
    bool isPlaylistEmpty();
    bool isShuffled();
    bool removeSongsFromQueue(QModelIndexList indexes);
    bool playing();
    bool clearQueue();

    int cycleRepeat();
    int setRepeat(int repeatMode);

    SongQueueModel queue;

private:
    QList<Song> dynamicPlaylist;
    int queueIdx = -1;
    int repeat = 0;
    bool shuffle;
    QMediaPlayer player;
    MprisController mpris;

    bool DBUS = false;


signals:
    void mediaLoaded(const Song &song, int dynPlstIdx);
    void mediaProgress(qint64 position, qint64 duration);
    void seeked(qint64 position);
    void queueIndexChanged(int idx);
    void playbackStateChanged(QMediaPlayer::PlaybackState state);
    void noMedia();
    void repeatModeChanged(int repeatMode);
    void shuffleChanged(bool shuffle);
    void volumeChanged(int volume);

public slots:
    void playPause();
    void pause();
    void play();
    void stop();
    void playSong(int plstIdx);
    void next();
    void prev();
    void setShuffle(bool);
    void seek(qint64 position);
    void relSeek(qint64 offset);
    void mediaStatusChanged(QMediaPlayer::MediaStatus);
    void setVolume(int newVolume);
};

#endif // MUSICPLAYER_H
