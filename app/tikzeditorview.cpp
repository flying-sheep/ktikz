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

#include "tikzeditorview.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QSettings>
#include <QTextCursor>
#include <QToolBar>
#include <QIcon>

#include <KStandardAction>
#include <KActionCollection>

#include "editgotolinewidget.h"
#include "editindentdialog.h"
#include "editreplacewidget.h"
#include "editreplacecurrentwidget.h"
#include "tikzeditor.h"
//#include "tikzeditorhighlighter.h"

TikzEditorView::TikzEditorView(QWidget *parent) : QWidget(parent)
{
	m_parentWidget = static_cast<MainWindow*>(parent);

	m_tikzEditor = new TikzEditor;
	m_tikzEditor->setWhatsThis(tr("<p>Enter your TikZ code here.  "
	    "The code should begin with \\begin{tikzpicture} and end with "
	    "\\end{tikzpicture}.</p>"));
/*
	commandInserter = new TikzCommandInserter(tikzEditor, this);
	tikzHighlighter = new TikzHighlighter(commandInserter, tikzEditor->document());
	tikzHighlighter->rehighlight(); // avoid that textEdit emits the signal contentsChanged() when it is still empty
	tikzController = new TikzPngPreviewer(textEdit);
*/

	m_replaceWidget = new ReplaceWidget(this);
	m_replaceWidget->setVisible(false);
	m_replaceCurrentWidget = new ReplaceCurrentWidget(this);
	m_replaceCurrentWidget->setVisible(false);

	m_goToLineWidget = new GoToLineWidget(this);
	m_goToLineWidget->setVisible(false);

//	QFrame *mainWidget = new QFrame;
//	mainWidget->setLineWidth(1);
//	mainWidget->setFrameShape(QFrame::StyledPanel);
//	mainWidget->setFrameShadow(QFrame::Sunken);
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	mainLayout->addWidget(m_tikzEditor);
	mainLayout->addWidget(m_replaceWidget);
	mainLayout->addWidget(m_replaceCurrentWidget);
	mainLayout->addWidget(m_goToLineWidget);

	createActions();

	connect(m_tikzEditor->document(), &QTextDocument::modificationChanged, this, &TikzEditorView::modificationChanged);
	connect(m_tikzEditor->document(), &QTextDocument::contentsChanged, this, &TikzEditorView::contentsChanged);
	connect(m_tikzEditor, &TikzEditor::cursorPositionChanged, this, &TikzEditorView::cursorPositionChanged);
	connect(m_tikzEditor, &TikzEditor::showStatusMessage, this, &TikzEditorView::showStatusMessage);

	connect(m_tikzEditor, &TikzEditor::focusIn, this, &TikzEditorView::focusIn);
	connect(m_tikzEditor, &TikzEditor::focusOut, this, &TikzEditorView::focusOut);

	connect(m_replaceWidget, &ReplaceWidget::searched, this, &TikzEditorView::search);
	connect(m_replaceWidget, &ReplaceWidget::replaced, this, static_cast<void (TikzEditorView::*)(const QString&)>(&TikzEditorView::replace)); //TODO
	connect(m_replaceWidget, &ReplaceWidget::focusEditor, m_tikzEditor, static_cast<void (TikzEditor::*)()>(&TikzEditor::setFocus));

	connect(m_replaceCurrentWidget, &ReplaceCurrentWidget::showReplaceWidget, m_replaceWidget, &ReplaceWidget::show);
	connect(m_replaceCurrentWidget, &ReplaceCurrentWidget::searched, this, &TikzEditorView::search);
	connect(m_replaceCurrentWidget, &ReplaceCurrentWidget::replaced, this, static_cast<void (TikzEditorView::*)(const QString&)>(&TikzEditorView::replace)); //TODO
	connect(m_replaceCurrentWidget, &ReplaceCurrentWidget::replacedAll, this, &TikzEditorView::replaceAll);
	connect(m_replaceCurrentWidget, &ReplaceCurrentWidget::setSearchFromBegin, this, &TikzEditorView::setSearchFromBegin);

	connect(m_goToLineWidget, &GoToLineWidget::wentToLine, this, &TikzEditorView::goToLine);
}

TikzEditorView::~TikzEditorView()
{
//	tikzHighlighter->deleteLater();
}

QPlainTextEdit *TikzEditorView::editor()
{
	return m_tikzEditor;
}

