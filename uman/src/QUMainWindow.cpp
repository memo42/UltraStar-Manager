#include "main.h"

#include "QUMainWindow.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <QListWidget>
#include <QListWidgetItem>

#include <QDir>
#include <QStringList>
#include <QPixmap>
#include <QIcon>
#include <QBrush>
#include <QHeaderView>
#include <QDateTime>
#include <QSettings>
#include <QMessageBox>
#include <QKeySequence>

#include <QFileDialog>
#include <QFileInfo>
#include <QFileInfoList>
#include <QProgressDialog>

#include "QUSongFile.h"
#include "QUDetailItem.h"
#include "QUMonty.h"
#include "QUDropDownDelegate.h"

#include "fileref.h"
#include "tag.h"
#include "tstring.h"

#include "QUTagOrderDialog.h"
#include "QUTextDialog.h"

QDir QUMainWindow::_baseDir = QDir();
QUMainWindow::QUMainWindow(QWidget *parent): QMainWindow(parent) {
	setupUi(this);
	
	initWindow();
	initMenu();
	initConfig();
	
	initSongTree();
	initDetailsTable();	
	initTaskList();
	
	initMonty();
}

/*!
 * Save configuration for next application start.
 */
QUMainWindow::~QUMainWindow() {
	QSettings settings;
	settings.setValue("allowMonty", QVariant(actionAllowMonty->isChecked()));
	
	settings.setValue("windowState", QVariant(this->saveState()));
}

/*!
 * Initializes the windows registry entry for uman. Lets the user
 * choose a path where the song files are located.
 */
void QUMainWindow::initConfig() {
	QCoreApplication::setOrganizationName("HPI");
	QCoreApplication::setApplicationName("UltraStar Manager");
	     
	QString path;
	QSettings settings;
	if(!settings.contains("songPath")) {
		path = QFileDialog::getExistingDirectory(this, "Choose your UltraStar song directory");
	
		if(!path.isEmpty())
			settings.setValue("songPath", QVariant(path));
	} else {
		path = settings.value("songPath").toString();
	}
	_baseDir.setPath(path);
	
	// read other settings
	actionAllowMonty->setChecked(settings.value("allowMonty", QVariant(true)).toBool());
	actionShowRelativeSongPath->setChecked(settings.value("showRelativeSongPath", QVariant(true)).toBool());
	completerChk->setChecked(settings.value("caseSensitiveAutoCompletion", QVariant(false)).toBool());
	
	this->restoreState(settings.value("windowState", QVariant()).toByteArray());
}

/*!
 * Set up initial window size and title text.
 */
void QUMainWindow::initWindow() {
	setWindowTitle(QString("UltraStar Manager %1.%2").arg(MAJOR_VERSION).arg(MINOR_VERSION));
	
	addDockWidget(Qt::RightDockWidgetArea, detailsDock);
	addDockWidget(Qt::RightDockWidgetArea, tasksDock);
	addDockWidget(Qt::RightDockWidgetArea, eventsDock);
}

void QUMainWindow::initMenu() {
	// song
	connect(actionExpandAll, SIGNAL(triggered()), songTree, SLOT(expandAll()));
	connect(actionExpandAll, SIGNAL(triggered()), this, SLOT(resizeToContents()));
	connect(actionCollapseAll, SIGNAL(triggered()), songTree, SLOT(collapseAll()));
	connect(actionCollapseAll, SIGNAL(triggered()), this, SLOT(resizeToContents()));
	connect(actionRefresh, SIGNAL(triggered()), this, SLOT(refreshSongs()));
	
	actionRefresh->setShortcut(QKeySequence::fromString("F5"));
	
	// view
	this->menuView->addAction(detailsDock->toggleViewAction());
	this->menuView->addAction(tasksDock->toggleViewAction());
	this->menuView->addAction(eventsDock->toggleViewAction());

	// options
	connect(actionShowRelativeSongPath, SIGNAL(toggled(bool)), this, SLOT(toggleRelativeSongPath(bool)));
	connect(actionTagSaveOrder, SIGNAL(triggered()), this, SLOT(editTagOrder()));
	connect(actionChangeSongDirectory, SIGNAL(triggered()), this, SLOT(changeSongDir()));
	
	actionShowRelativeSongPath->setIcon(QIcon(":/types/folder.png"));
		
	// help
	connect(actionShowMonty, SIGNAL(triggered()), helpFrame, SLOT(show()));
	connect(actionQt, SIGNAL(triggered()), this, SLOT(aboutQt()));
	connect(actionUman, SIGNAL(triggered()), this, SLOT(aboutUman()));
	
	actionAllowMonty->setIcon(QIcon(":/monty/normal.png"));
	actionShowMonty->setIcon(QIcon(":/monty/happy.png"));
}

/*!
 * Set up the song tree the first time.
 * \sa createSongFiles()
 * \sa createSongTree()
 */
