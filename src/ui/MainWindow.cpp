#include "MainWindow.h"

#include "../core/AudioController.h"
#include "../core/LibraryStore.h"
#include "../core/ListeningStats.h"
#include "../core/PlaybackQueue.h"
#include "../core/Playlist.h"
#include "../core/PlaylistManager.h"
#include "../core/Track.h"
#include "dialogs/ProfileDialog.h"
#include "dialogs/TrackMetadataDialog.h"
#include "widgets/CoverLabel.h"
#include "widgets/NeonButton.h"
#include "widgets/NowPlayingPanel.h"
#include "widgets/ParticleBackground.h"
#include "widgets/SpectrumVisualizer.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QResizeEvent>
#include <QSlider>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>

namespace {

bool isAudioFile(const QString &path) {
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QStringLiteral("mp3") || suffix == QStringLiteral("wav")
           || suffix == QStringLiteral("flac");
}

QString trackListLabel(Track *track, bool favorite) {
    const QString heart = favorite ? QStringLiteral("♥ ") : QString();
    const QString missing = track->fileExists() ? QString() : QStringLiteral(" [нет файла]");
    return QStringLiteral("%1%2 — %3  [%4]%5")
        .arg(heart, track->artist(), track->title(), track->formattedDuration(), missing);
}

QVector<QPair<QString, qint64>> topEntries(const QHash<QString, qint64> &data,
                                           const std::function<QString(const QString &)> &resolver,
                                           int limit = 8) {
    QVector<QPair<QString, qint64>> entries;
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        entries.append({resolver(it.key()), it.value()});
    }
    std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b) {
        return a.second > b.second;
    });
    if (entries.size() > limit) {
        entries.resize(limit);
    }
    for (auto &entry : entries) {
        entry.second /= 60000; // minutes for chart
    }
    return entries;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_playlistManager(new PlaylistManager(this))
    , m_queue(new PlaybackQueue(this))
    , m_audio(new AudioController(this))
    , m_stats(new ListeningStats(this))
    , m_statsTimer(new QTimer(this)) {
    setWindowTitle(QStringLiteral("Neon Audio Player"));
    resize(1200, 680);
    setMinimumSize(960, 540);
    setAcceptDrops(true);

    LibraryStore::load(m_playlistManager, m_stats);

    setupUi();
    loadStyleSheet();
    connectSignals();

    refreshPlaylistList();
    refreshTrackList();
    refreshQueueList();
    bool hasTracks = false;
    for (Playlist *playlist : m_playlistManager->playlists()) {
        if (!playlist->getTracks().isEmpty()) {
            hasTracks = true;
            break;
        }
    }
    setUiEnabled(hasTracks);
    if (hasTracks) {
        m_placeholderLabel->hide();
    }
    syncOverlayGeometry();
}

MainWindow::~MainWindow() {
    saveLibrary();
}

