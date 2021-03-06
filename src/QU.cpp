#include "QU.h"
#include "QUSongSupport.h"

#include <QDir>

QDir QU::BaseDir = QDir();
QU::QU(QObject *parent): QObject(parent) {}

/*!
 * Sort the fileinfo according to the order of the appearance in the supported types.
 */
bool QU::fileTypeLessThan(const QFileInfo &fi1, const QFileInfo &fi2) {
	QString suffix1 = QString("*.%1").arg(fi1.suffix());
	QString suffix2 = QString("*.%1").arg(fi2.suffix());

	if( QUSongSupport::allowedSongFiles().contains(suffix1) && QUSongSupport::allowedSongFiles().contains(suffix2) ) {
		foreach(QString type, QUSongSupport::allowedSongFiles()) {
			if(QString::compare(type, suffix1, Qt::CaseInsensitive) == 0)
				return true;
			if(QString::compare(type, suffix2, Qt::CaseInsensitive) == 0)
				return false;
		}
	} else if(QUSongSupport::allowedAudioFiles().contains(suffix1) && QUSongSupport::allowedAudioFiles().contains(suffix2)) {
		foreach(QString type, QUSongSupport::allowedAudioFiles()) {
			if(QString::compare(type, suffix1, Qt::CaseInsensitive) == 0)
				return true;
			if(QString::compare(type, suffix2, Qt::CaseInsensitive) == 0)
				return false;
		}
	} else if(QUSongSupport::allowedImageFiles().contains(suffix1) && QUSongSupport::allowedImageFiles().contains(suffix2)) {
		foreach(QString type, QUSongSupport::allowedImageFiles()) {
			if(QString::compare(type, suffix1, Qt::CaseInsensitive) == 0)
				return true;
			if(QString::compare(type, suffix2, Qt::CaseInsensitive) == 0)
				return false;
		}
	} else if(QUSongSupport::allowedVideoFiles().contains(suffix1) && QUSongSupport::allowedVideoFiles().contains(suffix2)) {
		foreach(QString type, QUSongSupport::allowedVideoFiles()) {
			if(QString::compare(type, suffix1, Qt::CaseInsensitive) == 0)
				return true;
			if(QString::compare(type, suffix2, Qt::CaseInsensitive) == 0)
				return false;
		}
	} else  if(QUSongSupport::allowedUltraStarPlaylistFiles().contains(suffix1) && QUSongSupport::allowedUltraStarPlaylistFiles().contains(suffix2)) {
		foreach(QString type, QUSongSupport::allowedUltraStarPlaylistFiles()) {
			if(QString::compare(type, suffix1, Qt::CaseInsensitive) == 0)
				return true;
			if(QString::compare(type, suffix2, Qt::CaseInsensitive) == 0)
				return false;
		}
	} else  if(QUSongSupport::allowedVocaluxePlaylistFiles().contains(suffix1) && QUSongSupport::allowedVocaluxePlaylistFiles().contains(suffix2)) {
		foreach(QString type, QUSongSupport::allowedVocaluxePlaylistFiles()) {
			if(QString::compare(type, suffix1, Qt::CaseInsensitive) == 0)
				return true;
			if(QString::compare(type, suffix2, Qt::CaseInsensitive) == 0)
				return false;
		}
	}

	return (QString::compare(suffix1, suffix2) < 0);
}