void QUMainWindow::initSongTree() {
	initSongTreeHeader();
	
	connect(songTree, SIGNAL(itemSelectionChanged()), this, SLOT(updateDetails()));
	connect(songTree, SIGNAL(itemSelectionChanged()), this, SLOT(updateStatusbar()));

	connect(songTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(resetLink(QTreeWidgetItem*, int))); 
	connect(songTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(showSongTextFile(QTreeWidgetItem*, int)));
	
	connect(songTree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(resizeToContents()));
	connect(songTree, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(resizeToContents()));
	
	refreshSongs();
}

void QUMainWindow::initSongTreeHeader() {
	QTreeWidgetItem *header = new QTreeWidgetItem();
	header->setText(0, QString("Folder (%1)").arg(_baseDir.path()));
	header->setIcon(0, QIcon(":/types/folder.png"));
	//header->setText(1, "Artist");
	header->setIcon(1, QIcon(":/types/user.png"));
	header->setToolTip(1, "Shows whether your folder includes the artist correctly:<br><i>Artist - Title ...</i>");
	//header->setText(2, "Title");
	header->setIcon(2, QIcon(":/types/font.png"));
	header->setToolTip(2, "Shows whether your folder includes the title correctly:<br><i>Artist - Title ...</i>");
	//header->setText(3, "Audio");
	header->setIcon(3, QIcon(":/types/music.png"));
	header->setToolTip(3, "Shows whether the song text file points to an <b>audio file</b> that can be found by UltraStar");
	//header->setText(4, "Cover");
	header->setIcon(4, QIcon(":/types/picture.png"));
	header->setToolTip(4, "Shows whether the song text file points to a <b>cover picture</b> that can be found by UltraStar");
	//header->setText(5, "Background");
	header->setIcon(5, QIcon(":/types/picture.png"));
	header->setToolTip(5, "Shows whether the song text file points to a <b>background picture</b> that can be found by UltraStar");
	//header->setText(6, "Video");
	header->setIcon(6, QIcon(":/types/film.png"));
	header->setToolTip(6, "Shows whether the song text file points to a <b>video file</b> that can be found by UltraStar");

	header->setText(7, "Language");
	header->setIcon(7, QIcon(":/types/language.png"));
	header->setText(8, "Edition");
	header->setIcon(8, QIcon(":/types/edition.png"));
	header->setText(9, "Genre");
	header->setIcon(9, QIcon(":/types/genre.png"));
	header->setText(10, "Year");
	header->setIcon(10, QIcon(":/types/date.png"));
	header->setText(11, "Creator");
	header->setIcon(11, QIcon(":/types/creator.png"));
	
	songTree->setHeaderItem(header);	
}

void QUMainWindow::initDetailsTable() {
	detailsTable->verticalHeader()->hide();
	detailsTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	detailsTable->setHorizontalHeaderLabels(QStringList() << "Tag" << "Value");
	
	detailsTable->horizontalHeader()->setResizeMode(0, QHeaderView::Interactive);
	detailsTable->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
	
	connect(detailsTable, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(saveSongChanges(QTableWidgetItem*)));
	
	QUDropDownDelegate *comboDelegate = new QUDropDownDelegate(detailsTable);
	detailsTable->setItemDelegateForColumn(1, comboDelegate);
	
	connect(completerChk, SIGNAL(toggled(bool)), this, SLOT(toggleCompleterChk(bool)));
}

/*!
 * Initializes all available tasks.
 * \sa doTasks()
 * \sa saveSongChanges()
 */
