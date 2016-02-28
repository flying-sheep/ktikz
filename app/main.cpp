/***************************************************************************
 *   Copyright (C) 2007 by Florian Hackenberger                            *
 *     <florian@hackenberger.at>                                           *
 *   Copyright (C) 2007, 2008, 2009, 2010 by Glad Deschrijver              *
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

#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <QWidget> // needed for abort() below

#include "ktikzapplication.h"

void debugOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	Q_UNUSED(context);
	// qDebug() and qWarning() only show messages when in debug mode
	switch (type)
	{
		case QtDebugMsg:
		case QtInfoMsg:
#ifndef QT_NO_DEBUG
			fprintf(stderr, "%s\n", msg.toUtf8().constData());
#endif
			break;
		case QtWarningMsg:
#ifndef QT_NO_DEBUG
			fprintf(stderr, "%s\n", msg.toUtf8().constData());
#endif
			break;
		case QtCriticalMsg:
			fprintf(stderr, "%s\n", msg.toUtf8().constData());
			break;
		case QtFatalMsg:
			fprintf(stderr, "Fatal: %s\n", msg.toUtf8().constData());
			abort();
	}
}

bool findTranslator(QTranslator *translator, const QString &transName, const QString &transDir)
{
	const QString qmFile = transName + ".qm";
	const QFileInfo fi(QDir(transDir), qmFile);
	if (fi.exists())
		return translator->load(qmFile, transDir);
	return false;
}

QTranslator *createTranslator(const QString &transName, const QString &transDir)
{
	const QString locale = QLocale().name();
	const QString localeShort = locale.left(2).toLower();

	bool foundTranslator = false;
	QTranslator *translator = new QTranslator(0);

	// find in transDir
	if (!foundTranslator)
		foundTranslator = findTranslator(translator, transName + '_' + locale, transDir);
	if (!foundTranslator)
		foundTranslator = findTranslator(translator, transName + '_' + localeShort, transDir);
	// find in dir which was set during compilation
#ifdef KTIKZ_TRANSLATIONS_INSTALL_DIR
	const QDir qmPath(KTIKZ_TRANSLATIONS_INSTALL_DIR);
	if (!foundTranslator)
		foundTranslator = findTranslator(translator, transName + '_' + locale, qmPath.absolutePath());
	if (!foundTranslator)
		foundTranslator = findTranslator(translator, transName + '_' + localeShort, qmPath.absolutePath());
#endif // KTIKZ_TRANSLATIONS_INSTALL_DIR
	// find in working dir
	if (!foundTranslator)
		foundTranslator = findTranslator(translator, transName + '_' + locale, "");
	if (!foundTranslator)
		foundTranslator = findTranslator(translator, transName + '_' + localeShort, "");

	return translator;
}

int main(int argc, char **argv)
{
	qInstallMessageHandler(debugOutput);

	Q_INIT_RESOURCE(ktikz);

	KAboutData aboutData("ktikz", i18n("KTikZ"), APPVERSION);
	aboutData.setShortDescription(i18n("A TikZ Editor"));
	aboutData.setLicense(KAboutLicense::GPL_V2);
	aboutData.setCopyrightStatement(i18n("Copyright 2007-2010 Florian Hackenberger, Glad Deschrijver"));
	aboutData.setOtherText(i18n("This is a program for creating TikZ (from the LaTeX pgf package) diagrams."));
	aboutData.setBugAddress("florian@hackenberger.at");
	aboutData.addAuthor(i18n("Florian Hackenberger"), i18n("Maintainer"), "florian@hackenberger.at");
	aboutData.addAuthor(i18n("Glad Deschrijver"), i18n("Developer"), "glad.deschrijver@gmail.com");
	
	KtikzApplication app(argc, argv);
	
	QCommandLineParser parser;
	aboutData.setupCommandLine(&parser);
	
	QCommandLineOption urlOption("+[URL]", i18n("TikZ document to open"));
	parser.addOption(urlOption);
	
	parser.process(app);
	
	QCoreApplication::setOrganizationName(ORGNAME);

	const QString translationsDirPath = qgetenv("KTIKZ_TRANSLATIONS_DIR");
	QTranslator *qtTranslator = createTranslator("qt", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	QTranslator *qtikzTranslator = createTranslator("qtikz", translationsDirPath);
	app.installTranslator(qtTranslator);
	app.installTranslator(qtikzTranslator);

	app.init();
	int success = app.exec();

	delete qtTranslator;
	delete qtikzTranslator;
	return success;
}