void TikzEditorView::setFont(const QFont &editorFont)
{
	m_tikzEditor->setFont(editorFont);
	m_tikzEditor->setTabStopWidth(m_tikzEditor->fontMetrics().width("    "));
}

void TikzEditorView::createActions()
{
	KActionCollection *ac = m_parentWidget->actionCollection();
	
	m_undoAction = KStandardAction::undo(m_tikzEditor, SLOT(undo()), ac);
	m_redoAction = KStandardAction::redo(m_tikzEditor, SLOT(redo()), ac);
	m_cutAction = KStandardAction::cut(m_tikzEditor, SLOT(cut()), ac);
	m_copyAction = KStandardAction::copy(m_tikzEditor, SLOT(copy()), ac);
	m_pasteAction = KStandardAction::paste(m_tikzEditor, SLOT(paste()), ac);
	m_selectAllAction = KStandardAction::selectAll(m_tikzEditor, SLOT(selectAll()), ac);
	m_undoAction->setStatusTip(tr("Undo the previous action"));
	m_redoAction->setStatusTip(tr("Redo the previous undone action"));
	m_cutAction->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
	m_copyAction->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
	m_pasteAction->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
	m_selectAllAction->setStatusTip(tr("Select all the content"));
	m_undoAction->setWhatsThis(tr("<p>Undo the previous action.</p>"));
	m_redoAction->setWhatsThis(tr("<p>Redo the previous undone action.</p>"));
	m_cutAction->setWhatsThis(tr("<p>Cut the current selection's contents to the clipboard.</p>"));
	m_copyAction->setWhatsThis(tr("<p>Copy the current selection's contents to the clipboard.</p>"));
	m_pasteAction->setWhatsThis(tr("<p>Paste the clipboard's contents into the current selection.</p>"));
	m_selectAllAction->setWhatsThis(tr("<p>Select all the content.</p>"));

	QAction *action;
	action = ac->addAction("edit_indent", new QAction(QIcon::fromTheme("format-indent-more"), tr("&Indent..."), this));
	ac->setDefaultShortcut(action, tr("Ctrl+I", "Edit|Indent"));
	action->setStatusTip(tr("Indent the current line or selection"));
	action->setWhatsThis(tr("<p>Indent the current line or selection.</p>"));
	connect(action, &QAction::triggered, this, &TikzEditorView::editIndent);
	m_editActions.append(action);

	action = ac->addAction("edit_comment", new QAction(tr("C&omment"), this));
	ac->setDefaultShortcut(action, tr("Ctrl+D", "Edit|Comment"));
	action->setStatusTip(tr("Comment the current line or selection"));
	action->setWhatsThis(tr("<p>Comment the current line or selection.</p>"));
	connect(action, &QAction::triggered, this, &TikzEditorView::editComment);
	m_editActions.append(action);

	action = ac->addAction("edit_uncomment", new QAction(tr("Unco&mment"), this));
	ac->setDefaultShortcut(action, tr("Ctrl+Shift+D", "Edit|Uncomment"));
	action->setStatusTip(tr("Uncomment the current line or selection"));
	action->setWhatsThis(tr("<p>Uncomment the current line or selection.</p>"));
	connect(action, &QAction::triggered, this, &TikzEditorView::editUncomment);
	m_editActions.append(action);

	action = new QAction(this);
	action->setSeparator(true);
	m_editActions.append(action);

	m_editActions.append(KStandardAction::find(this, SLOT(editFind()), ac));
	m_editActions.append(KStandardAction::findNext(this, SLOT(editFindNext()), ac));
	m_editActions.append(KStandardAction::findPrev(this, SLOT(editFindPrevious()), ac));
	m_editActions.append(KStandardAction::replace(this, SLOT(editReplace()), ac));
	m_editActions.append(KStandardAction::gotoLine(this, SLOT(editGoToLine()), ac));
	m_editActions.at(4)->setStatusTip(tr("Look up a piece of text in the document"));
	m_editActions.at(5)->setStatusTip(tr("Search the next occurrence of a text"));
	m_editActions.at(6)->setStatusTip(tr("Search the previous occurrence of a text"));
	m_editActions.at(7)->setStatusTip(tr("Search and replace a piece of text in the document"));
	m_editActions.at(8)->setStatusTip(tr("Go to a certain line in the document"));
	m_editActions.at(4)->setWhatsThis(tr("<p>Look up a piece of text in the document.</p>"));
	m_editActions.at(5)->setWhatsThis(tr("<p>Search the next occurrence of a text.</p>"));
	m_editActions.at(6)->setWhatsThis(tr("<p>Search the previous occurrence of a text.</p>"));
	m_editActions.at(7)->setWhatsThis(tr("<p>Search and replace a piece of text in the document.</p>"));
	m_editActions.at(8)->setWhatsThis(tr("<p>Go to a certain line in the document.</p>"));

	m_undoAction->setEnabled(false);
	m_redoAction->setEnabled(false);
	m_cutAction->setEnabled(false);
	m_copyAction->setEnabled(false);
	m_pasteAction->setEnabled(m_tikzEditor->canPaste());

	connect(m_tikzEditor, &QPlainTextEdit::undoAvailable, m_undoAction, &QAction::setEnabled);
	connect(m_tikzEditor, &QPlainTextEdit::redoAvailable, m_redoAction, &QAction::setEnabled);
	connect(m_tikzEditor, &QPlainTextEdit::copyAvailable, m_cutAction, &QAction::setEnabled);
	connect(m_tikzEditor, &QPlainTextEdit::copyAvailable, m_copyAction, &QAction::setEnabled);
	connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &TikzEditorView::setPasteEnabled);
}

