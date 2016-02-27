/***************************************************************************
 *   Copyright (C) 2009, 2010 by Glad Deschrijver                          *
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

#include "ktikzapplication.h"

#include <KCmdLineArgs>
#include <KUrl>

#include "mainwindow.h"

KtikzApplication::KtikzApplication(int &argc, char **argv)
    : KApplication()
{
	Q_UNUSED(argc);
	Q_UNUSED(argv);
	m_firstTime = true;
}

void KtikzApplication::init()
{
	if (isSessionRestored())
	{
		kRestoreMainWindows<MainWindow>();
		return;
	}

	MainWindow *mainWindow = new MainWindow;
	mainWindow->show();

	KUrl url;
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if (args->count() > 0)
	{
		url = args->url(0);
		if (url.isValid() && url.isLocalFile())
			mainWindow->loadUrl(url);
	}
	args->clear();
}

KtikzApplication::~KtikzApplication()
{
	// don't delete the MainWindow's here because they are already deleted on close
}

QString KtikzApplication::applicationName()
{
	return "KTikZ";
}

