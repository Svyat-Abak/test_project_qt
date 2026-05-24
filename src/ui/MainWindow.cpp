#include "MainWindow.h"

#include "../core/AudioController.h"
#include "../core/PlaybackQueue.h"
#include "../core/Playlist.h"
#include "../core/PlaylistManager.h"
#include "../core/Track.h"
#include "../core/TrackStore.h"
#include "widgets/CoverLabel.h"
#include "widgets/NeonButton.h"
#include "widgets/SpectrumVisualizer.h"

#include <QApplication>
#include <QFile>
#include <QPushButton>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPixmap>
#include <QSizePolicy>
#include <QSlider>
#include <QSplitter>
#include <QStyle>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {
QString trackListLabel(Track *track) {
    return QStringLiteral("%1 — %2  [%3]")
        .arg(track->artist(), track->title(), track->formattedDuration());
}
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_playlistManager(new PlaylistManager(this))
    , m_queue(new PlaybackQueue(this))
    , m_audio(new AudioController(this)) {
    setWindowTitle(QStringLiteral("Neon Audio Player"));
    resize(1100, 640);
    setMinimumSize(900, 520);
    setupUi();
    loadStyleSheet();
    connectSignals();
    refreshPlaylistList();
    refreshTrackList();
    refreshQueueList();
    setUiEnabled(false);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(6);

    auto *bodySplitter = new QSplitter(Qt::Horizontal, central);

    // Left sidebar
    auto *sidebar = new QWidget(bodySplitter);
    sidebar->setObjectName(QStringLiteral("sidebar"));
    sidebar->setMinimumWidth(220);
    sidebar->setMaximumWidth(280);
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(6, 6, 6, 6);
    sidebarLayout->setSpacing(4);

    auto *playlistHeader = new QLabel(QStringLiteral("Playlists"), sidebar);
    playlistHeader->setObjectName(QStringLiteral("sectionHeader"));
    m_playlistList = new QListWidget(sidebar);
    m_playlistList->setObjectName(QStringLiteral("playlistList"));

    auto *playlistButtons = new QHBoxLayout();
    auto *addPlaylistBtn = new NeonButton(QStringLiteral("+"), sidebar);
    addPlaylistBtn->setFixedSize(32, 32);
    addPlaylistBtn->setToolTip(QStringLiteral("Add playlist"));
    auto *renamePlaylistBtn = new NeonButton(QStringLiteral("✎"), sidebar);
    renamePlaylistBtn->setFixedSize(32, 32);
    renamePlaylistBtn->setToolTip(QStringLiteral("Rename playlist"));
    auto *removePlaylistBtn = new NeonButton(QStringLiteral("−"), sidebar);
    removePlaylistBtn->setFixedSize(32, 32);
    removePlaylistBtn->setToolTip(QStringLiteral("Remove playlist"));
    playlistButtons->addWidget(addPlaylistBtn);
    playlistButtons->addWidget(renamePlaylistBtn);
    playlistButtons->addWidget(removePlaylistBtn);

    auto *queueHeader = new QLabel(QStringLiteral("Queue"), sidebar);
    queueHeader->setObjectName(QStringLiteral("sectionHeader"));
    m_queueList = new QListWidget(sidebar);
    m_queueList->setObjectName(QStringLiteral("queueList"));

    auto *queueButtons = new QHBoxLayout();
    auto *addQueueBtn = new NeonButton(QStringLiteral("Q+"), sidebar);
    addQueueBtn->setFixedSize(32, 32);
    addQueueBtn->setToolTip(QStringLiteral("Add to queue"));
    auto *removeQueueBtn = new NeonButton(QStringLiteral("Q−"), sidebar);
    removeQueueBtn->setFixedSize(32, 32);
    removeQueueBtn->setToolTip(QStringLiteral("Remove from queue"));
    auto *clearQueueBtn = new NeonButton(QStringLiteral("Clr"), sidebar);
    clearQueueBtn->setFixedSize(32, 32);
    clearQueueBtn->setToolTip(QStringLiteral("Clear queue"));
    queueButtons->addWidget(addQueueBtn);
    queueButtons->addWidget(removeQueueBtn);
    queueButtons->addWidget(clearQueueBtn);

    sidebarLayout->addWidget(playlistHeader);
    sidebarLayout->addWidget(m_playlistList, 2);
    sidebarLayout->addLayout(playlistButtons);
    sidebarLayout->addWidget(queueHeader);
    sidebarLayout->addWidget(m_queueList, 1);
    sidebarLayout->addLayout(queueButtons);

    // Center panel
    auto *centerPanel = new QWidget(bodySplitter);
    centerPanel->setObjectName(QStringLiteral("centerPanel"));
    auto *centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setContentsMargins(6, 6, 6, 6);
    centerLayout->setSpacing(6);

    m_placeholderLabel = new QLabel(QStringLiteral("Load tracks to start the neon experience"), centerPanel);
    m_placeholderLabel->setObjectName(QStringLiteral("placeholderLabel"));
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setMaximumHeight(28);

    m_cover = new CoverLabel(centerPanel);
    m_cover->setFixedSize(140, 140);

    m_titleEdit = new QLineEdit(centerPanel);
    m_titleEdit->setObjectName(QStringLiteral("trackTitleEdit"));
    m_titleEdit->setPlaceholderText(QStringLiteral("Title"));
    m_artistEdit = new QLineEdit(centerPanel);
    m_artistEdit->setObjectName(QStringLiteral("trackArtistEdit"));
    m_artistEdit->setPlaceholderText(QStringLiteral("Artist"));

    auto *metaButtons = new QHBoxLayout();
    metaButtons->setSpacing(6);
    auto *setCoverBtn = new QPushButton(QStringLiteral("Обложка"), centerPanel);
    setCoverBtn->setObjectName(QStringLiteral("textButton"));
    setCoverBtn->setCursor(Qt::PointingHandCursor);
    setCoverBtn->setMaximumHeight(28);
    m_saveMetadataBtn = new QPushButton(QStringLiteral("Зафиксировать"), centerPanel);
    m_saveMetadataBtn->setObjectName(QStringLiteral("textButton"));
    m_saveMetadataBtn->setCursor(Qt::PointingHandCursor);
    m_saveMetadataBtn->setMaximumHeight(28);
    metaButtons->addWidget(setCoverBtn);
    metaButtons->addWidget(m_saveMetadataBtn);
    metaButtons->addStretch();

    auto *trackInfoLayout = new QVBoxLayout();
    trackInfoLayout->setSpacing(4);
    trackInfoLayout->addWidget(m_titleEdit);
    trackInfoLayout->addWidget(m_artistEdit);
    trackInfoLayout->addLayout(metaButtons);
    trackInfoLayout->addStretch();

    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(10);
    topRow->addWidget(m_cover, 0, Qt::AlignTop);
    topRow->addLayout(trackInfoLayout, 1);

    m_visualizer = new SpectrumVisualizer(24, centerPanel);
    m_visualizer->setFixedHeight(52);
    m_visualizer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_descriptionEdit = new QTextEdit(centerPanel);
    m_descriptionEdit->setPlaceholderText(QStringLiteral("Notes and description for this track…"));
    m_descriptionEdit->setObjectName(QStringLiteral("descriptionEdit"));
    m_descriptionEdit->setMinimumHeight(56);
    m_descriptionEdit->setMaximumHeight(80);

    auto *searchRow = new QHBoxLayout();
    m_searchEdit = new QLineEdit(centerPanel);
    m_searchEdit->setPlaceholderText(QStringLiteral("Search by title or artist…"));
    m_searchEdit->setObjectName(QStringLiteral("searchEdit"));
    auto *addTracksBtn = new QPushButton(QStringLiteral("Add Tracks"), centerPanel);
    addTracksBtn->setObjectName(QStringLiteral("textButton"));
    addTracksBtn->setCursor(Qt::PointingHandCursor);
    searchRow->addWidget(m_searchEdit, 1);
    searchRow->addWidget(addTracksBtn);

    m_trackList = new QListWidget(centerPanel);
    m_trackList->setObjectName(QStringLiteral("trackList"));

    centerLayout->addWidget(m_placeholderLabel);
    centerLayout->addLayout(topRow);
    centerLayout->addWidget(m_visualizer);
    centerLayout->addWidget(m_descriptionEdit);
    centerLayout->addLayout(searchRow);
    centerLayout->addWidget(m_trackList, 1);

    bodySplitter->addWidget(sidebar);
    bodySplitter->addWidget(centerPanel);
    bodySplitter->setStretchFactor(0, 0);
    bodySplitter->setStretchFactor(1, 1);

    // Bottom player bar (compact)
    auto *playerBar = new QWidget(central);
    playerBar->setObjectName(QStringLiteral("playerBar"));
    playerBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    playerBar->setMaximumHeight(96);
    auto *playerLayout = new QVBoxLayout(playerBar);
    playerLayout->setContentsMargins(8, 6, 8, 6);
    playerLayout->setSpacing(4);

    auto *progressRow = new QHBoxLayout();
    progressRow->setSpacing(6);
    m_currentTimeLabel = new QLabel(QStringLiteral("00:00"), playerBar);
    m_currentTimeLabel->setObjectName(QStringLiteral("timeLabel"));
    m_currentTimeLabel->setFixedWidth(40);
    m_progressSlider = new QSlider(Qt::Horizontal, playerBar);
    m_progressSlider->setObjectName(QStringLiteral("progressSlider"));
    m_progressSlider->setRange(0, 0);
    m_progressSlider->setFixedHeight(18);
    m_totalTimeLabel = new QLabel(QStringLiteral("00:00"), playerBar);
    m_totalTimeLabel->setObjectName(QStringLiteral("timeLabel"));
    m_totalTimeLabel->setFixedWidth(40);
    progressRow->addWidget(m_currentTimeLabel);
    progressRow->addWidget(m_progressSlider, 1);
    progressRow->addWidget(m_totalTimeLabel);

    auto *controlsRow = new QHBoxLayout();
    controlsRow->setSpacing(8);
    controlsRow->addStretch();
    auto *prevBtn = new NeonButton(QStringLiteral("⏮"), playerBar);
    prevBtn->setFixedSize(36, 36);
    m_playPauseButton = new NeonButton(QStringLiteral("▶"), playerBar);
    m_playPauseButton->setFixedSize(36, 36);
    auto *stopBtn = new NeonButton(QStringLiteral("■"), playerBar);
    stopBtn->setFixedSize(36, 36);
    auto *nextBtn = new NeonButton(QStringLiteral("⏭"), playerBar);
    nextBtn->setFixedSize(36, 36);
    controlsRow->addWidget(prevBtn);
    controlsRow->addWidget(m_playPauseButton);
    controlsRow->addWidget(stopBtn);
    controlsRow->addWidget(nextBtn);
    controlsRow->addStretch();

    auto *volumeIcon = new QLabel(QStringLiteral("🔊"), playerBar);
    volumeIcon->setObjectName(QStringLiteral("volumeIcon"));
    m_volumeSlider = new QSlider(Qt::Horizontal, playerBar);
    m_volumeSlider->setObjectName(QStringLiteral("volumeSlider"));
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(75);
    m_volumeSlider->setFixedHeight(18);
    m_volumeSlider->setMaximumWidth(120);
    controlsRow->addWidget(volumeIcon);
    controlsRow->addWidget(m_volumeSlider);

    playerLayout->addLayout(progressRow);
    playerLayout->addLayout(controlsRow);

    rootLayout->addWidget(bodySplitter, 1);
    rootLayout->addWidget(playerBar, 0);

    connect(addPlaylistBtn, &QPushButton::clicked, this, &MainWindow::onAddPlaylist);
    connect(renamePlaylistBtn, &QPushButton::clicked, this, &MainWindow::onRenamePlaylist);
    connect(removePlaylistBtn, &QPushButton::clicked, this, &MainWindow::onRemovePlaylist);
    connect(addQueueBtn, &QPushButton::clicked, this, &MainWindow::onAddToQueue);
    connect(removeQueueBtn, &QPushButton::clicked, this, &MainWindow::onRemoveFromQueue);
    connect(clearQueueBtn, &QPushButton::clicked, this, &MainWindow::onClearQueue);
    connect(setCoverBtn, &QPushButton::clicked, this, &MainWindow::onSetCover);
    connect(m_saveMetadataBtn, &QPushButton::clicked, this, &MainWindow::onFixMetadata);
    connect(m_titleEdit, &QLineEdit::textChanged, this, [this] {
        if (m_updatingMetadata) {
            return;
        }
        Track *track = activeTrack();
        if (track && !track->isMetadataLocked()) {
            track->setTitle(m_titleEdit->text());
            refreshTrackList(m_searchEdit->text());
        }
    });
    connect(m_artistEdit, &QLineEdit::textChanged, this, [this] {
        if (m_updatingMetadata) {
            return;
        }
        Track *track = activeTrack();
        if (track && !track->isMetadataLocked()) {
            track->setArtist(m_artistEdit->text());
            refreshTrackList(m_searchEdit->text());
        }
    });
    connect(addTracksBtn, &QPushButton::clicked, this, &MainWindow::onAddTracks);
    connect(prevBtn, &QPushButton::clicked, this, &MainWindow::onPrevious);
    connect(m_playPauseButton, &QPushButton::clicked, this, &MainWindow::onPlayPause);
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::onStop);
    connect(nextBtn, &QPushButton::clicked, this, &MainWindow::onNext);
}