void MainWindow::setupUi() {
    m_centralContainer = new QWidget(this);
    m_centralContainer->setObjectName(QStringLiteral("centralContainer"));
    setCentralWidget(m_centralContainer);

    m_particles = new ParticleBackground(m_centralContainer);
    m_contentWidget = new QWidget(m_centralContainer);
    m_contentWidget->setObjectName(QStringLiteral("contentLayer"));
    m_contentWidget->setAttribute(Qt::WA_StyledBackground, true);

    auto *rootLayout = new QVBoxLayout(m_contentWidget);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(6);

    m_mainSplitter = new QSplitter(Qt::Horizontal, m_contentWidget);

    auto *sidebar = new QWidget(m_mainSplitter);
    sidebar->setObjectName(QStringLiteral("sidebar"));
    sidebar->setMinimumWidth(220);
    sidebar->setMaximumWidth(280);
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(6, 6, 6, 6);
    sidebarLayout->setSpacing(4);

    auto *profileBtn = new QPushButton(QStringLiteral("Профиль"), sidebar);
    profileBtn->setObjectName(QStringLiteral("textButton"));
    profileBtn->setCursor(Qt::PointingHandCursor);

    auto *playlistHeader = new QLabel(QStringLiteral("Playlists"), sidebar);
    playlistHeader->setObjectName(QStringLiteral("sectionHeader"));
    m_playlistList = new QListWidget(sidebar);
    m_playlistList->setObjectName(QStringLiteral("playlistList"));

    auto *playlistButtons = new QHBoxLayout();
    auto *addPlaylistBtn = new NeonButton(QStringLiteral("+"), sidebar);
    addPlaylistBtn->setFixedSize(32, 32);
    auto *renamePlaylistBtn = new NeonButton(QStringLiteral("✎"), sidebar);
    renamePlaylistBtn->setFixedSize(32, 32);
    auto *removePlaylistBtn = new NeonButton(QStringLiteral("−"), sidebar);
    removePlaylistBtn->setFixedSize(32, 32);
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
    auto *removeQueueBtn = new NeonButton(QStringLiteral("Q−"), sidebar);
    removeQueueBtn->setFixedSize(32, 32);
    auto *clearQueueBtn = new NeonButton(QStringLiteral("Clr"), sidebar);
    clearQueueBtn->setFixedSize(32, 32);
    queueButtons->addWidget(addQueueBtn);
    queueButtons->addWidget(removeQueueBtn);
    queueButtons->addWidget(clearQueueBtn);

    sidebarLayout->addWidget(profileBtn);
    sidebarLayout->addWidget(playlistHeader);
    sidebarLayout->addWidget(m_playlistList, 2);
    sidebarLayout->addLayout(playlistButtons);
    sidebarLayout->addWidget(queueHeader);
    sidebarLayout->addWidget(m_queueList, 1);
    sidebarLayout->addLayout(queueButtons);

    auto *centerPanel = new QWidget(m_mainSplitter);
    centerPanel->setObjectName(QStringLiteral("centerPanel"));
    auto *centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setContentsMargins(6, 6, 6, 6);
    centerLayout->setSpacing(6);

    m_placeholderLabel = new QLabel(QStringLiteral("Перетащите треки сюда или нажмите Add Tracks"), centerPanel);
    m_placeholderLabel->setObjectName(QStringLiteral("placeholderLabel"));
    m_placeholderLabel->setAlignment(Qt::AlignCenter);

    m_cover = new CoverLabel(centerPanel);
    m_cover->setFixedSize(140, 140);

    m_titleEdit = new QLineEdit(centerPanel);
    m_titleEdit->setObjectName(QStringLiteral("trackTitleEdit"));
    m_titleEdit->setPlaceholderText(QStringLiteral("Title"));
    m_artistEdit = new QLineEdit(centerPanel);
    m_artistEdit->setObjectName(QStringLiteral("trackArtistEdit"));
    m_artistEdit->setPlaceholderText(QStringLiteral("Artist"));

    auto *metaButtons = new QHBoxLayout();
    auto *setCoverBtn = new QPushButton(QStringLiteral("Обложка"), centerPanel);
    setCoverBtn->setObjectName(QStringLiteral("textButton"));
    auto *editMetaBtn = new QPushButton(QStringLiteral("Метаданные…"), centerPanel);
    editMetaBtn->setObjectName(QStringLiteral("textButton"));
    m_saveMetadataBtn = new QPushButton(QStringLiteral("Зафиксировать"), centerPanel);
    m_saveMetadataBtn->setObjectName(QStringLiteral("textButton"));
    for (QPushButton *btn : {setCoverBtn, editMetaBtn, m_saveMetadataBtn}) {
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMaximumHeight(28);
    }
    metaButtons->addWidget(setCoverBtn);
    metaButtons->addWidget(editMetaBtn);
    metaButtons->addWidget(m_saveMetadataBtn);
    metaButtons->addStretch();

    auto *trackInfoLayout = new QVBoxLayout();
    trackInfoLayout->addWidget(m_titleEdit);
    trackInfoLayout->addWidget(m_artistEdit);
    trackInfoLayout->addLayout(metaButtons);

    auto *topRow = new QHBoxLayout();
    topRow->addWidget(m_cover, 0, Qt::AlignTop);
    topRow->addLayout(trackInfoLayout, 1);

    m_visualizer = new SpectrumVisualizer(24, centerPanel);
    m_visualizer->setFixedHeight(52);

    m_descriptionEdit = new QTextEdit(centerPanel);
    m_descriptionEdit->setObjectName(QStringLiteral("descriptionEdit"));
    m_descriptionEdit->setPlaceholderText(QStringLiteral("Заметки к треку…"));
    m_descriptionEdit->setMaximumHeight(80);

    auto *searchRow = new QHBoxLayout();
    m_searchEdit = new QLineEdit(centerPanel);
    m_searchEdit->setObjectName(QStringLiteral("searchEdit"));
    m_searchEdit->setPlaceholderText(QStringLiteral("Поиск…"));
    auto *addTracksBtn = new QPushButton(QStringLiteral("Add Tracks"), centerPanel);
    addTracksBtn->setObjectName(QStringLiteral("textButton"));
    addTracksBtn->setCursor(Qt::PointingHandCursor);
    searchRow->addWidget(m_searchEdit, 1);
    searchRow->addWidget(addTracksBtn);

    m_trackList = new QListWidget(centerPanel);
    m_trackList->setObjectName(QStringLiteral("trackList"));
    m_trackList->setAcceptDrops(false);

    centerLayout->addWidget(m_placeholderLabel);
    centerLayout->addLayout(topRow);
    centerLayout->addWidget(m_visualizer);
    centerLayout->addWidget(m_descriptionEdit);
    centerLayout->addLayout(searchRow);
    centerLayout->addWidget(m_trackList, 1);

    m_nowPlayingPanel = new NowPlayingPanel(m_mainSplitter);
    m_nowPlayingPanel->hide();

    m_mainSplitter->addWidget(sidebar);
    m_mainSplitter->addWidget(centerPanel);
    m_mainSplitter->addWidget(m_nowPlayingPanel);
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);
    m_mainSplitter->setStretchFactor(2, 0);

    auto *playerBar = new QWidget(m_contentWidget);
    playerBar->setObjectName(QStringLiteral("playerBar"));
    playerBar->setMaximumHeight(96);
    auto *playerLayout = new QVBoxLayout(playerBar);
    playerLayout->setContentsMargins(8, 6, 8, 6);
    playerLayout->setSpacing(4);

    auto *progressRow = new QHBoxLayout();
    m_nowPlayingCover = new CoverLabel(playerBar);
    m_nowPlayingCover->setFixedSize(44, 44);
    m_nowPlayingCover->setCursor(Qt::PointingHandCursor);
    m_nowPlayingCover->setToolTip(QStringLiteral("Сейчас играет — открыть панель"));

    m_currentTimeLabel = new QLabel(QStringLiteral("00:00"), playerBar);
    m_currentTimeLabel->setObjectName(QStringLiteral("timeLabel"));
    m_currentTimeLabel->setFixedWidth(40);
    m_progressSlider = new QSlider(Qt::Horizontal, playerBar);
    m_progressSlider->setObjectName(QStringLiteral("progressSlider"));
    m_progressSlider->setFixedHeight(18);
    m_heartBtn = new QPushButton(QStringLiteral("♡"), playerBar);
    m_heartBtn->setObjectName(QStringLiteral("heartButton"));
    m_heartBtn->setFixedSize(32, 32);
    m_heartBtn->setCursor(Qt::PointingHandCursor);
    m_heartBtn->setToolTip(QStringLiteral("Любимое"));
    m_totalTimeLabel = new QLabel(QStringLiteral("00:00"), playerBar);
    m_totalTimeLabel->setObjectName(QStringLiteral("timeLabel"));
    m_totalTimeLabel->setFixedWidth(40);

    progressRow->addWidget(m_nowPlayingCover);
    progressRow->addWidget(m_currentTimeLabel);
    progressRow->addWidget(m_progressSlider, 1);
    progressRow->addWidget(m_heartBtn);
    progressRow->addWidget(m_totalTimeLabel);

    auto *controlsRow = new QHBoxLayout();
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

    m_volumeSlider = new QSlider(Qt::Horizontal, playerBar);
    m_volumeSlider->setObjectName(QStringLiteral("volumeSlider"));
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(75);
    m_volumeSlider->setMaximumWidth(120);
    auto *volumeIcon = new QLabel(QStringLiteral("🔊"), playerBar);
    auto *volumeRow = new QHBoxLayout();
    volumeRow->addStretch();
    volumeRow->addWidget(volumeIcon);
    volumeRow->addWidget(m_volumeSlider);

    playerLayout->addLayout(progressRow);
    playerLayout->addLayout(controlsRow);
    playerLayout->addLayout(volumeRow);

    rootLayout->addWidget(m_mainSplitter, 1);
    rootLayout->addWidget(playerBar, 0);

    connect(profileBtn, &QPushButton::clicked, this, &MainWindow::onProfile);
    connect(addPlaylistBtn, &QPushButton::clicked, this, &MainWindow::onAddPlaylist);
    connect(renamePlaylistBtn, &QPushButton::clicked, this, &MainWindow::onRenamePlaylist);
    connect(removePlaylistBtn, &QPushButton::clicked, this, &MainWindow::onRemovePlaylist);
    connect(addQueueBtn, &QPushButton::clicked, this, &MainWindow::onAddToQueue);
    connect(removeQueueBtn, &QPushButton::clicked, this, &MainWindow::onRemoveFromQueue);
    connect(clearQueueBtn, &QPushButton::clicked, this, &MainWindow::onClearQueue);
    connect(setCoverBtn, &QPushButton::clicked, this, &MainWindow::onSetCover);
    connect(editMetaBtn, &QPushButton::clicked, this, &MainWindow::onOpenMetadataEditor);
    connect(m_saveMetadataBtn, &QPushButton::clicked, this, &MainWindow::onFixMetadata);
    connect(addTracksBtn, &QPushButton::clicked, this, &MainWindow::onAddTracks);
    connect(prevBtn, &QPushButton::clicked, this, &MainWindow::onPrevious);
    connect(m_playPauseButton, &QPushButton::clicked, this, &MainWindow::onPlayPause);
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::onStop);
    connect(nextBtn, &QPushButton::clicked, this, &MainWindow::onNext);
    connect(m_heartBtn, &QPushButton::clicked, this, &MainWindow::onToggleFavorite);
    connect(m_nowPlayingCover, &CoverLabel::clicked, this, &MainWindow::onToggleNowPlaying);

    syncOverlayGeometry();
}

