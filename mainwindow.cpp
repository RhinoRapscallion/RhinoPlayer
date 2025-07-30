#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QtMultimedia/QMediaPlayer>
#include <QAudioOutput>
#include <QMediaMetaData>
#include <QImage>
#include <QSlider>
#include <QDir>
#include <QFileDialog>
#include "musicdatabase.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect to database as program starts up, if the database does not exist or is corrupted
    // the program will automatically create a new one
    if (db.connectToDatabase("songs.db"))
    {
        qDebug() << ("Database Opened");
    }
    else if (db.createDatabase("songs.db"))
    {
        qDebug() << ("Database Created and Opened");
        //db.scanFolder("/home/rhino/Data/Media/Music"); // To Do Settings page. Hardcoded path bad.
    }

    // Sets up all of the list views. These views are for song selection.
    ui->Artists->setModel(&artistModel);
    showArtists();

    ui->Albums->setModel(&albumModel);

    ui->Songs->setModel(&songModel);

    ui->Playlist->setModel(&player.queue);

    artPlaceholder.load("://placeholderArt.png");
    ui->infoArt->setPixmap(artPlaceholder.scaled(150, 150));

    // Signal connections

    // The four connections below control the views. See specified functions for more information
    QObject::connect(ui->Artists->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=](QItemSelection selected, QItemSelection deselected) {
        if (deselected.count() > 0 && selected.count() < 1) {ui->Artists->selectionModel()->select(artistModel.index(0), QItemSelectionModel::Select);}
        else {
            showAlbums(selected.indexes().first());
            ui->Albums->selectionModel()->select(albumModel.index(0), QItemSelectionModel::Select);
        };
    });

    QObject::connect(ui->Albums->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=](QItemSelection selected, QItemSelection deselected) {
        if (deselected.count() > 0 && selected.count() < 1) {ui->Albums->selectionModel()->select(albumModel.index(0), QItemSelectionModel::Select); }
        else {
            showSongs(selected.indexes().first().row() - 1);
            ui->Songs->clearSelection();
        }
    });
    QObject::connect(ui->Songs, &QAbstractItemView::doubleClicked, &player, [=](QModelIndex index)
                     {
        player.addSong(db.getSong(index.row()), true);
        ui->Songs->clearSelection();
    });
    //QObject::connect(&player, &MusicPlayer::queueIndexChanged, this, [=](int index) {ui->Playlist->setCurrentIndex(ui->Playlist->model()->index(index, 0)); });

    // Sets up playlist controls
    QObject::connect(ui->Playlist, &QAbstractItemView::doubleClicked, this, [=](QModelIndex index) {
        player.playSong(index.row());
        ui->Playlist->clearSelection();
    });

    // Setup Volume Slider
    QObject::connect(ui->volumeSlider, &QAbstractSlider::valueChanged, &player, &MusicPlayer::setVolume);
    QObject::connect(&player, &MusicPlayer::volumeChanged, this, &MainWindow::volumeChanged);

    // Sets up the progress bar
    QObject::connect(&player, &MusicPlayer::mediaProgress, this, [=](qint64 pos, qint64 total) {
        ui->infoProgress->setMaximum(total);
        ui->infoProgress->setValue(pos);
    });

    // Used for now playing information, see MainWindow::MediaLoaded for more information
    QObject::connect(&player, &MusicPlayer::mediaLoaded, this, &MainWindow::mediaLoaded);
    QObject::connect(&player, &MusicPlayer::noMedia, this, [=]() {
        ui->infoArt->setPixmap(artPlaceholder.scaled(150, 150));
        ui->infoTitle->setText("No Media");
        ui->infoAlbum->setText("");
        ui->infoArtist->setText("");
    });

    // Changes button text & colors;
    QObject::connect(&player, &MusicPlayer::playbackStateChanged, this, &MainWindow::playbackStateChanged);
    QObject::connect(&player, &MusicPlayer::repeatModeChanged,    this, &MainWindow::repeatModeChanged);
    QObject::connect(&player, &MusicPlayer::shuffleChanged,       this, &MainWindow::shuffleChanged);

    // Playback controls
    QObject::connect(ui->playPause, &QPushButton::clicked, &player, &MusicPlayer::playPause);
    QObject::connect(ui->prevSong, &QPushButton::clicked, &player, &MusicPlayer::prev);
    QObject::connect(ui->nextSong, &QPushButton::clicked, &player, &MusicPlayer::next);
    QObject::connect(ui->infoProgress, &QAbstractSlider::sliderMoved, &player, &MusicPlayer::seek);
    QObject::connect(ui->repeat, &QPushButton::clicked, this, [=](){ player.cycleRepeat(); });
    QObject::connect(ui->shuffle, &QPushButton::clicked, this, [=](){ player.setShuffle(!player.isShuffled()); });

    QObject::connect(&db, &MusicDatabase::scanComplete, this, [=](){
        ui->statusbar->clearMessage();
        artistModel.setStringList(db.getArtists());
        albumModel.setStringList(db.getAlbums());
        songModel.setStringList(db.getSongNames());
    });

    QObject::connect(&db, &MusicDatabase::scanStatus, this, [=](QString File){ui->statusbar->showMessage(File);});


    // Keep models used for displaying and selecting songs from being edited
    ui->Artists->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->Albums->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->Songs->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->Playlist->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Actions for context menus
    playSong.setText("Play");
    QObject::connect(&playSong, &QAction::triggered, this, [=](){
        QModelIndexList lst = ui->Songs->selectionModel()->selectedIndexes();
        std::sort(lst.begin(), lst.end(), [=](QModelIndex x, QModelIndex y) { return x.row() < y.row(); });
        bool play = true;
        for (QModelIndex idx : lst)
        {
            player.addSong(db.getSong(idx.row()), play);
            if (play) play = false;
        }
    });
    ui->Songs->addAction(&playSong);

    insertSong.setText("Add to Queue");
    QObject::connect(&insertSong, &QAction::triggered, this, [=](){player.addSong(db.getSong(ui->Songs->currentIndex().row()), false); });
    ui->Songs->addAction(&insertSong);

    playNext.setText("Play Next");
    QObject::connect(&playNext, &QAction::triggered, this, [=](){player.insertNext(db.getSong(ui->Songs->currentIndex().row())); });
    ui->Songs->addAction(&playNext);

    insertArtist.setText("Add To Queue");
    QObject::connect(&insertArtist, &QAction::triggered, this, [=](){
        if (! ui->Artists->selectionModel()->hasSelection()) return;
        QModelIndex selectionIndex = ui->Artists->selectionModel()->selectedIndexes().first();

        if (selectionIndex.row() < 1) db.setArtist();
        else db.setArtist(artistModel.data(selectionIndex).toString());
        db.setAlbum();
        player.addSongs(db.getSongs());
    });
    ui->Artists->addAction(&insertArtist);

    insertAlbum.setText("Add To Queue");
    QObject::connect(&insertAlbum, &QAction::triggered, this, [=](){
        if (! ui->Albums->selectionModel()->hasSelection()) return;
        QModelIndex selectionIndex = ui->Albums->selectionModel()->selectedIndexes().first();

        if (selectionIndex.row() < 1) db.setAlbum();
        else db.setFiltersByAlbumID(ui->Albums->currentIndex().row() - 1, true);

        player.addSongs(db.getSongs());
    });
    ui->Albums->addAction(&insertAlbum);

    removeFromQueue.setText("Remove");
    QObject::connect(&removeFromQueue, &QAction::triggered, this, [=](){
        player.removeSongsFromQueue(ui->Playlist->selectionModel()->selectedIndexes());
    });
    ui->Playlist->addAction(&removeFromQueue);

    clearQueue.setText("Clear Queue");
    QObject::connect(&clearQueue, &QAction::triggered, this, [=](){
        player.setShuffle(false);
        ui->shuffle->setStyleSheet("");
        player.clearQueue();
    });
    ui->Playlist->addAction(&clearQueue);

    fileMenu.setTitle("File");
    ui->menubar->addMenu(&fileMenu);

    scanFolder.setText("Scan Folder");
    QObject::connect(&scanFolder, &QAction::triggered, this, [=](){
        db.scanFolder(QFileDialog::getExistingDirectory(this, "Select folder to scan", QDir::homePath()));
    });
    fileMenu.addAction(&scanFolder);

    resetDatabase.setText("Reset Database");
    QObject::connect(&resetDatabase, &QAction::triggered, this, [=](){
        db.createDatabase("songs.db");
        player.clearQueue();

        db.setArtist();
        db.setAlbum();

        albumModel.setStringList(QStringList());
        artistModel.setStringList(QStringList());
        songModel.setStringList(QStringList());
    });
    fileMenu.addAction(&resetDatabase);

    // Prefill views
    showAlbums(artistModel.index(0));
    showSongs();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Used to show albums, argument filters the albums to a single artist
