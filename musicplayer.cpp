#include "musicplayer.h"
#include <QAudioOutput>

MusicPlayer::MusicPlayer(QObject *parent)
    : QObject{parent}
{
    player.setAudioOutput(new QAudioOutput);

    QObject::connect(&player, &QMediaPlayer::mediaStatusChanged, this, &MusicPlayer::mediaStatusChanged);
    QObject::connect(&player, &QMediaPlayer::positionChanged, this, [=](qint64 position) { emit mediaProgress(position, player.duration()); });
    QObject::connect(&player, &QMediaPlayer::playbackStateChanged, this, &MusicPlayer::playbackStateChanged);
    queue.setPlayingIndex(-1);
    queueIdx = -1;
}

// Adds a song to the queue
// if play is true, will play the song as well
bool MusicPlayer::addSong(Song song, bool play)
{
    queue.append(song);


    if (play) {
        queueIdx = queue.rowCount() - 1;
        player.setSource(queue.data(queue.index(queueIdx, 0), Qt::UserRole).value<Song>().file);
    }

    return true;
}

// inserts the song after the currently playing one
// because queueIdx is set to -1 when not playing
// this function will insert the song first in the queue when not playing
bool MusicPlayer::insertNext(Song song)
{
    return queue.insert(song, queueIdx + 1);
}

bool MusicPlayer::addSongs(QList<Song> songs)
{
    for (Song &s : songs)
    {
        addSong(s, false);
    }

    return true;
}

// Plays or pauses the current song, does nothing if the queue is empty
// if the queue is not empty and the player is stopped, will play song at top of the queue
void MusicPlayer::playPause()
{
    if(player.playbackState() == QMediaPlayer::StoppedState)
    {
        if (!(queue.rowCount() != 0)) return;
        queueIdx = 0;
        player.setSource(queue.data(queue.index(queueIdx, 0), Qt::UserRole).value<Song>().file);
    }
    else
    {
        if (player.isPlaying()) player.pause();
        else player.play();
    }
}

// Handles the player states
// Will go to the next in queue if the song ends
// will tell the mainwindow if there is no media or if new media is loaded.
void MusicPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status)
    {
    case QMediaPlayer::LoadedMedia:
        if (queue.rowCount() < 1 || queueIdx < 0) break;
        emit mediaLoaded(queue.data(queue.index(queueIdx, 0), Qt::UserRole).value<Song>(), queueIdx);
        queue.setPlayingIndex(queueIdx);
        player.play();
        break;
    case QMediaPlayer::EndOfMedia:
        next();
        break;
    case QMediaPlayer::NoMedia:
        emit noMedia();
    default:
        break;
    }
}

// handles traversing the queue forward.
// Loads the next song in the queue
// follows the behavior set by Repeat when the last song is played
// Will not signal the main window as that is handled when the media is loaded
void MusicPlayer::next()
{
    queueIdx += 1;
    bool endOfQueue = false;

    switch (repeat)
    {
    case RepeatMode::RepeatShuffle:
        if (queueIdx < queue.rowCount()) break;
        queueIdx = -1;
        setShuffle(true);
        queueIdx = 0;
        break;
    case RepeatMode::RepeatPlaylist:
        if (queueIdx < queue.rowCount()) break;
        queueIdx = 0;
        break;
    case RepeatMode::RepeatSong:
        queueIdx -= 1;
        break;
    case RepeatMode::RepeatOff:
    default:
        if (queueIdx < queue.rowCount()) break;
        queueIdx = -1;
        player.stop();
        player.setSource(QUrl());
        endOfQueue = true;

    }

    emit queueIndexChanged(queueIdx);

    queue.setPlayingIndex(queueIdx);

    if (!endOfQueue) player.setSource(queue.data(queue.index(queueIdx, 0), Qt::UserRole).value<Song>().file);
}

// Cycles the repeat type, and returns the number corrisponding to the new value;
int MusicPlayer::cycleRepeat()
{
    switch (repeat)
    {
    case RepeatMode::RepeatOff:
        repeat = RepeatMode::RepeatPlaylist;
        break;
    case RepeatMode::RepeatPlaylist:
        repeat = RepeatMode::RepeatSong;
        break;
    case RepeatMode::RepeatSong:
        if (shuffle) {
            repeat = RepeatMode::RepeatShuffle;
            break;
        }
    case RepeatMode::RepeatShuffle:
    default:
        repeat = RepeatMode::RepeatOff;
        break;
    }

    emit repeatModeChanged(repeat);
    return repeat;
}

// handles traversing backwards in the queue
// will restart the current song if more than 1% through
// else will load the previous song
void MusicPlayer::prev()
{
    if (player.position() < player.duration() * 0.01)
    {
        queueIdx -= 1;
        if (queueIdx < 0) queueIdx = 0;
        player.setSource(queue.data(queue.index(queueIdx, 0), Qt::UserRole).value<Song>().file);
        queue.setPlayingIndex(queueIdx);
    }
    else
    {
        player.setSource(queue.data(queue.index(queueIdx, 0), Qt::UserRole).value<Song>().file);
    }
}

// Will load an arbitarary song, regardless of the play state
// does nothing if plstIdx is out of bounds
void MusicPlayer::playSong(int plstIdx)
{
    if(plstIdx < 0 || plstIdx >= queue.rowCount()) return;

    queueIdx = plstIdx;
    player.setSource(queue.data(queue.index(queueIdx, 0), Qt::UserRole).value<Song>().file);
    queue.setPlayingIndex(queueIdx);
}

// sets the playing position, calls internal player function
void MusicPlayer::seek(int position)
{
    player.setPosition(position);
}

// Returns the shuffle state of the player
bool MusicPlayer::isShuffled()
{
    return shuffle;
}

// Sets the shuffle state of the player
void MusicPlayer::setShuffle(bool _shuffle)
{
    shuffle = _shuffle;
    if (_shuffle)
    {
        shuffle = queue.shuffle(queueIdx);
        if (shuffle && queueIdx >= 0) queueIdx = 0;
    }
    else
    {
        queueIdx = queue.unshuffle(queueIdx);
        if (repeat == RepeatMode::RepeatShuffle)
        {
            repeat = RepeatMode::RepeatPlaylist;
            emit repeatModeChanged(repeat);
        }
    }

    queue.setPlayingIndex(queueIdx);

    emit queueIndexChanged(queueIdx);
}

// Removes the specified songs from the queue, Reorders and removes them one at a time
// This is needed as the QAbstractListModel and by extention SongQueueModel
// does not have the capabillity to remove arbitrary groups of items
bool MusicPlayer::removeSongsFromQueue(QModelIndexList indexes)
{
    std::sort(indexes.begin(), indexes.end(), [](QModelIndex x, QModelIndex y) {return x.row() > y.row(); });
    for (auto index : indexes)
    {
        queue.removeRows(index.row(), 1);
        if (index.row() < queueIdx) {
            queueIdx-=1;
            queue.setPlayingIndex(queueIdx);
        }
        else if (index.row() == queueIdx)
        {
            queueIdx=-1;
            queue.setPlayingIndex(-1);
            player.setSource(QUrl());
        }
    }
    return true;
}

// Clears the queue and ensures that everything is reset to allow the queue to be reused
bool MusicPlayer::clearQueue()
{
    queueIdx = -1;
    queue.clear();
    emit queueIndexChanged(-1);
    player.setSource(QUrl());
    return true;
}

// returns the playing state of the player
bool MusicPlayer::playing()
{
    return player.isPlaying();
}