void QUMainWindow::initTaskList() {
	QStringList tasks;
	tasks << "Set missing files automatically";
	tasks << "Remove unsupported tags";
	tasks << "Use ID3 tag for artist";
	tasks << "Use ID3 tag for title";
	tasks << "Use ID3 tag for genre";
	tasks << "Use ID3 tag for year";
	tasks << "Rename directory to \"Artist - Title\"";
	tasks << "Rename directory to \"Artist - Title [VIDEO] [SC]\" if checked or video present. Keep other folder tags.";
	tasks << "Rename songtext file to \"Artist - Title.txt\"";
	tasks << "Rename audio file to \"Artist - Title.*\"";
	tasks << "Rename cover to \"Artist - Title [CO].*\"";
	tasks << "Rename background to \"Artist - Title [BG].*\"";
	tasks << "Rename video to \"Artist - Title.*\"";
	tasks << "Rename video to \"Artist - Title [VD#*].*\" consider VIDEOGAP";
	
	taskList->addItems(tasks);
	
	for(int i = 0; i < taskList->count(); i++) {
		taskList->item(i)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
		taskList->item(i)->setCheckState(Qt::Unchecked);
	}
	
	taskList->item(0)->setIcon(QIcon(":/marks/wand.png"));
	taskList->item(1)->setIcon(QIcon(":/types/folder_blue.png"));
	taskList->item(2)->setIcon(QIcon(":/types/user.png"));
	taskList->item(3)->setIcon(QIcon(":/types/font.png"));
	taskList->item(4)->setIcon(QIcon(":/types/genre.png"));
	taskList->item(5)->setIcon(QIcon(":/types/date.png"));
	taskList->item(6)->setIcon(QIcon(":/types/folder.png"));
	taskList->item(7)->setIcon(QIcon(":/types/folder.png"));
	taskList->item(8)->setIcon(QIcon(":/types/text.png"));
	taskList->item(9)->setIcon(QIcon(":/types/music.png"));
	taskList->item(10)->setIcon(QIcon(":/types/picture.png"));
	taskList->item(11)->setIcon(QIcon(":/types/picture.png"));
	taskList->item(12)->setIcon(QIcon(":/types/film.png"));
	taskList->item(13)->setIcon(QIcon(":/types/film.png"));
	
	// set up tips
	taskList->item(7)->setToolTip("Looks for <b>[SC]</b> in the <b>#EDITION</b> tag to find out whether it is checked or not.<br>"
			"<br>"
			"Other folder tags like <b>[KARAOKE]</b> will be appended to the end if present.");
	taskList->item(13)->setToolTip("Uses <b>#VIDEOGAP:0</b> if tag not present.");
	
	// insert seperators
	taskList->insertItem(0, "Preparatory Tasks");
	taskList->insertItem(3, "ID3 Tag Tasks");
	taskList->insertItem(8, "Renaming Tasks");
	
	QList<int> rows; rows << 0 << 3 << 8;
	foreach(int row, rows) {
		taskList->item(row)->setTextAlignment(Qt::AlignLeft);
		taskList->item(row)->setFlags(Qt::ItemIsEnabled);		
		taskList->item(row)->setBackgroundColor(Qt::darkGray);
		taskList->item(row)->setTextColor(Qt::white);
		
		QFont f(taskList->item(row)->font());
		f.setBold(true);
		taskList->item(row)->setFont(f);
	}
	
	// connect task buttons
	connect(taskBtn, SIGNAL(clicked()), this, SLOT(doTasks()));
	connect(allTasksBtn, SIGNAL(clicked()), this, SLOT(checkAllTasks()));
	connect(noTasksBtn, SIGNAL(clicked()), this, SLOT(uncheckAllTasks()));
	connect(taskList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(uncheckAllExclusiveTasks(QListWidgetItem*)));
}

void QUMainWindow::initMonty() {
	montyLbl->setPixmap(monty->pic(QUMonty::seated));
	helpLbl->setText(monty->welcomeMsg(_songs.size()));
	
	connect(hideMontyBtn, SIGNAL(clicked()), helpFrame, SLOT(hide()));
	
	if(!actionAllowMonty->isChecked())
		helpFrame->hide();
}

/*!
 * Re-reads all possible song files and builds a new song tree.
 */
void QUMainWindow::refreshSongs() {
	songTree->clear();
	updateDetails();
	qDeleteAll(_songs);
	_songs.clear();
	
	createSongFiles();
	createSongTree();
}

/*!
 * Creates all instances of QUSongFile to fill the song tree.
 * \sa CreateSongTree();
 */
void QUMainWindow::createSongFiles() {
	QList<QDir> dirList;
	dirList.append(QDir(_baseDir));
	
	QProgressDialog progress("Looking for songs...", 0, 0, 1, this);
	progress.show();
	
	this->readSongDir(dirList);
	
	progress.setMaximum(dirList.size());
	progress.setLabelText("Reading song files...");
	
	foreach(QDir dir, dirList) {
		progress.setValue(progress.value() + 1);
		
		QStringList files = dir.entryList(QStringList("*.txt"), QDir::Files);
		
		if(!files.isEmpty()) {
			_songs.append(new QUSongFile(QFileInfo(dir, files.first()).filePath()));
		}
	}
}

/*!
 * Lists all available sub (and sub-sub a.s.o.) directories.
 */
void QUMainWindow::readSongDir(QList<QDir> &dirList) {
	if(dirList.last().entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name).isEmpty()) {
		return;
	} else {
		QDir thisDir(dirList.last());
		QStringList subDirs = dirList.last().entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name);
		
		foreach(QString dir, subDirs) {
			QDir newDir(thisDir);
			newDir.cd(dir);
			
			dirList.append(newDir);
			this->readSongDir(dirList);
		}
	}
}

/*!
 * Creates the song tree and enables the file system watcher for
 * the entries.
 */
void QUMainWindow::createSongTree() {
	foreach(QUSongFile* song, _songs) {
		QUSongItem *top = new QUSongItem(song, true);
		songTree->addTopLevelItem(top);
	}
	
	resizeToContents();
	songTree->sortItems(0, Qt::AscendingOrder);
}

void QUMainWindow::updateImage() {
//	QUSongItem *songItem = dynamic_cast<QUSongItem*>(songTable->currentItem());
//	
//	if(!songItem)
//		return;
//	
//	if(songTable->currentColumn() == 4) { // show cover
//		imageLbl->setPixmap(QPixmap(songItem->song()->coverFileInfo().filePath()));		
//	} else if(songTable->currentColumn() == 5) { // show background
//		imageLbl->setPixmap(QPixmap(songItem->song()->backgroundFileInfo().filePath()));		
//	}
}

void QUMainWindow::resizeToContents() {
	for(int i = 0; i < songTree->columnCount(); i++)
		songTree->resizeColumnToContents(i);
}

