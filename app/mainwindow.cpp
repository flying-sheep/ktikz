/***************************************************************************
 *   Copyright (C) 2007 by Florian Hackenberger                            *
 *     <florian@hackenberger.at>                                           *
 *   Copyright (C) 2007, 2008, 2009, 2010 by Glad Deschrijver              *
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

#include <QDebug>
#include "mainwindow.h"

#include <QAction>
#include <KActionCollection>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>
#include <QMenuBar>
#include <KMessageBox>
#include <QSaveFile>
#include <KStandardAction>
#include <QStatusBar>
#include <KXMLGUIFactory>
#include <KIO/Job>

#include <QCloseEvent>
#include <QCompleter>
#include <QDesktopServices>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QSettings>
#include <QStringListModel>
#include <QTextStream>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWhatsThis>
#include <QIcon>
#include <QFileDialog>
#include <KToggleAction>

#include "configdialog.h"
#include "ktikzapplication.h"
#include "loghighlighter.h"
#include "logtextedit.h"
#include "tikzcommandinserter.h"
#include "tikzdocumentationcontroller.h"
#include "tikzeditorhighlighter.h"
#include "tikzeditorview.h"
#include "../common/templatewidget.h"
#include "../common/tikzpreviewcontroller.h"
#include "../common/tikzpreview.h"

#include <poppler-qt5.h>

QList<MainWindow*> MainWindow::s_mainWindowList;
static const QString s_tempFileName = "tikzcode.pgf";

MainWindow::MainWindow()
{
	m_configDialog = 0;
	m_completer = 0;

	s_mainWindowList.append(this);


	setAttribute(Qt::WA_DeleteOnClose);
	setObjectName("ktikz#");
	setWindowIcon(QIcon::fromTheme("ktikz"));

	setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

	m_tikzPreviewController = new TikzPreviewController(this);

	m_tikzEditorView = new TikzEditorView(this);
	m_commandInserter = new TikzCommandInserter(this);
	m_commandInserter->setEditor(m_tikzEditorView->editor());
	m_tikzHighlighter = new TikzHighlighter(m_commandInserter, m_tikzEditorView->editor()->document());
	m_tikzHighlighter->rehighlight(); // avoid that textEdit emits the signal contentsChanged() when it is still empty

	QWidget *mainWidget = new QWidget(this);
	QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	mainLayout->addWidget(m_tikzPreviewController->templateWidget());
	mainLayout->addWidget(m_tikzEditorView);

	m_logDock = new QDockWidget(this);
	m_logDock->setObjectName("LogDock");
	m_logDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_logDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	m_logDock->setWindowTitle(tr("Messages"));
	addDockWidget(Qt::BottomDockWidgetArea, m_logDock);
	m_logTextEdit = new LogTextEdit;
	m_logTextEdit->setWhatsThis(tr("<p>The messages produced by "
	    "LaTeX are shown here.  If your TikZ code contains errors, "
	    "then a red border will appear and the errors will be "
	    "highlighted.</p>"));
	m_logTextEdit->setReadOnly(true);
	m_logHighlighter = new LogHighlighter(m_logTextEdit->document());
	m_logDock->setWidget(m_logTextEdit);

	m_previewDock = new QDockWidget(this);
	m_previewDock->setObjectName("PreviewDock");
	m_previewDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_previewDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	m_previewDock->setWindowTitle(tr("Preview"));
	m_previewDock->setWidget(m_tikzPreviewController->tikzPreview());
	addDockWidget(Qt::RightDockWidgetArea, m_previewDock);

	setCentralWidget(mainWidget);

	createActions();
	createCommandInsertWidget(); // must happen after createMenus and before readSettings
	createStatusBar();

	setupGUI(ToolBar | Keys | StatusBar | Save);
	setXMLFile("ktikzui.rc");
	createGUI();
	guiFactory()->addClient(this);

	connect(m_commandInserter, &TikzCommandInserter::showStatusMessage, statusBar(), &QStatusBar::showMessage);

	connect(m_tikzEditorView, &TikzEditorView::modificationChanged, this, &MainWindow::setDocumentModified);
	connect(m_tikzEditorView, &TikzEditorView::cursorPositionChanged, this, &MainWindow::showCursorPosition);
	connect(m_tikzEditorView, &TikzEditorView::showStatusMessage, statusBar(), &QStatusBar::showMessage);

	connect(m_tikzEditorView, &TikzEditorView::focusIn, this, &MainWindow::checkForFileChanges);
	connect(m_tikzEditorView, &TikzEditorView::focusOut, this, &MainWindow::saveLastInternalModifiedDateTime);

	connect(m_tikzPreviewController, &TikzPreviewController::logUpdated, m_logTextEdit, static_cast<void (LogTextEdit::*)(const QString&, bool)>(&LogTextEdit::logUpdated));

	readSettings(); // must be run after defining tikzController and tikzHighlighter, and after creating the toolbars, and after the connects

	// the following connects must happen after readSettings() because otherwise in that function the following signals would be unnecessarily triggered
	if (m_buildAutomatically)
		connect(m_tikzEditorView, &TikzEditorView::contentsChanged, m_tikzPreviewController, &TikzPreviewController::regeneratePreview);

	setCurrentUrl(QUrl());
	setDocumentModified(false);
	saveLastInternalModifiedDateTime();
	m_tikzEditorView->editor()->setFocus();

/*
	QDir dir(QDir::tempPath() + "/ktikz");
	if (!dir.exists())
		QDir::temp().mkdir("ktikz");
*/
}

