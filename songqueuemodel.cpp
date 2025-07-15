#include "songqueuemodel.h"
#include <QRandomGenerator>
#include <QMimeData>
#include <QFont>
#include <QBrush>

SongQueueModel::SongQueueModel(QObject *parent)
    : QAbstractListModel{parent}
{}

// Song Model Data Function
//
// Changes behavior dependong on role and if the queue is shuffled
//
// if unshuffled, returns data based on the Song under index.row()
//
// If shuffled, returns based on the Song under an index provided by ShuffleMap
//
// The shuffleMap must be empty to result in an unshuffled list
QVariant SongQueueModel::data(const QModelIndex &index, int role) const
{
    Song song = songList[shuffleMap.isEmpty() ? index.row() : shuffleMap[index.row()]];
    QFont font;
    QBrush brush;
    switch (role)
    {
    case Qt::DisplayRole:
        return QVariant(QString("%1 - %2 - %3").arg(song.artist, song.album, song.title));
        break;
    case Qt::UserRole:
        return QVariant::fromValue(songList[shuffleMap.isEmpty() ? index.row() : shuffleMap[index.row()]]);
        break;
    case Qt::FontRole:
        if (index.row() != playingIndex) return QVariant();
        font.setBold(true);
        font.setItalic(true);
        return QVariant(font);
        break;
    case Qt::ForegroundRole:
        if (index.row() != playingIndex) return QVariant();
        brush.setColor(QColor(85, 255, 0));
        return QVariant(brush);
    default:
        return QVariant();
    }
}

// returns the rowcount of the model
int SongQueueModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    else return songList.count();
}

// Append a Song object to the model
//
// This fuction will append to the end of the list
// regardless of the state of ShuffleMap, adding the new index if nessesary
void SongQueueModel::append(const Song &song)
{

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    songList.append(song);
    if (!shuffleMap.isEmpty()) shuffleMap.append(rowCount()-1);
    endInsertRows();
}

// The main shuffle algoritm
//
// Shuffles the now playing song to the top unless no song is playing
bool SongQueueModel::shuffle(int nowPlayingIdx)
{

    if (nowPlayingIdx < -1 || nowPlayingIdx >= songList.count() || songList.count() == 1) return false;

    emit layoutAboutToBeChanged();

    shuffleMap.clear();

    //int queueEndOffset = nowPlayingIdx == -1 ? -1 : 0;

    for (int i = 0; i < songList.count(); i++)
    {
        if (i != nowPlayingIdx || nowPlayingIdx == -1) shuffleMap.append(i);
    }

    int temp;

    for (int i = 0; i < songList.count()*3; i++)
    {
        temp = shuffleMap.takeAt(QRandomGenerator::global()->bounded(shuffleMap.count()));
        shuffleMap.insert(QRandomGenerator::global()->bounded(shuffleMap.count()+1), 1, temp);
    }

    if (nowPlayingIdx != -1) shuffleMap.prepend(nowPlayingIdx);

    emit layoutChanged();

    return true;
}

// This will clear the shuffleMap, unshuffling the songList
//
// Returns the new index of the now playing song
int SongQueueModel::unshuffle(int nowPlayingIdx)
{
    emit layoutAboutToBeChanged();

    int ret = shuffleMap.value(nowPlayingIdx, nowPlayingIdx);

    shuffleMap.clear();

    emit layoutChanged();

    return ret;
}

// inserts a song into a certain index
//
// if shuffled the song's unshuffled index will match it's shuffled index
bool SongQueueModel::insert(const Song & song, int idx)
{
    if (idx < 0 || idx > songList.count()) return false;
    beginInsertRows(QModelIndex(), idx, idx);

    if (!shuffleMap.isEmpty())
    {
        for (int i = 0; i < shuffleMap.count(); i++)
        {
            if (shuffleMap[i] >= idx) shuffleMap[i] += 1;
        }
        shuffleMap.insert(idx, idx);
    }

    songList.insert(idx, song);

    endInsertRows();

    return true;
}

// Returns the queue index of the inserted song
// Currently unused
int insertShuffled(const Song & song)
{
    return -1;
}

// Returns item flags
//
// songs cannot be edited
Qt::ItemFlags SongQueueModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

// Sets the now playing index
//
// if a song matches this index
// the color of the text is changed to green to indicate the playing song
void SongQueueModel::setPlayingIndex(int idx)
{
    emit layoutAboutToBeChanged();
    playingIndex = idx;
    qDebug() << idx;
    emit layoutChanged();
}

// Removes rows
// Ineffient when shuffle is on
bool SongQueueModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid()) return false;
    beginRemoveRows(parent, row, row+count);

    bool shuffled = !shuffleMap.isEmpty();
    int takeAtRow = row;

    for (int i = 0; i < count; i++)
    {
        if (shuffled)
        {
            takeAtRow = shuffleMap.takeAt(row);

            for (int j = 0; j < shuffleMap.count(); j++)
            {
                if (shuffleMap[j] >= takeAtRow) shuffleMap[j]--;
            }
        }

        songList.takeAt(takeAtRow);
    }

    endRemoveRows();
    return true;
}

// Clears the model
void SongQueueModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, songList.count()-1);
    songList.clear();
    endRemoveRows();
}
