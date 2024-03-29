/*
 *  kClicker is a tool to click automatically
 *  Copyright (C) 2017-2022  Cedric OCHS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common.h"
#include "mainwindow.h"
#include "configfile.h"

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#ifdef QT_STATICPLUGIN

#include <QtPlugin>

#if defined(Q_OS_WIN32)
	Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
	Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin);
#endif
#elif defined(Q_OS_MAC)
	// forward declaration because this function is not exposed
	extern void qt_set_sequence_auto_mnemonic(bool b);

	Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
#else
	Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif

	Q_IMPORT_PLUGIN(QSvgPlugin)
	Q_IMPORT_PLUGIN(QSvgIconPlugin)

#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

int main(int argc, char *argv[])
{
#if defined(_MSC_VER) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	QApplication app(argc, argv);

	QApplication::setApplicationName(PRODUCT);
	QApplication::setOrganizationName(AUTHOR);
	QApplication::setApplicationVersion(VERSION);
	QApplication::setWindowIcon(QIcon(":/icons/icon.svg"));

	ConfigFile* config = new ConfigFile();

	QLocale locale = QLocale::system();

	// load application translations
	QTranslator localTranslator;
	if (localTranslator.load(locale, TARGET, "_", ConfigFile::getInstance()->getTranslationsDirectory()))
	{
		QApplication::installTranslator(&localTranslator);
	}

	// load Qt default translations
	QTranslator qtTranslator;
	if (qtTranslator.load(locale, "qt", "_", ConfigFile::getInstance()->getQtTranslationsDirectory()))
	{
		QApplication::installTranslator(&qtTranslator);
	}

#ifdef Q_OS_MAC
	qt_set_sequence_auto_mnemonic(true);
#endif

	MainWindow mainWindow;
	mainWindow.setWindowTitle(QApplication::applicationName());
	mainWindow.show();

	// only memory leaks are from plugins
	int res = QApplication::exec();

	delete config;

	return res;
}
