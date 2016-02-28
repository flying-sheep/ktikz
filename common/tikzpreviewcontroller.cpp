/***************************************************************************
 *   Copyright (C) 2008, 2009, 2010 by Glad Deschrijver                    *
 *     <glad.deschrijver@gmail.com>                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#include "tikzpreviewcontroller.h"
#include <kactioncollection.h>
#include <QMimeDatabase>

#include <kjobwidgets.h>

#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QMimeType>
#include <KMessageBox>
#include <QTemporaryDir>
#include <KIO/Job>
#include <KIO/JobUiDelegate>

#include <QMenu>
#include <QPointer>
#include <QSettings>

#include "templatewidget.h"
#include "tikzpreview.h"
#include "tikzpreviewgenerator.h"
#include "mainwidget.h"
#include <QIcon>
#include <KToggleAction>

TikzPreviewController::TikzPreviewController(MainWidget *mainWidget)
{
	m_mainWidget = mainWidget;
	m_parentWidget = m_mainWidget->widget();

	m_templateWidget = new TemplateWidget(m_parentWidget);

	m_tikzPreview = new TikzPreview(mainWidget, m_parentWidget);
	m_tikzPreviewGenerator = new TikzPreviewGenerator(this);

	createActions();

	connect(m_tikzPreviewGenerator, &TikzPreviewGenerator::pixmapUpdated,
		m_tikzPreview, &TikzPreview::pixmapUpdated);
	connect(m_tikzPreviewGenerator, &TikzPreviewGenerator::showErrorMessage,
		m_tikzPreview, &TikzPreview::showErrorMessage);
	connect(m_tikzPreviewGenerator, &TikzPreviewGenerator::setExportActionsEnabled,
		this, &TikzPreviewController::setExportActionsEnabled);
	connect(m_tikzPreviewGenerator, &TikzPreviewGenerator::shortLogUpdated,
		this, &TikzPreviewController::logUpdated);
	connect(m_templateWidget, &TemplateWidget::fileNameChanged,
		this, &TikzPreviewController::setTemplateFileAndRegenerate);
	//	connect(m_templateWidget, &TemplateWidget::replaceTextChanged,
	//		this, &TikzPreviewController::setReplaceTextAndRegenerate;

	createTempDir();

//	applySettings(); // must be done after creation of m_tempDir
}

TikzPreviewController::~TikzPreviewController()
{
	delete m_tikzPreviewGenerator;
	removeTempDir();
}

/***************************************************************************/

void TikzPreviewController::createTempDir()
{
	m_tempDir = new QTemporaryDir();
	m_tempDir->setAutoRemove(true);
	m_tempTikzFileBaseName = QDir(m_tempDir->path()).dirName() + "temptikzcode";
	m_tikzPreviewGenerator->setTikzFileBaseName(m_tempTikzFileBaseName);
}

void TikzPreviewController::removeTempDir()
{
	delete m_tempDir;
}

const QString TikzPreviewController::tempDir() const
{
	return QDir(m_tempDir->path()).dirName();
}

/***************************************************************************/

TemplateWidget *TikzPreviewController::templateWidget() const
{
	return m_templateWidget;
}

TikzPreview *TikzPreviewController::tikzPreview() const
{
	return m_tikzPreview;
}

/***************************************************************************/

