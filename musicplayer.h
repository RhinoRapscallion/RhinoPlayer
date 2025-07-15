#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QObject>
#include <QList>
#include <QStack>
#include <QStringListModel>
#include <QMediaPlayer>
#include "song.h"
#include "songqueuemodel.h"

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

    SongQueueModel queue;

private:
    QList<Song> dynamicPlaylist;
    int queueIdx = -1;
    int repeat = 0;
    bool shuffle;
    QMediaPlayer player;

signals:
    void mediaLoaded(const Song &song, int dynPlstIdx);
    void mediaProgress(qint64 position, qint64 duration);
    void queueIndexChanged(int idx);
    void playbackStateChanged(QMediaPlayer::PlaybackState state);
    void noMedia();
    void repeatModeChanged(int repeatMode);

public slots:
    void playPause();
    void playSong(int plstIdx);
    void next();
    void prev();
    void setShuffle(bool);
    void seek(int position);
    void mediaStatusChanged(QMediaPlayer::MediaStatus);
};

#endif // MUSICPLAYER_H
