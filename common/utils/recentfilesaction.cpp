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

#include "recentfilesaction.h"

#include "action.h"
#include "icon.h"
#include "url.h"

#include <KActionCollection>
#include <KConfigGroup>

RecentFilesAction::RecentFilesAction(QObject *parent)
    : KRecentFilesAction(parent)
{
	Action::actionCollection()->addAction("file_open_recent", this);
	connect(this, SIGNAL(urlSelected(KUrl)), this, SLOT(selectUrl(KUrl)));
}

RecentFilesAction::RecentFilesAction(const QString &text, QObject *parent)
    : KRecentFilesAction(text, parent)
{
	Action::actionCollection()->addAction("file_open_recent", this);
	connect(this, SIGNAL(urlSelected(KUrl)), this, SLOT(selectUrl(KUrl)));
}

RecentFilesAction::RecentFilesAction(const Icon &icon, const QString &text, QObject *parent)
    : KRecentFilesAction(icon, text, parent)
{
	Action::actionCollection()->addAction("file_open_recent", this);
	connect(this, SIGNAL(urlSelected(KUrl)), this, SLOT(selectUrl(KUrl)));
}

void RecentFilesAction::loadEntries()
{
	KRecentFilesAction::loadEntries(KGlobal::config()->group("Recent Files"));
	setEnabled(true);
}

void RecentFilesAction::saveEntries()
{
	KRecentFilesAction::saveEntries(KGlobal::config()->group("Recent Files"));
	KGlobal::config()->sync();
}

void RecentFilesAction::selectUrl(const KUrl &url)
{
	emit urlSelected(Url(url));
}

void RecentFilesAction::addUrl(const Url &url, const QString &name)
{
	KRecentFilesAction::addUrl(url, name);
}

void RecentFilesAction::removeUrl(const Url &url)
{
	KRecentFilesAction::removeUrl(url);
}
