#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "musicdatabase.h"
#include "musicplayer.h"
#include <QMainWindow>
#include <QtMultimedia/QMediaPlayer>
#include <QStringListModel>
#include <qmenu.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    MusicDatabase db;

private:
    Ui::MainWindow *ui;
    QStringListModel artistModel;
    QStringListModel albumModel;
    QStringListModel songModel;
    QPixmap artPlaceholder;
    MusicPlayer player;
    int playlistIdx;
    bool filterSongsByArtists = false;

    QAction playSong;
    QAction insertSong;
    QAction playNext;

    QAction insertArtist;
    QAction insertAlbum;

    QAction removeFromQueue;
    QAction clearQueue;

    QMenu fileMenu;
    QAction resetDatabase;
    QAction scanFolder;



private slots:
    void tabChanged(int tab);
    void showArtists();
    void showAlbums(QModelIndex indexOfArtist);

    void showSongs(QString album = "");
    void showSongs(int idx);

    void mediaLoaded(const Song &song, int dynPlstIdx);
    void playbackStateChanged(QMediaPlayer::PlaybackState state);
    void repeatModeChanged(int repeatMode);
};
#endif // MAINWINDOW_H