void MainWindow::syncOverlayGeometry() {
    if (!m_centralContainer || !m_particles || !m_contentWidget) {
        return;
    }
    const QRect r = m_centralContainer->rect();
    m_particles->setGeometry(r);
    m_contentWidget->setGeometry(r);
    m_particles->lower();
    m_contentWidget->raise();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    syncOverlayGeometry();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    QStringList paths;
    for (const QUrl &url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            paths.append(url.toLocalFile());
        }
    }
    importAudioFiles(paths);
    event->acceptProposedAction();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveLibrary();
    QMainWindow::closeEvent(event);
}

void MainWindow::loadStyleSheet() {
    QFile styleFile(QStringLiteral(":/styles/main.qss"));
    if (!styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        styleFile.setFileName(QStringLiteral("resources/styles/main.qss"));
        if (!styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
    }
    qApp->setStyleSheet(QString::fromUtf8(styleFile.readAll()));
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
        saveLibrary();
    });
    connect(m_playlistManager, &PlaylistManager::trackRemoved, this, [this](Track *) {
        refreshTrackList(m_searchEdit->text());
        saveLibrary();
    });
    connect(m_playlistManager, &PlaylistManager::favoritesChanged, this, [this](Track *, bool) {
        refreshTrackList(m_searchEdit->text());
        updateHeartButton();
        saveLibrary();
    });
    connect(m_playlistManager, &PlaylistManager::libraryChanged, this, &MainWindow::saveLibrary);

    connect(m_queue, &PlaybackQueue::queueChanged, this, &MainWindow::refreshQueueList);

    connect(m_audio, &AudioController::positionChanged, this, &MainWindow::onPositionChanged);
    connect(m_audio, &AudioController::durationChanged, this, &MainWindow::onDurationChanged);
    connect(m_audio, &AudioController::playbackStateChanged, this, &MainWindow::onPlaybackStateChanged);
    connect(m_audio, &AudioController::trackMetadataUpdated, this, &MainWindow::onTrackMetadataUpdated);
    connect(m_audio, &AudioController::trackChanged, this, [this](Track *track) {
        displayTrack(track);
        syncNowPlayingUi(track);
    });
    connect(m_audio, &AudioController::finished, this, &MainWindow::onAudioFinished);

    connect(m_nowPlayingPanel, &NowPlayingPanel::editMetadataRequested, this, &MainWindow::openMetadataDialog);
    connect(m_nowPlayingPanel, &NowPlayingPanel::closeRequested, this, [this] {
        m_nowPlayingVisible = false;
        m_nowPlayingPanel->hide();
    });

    connect(m_statsTimer, &QTimer::timeout, this, &MainWindow::onStatsTick);
    m_statsTimer->setInterval(1000);

    m_audio->setTrackResolver([this](bool forward) { return resolveTrack(forward); });
}

