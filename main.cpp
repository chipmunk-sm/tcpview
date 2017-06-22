/* This file is part of tcpview - network connections viewer for Linux
 * Copyright (C) 2017 chipmunk-sm <dannico@linuxmail.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "window.h"

#include <QApplication>
#include <QStandardItemModel>
#include <QTime>
#include <QFileInfo>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include <unistd.h>
#include <chrono>
#include <thread>

#include "rootmodule.h"

int main(int argc, char *argv[])
{

    auto runRootModule = (argc == 2 && argv[1] != nullptr && strstr(argv[1], "rootmodule") != nullptr);
    if(runRootModule)
    {
        auto root = new CRootModule(getpid());
        if(root->m_error.length() > 1)
        {
            std::cout << "CRootModule failed " << root->m_processId << " error " << root->m_error << std::endl;
            delete root;
            return -1;
        }
        root->RunServer();
        delete root;
        return 0x0;
    }

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationDomain("");
    QCoreApplication::setOrganizationName("danizt");
    QCoreApplication::setApplicationName("tcpview");
    QCoreApplication::setApplicationVersion("1.0 beta");

#ifndef QT_NO_TRANSLATION

    auto currentLocaleName = QLocale::system().name();

    {
        auto translatorFileName = QLatin1String("qt_") + currentLocaleName;
        auto translator = new QTranslator(&app);
        if (translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
            app.installTranslator(translator);

    }

    {
        auto appPathName = QFileInfo(QCoreApplication::applicationFilePath());
        auto translatorName = appPathName.baseName() + QLatin1String("_") + currentLocaleName;

        auto translator = new QTranslator(&app);
        if (translator->load(translatorName))
            app.installTranslator(translator);
        //else if (translator->load(translatorName, desctopenvtranslationpath ))
        //    app.installTranslator(translator);
        else if (translator->load(translatorName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
            app.installTranslator(translator);
    }


#endif

    Window window;
    window.InitControlsText();
    window.RestoreAppState();
    window.show();
    return app.exec();
}