MainWindow::~MainWindow()
{
	s_mainWindowList.removeAll(this);

	writeSettings();


	delete m_tikzPreviewController;
	m_logHighlighter->deleteLater();
	m_tikzHighlighter->deleteLater();

/*
	QDir dir(QDir::tempPath() + "/ktikz");
	if (dir.exists())
		QDir::temp().rmdir("ktikz");
*/
}

KActionCollection *MainWindow::actionCollection() const
{
	return KXmlGuiWindow::actionCollection();
}

QWidget *MainWindow::widget()
{
	return this;
}

bool MainWindow::queryClose()
{
	return maybeSave();
}

void MainWindow::readProperties(const KConfigGroup &group)
{
	const QUrl url = QUrl(group.readPathEntry("CurrentUrl", QString()));
	if (url.isValid() && !url.isEmpty())
		loadUrl(url);
}

void MainWindow::saveProperties(KConfigGroup &group)
{
	group.writePathEntry("CurrentUrl", m_currentUrl.url());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (maybeSave())
	{
		QApplication::restoreOverrideCursor();
		event->accept();
	}
	else
		event->ignore();
}

void MainWindow::newFile()
{
	MainWindow *newMainWindow = new MainWindow;
	newMainWindow->show();
}

bool MainWindow::closeFile()
{
	if (maybeSave())
	{
		disconnect(m_tikzEditorView, &TikzEditorView::contentsChanged, m_tikzPreviewController, &TikzPreviewController::regeneratePreview);
		m_tikzEditorView->editor()->clear();
		connect(m_tikzEditorView, &TikzEditorView::contentsChanged, m_tikzPreviewController, &TikzPreviewController::regeneratePreview);
		setCurrentUrl(QUrl());
		m_tikzPreviewController->emptyPreview(); // abort still running processes
		m_logTextEdit->logUpdated("", false); // clear log window
		return true;
	}
	return false;
}

void MainWindow::open()
{
	const QUrl openUrl = QFileDialog::getOpenFileUrl(this, tr("Open PGF source file"), m_lastUrl, QString("*.pgf *.tikz *.tex|%1\n*|%2").arg(tr("PGF files", "filter")).arg(tr("All files", "filter")));
	if (openUrl.isValid() && !openUrl.isEmpty())
		loadUrl(openUrl);
}

bool MainWindow::save()
{
	if (!m_currentUrl.isValid() || m_currentUrl.isEmpty())
		return saveAs();
	else
		return saveUrl(m_currentUrl);
}

bool MainWindow::saveAs()
{
	const QUrl saveAsUrl = QFileDialog::getSaveFileUrl(this, tr("Save PGF source file"), m_currentUrl, QString("*.pgf *.tikz *.tex|%1\n*|%2").arg(tr("PGF files", "filter")).arg(tr("All files", "filter")));
	if (!saveAsUrl.isValid() || saveAsUrl.isEmpty())
		return false;
	return saveUrl(saveAsUrl);
}

void MainWindow::reload()
{
	const QUrl currentUrl = m_currentUrl;
	if (closeFile())
	{
		saveLastInternalModifiedDateTime();
		loadUrl(currentUrl);
	}
}