void MainWindow::loadStyleSheet() {
    QFile styleFile(QStringLiteral(":/styles/main.qss"));
    if (!styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        styleFile.setFileName(QStringLiteral("resources/styles/main.qss"));
        if (!styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
    }
    if (styleFile.isOpen()) {
        qApp->setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }
}

void MainWindow::connectSignals() {
    connect(m_playlistList, &QListWidget::currentRowChanged, this, [this](int) {
        onPlaylistSelectionChanged();
    });
    connect(m_trackList, &QListWidget::currentRowChanged, this, [this](int) {
        onTrackSelectionChanged();
    });
    connect(m_trackList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *) {
        onTrackDoubleClicked();
    });
    connect(m_queueList, &QListWidget::currentRowChanged, this, [this](int) {
        onQueueSelectionChanged();
    });
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_descriptionEdit, &QTextEdit::textChanged, this, &MainWindow::onDescriptionChanged);
    connect(m_progressSlider, &QSlider::sliderPressed, this, [this] { m_seeking = true; });
    connect(m_progressSlider, &QSlider::sliderReleased, this, [this] {
        m_seeking = false;
        onSeek(m_progressSlider->value());
    });
    connect(m_progressSlider, &QSlider::sliderMoved, this, &MainWindow::onSeek);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &MainWindow::onVolumeChanged);

    connect(m_playlistManager, &PlaylistManager::playlistsChanged, this, &MainWindow::refreshPlaylistList);
    connect(m_playlistManager, &PlaylistManager::currentPlaylistChanged, this, [this](Playlist *) {
        refreshTrackList(m_searchEdit->text());
    });
    connect(m_playlistManager, &PlaylistManager::trackAdded, this, [this](Track *) {
        refreshTrackList(m_searchEdit->text());
        setUiEnabled(true);
        m_placeholderLabel->hide();
    });
    connect(m_playlistManager, &PlaylistManager::trackRemoved, this, [this](Track *) {
        refreshTrackList(m_searchEdit->text());
    });

    connect(m_queue, &PlaybackQueue::queueChanged, this, &MainWindow::refreshQueueList);

    connect(m_audio, &AudioController::positionChanged, this, &MainWindow::onPositionChanged);
    connect(m_audio, &AudioController::durationChanged, this, &MainWindow::onDurationChanged);
    connect(m_audio, &AudioController::playbackStateChanged, this, &MainWindow::onPlaybackStateChanged);
    connect(m_audio, &AudioController::trackMetadataUpdated, this, &MainWindow::onTrackMetadataUpdated);
    connect(m_audio, &AudioController::trackChanged, this, &MainWindow::displayTrack);
    connect(m_audio, &AudioController::finished, this, &MainWindow::onAudioFinished);
    connect(m_titleEdit, &QLineEdit::returnPressed, this, &MainWindow::onFixMetadata);
    connect(m_artistEdit, &QLineEdit::returnPressed, this, &MainWindow::onFixMetadata);

    m_audio->setTrackResolver([this](bool forward) { return resolveTrack(forward); });
}