void QUMainWindow::resetLink(QTreeWidgetItem *item, int column) {
	if(column < 3 or column > 6)
		return;
	
	QUSongItem *songItem = dynamic_cast<QUSongItem*>(item);
	
	if(!songItem || songItem->isToplevel())
		return;
	
	QUSongFile *song = songItem->song();
	
	QString fileScheme("*." + QFileInfo(songItem->text(0)).suffix());
	
	if( songItem->icon(3).isNull() 
			and QUSongFile::allowedAudioFiles().contains(fileScheme, Qt::CaseInsensitive) 
			and column == 3 ) {
		addLogMsg(QString("Audio file changed from \"%1\" to: \"%2\".").arg(song->mp3()).arg(songItem->text(0)));
		song->setInfo(MP3_TAG, songItem->text(0));
		song->save();
	} else if( songItem->icon(4).isNull() 
			and QUSongFile::allowedPictureFiles().contains(fileScheme, Qt::CaseInsensitive) 
			and column == 4 ) {
		addLogMsg(QString("Cover changed from \"%1\" to: \"%2\".").arg(song->cover()).arg(songItem->text(0)));
		song->setInfo(COVER_TAG, songItem->text(0));
		song->save();
	} else if( songItem->icon(5).isNull() 
			and QUSongFile::allowedPictureFiles().contains(fileScheme, Qt::CaseInsensitive) 
			and column == 5 ) {
		addLogMsg(QString("Background changed from \"%1\" to: \"%2\".").arg(song->background()).arg(songItem->text(0)));
		song->setInfo(BACKGROUND_TAG, songItem->text(0));
		song->save();
	} else if( songItem->icon(6).isNull() 
			and QUSongFile::allowedVideoFiles().contains(fileScheme, Qt::CaseInsensitive) 
			and column == 6 ) {
		addLogMsg(QString("Video file changed from \"%1\" to: \"%2\".").arg(song->video()).arg(songItem->text(0)));
		song->setInfo(VIDEO_TAG, songItem->text(0));
		song->save();
	}

	(dynamic_cast<QUSongItem*>(songItem->parent()))->update();
	updateDetails();
	
	montyTalk();
}

void QUMainWindow::updateDetails() {
	detailsTable->clearContents();
	
	// build a list with all songs
	QList<QUSongFile*> songs;
	
	if(songTree->selectedItems().isEmpty()) { // nothing selected? Try to use the current item...
		QUSongItem *songItem = dynamic_cast<QUSongItem*>(songTree->currentItem());
		if(songItem)
			songs.append(songItem->song());
	} else { // look for all songs in the selection
		foreach(QTreeWidgetItem *item, songTree->selectedItems()) {
			QUSongItem *songItem = dynamic_cast<QUSongItem*>(item);
			if(songItem)
				songs.append(songItem->song());			
		}
	}
	
	if(songs.isEmpty())
		return;
	
	detailsTable->setItem(0, 0, new QTableWidgetItem(QIcon(":/types/font.png"), "Title"));
	detailsTable->setItem(1, 0, new QTableWidgetItem(QIcon(":/types/user.png"), "Artist"));
	detailsTable->setItem(2, 0, new QTableWidgetItem(QIcon(":/types/language.png"), "Language"));
	detailsTable->setItem(3, 0, new QTableWidgetItem(QIcon(":/types/edition.png"), "Edition"));
	detailsTable->setItem(4, 0, new QTableWidgetItem(QIcon(":/types/genre.png"), "Genre"));
	detailsTable->setItem(5, 0, new QTableWidgetItem(QIcon(":/types/date.png"), "Year"));
	detailsTable->setItem(6, 0, new QTableWidgetItem(QIcon(":/types/creator.png"), "Creator"));
	
	detailsTable->setItem(7, 0, new QTableWidgetItem("File Tags"));
	
	detailsTable->setItem(8, 0, new QTableWidgetItem(QIcon(":/types/music.png"), "MP3"));
	detailsTable->setItem(9, 0, new QTableWidgetItem(QIcon(":/types/picture.png"), "Cover"));
	detailsTable->setItem(10, 0, new QTableWidgetItem(QIcon(":/types/picture.png"), "Background"));
	detailsTable->setItem(11, 0, new QTableWidgetItem(QIcon(":/types/film.png"), "Video"));
	
	for(int i = 0; i < 12; i++)
		detailsTable->item(i, 0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	
	detailsTable->setItem(0, 1, new QUDetailItem(TITLE_TAG, songs));
	detailsTable->setItem(1, 1, new QUDetailItem(ARTIST_TAG, songs));
	detailsTable->setItem(2, 1, new QUDetailItem(LANGUAGE_TAG, songs));
	detailsTable->setItem(3, 1, new QUDetailItem(EDITION_TAG, songs));
	detailsTable->setItem(4, 1, new QUDetailItem(GENRE_TAG, songs));
	detailsTable->setItem(5, 1, new QUDetailItem(YEAR_TAG, songs));
	detailsTable->setItem(6, 1, new QUDetailItem(CREATOR_TAG, songs));
	
	detailsTable->setItem(7, 1, new QTableWidgetItem());
	
	detailsTable->setItem(8, 1, new QUDetailItem(MP3_TAG, songs));
	detailsTable->setItem(9, 1, new QUDetailItem(COVER_TAG, songs));
	detailsTable->setItem(10, 1, new QUDetailItem(BACKGROUND_TAG, songs));
	detailsTable->setItem(11, 1, new QUDetailItem(VIDEO_TAG, songs));

	detailsTable->item(7, 0)->setFlags(Qt::ItemIsEnabled);
	detailsTable->item(7, 1)->setFlags(Qt::ItemIsEnabled);
	
	// other song details, not editable
	detailsTable->setItem(12, 0, new QTableWidgetItem("Read-Only Tags"));
	
	detailsTable->setItem(13, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "Videogap"));
	detailsTable->setItem(14, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "Start"));
	detailsTable->setItem(15, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "End"));
	detailsTable->setItem(16, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "Relative"));
	detailsTable->setItem(17, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "BPM"));
	detailsTable->setItem(18, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "Gap"));
	
	detailsTable->setItem(12, 1, new QTableWidgetItem());
		
	detailsTable->setItem(13, 1, new QUDetailItem(VIDEOGAP_TAG, songs));
	detailsTable->setItem(14, 1, new QUDetailItem(START_TAG, songs));
	detailsTable->setItem(15, 1, new QUDetailItem(END_TAG, songs));
	detailsTable->setItem(16, 1, new QUDetailItem(RELATIVE_TAG, songs));
	detailsTable->setItem(17, 1, new QUDetailItem(BPM_TAG, songs));
	detailsTable->setItem(18, 1, new QUDetailItem(GAP_TAG, songs));
	
	for(int i = 12; i < 19; i++)
		detailsTable->item(i, 0)->setFlags(Qt::ItemIsEnabled);
	
	// set up the splitter
	detailsTable->setSpan(7, 0, 1, 2);
	detailsTable->setSpan(12, 0, 1, 2);
	
	QList<int> rows; rows << 7 << 12;
	QList<int> cols; cols << 0 << 1;
	
	foreach(int row, rows) {
		foreach(int col, cols) {
			detailsTable->item(row, col)->setFlags(Qt::ItemIsEnabled);
			detailsTable->item(row, col)->setBackgroundColor(Qt::darkGray);
			detailsTable->item(row, col)->setTextColor(Qt::white);
			detailsTable->item(row, col)->setTextAlignment(Qt::AlignCenter);
			
			QFont font(detailsTable->item(row, col)->font());
			font.setBold(true);
			// TODO: make the font smaller and the table row too
			
			detailsTable->item(row, col)->setFont(font);
		}
	}	
}