void MainWindow::checkForFileChanges()
{
	QDateTime lastExternalModifiedDateTime(QFileInfo(m_currentUrl.path()).lastModified());
	if (lastExternalModifiedDateTime <= m_lastInternalModifiedDateTime)
		return;

	QPointer<QMessageBox> fileChangedWarningMessageBox = new QMessageBox(this);
	fileChangedWarningMessageBox->setText(tr("The document was modified by another program.\nWhat do you want to do?"));
	fileChangedWarningMessageBox->setWindowTitle(KtikzApplication::applicationName());
	fileChangedWarningMessageBox->setIcon(QMessageBox::Warning);
	QAbstractButton *overwriteButton = fileChangedWarningMessageBox->addButton(tr("&Overwrite"), QMessageBox::AcceptRole);
	QAbstractButton *reloadButton = fileChangedWarningMessageBox->addButton(tr("&Reload file"), QMessageBox::AcceptRole);
	fileChangedWarningMessageBox->addButton(QMessageBox::Cancel);
	fileChangedWarningMessageBox->exec();
	if (fileChangedWarningMessageBox->clickedButton() == overwriteButton)
		saveUrl(m_currentUrl);
	else if (fileChangedWarningMessageBox->clickedButton() == reloadButton)
		reload();
	delete fileChangedWarningMessageBox;
}

void MainWindow::saveLastInternalModifiedDateTime()
{
	m_lastInternalModifiedDateTime = QDateTime::currentDateTime();
}

/***************************************************************************/

void MainWindow::showTikzDocumentation()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	const QString tikzDocFile = TikzDocumentationController::tikzDocumentationPath();
	const bool tikzDocFileExists = QFileInfo(tikzDocFile).exists(); // true if tikzDocFile is local and exists
	const QUrl tikzDocUrl = tikzDocFileExists ? QUrl("file:///" + tikzDocFile) : QUrl(tikzDocFile);

	if (!QDesktopServices::openUrl(tikzDocUrl))
	{
		if (!tikzDocFileExists)
			QMessageBox::warning(this, KtikzApplication::applicationName(),
			    tr("Cannot find TikZ documentation.\n"
			    "Go to Settings -> Configure %1 and change in the \"General\" tab "
			    "the path to the TikZ documentation.")
			    .arg(KtikzApplication::applicationName()));
		else // if tikzDocFile is local and exists then failure to open it is caused by not finding the correct application
			QMessageBox::warning(this, KtikzApplication::applicationName(),
			    tr("Cannot open TikZ documentation.\n"
			    "No application is found which can open the file \"%1\".")
			    .arg(tikzDocFile));
	}

	QApplication::restoreOverrideCursor();
}


/***************************************************************************/

bool MainWindow::isDocumentModified() const
{
	return isWindowModified();
}

void MainWindow::setDocumentModified(bool isModified)
{
	setWindowModified(isModified);
	m_saveAction->setEnabled(isModified);
	m_saveAsAction->setEnabled(m_currentUrl.isValid() && !m_currentUrl.isEmpty());
}

void MainWindow::logUpdated()
{
	m_logTextEdit->logUpdated(m_tikzPreviewController->getLogText());
//	m_logTextEdit->logUpdated(m_tikzController->getLogText(), m_tikzController->hasRunFailed());
}

/***************************************************************************/

void MainWindow::toggleWhatsThisMode()
{
	if (QWhatsThis::inWhatsThisMode())
		QWhatsThis::leaveWhatsThisMode();
	else
		QWhatsThis::enterWhatsThisMode();
}

