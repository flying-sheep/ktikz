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

#include "filedialog.h"

#include <KMessageBox>
#include <KIO/NetAccess>

Url FileDialog::getOpenUrl(QWidget *parent, const QString &caption, const Url &dir, const QString &filter)
{
	return KFileDialog::getOpenUrl(dir, filter, parent, caption);
}

Url FileDialog::getSaveUrl(QWidget *parent, const QString &caption, const Url &dir, const QString &filter)
{
	return KFileDialog::getSaveUrl(dir, filter, parent, caption, KFileDialog::ConfirmOverwrite);
}
