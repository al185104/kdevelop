/* This file is part of KDevelop
*  Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>
Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

*/

#ifndef ASTYLESTRINGITERATOR_H
#define ASTYLESTRINGITERATOR_H

#include <QString>
#include <QTextStream>

#include "astyle.h"

#include <string>

class AStyleStringIterator : public astyle::ASSourceIterator
{
public:
    explicit AStyleStringIterator(const QString &string);
    virtual ~AStyleStringIterator();

    astyle::streamoff tellg() override;
    int getStreamLength() const override;
    bool hasMoreLines() const override;
    std::string nextLine(bool emptyLineWasDeleted = false) override;
    std::string peekNextLine() override;
    void peekReset() override;

private:
    QString m_content;
    QTextStream m_is;
    qint64 m_peekStart;
};

#endif // ASTYLESTRINGITERATOR_H