void MainWindow::createActions()
{
	/* Open */
	m_newAction = KStandardAction::openNew(this, SLOT(newFile()), actionCollection());
	m_openAction = KStandardAction::open(this, SLOT(open()), actionCollection());
	m_openRecentAction = KStandardAction::openRecent(this, SLOT(loadUrl(QUrl)), actionCollection());
	m_saveAction = KStandardAction::save(this, SLOT(save()), actionCollection());
	m_saveAsAction = KStandardAction::saveAs(this, SLOT(saveAs()), actionCollection());
	m_reloadAction = actionCollection()->addAction("file_reload", new QAction(QIcon::fromTheme("view-refresh"), tr("Reloa&d"), this));
	m_reloadAction->setShortcut(QKeySequence::Refresh);
	m_reloadAction->setStatusTip(tr("Reload the current document"));
	m_reloadAction->setWhatsThis(tr("<p>Reload the current document from disk.</p>"));
	connect(m_reloadAction, &QAction::triggered, this, &MainWindow::reload);
	m_closeAction = KStandardAction::close(this, SLOT(closeFile()), actionCollection());
	m_exitAction = KStandardAction::quit(this, SLOT(close()), actionCollection());

	m_newAction->setStatusTip(tr("Create a new document"));
	m_openAction->setStatusTip(tr("Open an existing file"));
	m_openRecentAction->setStatusTip(tr("Open a recently opened file"));
	m_saveAction->setStatusTip(tr("Save the current document to disk"));
	m_saveAsAction->setStatusTip(tr("Save the document under a new name"));
	m_closeAction->setStatusTip(tr("Close the current document"));
	m_exitAction->setStatusTip(tr("Exit the application"));

	m_newAction->setWhatsThis(tr("<p>Create a new document.</p>"));
	m_openAction->setWhatsThis(tr("<p>Open an existing file.</p>"));
	m_openRecentAction->setWhatsThis(tr("<p>Open a recently opened file.</p>"));
	m_saveAction->setWhatsThis(tr("<p>Save the current document to disk.</p>"));
	m_saveAsAction->setWhatsThis(tr("<p>Save the document under a new name.</p>"));
	m_closeAction->setWhatsThis(tr("<p>Close the current document.</p>"));
	m_exitAction->setWhatsThis(tr("<p>Exit the application.</p>"));

	/* View */
	m_buildAction = actionCollection()->addAction("build", new QAction(QIcon::fromTheme("run-build"), tr("&Build"), this));
	m_buildAction->setShortcut(tr("Ctrl+B", "View|Build"));
	m_buildAction->setStatusTip(tr("Build preview"));
	m_buildAction->setWhatsThis(tr("<p>Generate preview by building the current TikZ code in the editor.</p>"));
	connect(m_buildAction, &QAction::triggered, m_tikzPreviewController, static_cast<void (TikzPreviewController::*)()>(&TikzPreviewController::generatePreview));

	m_viewLogAction = actionCollection()->addAction("view_log", new QAction(QIcon::fromTheme("run-build-file"), tr("View &Log"), this));
	m_viewLogAction->setStatusTip(tr("View log messages produced by the last executed process"));
	m_viewLogAction->setWhatsThis(tr("<p>Show the log messages produced by the last executed process in the Messages box.</p>"));
	connect(m_viewLogAction, &QAction::triggered, this, &MainWindow::logUpdated);

	/* Configure */
	m_configureAction = KStandardAction::preferences(this, SLOT(configure()), actionCollection());
	m_configureAction->setText(tr("&Configure %1...").arg(KtikzApplication::applicationName()));
	m_configureAction->setStatusTip(tr("Configure the settings of this application"));
	m_configureAction->setWhatsThis(tr("<p>Configure the settings of this application.</p>"));
	
	actionCollection()->addAction("toggle_preview", m_previewDock->toggleViewAction());
	actionCollection()->addAction("toggle_log", m_logDock->toggleViewAction());

	/* Help */
	m_showTikzDocAction = actionCollection()->addAction("show_tikz_doc", new QAction(QIcon::fromTheme("help-contents"), tr("TikZ &Manual"), this));
	m_showTikzDocAction->setStatusTip(tr("Show the manual of TikZ and PGF"));
	m_showTikzDocAction->setWhatsThis(tr("<p>Show the manual of TikZ and PGF.</p>"));
	connect(m_showTikzDocAction, &QAction::triggered, this, &MainWindow::showTikzDocumentation);
	
	m_whatsThisAction = KStandardAction::whatsThis(this, SLOT(toggleWhatsThisMode()), actionCollection());
}


void MainWindow::createCommandInsertWidget()
{
	QSettings settings(ORGNAME, APPNAME);
	bool commandsInDock = settings.value("CommandsInDock", false).toBool();

	if (commandsInDock)
	{
		m_commandsDock = m_commandInserter->getDockWidget(this);
		addDockWidget(Qt::LeftDockWidgetArea, m_commandsDock);

		actionCollection()->addAction("toggle_commands_list", m_commandsDock->toggleViewAction());
		connect(m_commandsDock->toggleViewAction(), &QAction::toggled, this, &MainWindow::setDockWidgetStatusTip);
	}
	else
	{
		QAction *insertAction = new QAction(tr("&Insert"), this);
		insertAction->setMenu(m_commandInserter->getMenu());
	}
}