void MainWindow::showAlbums(QModelIndex indexOfArtist)
{
    if (indexOfArtist.row() == 0) {
        db.setArtist("");
        filterSongsByArtists = false;
    }
    else {
        QModelIndex index = artistModel.index(indexOfArtist.row());
        db.setArtist(artistModel.data(index).toString());
        filterSongsByArtists = true;
    }

    albumModel.setStringList(db.getAlbums());
    albumModel.insertRows(0, 1);
    albumModel.setData(albumModel.index(0), QVariant("All Albums"));
}

void MainWindow::showArtists()
{
    artistModel.setStringList(db.getArtists());
    artistModel.insertRows(0, 1);
    artistModel.setData(artistModel.index(0), QVariant("All Artists"));
}

// Used to show songs of a particular album, it is expected that the artist
// has already been filtered.
void MainWindow::showSongs(QString album)
{
    db.setAlbum(album);
    songModel.setStringList(db.getSongNames());
}

// Shows a songlist based on the Row of the album selected. This is used due to limitations with the informaton
// stored within the StringListModel used to show and select the album.
// See MusicDatabase::getSongNamesByAlbumID for more information.
// the artist does not need to be filtered
void MainWindow::showSongs(int idx)
{
    QStringList songList;
    if (idx >= 0) { songList = db.getSongNamesByAlbumID(idx, filterSongsByArtists); }
    else {
        db.setAlbum();
        songList = db.getSongNames();
    }
    songModel.setStringList(songList);
}