void MainWindow::refreshPlaylistList() {
    const int previousRow = m_playlistList->currentRow();
    m_playlistList->clear();
    for (Playlist *playlist : m_playlistManager->playlists()) {
        auto *item = new QListWidgetItem(playlist->name());
        item->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(playlist)));
        m_playlistList->addItem(item);
    }
    if (!m_playlistManager->playlists().isEmpty()) {
        const int row = qBound(0, m_playlistManager->currentIndex(), m_playlistList->count() - 1);
        m_playlistList->setCurrentRow(previousRow >= 0 ? previousRow : row);
    }
}

void MainWindow::refreshTrackList(const QString &filter) {
    m_trackList->clear();
    Playlist *playlist = m_playlistManager->currentPlaylist();
    if (!playlist) {
        return;
    }
    const QVector<Track *> tracks = playlist->search(filter);
    for (Track *track : tracks) {
        auto *item = new QListWidgetItem(trackListLabel(track));
        item->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(track)));
        m_trackList->addItem(item);
    }
    if (m_trackList->count() == 0) {
        m_placeholderLabel->setVisible(playlist->getTracks().isEmpty());
    }
}

void MainWindow::refreshQueueList() {
    m_queueList->clear();
    for (Track *track : m_queue->tracks()) {
        auto *item = new QListWidgetItem(track->title());
        item->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(track)));
        m_queueList->addItem(item);
    }
    const int idx = m_queue->currentIndex();
    if (idx >= 0 && idx < m_queueList->count()) {
        m_queueList->setCurrentRow(idx);
    }
}