QMenu *TikzEditorView::menu()
{
	QMenu *editMenu = new QMenu(tr("&Edit"), m_parentWidget);
	editMenu->addAction(m_undoAction);
	editMenu->addAction(m_redoAction);
	editMenu->addSeparator();
	editMenu->addAction(m_cutAction);
	editMenu->addAction(m_copyAction);
	editMenu->addAction(m_pasteAction);
	editMenu->addSeparator();
	editMenu->addAction(m_selectAllAction);
	editMenu->addSeparator();
	editMenu->addActions(m_editActions);
	return editMenu;
}

QToolBar *TikzEditorView::toolBar()
{
	QToolBar *editToolBar = new QToolBar(tr("Edit"), m_parentWidget);
	editToolBar->setObjectName("EditToolBar");
	editToolBar->addAction(m_undoAction);
	editToolBar->addAction(m_redoAction);
//	editToolBar->addAction(m_cutAction);
//	editToolBar->addAction(m_copyAction);
//	editToolBar->addAction(m_pasteAction);
	return editToolBar;
}

void TikzEditorView::setPasteEnabled()
{
	m_pasteAction->setEnabled(m_tikzEditor->canPaste());
}

void TikzEditorView::applySettings()
{
	QSettings settings(ORGNAME, APPNAME);

	settings.beginGroup("Editor");
	QFont editorFont;
	editorFont.fromString(settings.value("Font", qApp->font().toString()).toString());
	setFont(editorFont);
	m_tikzEditor->setShowWhiteSpaces(settings.value("ShowWhiteSpaces", false).toBool());
	m_tikzEditor->setShowTabulators(settings.value("ShowTabulators", false).toBool());
	m_tikzEditor->setShowMatchingBrackets(settings.value("ShowMatchingBrackets", true).toBool());
	m_tikzEditor->setWhiteSpacesColor(settings.value("ColorWhiteSpaces", QColor(Qt::gray)).value<QColor>());
	m_tikzEditor->setTabulatorsColor(settings.value("ColorTabulators", QColor(Qt::gray)).value<QColor>());
	m_tikzEditor->setMatchingColor(settings.value("ColorMatchingBrackets", QColor(Qt::yellow)).value<QColor>());
	settings.endGroup();

/*
	settings.beginGroup("Highlighting");
	customHighlighting = settings.value("Customize", true).toBool();
	QMap<QString, QTextCharFormat> formatList = tikzHighlighter->getDefaultHighlightFormats();
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

	tikzHighlighter->setTextCharFormats(formatList);
	tikzHighlighter->rehighlight();
*/
}

/***************************************************************************/

/*
void TikzEditorView::showCursorPosition()
{
	QTextCursor cur = m_tikzEditor->textCursor();
	const int curPos = cur.position();
	cur.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
	emit cursorPositionChanged(cur.blockNumber() + 1, curPos - cur.position() + 1);
}
*/

/***************************************************************************/
/* Go to line */

