#ifndef SONGQUEUEMODEL_H
#define SONGQUEUEMODEL_H

#include <QAbstractListModel>
#include "song.h"
#include <QList>

// This class is used as the dataModel for the Queue View and serves to store the order of the songs to be played
// The queue can be shuffled without losing the order of the main queue
class SongQueueModel : public QAbstractListModel
{
public:
    explicit SongQueueModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void append(const Song &song);
    bool insert(const Song & song, int idx);
    bool insertShuffled(const Song & song);
    bool shuffle(int nowPlayingIdx);
    int unshuffle(int nowPlayingIdx);
    void setPlayingIndex(int idx);
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    void clear();

private:
    QList<Song> songList;
    QList<int> shuffleMap;
    int playingIndex = -1;
};

#endif // SONGQUEUEMODEL_H
