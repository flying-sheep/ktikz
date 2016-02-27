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

#include "toggleaction.h"

#include "action.h"
#include "icon.h"

#include <KActionCollection>

ToggleAction::ToggleAction(QObject *parent, const QString &name)
    : KToggleAction(parent)
{
	if (!name.isEmpty())
		Action::actionCollection()->addAction(name, this);
}

ToggleAction::ToggleAction(const QString &text, QObject *parent, const QString &name)
    : KToggleAction(text, parent)
{
	if (!name.isEmpty())
		Action::actionCollection()->addAction(name, this);
}

ToggleAction::ToggleAction(const Icon &icon, const QString &text, QObject *parent, const QString &name)
    : KToggleAction(icon, text, parent)
{
	if (!name.isEmpty())
		Action::actionCollection()->addAction(name, this);
}
