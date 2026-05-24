#include "NowPlayingPanel.h"
#include "../../core/Track.h"
#include "CoverLabel.h"

#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>

NowPlayingPanel::NowPlayingPanel(QWidget *parent)
    : QWidget(parent) {
    setObjectName(QStringLiteral("nowPlayingPanel"));
    setMinimumWidth(300);
    setMaximumWidth(380);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *closeBtn = new QPushButton(QStringLiteral("✕"), this);
    closeBtn->setObjectName(QStringLiteral("textButton"));
    closeBtn->setFixedSize(28, 28);
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &NowPlayingPanel::closeRequested);

    m_cover = new CoverLabel(this);
    m_cover->setMinimumSize(260, 260);
    m_cover->setMaximumHeight(320);

    m_title = new QLabel(this);
    m_title->setObjectName(QStringLiteral("npTitle"));
    m_title->setWordWrap(true);
    m_artist = new QLabel(this);
    m_artist->setObjectName(QStringLiteral("npArtist"));
    m_album = new QLabel(this);
    m_genre = new QLabel(this);
    m_year = new QLabel(this);
    m_duration = new QLabel(this);
    m_path = new QLabel(this);
    m_path->setWordWrap(true);
    m_path->setObjectName(QStringLiteral("npPath"));

    m_description = new QTextEdit(this);
    m_description->setReadOnly(true);
    m_description->setObjectName(QStringLiteral("descriptionEdit"));
    m_description->setMinimumHeight(80);

    m_editBtn = new QPushButton(QStringLiteral("Редактировать метаданные"), this);
    m_editBtn->setObjectName(QStringLiteral("textButton"));
    m_editBtn->setCursor(Qt::PointingHandCursor);
    connect(m_editBtn, &QPushButton::clicked, this, [this] {
        if (m_track) {
            emit editMetadataRequested(m_track);
        }
    });

    auto *topRow = new QHBoxLayout();
    topRow->addStretch();
    topRow->addWidget(closeBtn);

    layout->addLayout(topRow);
    layout->addWidget(m_cover, 0, Qt::AlignHCenter);
    layout->addWidget(m_title);
    layout->addWidget(m_artist);
    layout->addWidget(m_album);
    layout->addWidget(m_genre);
    layout->addWidget(m_year);
    layout->addWidget(m_duration);
    layout->addWidget(m_description, 1);
    layout->addWidget(m_path);
    layout->addWidget(m_editBtn);
}

void NowPlayingPanel::setTrack(Track *track) {
    m_track = track;
    if (!track) {
        clear();
        return;
    }

    m_title->setText(track->title());
    m_artist->setText(QStringLiteral("Исполнитель: %1").arg(track->artist()));
    m_album->setText(QStringLiteral("Альбом: %1").arg(track->album().isEmpty() ? QStringLiteral("—") : track->album()));
    m_genre->setText(QStringLiteral("Жанр: %1").arg(track->genre().isEmpty() ? QStringLiteral("—") : track->genre()));
    m_year->setText(track->year() > 0
                        ? QStringLiteral("Год: %1").arg(track->year())
                        : QStringLiteral("Год: —"));
    m_duration->setText(QStringLiteral("Длительность: %1").arg(track->formattedDuration()));
    m_path->setText(QStringLiteral("Файл: %1").arg(track->filePath()));
    m_description->setPlainText(track->descriptionText().isEmpty()
                                    ? QStringLiteral("Нет текста / заметок")
                                    : track->descriptionText());

    if (!track->coverPath().isEmpty() && QFile::exists(track->coverPath())) {
        m_cover->loadCoverFromPath(track->coverPath());
    } else {
        m_cover->clearCover();
        m_cover->setPlaceholderText(QStringLiteral("No Cover"));
    }
}

void NowPlayingPanel::clear() {
    m_track = nullptr;
    m_title->clear();
    m_artist->clear();
    m_album->clear();
    m_genre->clear();
    m_year->clear();
    m_duration->clear();
    m_path->clear();
    m_description->clear();
    m_cover->clearCover();
}