void MainWindow::displayTrack(Track *track) {
    m_editingTrack = track;
    m_updatingMetadata = true;

    if (!track) {
        m_titleEdit->clear();
        m_artistEdit->clear();
        m_cover->clearCover();
        m_descriptionEdit->clear();
        m_totalTimeLabel->setText(QStringLiteral("00:00"));
        updateMetadataEditState(nullptr);
        m_updatingMetadata = false;
        return;
    }

    m_titleEdit->setText(track->title());
    m_artistEdit->setText(track->artist());
    m_totalTimeLabel->setText(track->formattedDuration());
    m_descriptionEdit->setPlainText(track->descriptionText());
    m_cover->loadCoverFromPath(track->coverPath());
    if (track->coverPath().isEmpty() || !QFile::exists(track->coverPath())) {
        m_cover->setPlaceholderText(QStringLiteral("No Cover"));
    }
    updateMetadataEditState(track);

    m_updatingMetadata = false;
}

void MainWindow::applyEditsFromUi(Track *track) {
    if (!track) {
        return;
    }
    track->setTitle(m_titleEdit->text().trimmed().isEmpty() ? track->title() : m_titleEdit->text().trimmed());
    track->setArtist(m_artistEdit->text().trimmed().isEmpty() ? track->artist() : m_artistEdit->text().trimmed());
    track->setDescriptionText(m_descriptionEdit->toPlainText());
}

