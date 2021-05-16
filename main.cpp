/* This file is part of "TcpView For Linux" - network connections viewer for Linux
 * Copyright (C) 2021 chipmunk-sm <dannico@linuxmail.org>
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

#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "source/rootmodule.h"
#include "source/clanguage.h"

int main(int argc, char *argv[])
{

    auto runRootModule = (argc == 3 && argv[1] != nullptr && strstr(argv[1], "rootmodule") != nullptr);
    if(runRootModule)
    {
        auto root = std::make_shared<CRootModule>(getpid(), argv[2]);
        if(root->isAbort())
            return -1;

        root->RunServer();
        return 0x0;
    }

    Q_INIT_RESOURCE(tcpview);

    QApplication app(argc, argv);

    QCoreApplication::setOrganizationDomain("");
    QCoreApplication::setOrganizationName("chipmunk-sm");
    QCoreApplication::setApplicationName("tcpview");
    QCoreApplication::setApplicationVersion("3.0 beta");

    CLanguage  m_lang;
    m_lang.SetLangByLocale();

    MainWindow w;
    w.show();

    return app.exec();

}