/*!
 * Displays ID3 tag information of a selected MP3 file in the status bar.
 */
void QUMainWindow::updateStatusbar() {
	QUSongItem *item = dynamic_cast<QUSongItem*>(songTree->currentItem());
	
	if(!item)
		return;
	
	QFileInfo fi(item->song()->songFileInfo().dir(), item->text(0));
	QString fileScheme("*." + fi.suffix());
	
	if(!QUSongFile::allowedAudioFiles().contains(fileScheme, Qt::CaseInsensitive)) {
		this->statusBar()->clearMessage();
		return; // read only audio files for ID3 tag
	}
	
	TagLib::FileRef ref(fi.absoluteFilePath().toLocal8Bit().data());
	
	QString text("Audio file selected: ARTIST = \"%1\"; TITLE = \"%2\"; GENRE = \"%3\"; YEAR = \"%4\"");
	
	QString artist(TStringToQString(ref.tag()->artist())); if(artist == "") artist = "n/a";
	QString title(TStringToQString(ref.tag()->title())); if(title == "") title = "n/a";
	QString genre(TStringToQString(ref.tag()->genre())); if(genre == "") genre = "n/a";
	QString year(QVariant(ref.tag()->year()).toString()); if(year == "0") year = "n/a";
	
	this->statusBar()->showMessage(text.arg(artist).arg(title).arg(genre).arg(year));
}

/*!
 * Save all changes of the details into the song file.
 * \sa updateDetails()
 */
void QUMainWindow::saveSongChanges(QTableWidgetItem *item) {
	QUDetailItem *detailItem = dynamic_cast<QUDetailItem*>(detailsTable->currentItem());
	
	if(!detailItem)
		return;

	// save changes for each song
	foreach(QUSongFile *song, detailItem->songs()) {
		song->setInfo(detailItem->tag(), detailItem->text());
		song->save();		
	}
	
	// update all selected items with these new details in the song tree
	QList<QTreeWidgetItem*> selectedItems = songTree->selectedItems();
	
	if(selectedItems.isEmpty()) { // no songs selected? Try the current item...
		QUSongItem *songItem = dynamic_cast<QUSongItem*>(songTree->currentItem());
		if(songItem)
			songItem->update();
	} else {
		foreach(QTreeWidgetItem *i, selectedItems) {
			QUSongItem *songItem = dynamic_cast<QUSongItem*>(i);
			if(songItem)
				songItem->update();			
		}
	}
	
	updateDetails(); // to show "n/a" if text was deleted completely
}