void TikzPreviewController::createActions()
{
	KActionCollection *ac = m_mainWidget->actionCollection();
	// File
	m_exportAction = ac->addAction("file_export_as", new QAction(QIcon::fromTheme("document-export"), tr("E&xport"), m_parentWidget));
	m_exportAction->setStatusTip(tr("Export image to various formats"));
	m_exportAction->setWhatsThis(tr("<p>Export image to various formats.</p>"));
	QMenu *exportMenu = new QMenu(m_parentWidget);
	m_exportAction->setMenu(exportMenu);

	QAction *exportEpsAction = ac->addAction("file_export_eps", new QAction(QIcon::fromTheme("image-x-eps"), tr("&Encapsulated PostScript (EPS)"), m_parentWidget));
	exportEpsAction->setData("image/x-eps");
	exportEpsAction->setStatusTip(tr("Export to EPS"));
	exportEpsAction->setWhatsThis(tr("<p>Export to EPS.</p>"));
	connect(exportEpsAction, &QAction::triggered, this, &TikzPreviewController::exportImage);
	exportMenu->addAction(exportEpsAction);

	QAction *exportPdfAction = ac->addAction("file_export_pdf", new QAction(QIcon::fromTheme("application-pdf"), tr("&Portable Document Format (PDF)"), m_parentWidget));
	exportPdfAction->setData("application/pdf");
	exportPdfAction->setStatusTip(tr("Export to PDF"));
	exportPdfAction->setWhatsThis(tr("<p>Export to PDF.</p>"));
	connect(exportPdfAction, &QAction::triggered, this, &TikzPreviewController::exportImage);
	exportMenu->addAction(exportPdfAction);

	QAction *exportPngAction = ac->addAction("file_export_png", new QAction(QIcon::fromTheme("image-png"), tr("Portable Network &Graphics (PNG)"), m_parentWidget));
	exportPngAction->setData("image/png");
	exportPngAction->setStatusTip(tr("Export to PNG"));
	exportPngAction->setWhatsThis(tr("<p>Export to PNG.</p>"));
	connect(exportPngAction, &QAction::triggered, this, &TikzPreviewController::exportImage);
	exportMenu->addAction(exportPngAction);

	setExportActionsEnabled(false);

	// View
	m_procStopAction = ac->addAction("stop_process", new QAction(QIcon::fromTheme("process-stop"), tr("&Stop Process"), m_parentWidget));
	ac->setDefaultShortcut(m_procStopAction, tr("Escape", "View|Stop Process"));
	m_procStopAction->setStatusTip(tr("Abort current process"));
	m_procStopAction->setWhatsThis(tr("<p>Abort the execution of the currently running process.</p>"));
	m_procStopAction->setEnabled(false);
	connect(m_procStopAction, &QAction::triggered, m_tikzPreviewGenerator, &TikzPreviewGenerator::abortProcess);

	m_shellEscapeAction = new KToggleAction(QIcon::fromTheme("application-x-executable"), tr("S&hell Escape"), m_parentWidget);
	ac->addAction("shell_escape", m_shellEscapeAction);
	m_shellEscapeAction->setStatusTip(tr("Enable the \\write18{shell-command} feature"));
	m_shellEscapeAction->setWhatsThis(tr("<p>Enable LaTeX to run shell commands, this is needed when you want to plot functions using gnuplot within TikZ."
	    "</p><p><strong>Warning:</strong> Enabling this may cause malicious software to be run on your computer! Check the LaTeX code to see which commands are executed.</p>"));
	connect(m_shellEscapeAction, &KToggleAction::toggled, this, &TikzPreviewController::toggleShellEscaping);

	connect(m_tikzPreviewGenerator, &TikzPreviewGenerator::processRunning,
			this, &TikzPreviewController::setProcessRunning);
}


/***************************************************************************/

void TikzPreviewController::showJobError(KJob *job)
{
	if (job->error() != 0)
	{
		KJobUiDelegate *ui = static_cast<KIO::Job*>(job)->ui();
		if (!ui)
		{
			qCritical() << "Saving failed; job->ui() is null.";
			return;
		}
		KJobWidgets::setWindow(job, m_parentWidget);
		ui->showErrorMessage();
	}
}

