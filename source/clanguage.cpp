/* This file is part of "TcpView For Linux" - network connections viewer for Linux
 * Copyright (C) 2019 chipmunk-sm <dannico@linuxmail.org>
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


#include "clanguage.h"

#include <QTranslator>
#include <QLibraryInfo>
#include <QFileInfo>
#include <QCoreApplication>
#include <QLocale>
#include <iostream>

CLanguage::CLanguage()
{

    LoadTranslations(QDir(":/translations"));

}

void CLanguage::LoadTranslations(const QDir &dir)
{
    auto fileNames = dir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);
    foreach (const QString &str, fileNames)
    {
        auto path = dir.filePath(str);
        auto langName = ExtractLanguageName(path);

//        std::cout << langName.toStdString()  << "\t" << str.toStdString()<< "\t" << path.toStdString() << std::endl;

        if(langName.length() < 1)
            continue;

        if(m_langList.find(langName) != m_langList.end())
            continue;

        m_langList.insert(langName, path);
        m_langNames.append(langName);
    }

}

void CLanguage::SetLang(const QString &langName)
{

    auto it= m_langList.find(langName);
    if(it == m_langList.end())
        return;

    if (!m_translator.load(it.value()))
        return;

    QCoreApplication::instance()->installTranslator(&m_translator);
}

void CLanguage::SetLangByLocale()
{

    //xx_XX
    auto localeName = QLocale::system().name();
    if(SetLangByLocale(localeName))
        return;

    //xx
    auto list = localeName.split(QRegExp("(_|-)"), QString::SkipEmptyParts);
    foreach (auto tmp, list)
    {
        if(SetLangByLocale(tmp))
            return;
        break;
    }

}

bool CLanguage::SetLangByLocale(QString localeName)
{

    localeName = QString("language_%1.qm").arg(localeName);

    foreach (auto value, m_langList)
    {
        if(value.contains(localeName, Qt::CaseInsensitive))
        {
            if (!m_translator.load(value))
                return false;

            QCoreApplication::instance()->installTranslator(&m_translator);
            return true;
        }
    }

    return false;

}

const QStringList CLanguage::GetListLangNames()
{
    return m_langNames;
}

QString CLanguage::ExtractLanguageName(const QString &fileName)
{
    auto writeLanguageName = QObject::tr("English");
    Q_UNUSED(writeLanguageName);

    QTranslator translator;
    translator.load(fileName);
    return translator.translate("QObject", "English");
}