void QUMainWindow::checkAllTasks() {
	for(int i = 0; i < taskList->count(); i++) {
		if(taskList->item(i)->flags() & Qt::ItemIsUserCheckable)
			taskList->item(i)->setCheckState(Qt::Checked);
	}
}

void QUMainWindow::uncheckAllTasks() {
	for(int i = 0; i < taskList->count(); i++) {
		if(taskList->item(i)->flags() & Qt::ItemIsUserCheckable)
			taskList->item(i)->setCheckState(Qt::Unchecked);
	}
}

/*!
 * Unchecks all tasks that cannot be used with several other
 * tasks together.
 */
void QUMainWindow::uncheckAllExclusiveTasks(QListWidgetItem *item) {
	if(taskList->row(item) == 9 
			and (item->checkState() == Qt::Checked)
			and (taskList->item(10)->checkState() == Qt::Checked) ) {
		taskList->item(10)->setCheckState(Qt::Unchecked);
	} else if(taskList->row(item) == 10 
			and (item->checkState() == Qt::Checked)
			and (taskList->item(9)->checkState() == Qt::Checked) ) {
		taskList->item(9)->setCheckState(Qt::Unchecked);
	} else if(taskList->row(item) == 15 
			and (item->checkState() == Qt::Checked)
			and (taskList->item(16)->checkState() == Qt::Checked) ) {
		taskList->item(16)->setCheckState(Qt::Unchecked);
	} else if(taskList->row(item) == 16 
			and (item->checkState() == Qt::Checked)
			and (taskList->item(15)->checkState() == Qt::Checked) ) {
		taskList->item(15)->setCheckState(Qt::Unchecked);
	}	
}

/*!
 * Does all checked tasks for all selected songs. You can only select
 * toplevel items (folders) which represent songs.
 * \sa initTaskList();
 */
void QUMainWindow::doTasks() {
	QProgressDialog progressDlg("", 0, 0, 1, this);	
	
	if(songTree->selectedItems().size() > 10) {
		progressDlg.setMaximum(songTree->selectedItems().size());
		progressDlg.setValue(0);
		progressDlg.setLabelText("Processing song files...");
		progressDlg.show();
	}
	
	QList<QTreeWidgetItem*> itemList = songTree->selectedItems();
	
	if(itemList.isEmpty()) // if no songs are selected use the current item (which has also a song)
		itemList.append(songTree->currentItem());
	
	foreach(QTreeWidgetItem *item, itemList) {
		QUSongItem *songItem = dynamic_cast<QUSongItem*>(item);

		if(songItem) {	
			QUSongFile *song = songItem->song();
			
			if(taskList->item(1)->checkState() == Qt::Checked)
				songItem->autoSetFiles();
			if(taskList->item(2)->checkState() == Qt::Checked)
				this->removeUnsupportedTags(song);
			// seperator here
			if(taskList->item(4)->checkState() == Qt::Checked)
				useID3TagForArtist(song);
			if(taskList->item(5)->checkState() == Qt::Checked)
				useID3TagForTitle(song);
			if(taskList->item(6)->checkState() == Qt::Checked)
				useID3TagForGenre(song);
			if(taskList->item(7)->checkState() == Qt::Checked)
				useID3TagForYear(song);
			// seperator here
			if(taskList->item(9)->checkState() == Qt::Checked)
				renameSongDir(song);
			if(taskList->item(10)->checkState() == Qt::Checked)
				renameSongDirCheckedVideo(song);
			if(taskList->item(11)->checkState() == Qt::Checked)
				renameSongTxt(song);
			if(taskList->item(12)->checkState() == Qt::Checked)
				renameSongMp3(song);
			if(taskList->item(13)->checkState() == Qt::Checked)
				renameSongCover(song);
			if(taskList->item(14)->checkState() == Qt::Checked)
				renameSongBackground(song);
			if(taskList->item(15)->checkState() == Qt::Checked)
				renameSongVideo(song);
			if(taskList->item(16)->checkState() == Qt::Checked)
				renameSongVideogap(song);

			song->save();
			songItem->update();
		}
		
		progressDlg.setValue(progressDlg.value() + 1);
	}

	updateDetails();
	progressDlg.hide();
	montyTalk();
}

void QUMainWindow::useID3TagForArtist(QUSongFile *song) {
	QString done("ID3Tag of \"%1\" used for artist. Changed from: \"%2\" to: \"%3\".");
	QString fail("ID3Tag was NOT available to be used for artist: \"%1\"");
	
	QString oldArtist(song->artist());
	
	if(song->useID3TagForArtist()) {
		addLogMsg(done.arg(song->mp3()).arg(oldArtist).arg(song->artist()));
	} else {
		addLogMsg(fail.arg(song->mp3()), 1);
	}
}

void QUMainWindow::useID3TagForTitle(QUSongFile *song) {
	QString done("ID3Tag of \"%1\" used for title. Changed from: \"%2\" to: \"%3\".");
	QString fail("ID3Tag was NOT available to be used for title: \"%1\"");
	
	QString oldTitle(song->title());
	
	if(song->useID3TagForTitle()) {
		addLogMsg(done.arg(song->mp3()).arg(oldTitle).arg(song->title()));
	} else {
		addLogMsg(fail.arg(song->mp3()), 1);
	}	
}

