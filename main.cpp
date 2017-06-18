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
    auto rootUser = geteuid() == 0;

    if(/*rootUser &&*/ runRootModule)
    {
        auto root = new CRootModule(getpid());
        if(root->m_error.length() > 1)
        {
            std::cout << "CRootModule failed " << root->m_processId << " error " << root->m_error << std::endl;
            delete root;
            return 0xff;
        }
        root->RunServer();
        delete root;
        return 0x0;
    }

    if(rootUser)
    {
        return 0xF;
    }

    if(runRootModule)
    {
        return 0xE;
    }

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationDomain("");
    QCoreApplication::setOrganizationName("DaniZt");
    QCoreApplication::setApplicationName("TcpView");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

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
