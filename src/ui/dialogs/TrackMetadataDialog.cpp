#include "TrackMetadataDialog.h"
#include "../../core/Track.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {
qint64 parseDurationInput(const QString &text) {
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return 0;
    }
    const QStringList parts = trimmed.split(':');
    if (parts.size() == 2) {
        bool okMin = false;
        bool okSec = false;
        const int min = parts.at(0).toInt(&okMin);
        const int sec = parts.at(1).toInt(&okSec);
        if (okMin && okSec) {
            return static_cast<qint64>(min) * 60000 + sec * 1000;
        }
    }
    bool ok = false;
    const int sec = trimmed.toInt(&ok);
    return ok ? static_cast<qint64>(sec) * 1000 : 0;
}

QString durationInputFromMs(qint64 ms) {
    if (ms <= 0) {
        return QString();
    }
    return Track::formatMs(ms);
}
} // namespace

TrackMetadataDialog::TrackMetadataDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(QStringLiteral("Редактирование метаданных"));
    setModal(true);
    setMinimumWidth(460);
    setObjectName(QStringLiteral("metadataDialog"));

    auto *root = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    m_title = new QLineEdit(this);
    m_artist = new QLineEdit(this);
    m_album = new QLineEdit(this);
    m_genre = new QLineEdit(this);
    m_year = new QLineEdit(this);
    m_duration = new QLineEdit(this);
    m_duration->setPlaceholderText(QStringLiteral("мм:сс"));
    m_useCustomDuration = new QCheckBox(QStringLiteral("Использовать свою длительность"), this);

    auto *coverRow = new QHBoxLayout();
    m_coverPath = new QLineEdit(this);
    auto *browseCover = new QPushButton(QStringLiteral("Обложка…"), this);
    browseCover->setObjectName(QStringLiteral("textButton"));
    coverRow->addWidget(m_coverPath, 1);
    coverRow->addWidget(browseCover);

    m_description = new QTextEdit(this);
    m_description->setMinimumHeight(100);

    form->addRow(QStringLiteral("Название"), m_title);
    form->addRow(QStringLiteral("Исполнитель"), m_artist);
    form->addRow(QStringLiteral("Альбом"), m_album);
    form->addRow(QStringLiteral("Жанр"), m_genre);
    form->addRow(QStringLiteral("Год"), m_year);
    form->addRow(QStringLiteral("Длительность"), m_duration);
    form->addRow(QString(), m_useCustomDuration);
    form->addRow(QStringLiteral("Путь обложки"), coverRow);
    form->addRow(QStringLiteral("Текст / заметки"), m_description);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(browseCover, &QPushButton::clicked, this, &TrackMetadataDialog::onBrowseCover);
    connect(buttons, &QDialogButtonBox::accepted, this, &TrackMetadataDialog::onSave);
    connect(buttons, &QDialogButtonBox::rejected, this, &TrackMetadataDialog::onCancel);

    root->addLayout(form);
    root->addWidget(buttons);
}

void TrackMetadataDialog::setTrack(Track *track) {
    m_track = track;
    if (!track) {
        return;
    }
    m_title->setText(track->title());
    m_artist->setText(track->artist());
    m_album->setText(track->album());
    m_genre->setText(track->genre());
    m_year->setText(track->year() > 0 ? QString::number(track->year()) : QString());
    m_coverPath->setText(track->coverPath());
    m_description->setPlainText(track->descriptionText());
    const qint64 override = track->durationOverride();
    m_useCustomDuration->setChecked(override > 0);
    m_duration->setText(override > 0 ? durationInputFromMs(override) : durationInputFromMs(track->duration()));
}

Track *TrackMetadataDialog::track() const { return m_track; }

void TrackMetadataDialog::onBrowseCover() {
    const QString path = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Обложка"),
        m_coverPath->text(),
        QStringLiteral("Images (*.png *.jpg *.jpeg)"),
        nullptr,
        QFileDialog::DontUseNativeDialog);
    if (!path.isEmpty()) {
        m_coverPath->setText(path);
    }
}

void TrackMetadataDialog::applyToTrack() {
    if (!m_track) {
        return;
    }
    m_track->setTitle(m_title->text().trimmed());
    m_track->setArtist(m_artist->text().trimmed());
    m_track->setAlbum(m_album->text().trimmed());
    m_track->setGenre(m_genre->text().trimmed());
    m_track->setYear(m_year->text().toInt());
    m_track->setCoverPath(m_coverPath->text().trimmed());
    m_track->setDescriptionText(m_description->toPlainText());
    if (m_useCustomDuration->isChecked()) {
        m_track->setDurationOverride(parseDurationInput(m_duration->text()));
    } else {
        m_track->setDurationOverride(0);
    }
    m_track->setMetadataLocked(true);
}

void TrackMetadataDialog::onSave() {
    applyToTrack();
    emit saved(m_track);
    accept();
}

void TrackMetadataDialog::onCancel() {
    reject();
}
