/***************************************************************************
 *   Copyright (C) 2008 by Glad Deschrijver                                *
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

#include "editreplacewidget.h"

#include <QLineEdit>
#include <QKeyEvent>

#include <QIcon>
#include <KLineEdit>

ReplaceWidget::ReplaceWidget(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
	ui.comboBoxFind->setLineEdit(new KLineEdit(this));
	ui.comboBoxReplace->setLineEdit(new KLineEdit(this));
	ui.pushButtonClose->setIcon(QIcon::fromTheme("window-close"));
	ui.pushButtonBackward->setIcon(QIcon::fromTheme("go-up"));
	ui.pushButtonForward->setIcon(QIcon::fromTheme("go-down"));

	setFocusProxy(ui.comboBoxFind);

	connect(ui.pushButtonBackward, &QPushButton::clicked, this, &ReplaceWidget::setBackward);
	connect(ui.pushButtonForward, &QPushButton::toggled, this, &ReplaceWidget::setForward);
	connect(ui.pushButtonFind, &QPushButton::clicked, this, &ReplaceWidget::doFind);
	connect(ui.pushButtonReplace, &QPushButton::clicked, this, &ReplaceWidget::doReplace);
	connect(ui.pushButtonClose, &QPushButton::clicked, this, &ReplaceWidget::hide);
}

ReplaceWidget::~ReplaceWidget()
{
}

void ReplaceWidget::setBackward(bool backward = true)
{
	ui.pushButtonBackward->setChecked(backward);
	ui.pushButtonForward->setChecked(!backward);
}

void ReplaceWidget::setForward(bool forward = true)
{
	ui.pushButtonBackward->setChecked(!forward);
	ui.pushButtonForward->setChecked(forward);
}

void ReplaceWidget::hide()
{
	setVisible(false);
	emit focusEditor();
}

void ReplaceWidget::doFind()
{
	const QString currentText = ui.comboBoxFind->currentText();
	if (currentText.isEmpty()) return;
	if (ui.comboBoxFind->findText(currentText) < 0)
		ui.comboBoxFind->addItem(currentText);
	
	emit searched(currentText,
		ui.checkBoxCaseSensitive->isChecked(),
		ui.checkBoxWholeWords->isChecked(),
		ui.pushButtonForward->isChecked());
}

void ReplaceWidget::doReplace()
{
	const QString currentText = ui.comboBoxFind->currentText();
	if (currentText.isEmpty()) return;
	if (ui.comboBoxFind->findText(currentText) < 0)
		ui.comboBoxFind->addItem(currentText);
	const QString replacementText = ui.comboBoxReplace->currentText();
	if (ui.comboBoxReplace->findText(replacementText) < 0)
		ui.comboBoxReplace->addItem(replacementText);

	emit replaced(currentText,
	    replacementText,
	    ui.checkBoxCaseSensitive->isChecked(),
	    ui.checkBoxWholeWords->isChecked(),
	    ui.pushButtonForward->isChecked());
}

void ReplaceWidget::setText(const QString &text)
{
	ui.comboBoxFind->lineEdit()->setText(text);
	ui.comboBoxFind->setFocus();
	ui.comboBoxFind->lineEdit()->selectAll();
}

void ReplaceWidget::showEvent(QShowEvent *event)
{
	ui.comboBoxFind->setFocus();
	QWidget::showEvent(event);
}

void ReplaceWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
		hide();
	else if (event->key() == Qt::Key_Return)
		doFind();
	QWidget::keyPressEvent(event);
}
