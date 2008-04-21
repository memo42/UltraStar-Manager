#include "QUBooleanSongData.h"

QUBooleanSongData::QUBooleanSongData(const QString &tag, QObject *parent): QUAbstractReportData(parent) {
	_tag = tag;
	this->setType(QU::icon);
	
	if(QString::compare(_tag, MP3_TAG, Qt::CaseInsensitive) == 0) {
		this->setIcon(QIcon(":/types/music.png"));
		this->setDescription(tr("Audio file exists?"));
	} else if(QString::compare(_tag, COVER_TAG, Qt::CaseInsensitive) == 0) {
		this->setIcon(QIcon(":/types/picture.png"));
		this->setDescription(tr("Cover file exists?"));
	} else if(QString::compare(_tag, BACKGROUND_TAG, Qt::CaseInsensitive) == 0) {
		this->setIcon(QIcon(":/types/picture.png"));
		this->setDescription(tr("Background file exists?"));
	} else if(QString::compare(_tag, VIDEO_TAG, Qt::CaseInsensitive) == 0) {
		this->setIcon(QIcon(":/types/film.png"));
		this->setDescription(tr("Video file exists?"));
	}
}

QString QUBooleanSongData::data(QUSongFile *song) {
	bool result = false;
	
	if(QString::compare(_tag, MP3_TAG, Qt::CaseInsensitive) == 0)
		result = song->hasMp3();
	else if(QString::compare(_tag, COVER_TAG, Qt::CaseInsensitive) == 0)
		result = song->hasCover();
	else if(QString::compare(_tag, BACKGROUND_TAG, Qt::CaseInsensitive) == 0)
		result = song->hasBackground();
	else if(QString::compare(_tag, VIDEO_TAG, Qt::CaseInsensitive) == 0)
		result = song->hasVideo();

	if(result)
		return ":/marks/tick.png";
	else
		return ":/marks/cross.png";
}

QString QUBooleanSongData::headerData() {
	return this->description();
}