void MainWindow::updateMetadataEditState(Track *track) {
    const bool locked = track && track->isMetadataLocked();
    m_titleEdit->setReadOnly(locked);
    m_artistEdit->setReadOnly(locked);
    m_descriptionEdit->setReadOnly(locked);
    m_saveMetadataBtn->setEnabled(track != nullptr);
    m_saveMetadataBtn->setText(locked ? QStringLiteral("Изменить") : QStringLiteral("Зафиксировать"));
}

void MainWindow::selectTrackInList(Track *track) {
    if (!track) {
        return;
    }
    for (int i = 0; i < m_trackList->count(); ++i) {
        auto *item = m_trackList->item(i);
        if (reinterpret_cast<Track *>(item->data(Qt::UserRole).value<quintptr>()) == track) {
            m_trackList->setCurrentRow(i);
            return;
        }
    }
}

Track *MainWindow::activeTrack() const {
    if (Track *selected = selectedTrack()) {
        return selected;
    }
    if (m_editingTrack) {
        return m_editingTrack;
    }
    return m_audio->currentTrack();
}

Track *MainWindow::selectedTrack() const {
    QListWidgetItem *item = m_trackList->currentItem();
    if (!item) {
        return nullptr;
    }
    return reinterpret_cast<Track *>(item->data(Qt::UserRole).value<quintptr>());
}

