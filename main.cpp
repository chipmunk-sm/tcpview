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

#include "mainwindow.h"
#include <QApplication>
#include "source/rootmodule.h"

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
    QCoreApplication::setApplicationVersion("2.0 beta");

    MainWindow w;
    w.show();

    return app.exec();

}
