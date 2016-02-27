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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <KXmlGuiWindow>
class KUrl;
#include <QDateTime>
#include <QPointer>
#include "../common/mainwidget.h"

class QAction;
class QCloseEvent;
class QCompleter;
class QDockWidget;
class QLabel;
class QMenu;
class QSyntaxHighlighter;
class QToolButton;

class Action;
class ConfigDialog;
class LogTextEdit;
class RecentFilesAction;
class TikzCommandInserter;
class TikzEditorView;
class TikzHighlighter;
class TikzPreviewController;
class Url;

/** Provides a tiny application for simple editing of TikZ graphics
 * @author Florian Hackenberger
 */
class MainWindow : public KXmlGuiWindow, public MainWidget
{
	Q_OBJECT

public:
	MainWindow();
	virtual ~MainWindow();

	virtual QWidget *widget();
	bool isDocumentModified() const;
	QString tikzCode() const;
	Url url() const;

	static QList<MainWindow*> mainWindowList() { return s_mainWindowList; }

public slots:
	void loadUrl(const Url &url);
	bool save();

signals:
	void setSearchFromBegin(bool searchFromBegin);

protected:
	bool queryClose();
	void readProperties(const KConfigGroup &group);
	void saveProperties(KConfigGroup &group);
	void closeEvent(QCloseEvent *event);

private slots:
	void checkForFileChanges();
	void saveLastInternalModifiedDateTime();
	void setDockWidgetStatusTip(bool enabled);
	void setToolBarStatusTip(bool enabled);
	void newFile();
	bool closeFile();
	void open();
	bool saveAs();
	void reload();
	void showTikzDocumentation();
	void configure();
	void applySettings();
	void setDocumentModified(bool isModified);
	void logUpdated();
	void toggleWhatsThisMode();
	void showCursorPosition(int row, int col);

private:
	void createActions();
	void createCommandInsertWidget();
	void createStatusBar();
	void readSettings();
	void writeSettings();
	bool maybeSave();
	bool saveUrl(const Url &url);
	void setCurrentUrl(const Url &url);
	QString strippedName(const Url &url) const;
	void showPdfPage();
	void updateCompleter();

	static QList<MainWindow*> s_mainWindowList;

	TikzEditorView *m_tikzEditorView;
	TikzHighlighter *m_tikzHighlighter;
	QCompleter *m_completer;
	bool m_useCompletion;

	TikzPreviewController *m_tikzPreviewController;
	bool m_buildAutomatically;

	QDockWidget *m_previewDock;

	QDockWidget *m_logDock;
	LogTextEdit *m_logTextEdit;
	QSyntaxHighlighter *m_logHighlighter;

	QDockWidget *m_commandsDock;
	TikzCommandInserter *m_commandInserter;

	QLabel *m_positionLabel;

	QMenu *m_settingsMenu;
	QMenu *m_sideBarMenu;
	QToolBar *m_fileToolBar;
	QToolBar *m_editToolBar;
	QToolBar *m_viewToolBar;
	QToolBar *m_runToolBar;
	QAction *m_newAction;
	QAction *m_openAction;
	RecentFilesAction *m_openRecentAction;
	QAction *m_saveAction;
	QAction *m_saveAsAction;
	QAction *m_exportAction;
	QAction *m_exportEpsAction;
	QAction *m_exportPdfAction;
	QAction *m_exportPngAction;
	Action *m_reloadAction;
	QAction *m_closeAction;
	QAction *m_exitAction;
	QAction *m_procStopAction;
	Action *m_buildAction;
	QAction *m_viewLogAction;
	QAction *m_shellEscapeAction;
	QAction *m_configureAction;
	QAction *m_showTikzDocAction;
	QAction *m_whatsThisAction;
	QToolButton *m_shellEscapeButton;
	bool m_useShellEscaping;

	QPointer<ConfigDialog> m_configDialog;

	Url m_currentUrl;
	Url m_lastUrl;
	QDateTime m_lastInternalModifiedDateTime;
};

#endif