Track *MainWindow::resolveTrack(bool forward) {
    if (!m_queue->isEmpty()) {
        if (forward) {
            return m_queue->advanceNext();
        }
        return m_queue->advancePrevious();
    }
    Playlist *playlist = m_playlistManager->currentPlaylist();
    if (!playlist || playlist->getTracks().isEmpty()) {
        return nullptr;
    }
    Track *current = m_audio->currentTrack();
    int index = current ? playlist->indexOf(current) : -1;
    if (index < 0) {
        return playlist->getTracks().first();
    }
    const int count = playlist->getTracks().size();
    const int nextIndex = forward ? index + 1 : index - 1;
    if (nextIndex < 0 || nextIndex >= count) {
        return nullptr;
    }
    return playlist->getTracks().at(nextIndex);
}

QString MainWindow::formatTime(qint64 ms) const {
    if (ms <= 0) {
        return QStringLiteral("00:00");
    }
    const qint64 totalSeconds = ms / 1000;
    return QStringLiteral("%1:%2")
        .arg(totalSeconds / 60, 2, 10, QChar('0'))
        .arg(totalSeconds % 60, 2, 10, QChar('0'));
}

void MainWindow::updatePlayPauseButton(bool playing) {
    m_playPauseButton->setText(playing ? QStringLiteral("⏸") : QStringLiteral("▶"));
    m_visualizer->setActive(playing);
}

void MainWindow::setUiEnabled(bool hasTracks) {
    m_playPauseButton->setEnabled(hasTracks);
    m_progressSlider->setEnabled(hasTracks);
    if (!hasTracks) {
        m_placeholderLabel->show();
    }
}

void MainWindow::onAddTracks() {
    const QStringList paths = QFileDialog::getOpenFileNames(
        this,
        QStringLiteral("Add Audio Tracks"),
        QString(),
        QStringLiteral("Audio Files (*.mp3 *.wav *.flac);;All Files (*)"));
    if (paths.isEmpty()) {
        return;
    }
    Track *last = nullptr;
    for (const QString &path : paths) {
        last = m_playlistManager->addTrackToCurrent(path);
        m_queue->addTrack(last);
    }
    if (last) {
        displayTrack(last);
        m_trackList->setCurrentRow(m_trackList->count() - 1);
    }
}

void MainWindow::onAddPlaylist() {
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, QStringLiteral("New Playlist"), QStringLiteral("Playlist name:"),
        QLineEdit::Normal, QStringLiteral("My Playlist"), &ok);
    if (!ok || name.trimmed().isEmpty()) {
        return;
    }
    Playlist *playlist = m_playlistManager->createPlaylist(name.trimmed());
    m_playlistManager->setCurrentPlaylist(playlist);
    refreshPlaylistList();
    m_playlistList->setCurrentRow(m_playlistList->count() - 1);
}