void MainWindow::saveLibrary() {
    LibraryStore::save(m_playlistManager, m_stats);
}

void MainWindow::importAudioFiles(const QStringList &paths) {
    Track *last = nullptr;
    for (const QString &path : paths) {
        if (!isAudioFile(path)) {
            continue;
        }
        last = m_playlistManager->addTrackToCurrent(path);
        if (last) {
            m_queue->addTrack(last);
        }
    }
    if (last) {
        displayTrack(last);
        selectTrackInList(last);
        saveLibrary();
    }
}

void MainWindow::refreshPlaylistList() {
    const int previousRow = m_playlistList->currentRow();
    m_playlistList->clear();
    for (Playlist *playlist : m_playlistManager->playlists()) {
        QString label = playlist->name();
        if (playlist->isFavorites()) {
            label = QStringLiteral("❤ %1").arg(playlist->name());
        }
        auto *item = new QListWidgetItem(label);
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
    const bool favPlaylist = playlist->isFavorites();
    for (Track *track : playlist->search(filter)) {
        const bool fav = m_playlistManager->isFavorite(track);
        auto *item = new QListWidgetItem(trackListLabel(track, fav && !favPlaylist));
        item->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(track)));
        if (!track->fileExists()) {
            item->setForeground(QColor(255, 120, 120));
        }
        m_trackList->addItem(item);
    }
    m_placeholderLabel->setVisible(playlist->getTracks().isEmpty());
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
        updateHeartButton();
        m_updatingMetadata = false;
        return;
    }

    m_titleEdit->setText(track->title());
    m_artistEdit->setText(track->artist());
    m_totalTimeLabel->setText(track->formattedDuration());
    m_descriptionEdit->setPlainText(track->descriptionText());
    m_cover->loadCoverFromPath(track->coverPath());
    m_cover->setFavoriteBadge(m_playlistManager->isFavorite(track));
    if (track->coverPath().isEmpty() || !QFile::exists(track->coverPath())) {
        m_cover->setPlaceholderText(m_playlistManager->isFavorite(track) ? QStringLiteral("❤") : QStringLiteral("No Cover"));
    }
    updateMetadataEditState(track);
    updateHeartButton();
    m_updatingMetadata = false;
}

