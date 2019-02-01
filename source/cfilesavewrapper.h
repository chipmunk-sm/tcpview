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

#ifndef CFILESAVEWRAPPER_H
#define CFILESAVEWRAPPER_H

#include <QFile>
#include <QString>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <vector>

#define MSG_ERR_WRITE_FILE QObject::tr("Error: Cannot write file\n")

class CFileSaveWrapper
{
public:
    CFileSaveWrapper();
    ~CFileSaveWrapper();
    void Open(const QString fileName, const QString fileType, QWidget *parent);
    void Close();

    void ElemStart(const QString Val);
    void ElemVal(const QString Val);
    void ElemEnd(const QString Val);


private:
    QWidget             *m_parent;
    QFile               *m_pQFile;
    QXmlStreamWriter    *m_pQXmlStreamWriter;
    QTextStream         *m_pQTextStream;
    QVector<QString>    m_tag;
};

#endif // CFILESAVEWRAPPER_H
