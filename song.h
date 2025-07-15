#ifndef SONG_H
#define SONG_H

#include <QString>

struct Song
{
    QString artist;
    QString albumArtist;
    QString album;
    QString title;
    QString file;
    QString image;
    int track;
};

#endif // SONG_H