void MainWindow::syncNowPlayingUi(Track *track) {
    if (!track) {
        m_nowPlayingCover->clearCover();
        m_nowPlayingCover->setFavoriteBadge(false);
        return;
    }
    m_nowPlayingCover->loadCoverFromPath(track->coverPath());
    m_nowPlayingCover->setFavoriteBadge(m_playlistManager->isFavorite(track));
    if (m_nowPlayingVisible) {
        m_nowPlayingPanel->setTrack(track);
    }
    updateHeartButton();
}

void MainWindow::applyEditsFromUi(Track *track) {
    if (!track) {
        return;
    }
    if (!m_titleEdit->text().trimmed().isEmpty()) {
        track->setTitle(m_titleEdit->text().trimmed());
    }
    if (!m_artistEdit->text().trimmed().isEmpty()) {
        track->setArtist(m_artistEdit->text().trimmed());
    }
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

void MainWindow::updateHeartButton() {
    Track *track = m_audio->currentTrack() ? m_audio->currentTrack() : activeTrack();
    const bool fav = track && m_playlistManager->isFavorite(track);
    m_heartBtn->setText(fav ? QStringLiteral("♥") : QStringLiteral("♡"));
    m_heartBtn->setStyleSheet(fav ? QStringLiteral("color: #ff00cc; font-size: 16px;") : QString());
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
        return forward ? m_queue->advanceNext() : m_queue->advancePrevious();
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
    const int nextIndex = forward ? index + 1 : index - 1;
    if (nextIndex < 0 || nextIndex >= playlist->getTracks().size()) {
        return nullptr;
    }
    return playlist->getTracks().at(nextIndex);
}

QString MainWindow::formatTime(qint64 ms) const {
    return Track::formatMs(ms);
}

qint64 MainWindow::trackDurationForSlider(Track *track) const {
    if (!track) {
        return m_audio->duration();
    }
    const qint64 effective = track->effectiveDuration();
    if (effective > 0) {
        return effective;
    }
    return m_audio->duration();
}

void MainWindow::updatePlayPauseButton(bool playing) {
    m_playPauseButton->setText(playing ? QStringLiteral("⏸") : QStringLiteral("▶"));
    m_visualizer->setActive(playing);
}

void MainWindow::setUiEnabled(bool hasTracks) {
    m_playPauseButton->setEnabled(hasTracks);
    m_progressSlider->setEnabled(hasTracks);
    m_heartBtn->setEnabled(hasTracks);
}

void MainWindow::openMetadataDialog(Track *track) {
    if (!track) {
        track = activeTrack();
    }
    if (!track) {
        return;
    }

    TrackMetadataDialog dialog(this);
    dialog.setTrack(track);
    if (dialog.exec() == QDialog::Accepted) {
        displayTrack(track);
        refreshTrackList(m_searchEdit->text());
        syncNowPlayingUi(track);
        if (m_nowPlayingVisible) {
            m_nowPlayingPanel->setTrack(track);
        }
        saveLibrary();
    }
}

void MainWindow::onAddTracks() {
    const QStringList paths = QFileDialog::getOpenFileNames(
        this,
        QStringLiteral("Добавить треки"),
        QString(),
        QStringLiteral("Audio (*.mp3 *.wav *.flac);;All Files (*)"));
    importAudioFiles(paths);
}

void MainWindow::onAddPlaylist() {
    bool ok = false;
    const QString name = QInputDialog::getText(this, QStringLiteral("Плейлист"), QStringLiteral("Название:"),
                                               QLineEdit::Normal, QStringLiteral("My Playlist"), &ok);
    if (!ok || name.trimmed().isEmpty()) {
        return;
    }
    Playlist *playlist = m_playlistManager->createPlaylist(name.trimmed());
    m_playlistManager->setCurrentPlaylist(playlist);
    refreshPlaylistList();
}

void MainWindow::onRemovePlaylist() {
    QListWidgetItem *item = m_playlistList->currentItem();
    if (!item) {
        return;
    }
    auto *playlist = reinterpret_cast<Playlist *>(item->data(Qt::UserRole).value<quintptr>());
    if (playlist->isFavorites()) {
        QMessageBox::information(this, QStringLiteral("Плейлист"), QStringLiteral("Плейлист «Любимое» нельзя удалить."));
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
    if (playlist->isFavorites()) {
        return;
    }
    bool ok = false;
    const QString name = QInputDialog::getText(this, QStringLiteral("Переименовать"), QStringLiteral("Название:"),
                                               QLineEdit::Normal, playlist->name(), &ok);
    if (ok && !name.trimmed().isEmpty()) {
        m_playlistManager->renamePlaylist(playlist, name.trimmed());
        refreshPlaylistList();
    }
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
    if (!track->fileExists()) {
        QMessageBox::warning(this, QStringLiteral("Файл не найден"),
                             QStringLiteral("Файл отсутствует по пути:\n%1").arg(track->filePath()));
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
    if (track) {
        m_queue->addTrack(track);
    }
}

void MainWindow::onRemoveFromQueue() {
    QListWidgetItem *item = m_queueList->currentItem();
    if (!item) {
        return;
    }
    m_queue->removeTrack(reinterpret_cast<Track *>(item->data(Qt::UserRole).value<quintptr>()));
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
    Track *track = m_audio->currentTrack() ? m_audio->currentTrack() : selectedTrack();
    if (!track) {
        return;
    }
    if (!track->fileExists()) {
        QMessageBox::warning(this, QStringLiteral("Файл не найден"), track->filePath());
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
    if (!track->fileExists()) {
        QMessageBox::warning(this, QStringLiteral("Файл не найден"), track->filePath());
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
    if (!track || !track->fileExists()) {
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
    const qint64 maxDur = trackDurationForSlider(m_audio->currentTrack());
    if (maxDur > 0) {
        m_audio->setPosition(position);
        m_currentTimeLabel->setText(formatTime(position));
    }
}

void MainWindow::onVolumeChanged(int value) {
    m_audio->setVolume(value / 100.0f);
}

void MainWindow::onSetCover() {
    openMetadataDialog(activeTrack());
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
    displayTrack(track);
    refreshTrackList(m_searchEdit->text());
    saveLibrary();
}

void MainWindow::onOpenMetadataEditor() {
    openMetadataDialog(activeTrack());
}

void MainWindow::onToggleFavorite() {
    Track *track = m_audio->currentTrack() ? m_audio->currentTrack() : activeTrack();
    if (!track) {
        return;
    }
    m_playlistManager->toggleFavorite(track);
    displayTrack(track);
    syncNowPlayingUi(track);
}

void MainWindow::onToggleNowPlaying() {
    Track *track = m_audio->currentTrack();
    if (!track) {
        return;
    }
    m_nowPlayingVisible = !m_nowPlayingVisible;
    m_nowPlayingPanel->setVisible(m_nowPlayingVisible);
    if (m_nowPlayingVisible) {
        m_nowPlayingPanel->setTrack(track);
    }
}

void MainWindow::onProfile() {
    const auto trackResolver = [this](const QString &id) -> QString {
        Track *track = m_playlistManager->findTrackById(id);
        return track ? track->title() : id.left(8);
    };

    ProfileDialog dialog(this);
    dialog.setStats(
        m_stats->totalListenMs(),
        topEntries(m_stats->trackListenMs(), trackResolver),
        topEntries(m_stats->playlistListenMs(), [](const QString &key) { return key; }));
    dialog.exec();
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
    Q_UNUSED(duration)
    Track *track = m_audio->currentTrack();
    const qint64 dur = trackDurationForSlider(track);
    m_progressSlider->setRange(0, static_cast<int>(qMax<qint64>(0, dur)));
    m_totalTimeLabel->setText(formatTime(dur));
}

void MainWindow::onPlaybackStateChanged(bool playing) {
    updatePlayPauseButton(playing);
    if (playing) {
        m_stats->startSession(m_audio->currentTrack(), m_playlistManager->currentPlaylist());
        m_statsTimer->start();
    } else {
        m_stats->stopSession();
        m_statsTimer->stop();
        saveLibrary();
    }
}

void MainWindow::onStatsTick() {
    m_stats->tick();
}

void MainWindow::onTrackMetadataUpdated(Track *track) {
    if (!track || track->isMetadataLocked()) {
        refreshTrackList(m_searchEdit->text());
        return;
    }
    if (track == m_editingTrack || track == m_audio->currentTrack()) {
        displayTrack(track);
        syncNowPlayingUi(track);
    }
    refreshTrackList(m_searchEdit->text());
    saveLibrary();
}

void MainWindow::onAudioFinished() {
    Track *played = m_audio->currentTrack();
    if (played && m_queue->contains(played)) {
        m_queue->consumeTrack(played);
        refreshQueueList();
        Track *next = m_queue->currentTrack();
        if (next && next->fileExists()) {
            m_audio->loadTrack(next);
            m_audio->play();
            displayTrack(next);
            selectTrackInList(next);
            return;
        }
    }
    m_audio->stop();
    updatePlayPauseButton(false);
    m_stats->stopSession();
    m_statsTimer->stop();
}
