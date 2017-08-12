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

#ifndef CLANGUAGE_H
#define CLANGUAGE_H

#include <QStringList>
#include <QMap>
#include <QMainWindow>
#include <QCoreApplication>
#include <QTranslator>


class CLanguage
{
public:

    CLanguage();
    void SetLang(const QString &langName);
    void SetLangByLocale();
    const QStringList GetListLangNames();

private:
    QString ExtractLanguageName(const QString &fileName);
    void GetSysLocale();

    QStringList            m_langNames;
    QMap<QString, QString> m_langList;
    QString                m_sysLocale;
    QTranslator            m_translator;

};

#endif // CLANGUAGE_H
