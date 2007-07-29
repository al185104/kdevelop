/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef SCOPE_H
#define SCOPE_H

#include <QtCore/QHash>

namespace QMake
{
    class AssignmentAST;
    class ScopeBodyAST;
}
class QString;

class Scope
{
public:
    Scope();
    ~Scope();
    void setParent( Scope* s );
    Scope* parent() const;

    void setAst( QMake::ScopeBodyAST* a );
    QMake::ScopeBodyAST* ast() const;

    void addVariable(const QString& var, QMake::AssignmentAST* node );

    void addSubScope( Scope* s );

private:
    Scope* m_parent;
    QHash<QString, QMake::AssignmentAST*> m_variables;
    QMake::ScopeBodyAST* m_ast;
    QList<Scope*> m_subScopes;
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