void MainWindow::onRemovePlaylist() {
    QListWidgetItem *item = m_playlistList->currentItem();
    if (!item) {
        return;
    }
    auto *playlist = reinterpret_cast<Playlist *>(item->data(Qt::UserRole).value<quintptr>());
    if (m_playlistManager->playlists().size() <= 1) {
        QMessageBox::information(this, QStringLiteral("Playlist"), QStringLiteral("Cannot remove the last playlist."));
        return;
    }
    m_playlistManager->removePlaylist(playlist);
    refreshTrackList();
}

void MainWindow::onRenamePlaylist() {
    QListWidgetItem *item = m_playlistList->currentItem();
    if (!item) {
        return;
    }
    auto *playlist = reinterpret_cast<Playlist *>(item->data(Qt::UserRole).value<quintptr>());
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, QStringLiteral("Rename Playlist"), QStringLiteral("New name:"),
        QLineEdit::Normal, playlist->name(), &ok);
    if (!ok || name.trimmed().isEmpty()) {
        return;
    }
    m_playlistManager->renamePlaylist(playlist, name.trimmed());
    refreshPlaylistList();
}

void MainWindow::onPlaylistSelectionChanged() {
    QListWidgetItem *item = m_playlistList->currentItem();
    if (!item) {
        return;
    }
    auto *playlist = reinterpret_cast<Playlist *>(item->data(Qt::UserRole).value<quintptr>());
    m_playlistManager->setCurrentPlaylist(playlist);
    refreshTrackList(m_searchEdit->text());
}

void MainWindow::onTrackSelectionChanged() {
    displayTrack(selectedTrack());
}

void MainWindow::onTrackDoubleClicked() {
    Track *track = selectedTrack();
    if (!track) {
        return;
    }
    m_audio->loadTrack(track);
    m_audio->play();
    const int queueIndex = m_queue->tracks().indexOf(track);
    if (queueIndex >= 0) {
        m_queue->setCurrentIndex(queueIndex);
        refreshQueueList();
    }
}

void MainWindow::onSearchTextChanged(const QString &text) {
    refreshTrackList(text);
}

void MainWindow::onAddToQueue() {
    Track *track = selectedTrack();
    if (!track) {
        return;
    }
    m_queue->addTrack(track);
}

void MainWindow::onRemoveFromQueue() {
    QListWidgetItem *item = m_queueList->currentItem();
    if (!item) {
        return;
    }
    auto *track = reinterpret_cast<Track *>(item->data(Qt::UserRole).value<quintptr>());
    m_queue->removeTrack(track);
}

void MainWindow::onClearQueue() {
    m_queue->clear();
}

void MainWindow::onQueueSelectionChanged() {
    QListWidgetItem *item = m_queueList->currentItem();
    if (!item) {
        return;
    }
    auto *track = reinterpret_cast<Track *>(item->data(Qt::UserRole).value<quintptr>());
    const int idx = m_queue->tracks().indexOf(track);
    if (idx >= 0) {
        m_queue->setCurrentIndex(idx);
    }
    displayTrack(track);
}

void MainWindow::onPlayPause() {
    if (m_audio->isPlaying()) {
        m_audio->pause();
        return;
    }
    Track *track = m_audio->currentTrack();
    if (!track) {
        track = selectedTrack();
    }
    if (!track) {
        return;
    }
    if (m_audio->currentTrack() != track) {
        m_audio->loadTrack(track);
    }
    m_audio->play();
}

void MainWindow::onStop() {
    m_audio->stop();
    m_progressSlider->setValue(0);
    m_currentTimeLabel->setText(QStringLiteral("00:00"));
}

void MainWindow::onNext() {
    Track *track = resolveTrack(true);
    if (!track) {
        m_audio->stop();
        updatePlayPauseButton(false);
        return;
    }
    if (m_queue->contains(track)) {
        m_queue->setCurrentIndex(m_queue->tracks().indexOf(track));
        refreshQueueList();
    }
    m_audio->loadTrack(track);
    m_audio->play();
    displayTrack(track);
    selectTrackInList(track);
}