void TikzEditorView::setLine(const QString &line)
{
	bool ok;
	const int lineNumber = line.toInt(&ok, 10);
	if (ok) setLine(lineNumber);
}


void TikzEditorView::setLine(int lineNumber)
{
	m_replaceWidget->setVisible(false);
	m_goToLineWidget->setVisible(true);
	m_goToLineWidget->setFocus();
	m_goToLineWidget->setMaximumValue(m_tikzEditor->numOfLines());
	m_goToLineWidget->setValue(lineNumber);
}

void TikzEditorView::goToLine(int lineNumber)
{
	m_goToLineWidget->setVisible(false);
	m_tikzEditor->goToLine(lineNumber);
	m_tikzEditor->setFocus();
}

void TikzEditorView::editGoToLine()
{
	int lineNumber;
	lineNumber = m_tikzEditor->textCursor().blockNumber() + 1;
	if (lineNumber < 1)
		lineNumber = 1;
	setLine(lineNumber);
}

/***************************************************************************/

void TikzEditorView::editIndent()
{
	QPointer<IndentDialog> indentDialog = new IndentDialog(this, tr("Indent"));
	if (!indentDialog->exec())
	{
		delete indentDialog;
		return;
	}

	const QString insertString = indentDialog->insertChar();
	const int numOfInserts = indentDialog->numOfInserts();
	delete indentDialog;

	bool go = true;
	QTextCursor textCursor = m_tikzEditor->textCursor();
	if (textCursor.hasSelection())
	{
		textCursor.beginEditBlock();
		const int start = textCursor.selectionStart();
		int end = textCursor.selectionEnd();
		textCursor.setPosition(start, QTextCursor::MoveAnchor);
		textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
		while (textCursor.position() < end && go)
		{
			for (int i = 0; i < numOfInserts; ++i)
				textCursor.insertText(insertString);
			end++;
			go = textCursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
		}
		textCursor.endEditBlock();
	}
	else
	{
		textCursor.beginEditBlock();
		textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
		for (int i = 0; i < numOfInserts; ++i)
			textCursor.insertText(insertString);
		textCursor.endEditBlock();
	}
}

void TikzEditorView::editComment()
{
	bool go = true;
	QTextCursor textCursor = m_tikzEditor->textCursor();
	if (textCursor.hasSelection())
	{
		textCursor.beginEditBlock();
		const int start = textCursor.selectionStart();
		int end = textCursor.selectionEnd();
		textCursor.setPosition(start, QTextCursor::MoveAnchor);
		textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
		while (textCursor.position() < end && go)
		{
			textCursor.insertText("% ");
			++end;
			++end;
			go = textCursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
		}
		textCursor.endEditBlock();
	}
	else
	{
		textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
		textCursor.insertText("% ");
	}
}

void TikzEditorView::editUncomment()
{
	bool go = true;
	QTextCursor textCursor = m_tikzEditor->textCursor();
	if (textCursor.hasSelection())
	{
		textCursor.beginEditBlock();
		const int start = textCursor.selectionStart();
		int end = textCursor.selectionEnd() - 2;
		textCursor.setPosition(start, QTextCursor::MoveAnchor);
		textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
		while (textCursor.position() < end && go)
		{
			textCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
			if (textCursor.selectedText() == QLatin1String("% "))
			{
				textCursor.removeSelectedText();
				--end;
			}
			go = textCursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
		}
		textCursor.endEditBlock();
	}
	else
	{
		textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
		textCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
		if (textCursor.selectedText() == QLatin1String("% "))
			textCursor.removeSelectedText();
	}
}

/***************************************************************************/

void TikzEditorView::editFind()
{
	m_goToLineWidget->setVisible(false);
	m_replaceWidget->setVisible(true);
	m_replaceWidget->setFocus();
	const QTextCursor textCursor = m_tikzEditor->textCursor();
	if (textCursor.hasSelection())
		m_replaceWidget->setText(textCursor.selectedText());
}

