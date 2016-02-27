/***************************************************************************
 *   Copyright (C) 2009 by Glad Deschrijver                                *
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

#include "standardaction.h"

#include <KActionCollection>
#include <KRecentFilesAction>
#include <KStandardAction>
#include "icon.h"

namespace StandardAction
{
// XXX the following is an ugly hack, but I don't know how to promote a KAction to an Action
Action *copyAction(KAction *action, const QObject *recvr, const char *slot)
{
	Action *newAction = new Action(action->icon(), action->text(), action->parent());
	newAction->setShortcut(action->shortcut());
	newAction->setData(action->data());
	newAction->setObjectName(action->objectName());
	QObject::connect(newAction, SIGNAL(triggered()), recvr, slot);
	Action::actionCollection()->addAction(newAction->objectName(), newAction);
	return newAction;
}

Action *openNew(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::openNew(recvr, slot, parent), recvr, slot);
}
/*
KAction *openNew(const QObject *recvr, const char *slot, QObject *parent)
{
	KAction *action = KStandardAction::openNew(recvr, slot, parent);
	Action::actionCollection()->addAction(action->objectName(), action);
	return action;
}
*/
Action *open(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::open(recvr, slot, parent), recvr, slot);
}
RecentFilesAction *openRecent(const QObject *recvr, const char *slot, QObject *parent)
{
	KRecentFilesAction *action = KStandardAction::openRecent(recvr, slot, parent);
	RecentFilesAction *newAction = new RecentFilesAction(action->icon(), action->text(), action->parent());
	newAction->setShortcut(action->shortcut());
	newAction->setData(action->data());
	newAction->setObjectName(action->objectName());
	newAction->setToolBarMode(KRecentFilesAction::MenuMode);
	newAction->setToolButtonPopupMode(QToolButton::MenuButtonPopup);
	QObject::connect(newAction, SIGNAL(urlSelected(Url)), recvr, slot);
	Action::actionCollection()->addAction(newAction->objectName(), newAction);
	return newAction;
}
/*
KRecentFilesAction *openRecent(const QObject *recvr, const char *slot, QObject *parent)
{
	KRecentFilesAction *action = KStandardAction::openRecent(recvr, slot, parent);
	action->setToolBarMode(KRecentFilesAction::MenuMode);
	action->setToolButtonPopupMode(QToolButton::MenuButtonPopup);
	Action::actionCollection()->addAction(action->objectName(), action);
	return action;
}
*/
Action *save(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::save(recvr, slot, parent), recvr, slot);
}
Action *saveAs(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::saveAs(recvr, slot, parent), recvr, slot);
}
Action *close(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::close(recvr, slot, parent), recvr, slot);
}
Action *quit(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::quit(recvr, slot, parent), recvr, slot);
}
Action *undo(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::undo(recvr, slot, parent), recvr, slot);
}
Action *redo(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::redo(recvr, slot, parent), recvr, slot);
}
Action *cut(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::cut(recvr, slot, parent), recvr, slot);
}
Action *copy(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::copy(recvr, slot, parent), recvr, slot);
}
Action *paste(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::paste(recvr, slot, parent), recvr, slot);
}
Action *selectAll(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::selectAll(recvr, slot, parent), recvr, slot);
}
Action *find(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::find(recvr, slot, parent), recvr, slot);
}
Action *findNext(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::findNext(recvr, slot, parent), recvr, slot);
}
Action *findPrev(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::findPrev(recvr, slot, parent), recvr, slot);
}
Action *replace(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::replace(recvr, slot, parent), recvr, slot);
}
Action *gotoLine(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::gotoLine(recvr, slot, parent), recvr, slot);
}
Action *zoomIn(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::zoomIn(recvr, slot, parent), recvr, slot);
}
Action *zoomOut(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::zoomOut(recvr, slot, parent), recvr, slot);
}
Action *preferences(const QObject *recvr, const char *slot, QObject *parent)
{
	return copyAction(KStandardAction::preferences(recvr, slot, parent), recvr, slot);
}
}
