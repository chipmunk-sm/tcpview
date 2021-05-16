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

#include "cfilesavewrapper.h"

#include <QMessageBox>

CFileSaveWrapper::CFileSaveWrapper()
    : m_parent(nullptr)
    , m_pQFile(nullptr)
    , m_pQXmlStreamWriter(nullptr)
    , m_pQTextStream(nullptr)
{
}

CFileSaveWrapper::~CFileSaveWrapper()
{
    Close();
}

void CFileSaveWrapper::Open(const QString fileName, const QString fileType, QWidget *parent)
{

    m_parent = parent;

    m_pQFile = new QFile(fileName);
    if (!m_pQFile->open(QFile::WriteOnly | QFile::Text))
    {
        QString mess(QObject::tr("Error: Cannot write file\n") + fileName + "\n" +  m_pQFile->errorString());
        QMessageBox::critical(m_parent, QObject::tr("Save"), mess, QMessageBox::Ok);
        return;
    }

    if(fileType.startsWith("xml", Qt::CaseInsensitive))
    {
        m_pQXmlStreamWriter = new QXmlStreamWriter(m_pQFile);
        m_pQXmlStreamWriter->setAutoFormatting(true);
        m_pQXmlStreamWriter->writeStartDocument();
        m_pQXmlStreamWriter->writeStartElement("root");
    }
    else if(fileType.startsWith("csv", Qt::CaseInsensitive))
    {
        m_pQTextStream = new QTextStream(m_pQFile);
    }
}

void CFileSaveWrapper::Close()
{

    if(m_pQXmlStreamWriter != nullptr)
    {
        m_pQXmlStreamWriter->writeEndDocument();
        delete m_pQXmlStreamWriter;
        m_pQXmlStreamWriter = nullptr;
    }

    if(m_pQTextStream != nullptr)
    {
        m_pQTextStream->flush();
        delete m_pQTextStream;
        m_pQTextStream = nullptr;
    }

    if (m_pQFile != nullptr)
    {
        m_pQFile->close();
        if (m_pQFile->error())
        {
            QString mess(QObject::tr("Error: Cannot write file\n") + m_pQFile->errorString());
            QMessageBox::critical(m_parent, QObject::tr("Save"), mess, QMessageBox::Ok);
        }
        delete m_pQFile;
        m_pQFile = nullptr;
    }
    m_tag.clear();
}

void CFileSaveWrapper::ElemStart(const QString Val)
{
    if( m_pQXmlStreamWriter)
        m_pQXmlStreamWriter->writeStartElement(Val);

    m_tag.push_back(Val);
}

void CFileSaveWrapper::ElemVal(const QString Val)
{
    if( m_pQXmlStreamWriter)
        m_pQXmlStreamWriter->writeCharacters(Val);
    else if(m_pQTextStream)
        *m_pQTextStream << Val;
}

void CFileSaveWrapper::ElemEnd(const QString Val)
{
    if( m_pQXmlStreamWriter)
        m_pQXmlStreamWriter->writeEndElement(/*Val*/);
    else if(m_pQTextStream)
        *m_pQTextStream << ((Val.compare("c") == 0) ? "\t" : "\n");

    if(m_tag.isEmpty() || m_tag.last().compare(Val) != 0)
        throw std::runtime_error("Error writeEndElement.");

    m_tag.pop_back();
}