// Sets information for now playing media.
void MainWindow::mediaLoaded(const Song &song, int dynPlstIdx)
{
    ui->infoTitle ->setText(song.title );
    ui->infoAlbum ->setText(song.album );
    ui->infoArtist->setText(song.artist);

    if (!song.image.isEmpty())
    {
        QPixmap art;
        art.load(song.image);
        ui->infoArt->setPixmap(art.scaled({150, 150}, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    playlistIdx = dynPlstIdx;

    //ui->Playlist->setCurrentIndex(ui->Playlist->model()->index(dynPlstIdx, 0));
}

// Changes the play button text depending on the state of the player
void MainWindow::playbackStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        ui->playPause->setText("Pause");
        break;
    default:
        ui->playPause->setText("Play");
        break;
    }
}

// Changes the repeat button text and color in response to RepeatMode changes;
void MainWindow::repeatModeChanged(int repeatMode)
{
    switch (repeatMode)
    {
    case RepeatMode::RepeatPlaylist:
        ui->repeat->setText("Repeat: On");
        ui->repeat->setStyleSheet("color: rgb(85, 255, 0);");
        break;
    case RepeatMode::RepeatSong:
        ui->repeat->setText("Repeat: Song");
        ui->repeat->setStyleSheet("color: rgb(255,191,0)");
        break;
    case RepeatMode::RepeatShuffle:
        ui->repeat->setText("Repeat: Reshuffle");
        ui->repeat->setStyleSheet("color: rgb(255,50,50)");
        break;
    case RepeatMode::RepeatOff:
    default:
        ui->repeat->setText("Repeat: Off");
        ui->repeat->setStyleSheet("");
        break;
    }
}

void MainWindow::shuffleChanged(bool shuffle)
{
    ui->shuffle->setStyleSheet(shuffle ? "color: rgb(85, 255, 0);" : "");
}

void MainWindow::volumeChanged(qint64 volume)
{
    ui->volumeSlider->setValue(volume);
}