void QUMainWindow::useID3TagForGenre(QUSongFile *song) {
	QString done("ID3Tag of \"%1\" used for genre. Changed from: \"%2\" to: \"%3\".");
	QString fail("ID3Tag was NOT available to be used for genre: \"%1\"");
	
	QString oldGenre(song->genre());
	
	if(song->useID3TagForGenre()) {
		addLogMsg(done.arg(song->mp3()).arg(oldGenre).arg(song->genre()));
	} else {
		addLogMsg(fail.arg(song->mp3()), 1);
	}
}

void QUMainWindow::useID3TagForYear(QUSongFile *song) {
	QString done("ID3Tag of \"%1\" used for year. Changed from: \"%2\" to: \"%3\".");
	QString fail("ID3Tag was NOT available to be used for year: \"%1\"");
	
	QString oldYear(song->year());
	
	if(song->useID3TagForYear()) {
		addLogMsg(done.arg(song->mp3()).arg(oldYear).arg(song->year()));
	} else {
		addLogMsg(fail.arg(song->mp3()), 1);
	}
}

void QUMainWindow::renameSongDir(QUSongFile *song) {
	QString done("Song directory renamed from: \"%1\" to: \"%2\".");
	QString fail("Song directory: \"%1\" was NOT renamed.");
	
	QString oldName(song->songFileInfo().dir().dirName());
	
	if(song->renameSongDir(song->artist() + " - " + song->title())) {
		addLogMsg(done.arg(oldName).arg(song->songFileInfo().dir().dirName()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}
}

void QUMainWindow::renameSongDirCheckedVideo(QUSongFile *song) {
	QString done("Song directory renamed from: \"%1\" to: \"%2\".");
	QString fail("Song directory: \"%1\" was NOT renamed.");
	
	QString oldName(song->songFileInfo().dir().dirName());
	QString newName("%1 - %2");
	
	if(song->hasVideo())
		newName.append(" [VIDEO]");
	
	if(song->edition().contains("[SC]", Qt::CaseInsensitive))
		newName.append(" [SC]");
	
	// extract additional folder tags from old name -> "[...]"
	QStringList folderTags(oldName.split("[").filter("]"));
	folderTags.sort();
	foreach(QString folderTag, folderTags) {
		if(!folderTag.contains("sc", Qt::CaseInsensitive) and !folderTag.contains("video", Qt::CaseInsensitive))
			newName.append(" [" + folderTag.trimmed());
	}
	
	// do the renaming...
	if(song->renameSongDir(newName.arg(song->artist()).arg(song->title()))) {
		addLogMsg(done.arg(oldName).arg(song->songFileInfo().dir().dirName()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}
}

void QUMainWindow::renameSongTxt(QUSongFile *song) {
	QString done("Song text file renamed from: \"%1\" to: \"%2\".");
	QString fail("Song text file: \"%1\" was NOT renamed.");
	
	QString oldName(song->songFileInfo().fileName());
	
	if(song->renameSongTxt(song->artist() + " - " + song->title() + ".txt")) {
		addLogMsg(done.arg(oldName).arg(song->songFileInfo().fileName()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}	
}

void QUMainWindow::renameSongMp3(QUSongFile *song) {
	QString done("Audio file renamed from: \"%1\" to: \"%2\".");
	QString fail("Audio file: \"%1\" was NOT renamed.");
	
	QString oldName(song->mp3());
	
	if(song->renameSongMp3(song->artist() + " - " + song->title() + "." + QFileInfo(song->mp3()).suffix().toLower())) {
		addLogMsg(done.arg(oldName).arg(song->mp3()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}		
}

void QUMainWindow::renameSongCover(QUSongFile *song) {
	QString done("Cover file renamed from: \"%1\" to: \"%2\".");
	QString fail("Cover file: \"%1\" was NOT renamed.");

	QString oldName(song->cover());
	
	if(song->renameSongCover(song->artist() + " - " + song->title() + " [CO]." + QFileInfo(song->cover()).suffix().toLower())) {
		addLogMsg(done.arg(oldName).arg(song->cover()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}	
}

void QUMainWindow::renameSongBackground(QUSongFile *song) {
	QString done("Background file renamed from: \"%1\" to: \"%2\".");
	QString fail("Background file: \"%1\" was NOT renamed.");
	
	QString oldName(song->background());
	
	if(song->renameSongBackground(song->artist() + " - " + song->title() + " [BG]." + QFileInfo(song->background()).suffix().toLower())) {
		addLogMsg(done.arg(oldName).arg(song->background()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}	
}

void QUMainWindow::renameSongVideo(QUSongFile *song) {
	QString done("Video file renamed from: \"%1\" to: \"%2\".");
	QString fail("Video file: \"%1\" was NOT renamed.");
	
	QString oldName(song->video());
	
	if(song->renameSongVideo(song->artist() + " - " + song->title() + "." + QFileInfo(song->video()).suffix().toLower())) {
		addLogMsg(done.arg(oldName).arg(song->video()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}	
}

void QUMainWindow::renameSongVideogap(QUSongFile *song) {
	QString done("Video file renamed from: \"%1\" to: \"%2\".");
	QString fail("Video file: \"%1\" was NOT renamed.");
	
	QString oldName(song->video());
	QString newName("%1 - %2 [VD#%3]." + QFileInfo(song->video()).suffix().toLower());
	
	// set videogap 0 for the videoname
	QString vgap(song->videogap()); if(vgap == "n/a") vgap = "0";
	
	if(song->renameSongVideo(newName.arg(song->artist()).arg(song->title()).arg(vgap))) {
		addLogMsg(done.arg(oldName).arg(song->video()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}		
}

void QUMainWindow::removeUnsupportedTags(QUSongFile *song) {
	QStringList removedTags(song->unsupportedTags().split("\n#"));
	
	song->removeUnsupportedTags(); // always works until now :)
	
	foreach(QString removedTag, removedTags) {
		addLogMsg(QString("Unsupported Tag removed: #%1").arg(removedTag));
	}
}

void QUMainWindow::addLogMsg(const QString &msg, int type) {
	if(type == 0 and disableInfoChk->isChecked())
		return;
	if(type == 1 and disableWarningChk->isChecked())
		return;
	
	log->insertItem(0, QDateTime::currentDateTime().toString("[hh:mm:ss] ") + msg);
	
	switch(type) {
	case 0: log->item(0)->setIcon(QIcon(":/marks/information.png")); break;
	case 1: log->item(0)->setIcon(QIcon(":/marks/error.png")); break;
	case 2: log->item(0)->setIcon(QIcon(":/marks/help.png")); break;
	}
	
}

void QUMainWindow::aboutQt() {
	QMessageBox::aboutQt(this, "About Qt");
}

void QUMainWindow::aboutUman() {
	QString aboutStr("<b>UltraStar Manager</b><br>"
			"Version %1.%2.%3<br>"
			"<br>"
			"�2008 by Marcel Taeumel<br>"
			"<br>"
			"<i>Tested By</i><br>"
			"Michael Gr�newald");
	
	QMessageBox::about(this, "About", aboutStr.arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(PATCH_VERSION));
}

void QUMainWindow::editTagOrder() {
	QUTagOrderDialog *dlg = new QUTagOrderDialog(this);
	dlg->exec();
	delete dlg;	
	
	montyTalk();
}

/*!
 * Enables the user to set up a new location where all
 * UltraStar songs can be found.
 */
void QUMainWindow::changeSongDir() {
	QSettings settings;
	QString path = settings.value("songPath").toString();
	
	path = QFileDialog::getExistingDirectory(this, "Choose your UltraStar song directory", path);
	
	if(!path.isEmpty()) {
		settings.setValue("songPath", QVariant(path));
		_baseDir.setPath(path);
		refreshSongs();
		
		monty->setSongCount(_songs.size());
		montyTalk();
		
		addLogMsg(QString("UltraStar song directory changed to: \"%1\".").arg(_baseDir.path()));
		
		songTree->headerItem()->setText(0, QString("Folder (%1)").arg(_baseDir.path()));
	}
}

void QUMainWindow::montyTalk() {
	if(!actionAllowMonty->isChecked())
		return;
	
	helpFrame->show();
	monty->talk(montyLbl, helpLbl);
}

/*!
 * Updates all toplevel items to display the relative song path
 * or only the song directory - like selected.
 */
void QUMainWindow::toggleRelativeSongPath(bool checked) {
	QSettings settings;
	settings.setValue("showRelativeSongPath", QVariant(checked));
	
	QList<QUSongItem*> tmpList; // tmp list needed so avoid sorting problems
	
	for(int i = 0; i < songTree->topLevelItemCount(); i++)
		tmpList.append(dynamic_cast<QUSongItem*>(songTree->topLevelItem(i)));
	
	foreach(QUSongItem *item, tmpList) {
		item->updateAsDirectory(checked);
	}
	
	if(checked)
		addLogMsg("Relative song paths are displayed in the song tree now.");
	else
		addLogMsg("Only song directories are displayed in the song tree now.");
}

/*!
 * Toggle whether the completion for detail edition should be case-sensitive.
 */
void QUMainWindow::toggleCompleterChk(bool checked) {
	QSettings settings;
	settings.setValue("caseSensitiveAutoCompletion", QVariant(checked));
}

/*!
 * Shows the content of the current song text file. (read-only)
 */
void QUMainWindow::showSongTextFile(QTreeWidgetItem *item, int column) {
	if(column != 0)
		return;
	
	QUSongItem *songItem = dynamic_cast<QUSongItem*>(item);
	
	if(!songItem)
		return;
	
	QString fileScheme("*." + QFileInfo(item->text(0)).suffix());
	
	if(QString::compare(fileScheme, "*.txt", Qt::CaseInsensitive) != 0)
		return; // display only text files
	
	QUTextDialog *dlg = new QUTextDialog(songItem->song(), this);
	
	dlg->exec();
	
	delete dlg;
}
