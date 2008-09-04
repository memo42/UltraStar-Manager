#include "QUHtmlReport.h"
#include "QUMonty.h"
#include "QUProgressDialog.h"
#include "QUMainWindow.h"

#include <QDomNodeList>
#include <QFile>

QUHtmlReport::QUHtmlReport(
		const QList<QUSongFile*> &songFiles,
		const QList<QUAbstractReportData*> &reportDataList,
		const QFileInfo &fi,
		QU::ReportOptions options,
		const QVariant &userData,
		const QString &cssFilePath,
		QObject *parent):
	QUAbstractReport(songFiles, reportDataList, fi, options, userData, parent)
{
	_css.setFile(cssFilePath);

	this->createHead();
	this->createBody();

	// copy style sheet to destination
	QFile::copy(_css.filePath(), QFileInfo(fileInfo().dir(), _css.fileName()).filePath());
}

QString QUHtmlReport::content() const {
	return _report.toString();
}

QDomElement QUHtmlReport::html() {
	if(_report.firstChildElement("html").isNull())
		_report.appendChild(_report.createElement("html"));

	return _report.firstChildElement("html");
}

void QUHtmlReport::createHead() {
	QDomElement  head = _report.createElement("head");
	QDomElement  link = _report.createElement("link");
	QDomElement title = _report.createElement("title");

	title.appendChild(_report.createTextNode(tr("UltraStar Manager - Song Report")));

	link.setAttribute("href", _css.fileName());
	link.setAttribute("txpe", "text/css");
	link.setAttribute("rel", "stylesheet");

	head.appendChild(title);
	head.appendChild(link);

	this->html().appendChild(head);
}

void QUHtmlReport::createBody() {
	QDomElement body = _report.createElement("body");

	appendBasePath(body);
	appendUserData(body);
	appendSongsTable(body);

	this->html().appendChild(body);
}

void QUHtmlReport::appendBasePath(QDomNode &parent) {
	QDomElement div = _report.createElement("div");
	div.appendChild(_report.createTextNode(QString(tr("Songs Path: \"%1\"")).arg(QUMainWindow::BaseDir.path())));
	div.setAttribute("class", "path");

	if(options().testFlag(QU::reportPrependCurrentPath))
		parent.appendChild(div);
}

void QUHtmlReport::appendUserData(QDomNode &parent) {
	QDomElement div = _report.createElement("div");
	div.appendChild(_report.createTextNode(userData().toString()));
	div.setAttribute("class", "userdata");

	if(options().testFlag(QU::reportPrependUserData))
		parent.appendChild(div);
}

void QUHtmlReport::appendSongsTable(QDomNode &parent) {
	QDomElement table = _report.createElement("table");

	appendSongsTableHead(table);
	appendSongsTableBody(table);

	parent.appendChild(table);
}

void QUHtmlReport::appendSongsTableHead(QDomNode &parent) {
	QDomElement thead = _report.createElement("thead");
	QDomElement tr    = _report.createElement("tr");

	foreach(QUAbstractReportData *rd, reportDataList()) {
		QDomElement th = _report.createElement("th");

		if(!rd->headerIconData().isEmpty()) {
			QDomElement img = _report.createElement("img");
			QDomAttr src = _report.createAttribute("src");
			QDomAttr alt = _report.createAttribute("alt");
			QDomAttr title = _report.createAttribute("title");

			src.setNodeValue(monty->useImageFromResource(rd->headerIconData(), fileInfo().dir()));
			alt.setNodeValue(rd->headerTextData());
			title.setNodeValue(rd->headerTextData());

			img.setAttributeNode(src);
			img.setAttributeNode(alt);
			img.setAttributeNode(title);

			th.appendChild(img);
		} else if(!rd->headerTextData().isEmpty())
			th.appendChild(_report.createTextNode(rd->headerTextData()));

		tr.appendChild(th);
	}

	thead.appendChild(tr);
	parent.appendChild(thead);
}

void QUHtmlReport::appendSongsTableBody(QDomNode &parent) {
	QDomElement tbody = _report.createElement("tbody");

	QUProgressDialog pDlg(tr("Creating HTML report..."), songs().size());
	pDlg.setPixmap(":/types/folder.png");
	pDlg.show();
	bool odd = true;

	foreach(QUSongFile *song, songs()) {
		pDlg.update(QString("%1 - %2").arg(song->artist()).arg(song->title()));
		if(pDlg.cancelled()) break;

		QDomElement tr = _report.createElement("tr");
		tr.setAttribute("class", odd ? "odd" : "even"); odd = !odd;

		foreach(QUAbstractReportData *rd, reportDataList()) {
			QDomElement td = _report.createElement("td");

			if(!rd->iconData(song).isEmpty()) {
				QDomElement img = _report.createElement("img");
				QDomAttr src = _report.createAttribute("src");
				QDomAttr alt = _report.createAttribute("alt");
				QDomAttr title = _report.createAttribute("title");

				src.setNodeValue(monty->useImageFromResource(rd->iconData(song), fileInfo().dir()));
				alt.setNodeValue(rd->textData(song));
				title.setNodeValue(rd->textData(song));

				img.setAttributeNode(src);
				img.setAttributeNode(alt);
				img.setAttributeNode(title);

				td.appendChild(img);
			} else if(!rd->textData(song).isEmpty()) {
				td.appendChild(_report.createTextNode(rd->textData(song)));
			}

			tr.appendChild(td);
		}

		tbody.appendChild(tr);
	}

	parent.appendChild(tbody);
}