void MainWindow::createStatusBar()
{
	QStatusBar *status = statusBar();

	m_positionLabel = new QLabel(status);
	m_positionLabel->setMinimumWidth(150);
	status->addPermanentWidget(m_positionLabel, 0);

	status->showMessage(tr("Ready"));
}

void MainWindow::setDockWidgetStatusTip(bool enabled)
{
	QAction *action = qobject_cast<QAction*>(sender());
	QString dockName = action->text();
	dockName.remove('&');
	if (enabled)
		action->setStatusTip(tr("Hide sidebar \"%1\"").arg(dockName));
	else
		action->setStatusTip(tr("Show sidebar \"%1\"").arg(dockName));
}

void MainWindow::setToolBarStatusTip(bool enabled)
{
	QAction *action = qobject_cast<QAction*>(sender());
	if (enabled)
		action->setStatusTip(tr("Hide toolbar \"%1\"").arg(action->text()));
	else
		action->setStatusTip(tr("Show toolbar \"%1\"").arg(action->text()));
}

/***************************************************************************/

void MainWindow::configure()
{
	if (!m_configDialog)
	{
		m_configDialog = new ConfigDialog(this);
		m_configDialog->setTranslatedHighlightTypeNames(m_tikzHighlighter->getTranslatedHighlightTypeNames());
		m_configDialog->setHighlightTypeNames(m_tikzHighlighter->getHighlightTypeNames());
		m_configDialog->setDefaultHighlightFormats(m_tikzHighlighter->getDefaultHighlightFormats());
		connect(m_configDialog, &ConfigDialog::settingsChanged, this, &MainWindow::applySettings);
	}
	disconnect(m_tikzEditorView, &TikzEditorView::contentsChanged, m_tikzPreviewController, &TikzPreviewController::regeneratePreview);
	m_configDialog->readSettings();
	m_configDialog->exec();
	if (m_buildAutomatically)
		connect(m_tikzEditorView, &TikzEditorView::contentsChanged, m_tikzPreviewController, &TikzPreviewController::regeneratePreview);
}

void MainWindow::applySettings()
{
	QSettings settings(ORGNAME, APPNAME);

	m_tikzEditorView->applySettings();
	m_tikzPreviewController->applySettings();

	m_buildAutomatically = settings.value("BuildAutomatically", true).toBool();
	m_buildAction->setVisible(!m_buildAutomatically);

	settings.beginGroup("Editor");
	m_useCompletion = settings.value("UseCompletion", true).toBool();
	if (m_useCompletion)
	{
		if (!m_completer)
			m_completer = new QCompleter(this);
		updateCompleter();
		m_tikzEditorView->setCompleter(m_completer);
	}
	else if (m_completer)
	{
		m_tikzEditorView->setCompleter(0); // do this before deleting m_completer because a signal is disconnected from m_completer
		delete m_completer;
		m_completer = 0;
	}
	settings.endGroup();

	settings.beginGroup("Highlighting");
	bool customHighlighting = settings.value("Customize", true).toBool();
	QMap<QString, QTextCharFormat> formatList = m_tikzHighlighter->getDefaultHighlightFormats();
	if (customHighlighting)
	{
		int numOfRules = settings.value("Number", 0).toInt();
		for (int i = 0; i < numOfRules; ++i)
		{
			QString name = settings.value("Item" + QString::number(i) + "/Name").toString();
			QString colorName = settings.value("Item" + QString::number(i) + "/Color").toString();
			QString fontName = settings.value("Item" + QString::number(i) + "/Font").toString();
			QFont font;
			font.fromString(fontName);
			QTextCharFormat format;
			format.setForeground(QBrush(QColor(colorName)));
			format.setFont(font);
			formatList[name] = format;
		}
	}
	settings.endGroup();

	m_tikzHighlighter->setTextCharFormats(formatList);
	m_tikzHighlighter->rehighlight();
}

