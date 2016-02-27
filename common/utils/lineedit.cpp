/****************************************************************************
**
** Copyright (c) 2007 Trolltech ASA <info@trolltech.com>
** Modified (c) 2009 by Glad Deschrijver <glad.deschrijver@gmail.com>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#include "lineedit.h"

LineEdit::LineEdit(const QString &text, QWidget *parent)
    : KLineEdit(text, parent)
{
	setClearButtonShown(true);
}

LineEdit::LineEdit(QWidget *parent)
    : KLineEdit(parent)
{
	setClearButtonShown(true);
}
