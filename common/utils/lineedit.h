/****************************************************************************
**
** Copyright (c) 2007 Trolltech ASA <info@trolltech.com>
** Modified (c) 2009 by Glad Deschrijver <glad.deschrijver@gmail.com>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#ifndef KTIKZ_LINEEDIT_H
#define KTIKZ_LINEEDIT_H

#include <KLineEdit>

class LineEdit : public KLineEdit
{
	Q_OBJECT

public:
	explicit LineEdit(const QString &text, QWidget *parent = 0);
	LineEdit(QWidget *parent = 0);
};

#endif // LINEEDIT_H