void MainWindow::readSettings()
{
	m_openRecentAction->loadEntries(KSharedConfig::openConfig()->group("Recent Files"));

	QSettings settings(ORGNAME, APPNAME);
	settings.beginGroup("MainWindow");
//	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
//	move(pos);
	QSize size = settings.value("size", QSize(800, 600)).toSize();
	resize(size);
	restoreState(settings.value("MainWindowState").toByteArray());
	settings.endGroup();

	applySettings();
}

void MainWindow::writeSettings()
{
	m_openRecentAction->saveEntries(KSharedConfig::openConfig()->group("Recent Files"));

	QSettings settings(ORGNAME, APPNAME);

//	settings.setValue("TemplateFile", m_tikzEditorView->templateFile());

	settings.beginGroup("MainWindow");
//	settings.setValue("pos", pos());
	settings.setValue("size", size());
	settings.setValue("MainWindowState", QMainWindow::saveState());
/*
	int toolBarStyleNumber = 0;
	switch (m_fileToolBar->toolButtonStyle())
	{
		case Qt::ToolButtonIconOnly: toolBarStyleNumber = 0; break;
		case Qt::ToolButtonTextOnly: toolBarStyleNumber = 1; break;
		case Qt::ToolButtonTextBesideIcon: toolBarStyleNumber = 2; break;
		case Qt::ToolButtonTextUnderIcon: toolBarStyleNumber = 3; break;
	}
	settings.setValue("ToolBarStyle", toolBarStyleNumber);
*/
	settings.endGroup();
}

/***************************************************************************/

bool MainWindow::maybeSave()
{
	if (m_tikzEditorView->editor()->document()->isModified())
	{
		const int ret = QMessageBox::warning(this, KtikzApplication::applicationName(),
		    tr("The document has been modified.\n"
		       "Do you want to save your changes?"),
		    QMessageBox::Save | QMessageBox::Default,
		    QMessageBox::Discard,
		    QMessageBox::Cancel | QMessageBox::Escape);
		if (ret == QMessageBox::Save)
			return save();
		else if (ret == QMessageBox::Cancel)
			return false;
	}
	return true;
}