void MainWindow::onPrevious() {
    Track *track = resolveTrack(false);
    if (!track) {
        return;
    }
    if (m_queue->contains(track)) {
        m_queue->setCurrentIndex(m_queue->tracks().indexOf(track));
        refreshQueueList();
    }
    m_audio->loadTrack(track);
    m_audio->play();
    displayTrack(track);
    selectTrackInList(track);
}

void MainWindow::onSeek(int position) {
    if (m_audio->duration() > 0) {
        m_audio->setPosition(position);
        m_currentTimeLabel->setText(formatTime(position));
    }
}

void MainWindow::onVolumeChanged(int value) {
    m_audio->setVolume(value / 100.0f);
}

void MainWindow::onSetCover() {
    Track *track = activeTrack();
    if (!track) {
        QMessageBox::information(this, QStringLiteral("Обложка"),
                                 QStringLiteral("Сначала выберите трек в списке."));
        return;
    }

    QString startDir = QDir::homePath();
    if (!track->coverPath().isEmpty()) {
        startDir = QFileInfo(track->coverPath()).absolutePath();
    } else if (!track->filePath().isEmpty()) {
        startDir = QFileInfo(track->filePath()).absolutePath();
    }

    const QString path = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Выберите обложку"),
        startDir,
        QStringLiteral("Images (*.png *.jpg *.jpeg);;All Files (*)"),
        nullptr,
        QFileDialog::DontUseNativeDialog);
    if (path.isEmpty()) {
        return;
    }

    track->setCoverPath(path);
    m_editingTrack = track;
    m_cover->loadCoverFromPath(path);
    TrackStore::save(track);
    selectTrackInList(track);
}

void MainWindow::onFixMetadata() {
    Track *track = activeTrack();
    if (!track) {
        return;
    }

    if (track->isMetadataLocked()) {
        track->setMetadataLocked(false);
        updateMetadataEditState(track);
        return;
    }

    applyEditsFromUi(track);
    track->setMetadataLocked(true);
    TrackStore::save(track);
    displayTrack(track);
    refreshTrackList(m_searchEdit->text());
    selectTrackInList(track);
}

void MainWindow::onDescriptionChanged() {
    if (m_updatingMetadata) {
        return;
    }
    Track *track = activeTrack();
    if (!track || track->isMetadataLocked()) {
        return;
    }
    track->setDescriptionText(m_descriptionEdit->toPlainText());
}

void MainWindow::onPositionChanged(qint64 position) {
    if (!m_seeking) {
        m_progressSlider->blockSignals(true);
        m_progressSlider->setValue(static_cast<int>(position));
        m_progressSlider->blockSignals(false);
    }
    m_currentTimeLabel->setText(formatTime(position));
}

void MainWindow::onDurationChanged(qint64 duration) {
    m_progressSlider->setRange(0, static_cast<int>(duration));
    m_totalTimeLabel->setText(formatTime(duration));
}

void MainWindow::onPlaybackStateChanged(bool playing) {
    updatePlayPauseButton(playing);
}

void MainWindow::onTrackMetadataUpdated(Track *track) {
    if (!track || track->isMetadataLocked()) {
        refreshTrackList(m_searchEdit->text());
        return;
    }
    if (track == m_editingTrack || track == m_audio->currentTrack()) {
        displayTrack(track);
    }
    refreshTrackList(m_searchEdit->text());
}

void MainWindow::onAudioFinished() {
    Track *played = m_audio->currentTrack();
    if (played && m_queue->contains(played)) {
        m_queue->consumeTrack(played);
        refreshQueueList();

        Track *next = m_queue->currentTrack();
        if (next) {
            m_audio->loadTrack(next);
            m_audio->play();
            displayTrack(next);
            selectTrackInList(next);
            return;
        }
    }

    m_audio->stop();
    updatePlayPauseButton(false);
}