bool TikzEditorView::search(const QString &text, bool isCaseSensitive,
    bool findWholeWords, bool forward, bool startAtCursor)
{
	bool isFound = false;

	QTextDocument::FindFlags flags = 0;
	if (isCaseSensitive) flags |= QTextDocument::FindCaseSensitively;
	if (findWholeWords) flags |= QTextDocument::FindWholeWords;

	QTextCursor textCursor = m_tikzEditor->textCursor();
	if (!startAtCursor)
	{
		if (forward) textCursor.movePosition(QTextCursor::Start);
		else textCursor.movePosition(QTextCursor::End);
		m_tikzEditor->setTextCursor(textCursor);
	}
	else
		textCursor.setPosition(textCursor.selectionStart());
	if (!forward) flags |= QTextDocument::FindBackward;
	else textCursor.movePosition(QTextCursor::Right);
	const QTextCursor found = m_tikzEditor->document()->find(text, textCursor, flags);

	if (found.isNull())
	{
		const QString msg = (forward) ?
		    tr("End of document reached.\n\nContinue from the beginning?")
		    : tr("Beginning of document reached.\n\nContinue from the end?");
		const int ret = QMessageBox::warning(this, "Find", msg,
		    QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape);
		if (ret == QMessageBox::Yes)
		{
			return search(text, isCaseSensitive, findWholeWords, forward, false);
		}
	}
	else
	{
		m_tikzEditor->setTextCursor(found);
		emit setSearchFromBegin(false);
		isFound = true;
	}
	m_tikzEditor->viewport()->repaint();
	return isFound;
}

void TikzEditorView::editFindNext()
{
	m_goToLineWidget->setVisible(false);
	m_replaceWidget->setVisible(true);
	m_replaceWidget->setFocus();
	m_replaceWidget->setForward(true);
	m_replaceWidget->doFind();
}

void TikzEditorView::editFindPrevious()
{
	m_goToLineWidget->setVisible(false);
	m_replaceWidget->setVisible(true);
	m_replaceWidget->setFocus();
	m_replaceWidget->setForward(false);
	m_replaceWidget->doFind();
}

void TikzEditorView::editReplace()
{
/*
	m_goToLineWidget->setVisible(false);
	m_replaceWidget->setVisible(true);
	m_replaceWidget->setFocus();
	const QTextCursor textCursor = m_tikzEditor->textCursor();
	if (textCursor.hasSelection())
		m_replaceWidget->setText(textCursor.selectedText());
*/
	editFind();
}

void TikzEditorView::replace(const QString &replacement)
{
	QTextCursor textCursor = m_tikzEditor->textCursor();

	if (textCursor.hasSelection())
	{
		const int start = textCursor.selectionStart();
		textCursor.beginEditBlock();
		textCursor.removeSelectedText();
		textCursor.insertText(replacement);
		textCursor.setPosition(start, QTextCursor::MoveAnchor);
		textCursor.setPosition(start + replacement.length(), QTextCursor::KeepAnchor);
		textCursor.endEditBlock();
		m_tikzEditor->setTextCursor(textCursor);
	}
}

void TikzEditorView::replace(const QString &text, const QString &replacement,
    bool isCaseSensitive, bool findWholeWords, bool forward, bool startAtCursor)
{
	m_replaceWidget->setVisible(false);
	m_replaceCurrentWidget->setReplacement(text, replacement);
	m_replaceCurrentWidget->setVisible(true);
	m_replaceCurrentWidget->search(text, replacement, isCaseSensitive, findWholeWords, forward, startAtCursor);
/*
	bool go = true;
	while (go && search(text, isCaseSensitive, findWholeWords, forward, startAtCursor))
	{
		switch(QMessageBox::warning(this, "Replace", tr("Replace this occurrence?"), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes))
		{
			case QMessageBox::Yes:
				replace(replacement);
				emit setSearchFromBegin(false);
				break;
			case QMessageBox::YesToAll:
				replace(replacement);
				replaceAll(text, replacement, isCaseSensitive, findWholeWords, forward, startAtCursor);
				return;
				break;
			case QMessageBox::No:
				emit setSearchFromBegin(false);
				break;
			case QMessageBox::Cancel:
				go = false;
				break;
			default:
				break;
		}
	}
	if (go)
		emit setSearchFromBegin(true);
*/
}

void TikzEditorView::replaceAll(const QString &text, const QString &replacement,
    bool isCaseSensitive, bool findWholeWords, bool forward, bool startAtCursor)
{
	while (search(text, isCaseSensitive, findWholeWords, forward, startAtCursor))
	{
		replace(replacement);
		emit setSearchFromBegin(false);
	}
	emit setSearchFromBegin(true);
}

void TikzEditorView::setCompleter(QCompleter *completer)
{
	m_tikzEditor->setCompleter(completer);
}