QUrl TikzPreviewController::getExportUrl(const QUrl &url, const QString &mimeName) const
{
	QMimeDatabase db;
	QMimeType mimeType = db.mimeTypeForName(mimeName);
	const QString exportUrlExtension = db.suffixForFileName(url.path());

	const QUrl exportUrl = QUrl(url.url().left(url.url().length()
		- (exportUrlExtension.isEmpty() ? 0 : exportUrlExtension.length() + 1)) // the extension is empty when the text/x-pgf mimetype is not correctly installed or when the file does not have a correct extension
		+ mimeType.globPatterns().at(0).mid(1)); // first extension in the list of possible extensions (without *)

	return QFileDialog::getSaveFileUrl(
		m_parentWidget,
		tr("Export Image"),
		exportUrl,
		mimeType.globPatterns().join(" ") + '|'
		//	+ mimeType.comment() + "\n*|" + i18nc("@item:inlistbox filter", "All files"),
			+ mimeType.comment() + "\n*|" + tr("All files"));
}

void TikzPreviewController::exportImage()
{
	QAction *action = qobject_cast<QAction*>(sender());
	const QString mimeType = action->data().toString();

	const QPixmap tikzImage = m_tikzPreview->pixmap();
	if (tikzImage.isNull())
		return;

	const QUrl exportUrl = getExportUrl(m_mainWidget->url(), mimeType);
	if (!exportUrl.isValid())
		return;

	QString extension;
	if (mimeType == QLatin1String("application/pdf"))
	{
		extension = ".pdf";
	}
	else if (mimeType == QLatin1String("image/x-eps"))
	{
		if (!m_tikzPreviewGenerator->generateEpsFile())
			return;
		extension = ".eps";
	}
	else if (mimeType == QLatin1String("image/png"))
	{
		extension = ".png";
		tikzImage.save(m_tempTikzFileBaseName + extension);
	}
	KIO::Job *job = KIO::file_copy(QUrl::fromLocalFile(m_tempTikzFileBaseName + extension), exportUrl, -1, KIO::Overwrite | KIO::HideProgressInfo);
	connect(job, &KIO::Job::result, this, &TikzPreviewController::showJobError);
}

/***************************************************************************/

bool TikzPreviewController::setTemplateFile(const QString &path)
{
	const QUrl url = QUrl::fromUserInput(path);
	const QUrl localUrl = QUrl::fromLocalFile(QDir(m_tempDir->path()).dirName() + "tikztemplate.tex");

	auto statJob = KIO::stat(url, KIO::StatJob::SourceSide, 0);
	bool readable = statJob->exec() && ((S_IRUSR|S_IRGRP|S_IROTH) & statJob->statResult().numberValue(KIO::UDSEntry::UDS_ACCESS));
	if (url.isValid() && !url.isLocalFile() && readable)
	{
		auto copyJob = KIO::file_copy(url, localUrl, -1, KIO::Overwrite | KIO::HideProgressInfo);
		if (!copyJob->exec())
		{
		//	KMessageBox::information(m_parentWidget, i18nc("@info", "Template file could not be copied to a temporary file <filename>%1</filename>.", localUrl.prettyUrl()));
			KMessageBox::information(m_parentWidget, tr("Template file could not be copied to a temporary file \"%1\".").arg(localUrl.toDisplayString()));
			return false;
		}
		else
			m_tikzPreviewGenerator->setTemplateFile(localUrl.path());
	}
	else
		m_tikzPreviewGenerator->setTemplateFile(path);
	return true;
}

bool TikzPreviewController::setTemplateFileAndRegenerate(const QString &path) {
	bool r = setTemplateFile(path);
	generatePreview(true);
	return r;
}

void TikzPreviewController::setReplaceTextAndRegenerate(const QString &replace)
{
	m_tikzPreviewGenerator->setReplaceText(replace);
	generatePreview(true);
}

/***************************************************************************/

QString TikzPreviewController::tikzCode() const
{
	return m_mainWidget->tikzCode();
}

QString TikzPreviewController::getLogText()
{
	return m_tikzPreviewGenerator->getLogText();
}

