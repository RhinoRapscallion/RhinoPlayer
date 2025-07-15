#ifndef MUSICDATABASE_H
#define MUSICDATABASE_H

#include "song.h"
#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QMediaPlayer>



struct Album
{
    QString artist;
    QString album;
};

class MusicDatabase : public QObject
{
    Q_OBJECT
public:
    explicit MusicDatabase(QObject *parent = nullptr);
    ~MusicDatabase();
    bool valid;

    bool isValid();
    bool connectToDatabase(QString databaseFilePath);
    bool createDatabase(QString databaseFilePath);
    void scanFolder(QString directory);

    bool filteredByArtist();
    bool filteredByAlbum();

    QStringList getArtists();
    QStringList getAlbums();
    QStringList getSongNames();
    QStringList getSongNamesByAlbumID(int idx, bool filterByArtist = false);
    bool setFiltersByAlbumID(int idx, bool filterByArtist = false);
    Song getSong(int idx);
    Song getSong(QString title);
    QList<Song> getSongs();

    QMediaPlayer* mp = nullptr;

public slots:
    void setArtist(QString Artist = "");
    void setAlbum(QString Album = "");
    void scanMedia(QMediaPlayer::MediaStatus status);

signals:
    void songsFiltered();
    void scanComplete();
    void scanStatus(QString File);

private:
    QStringList scanList;
    QString filterArtist;
    QString filterAlbum;
};

#endif // MUSICDATABASE_H