void MainWindow::loadUrl(const QUrl &url)
{
	// check whether the file can be opened
	if (!url.isValid() || url.isEmpty())
		return;

	const QString localFileName = url.isLocalFile() ? url.path() : m_tikzPreviewController->tempDir() + s_tempFileName;

	auto statJob = KIO::stat(url, KIO::StatJob::SourceSide, 0);
	bool readable = statJob->exec() && ((S_IRUSR|S_IRGRP|S_IROTH) & statJob->statResult().numberValue(KIO::UDSEntry::UDS_ACCESS));
	if (!url.isLocalFile() && readable)
	{
		auto copyJob = KIO::file_copy(url, QUrl::fromLocalFile(localFileName), -1, KIO::Overwrite | KIO::HideProgressInfo);
		if (!copyJob->exec())
		{
		//	KMessageBox::information(this, i18nc("@info", "Could not copy <filename>%1</filename> to temporary file <filename>%2</filename>.", url.prettyUrl(), localFileName));
			KMessageBox::information(this, tr("Could not copy \"%1\" to temporary file \"%2\".").arg(url.toDisplayString()).arg(localFileName));
			return;
		}
	}
	
	QFile localFile(localFileName);
	if (!localFile.open(QFile::ReadOnly | QFile::Text))
	{
	//	KMessageBox::error(this, i18nc("@info", "Cannot read file <filename>%1</filename>:<nl/><message>%2</message>", localFileName, localFile.errorString()), i18nc("@title:window", "File Read Error"));
		KMessageBox::error(this, tr("Cannot read file \"%1\":\n%2").arg(localFileName).arg(localFile.errorString()), tr("File Read Error", "@title:window"));
		m_openRecentAction->removeUrl(url);
		return;
	}
	
	// only open a new window (if necessary) if the file can actually be opened
	if (!m_tikzEditorView->editor()->document()->isEmpty())
	{
		MainWindow *newMainWindow = new MainWindow;
		newMainWindow->loadUrl(url);
		newMainWindow->show();
		return;
	}
	
	// load the file and generate preview
	disconnect(m_tikzEditorView, &TikzEditorView::contentsChanged, m_tikzPreviewController, &TikzPreviewController::regeneratePreview);
	QTextStream in(&localFile);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	m_tikzEditorView->editor()->setPlainText(in.readAll());
	QApplication::restoreOverrideCursor();
	m_tikzPreviewController->generatePreview();
	if (m_buildAutomatically)
		connect(m_tikzEditorView, &TikzEditorView::contentsChanged, m_tikzPreviewController, &TikzPreviewController::regeneratePreview);
		
	m_lastUrl = url;
	setCurrentUrl(url);
	m_openRecentAction->addUrl(url);
	statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveUrl(const QUrl &url)
{
	if (!url.isValid() || url.isEmpty())
		return false;

	const QString localFileName = url.isLocalFile() ? url.path() : m_tikzPreviewController->tempDir() + s_tempFileName;

	QSaveFile localFile(localFileName);
	if (!localFile.open(QIODevice::WriteOnly))
	{
//		KMessageBox::error(this, i18nc("@info", "Cannot write file <filename>%1</filename>:<nl/><message>%2</message>", localFileName, localFile.errorString()), i18nc("@title:window", "File Save Error"));
		KMessageBox::error(this, tr("Cannot write file \"%1\":\n%2").arg(localFileName).arg(localFile.errorString()), tr("File Save Error", "@title:window"));
		return false;
	}

	QTextStream out(&localFile);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	out << m_tikzEditorView->editor()->toPlainText().toUtf8();
	out.flush();
	QApplication::restoreOverrideCursor();

	if (!localFile.commit())
	{
//		KMessageBox::error(this, i18nc("@info", "Cannot write file <filename>%1</filename>:<nl/><message>%2</message>", localFileName, localFile.errorString()), i18nc("@title:window", "File Save Error"));
		KMessageBox::error(this, tr("Cannot write file \"%1\":\n%2").arg(localFileName).arg(localFile.errorString()), tr("File Save Error", "@title:window"));
		return false;
	}

	if (!url.isLocalFile())
	{
		KMessageBox::information(this, tr("Remote URLs unimplemented")); //TODO
		return false;
// 		KIO::Job *job = KIO::file_copy(QUrl::fromPath(localFileName), url, -1, KIO::Overwrite | KIO::HideProgressInfo);
// 		if (!KIO::NetAccess::synchronousRun(job, this))
// 		{
// //			KMessageBox::information(this, i18nc("@info", "Could not copy temporary file <filename>%1</filename> to <filename>%2</filename>.", localFileName, url.prettyUrl()));
// 			KMessageBox::information(this, tr("Could not copy temporary file \"%1\" to \"%2\".").arg(localFileName).arg(url.toDisplayString()));
// 			return false;
// 		}
	}

	m_lastUrl = url;
	setCurrentUrl(url);
	m_openRecentAction->addUrl(url);
	saveLastInternalModifiedDateTime(); // make sure that the save in this function is not seen as an "external" save
	statusBar()->showMessage(tr("File saved", "@info:status"), 2000);
	return true;
}

QUrl MainWindow::url() const
{
	return m_currentUrl;
}

void MainWindow::setCurrentUrl(const QUrl &url)
{
	m_currentUrl = url;
	m_tikzEditorView->editor()->document()->setModified(false);
	setDocumentModified(false);
	setWindowTitle(tr("%1[*] - %2").arg(strippedName(m_currentUrl)).arg(KtikzApplication::applicationName()));
}

QString MainWindow::strippedName(const QUrl &url) const
{
	if (!url.isValid() || url.isEmpty())
		return "untitled.txt";
	const QString fileName = url.fileName();
	return (fileName.isEmpty()) ? "untitled.txt" : fileName;
}

/***************************************************************************/

void MainWindow::showCursorPosition(int row, int col)
{
	m_positionLabel->setText(tr("Line: %1\tCol: %2", "@info:status").arg(QString::number(row)).arg(QString::number(col)));
}

/***************************************************************************/

QString MainWindow::tikzCode() const
{
	return m_tikzEditorView->editor()->toPlainText();
}

/***************************************************************************/

void MainWindow::updateCompleter()
{
	QStringList words = m_commandInserter->getCommandWords();
	QStringListModel *model = new QStringListModel(words, m_completer);
	m_completer->setModel(model);
	m_completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
	m_completer->setCaseSensitivity(Qt::CaseSensitive);
	m_completer->setWrapAround(false);
}