void TikzPreviewController::generatePreview()
{
	QAction *action = qobject_cast<QAction*>(sender());
	bool templateChanged = (action == 0) ? true : false; // XXX dirty hack: the template hasn't changed when the Build button in the app has been pressed (if available), the other cases in which this function is called is when a file is opened, in which case everything should be cleaned up and regenerated
	generatePreview(templateChanged);
}

void TikzPreviewController::generatePreview(bool templateChanged)
{
	if (templateChanged) // old aux files may contain commands available in the old template, but not anymore in the new template
		cleanUp();
	// TODO: m_tikzPreviewGenerator->addToTexinputs(QFileInfo(m_mainWidget->url().path()).absolutePath());
//	m_tikzPreviewGenerator->setTikzFilePath(m_mainWidget->url().path()); // the directory in which the pgf file is located is added to TEXINPUTS before running latex
	m_tikzPreviewGenerator->generatePreview(templateChanged);
}

void TikzPreviewController::regeneratePreview()
{
//	m_tikzPreviewGenerator->setTikzFilePath(m_mainWidget->url().path()); // the directory in which the pgf file is located is added to TEXINPUTS before running latex
	m_tikzPreviewGenerator->regeneratePreview();
}

void TikzPreviewController::emptyPreview()
{
	setExportActionsEnabled(false);
	m_tikzPreviewGenerator->abortProcess(); // abort still running processes
	m_tikzPreview->emptyPreview();
}

/***************************************************************************/

void TikzPreviewController::applySettings()
{
	QSettings settings(ORGNAME, APPNAME);
	m_tikzPreviewGenerator->setLatexCommand(settings.value("LatexCommand", "pdflatex").toString());
	m_tikzPreviewGenerator->setPdftopsCommand(settings.value("PdftopsCommand", "pdftops").toString());
	const bool useShellEscaping = settings.value("UseShellEscaping", false).toBool();

	disconnect(m_shellEscapeAction, &KToggleAction::toggled, this, &TikzPreviewController::toggleShellEscaping);
	m_shellEscapeAction->setChecked(useShellEscaping);
	m_tikzPreviewGenerator->setShellEscaping(useShellEscaping);
	connect(m_shellEscapeAction, &KToggleAction::toggled, this, &TikzPreviewController::toggleShellEscaping);

	setTemplateFile(settings.value("TemplateFile").toString());
	const QString replaceText = settings.value("TemplateReplaceText", "<>").toString();
	m_tikzPreviewGenerator->setReplaceText(replaceText);
	m_templateWidget->setReplaceText(replaceText);
		m_templateWidget->setEditor(settings.value("TemplateEditor", KTIKZ_TEMPLATE_EDITOR_DEFAULT).toString());
}

void TikzPreviewController::setExportActionsEnabled(bool enabled)
{
	m_exportAction->setEnabled(enabled);
}

void TikzPreviewController::setProcessRunning(bool isRunning)
{
	m_procStopAction->setEnabled(isRunning);
	if (isRunning)
		QApplication::setOverrideCursor(Qt::BusyCursor);
	else
		QApplication::restoreOverrideCursor();
	m_tikzPreview->setProcessRunning(isRunning);
}

void TikzPreviewController::toggleShellEscaping(bool useShellEscaping)
{

	QSettings settings(ORGNAME, APPNAME);
	settings.setValue("UseShellEscaping", useShellEscaping);

	m_tikzPreviewGenerator->setShellEscaping(useShellEscaping);
	generatePreview(false);
}

/***************************************************************************/

bool TikzPreviewController::cleanUp()
{
	bool success = true;

	const QFileInfo tempTikzFileInfo(m_tempTikzFileBaseName + ".tex");
	QDir tempTikzDir(tempTikzFileInfo.absolutePath());
	QStringList filters;
	filters << tempTikzFileInfo.completeBaseName() + ".*";

	foreach (const QString &fileName, tempTikzDir.entryList(filters))
		success = success && tempTikzDir.remove(fileName);
	return success;
}
