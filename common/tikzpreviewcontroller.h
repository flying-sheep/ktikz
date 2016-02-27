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

#ifndef KTIKZ_TIKZPREVIEWCONTROLLER_H
#define KTIKZ_TIKZPREVIEWCONTROLLER_H

#include <QObject>

#include <KUrl>
class KAction;
class KActionCollection;
class KJob;
class KTempDir;
class KToggleAction;

class TemplateWidget;
class TikzPreview;
class TikzPreviewGenerator;
class MainWidget;

class TikzPreviewController : public QObject
{
	Q_OBJECT

public:
	TikzPreviewController(MainWidget *mainWidget);
	virtual ~TikzPreviewController();

	const QString tempDir() const;
	TemplateWidget *templateWidget() const;
	TikzPreview *tikzPreview() const;
	QString tikzCode() const;
	QString getLogText();
	void emptyPreview();
	void applySettings();

public slots:
	void generatePreview();
	void regeneratePreview();

private slots:
	void showJobError(KJob *job);
	void setTemplateFileAndRegenerate(const QString &path);
	void setReplaceTextAndRegenerate(const QString &replace);
	void exportImage();
	void setExportActionsEnabled(bool enabled);
	void setProcessRunning(bool isRunning);
	void toggleShellEscaping(bool useShellEscaping);

signals:
    void logUpdated(const QString &logText, bool runFailed);

private:
	void createTempDir();
	void removeTempDir();

	void createActions();
	bool setTemplateFile(const QString &path);
	KUrl getExportUrl(const KUrl &url, const QString &mimeType) const;
	void generatePreview(bool templateChanged);
	bool cleanUp();

	MainWidget *m_mainWidget;
	QWidget *m_parentWidget;

	TemplateWidget *m_templateWidget;
	TikzPreview *m_tikzPreview;
	TikzPreviewGenerator *m_tikzPreviewGenerator;

	KActionCollection *m_actionCollection;
	KAction *m_exportAction;
	KAction *m_procStopAction;
	KToggleAction *m_shellEscapeAction;

	KTempDir *m_tempDir;
	QString m_tempTikzFileBaseName;
};

#endif
