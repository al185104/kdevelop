/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef PARSESESSION_H
#define PARSESESSION_H

#include <QList>

#include <clang-c/Index.h>

#include <KUrl>

#include <language/duchain/indexedstring.h>
#include <language/duchain/problem.h>
#include <language/interfaces/iastcontainer.h>
#include <util/path.h>

#include "duchainexport.h"

class ClangIndex;

class KDEVCLANGDUCHAIN_EXPORT ParseSessionData : public KDevelop::IAstContainer
{
public:
    using Ptr = KSharedPtr<ParseSessionData>;

    enum Option {
        NoOption,
        DisableSpellChecking,
        SkipFunctionBodies,
        PrecompiledHeader
    };
    Q_DECLARE_FLAGS(Options, Option)

    /**
     * Parse the given @p contents.
     *
     * @param url The url for the document you want to parse.
     * @param contents The contents of the document you want to parse.
     */
    ParseSessionData(const KDevelop::IndexedString& url, const QByteArray& contents, ClangIndex* index,
                     const KDevelop::Path::List& includes = {}, const KDevelop::Path& pchInclude = {},
                     const QHash<QString, QString>& defines = {}, Options options = Options());

    ~ParseSessionData();

private:
    friend class ParseSession;

    void setUnit(CXTranslationUnit unit, const char* fileName);

    QMutex m_mutex;

    KDevelop::IndexedString m_url;
    CXTranslationUnit m_unit;
    CXFile m_file;

    KDevelop::Path::List m_includes;
    QHash<QString, QString> m_defines;
};

/**
 * Thread-safe utility class around a CXTranslationUnit.
 *
 * It will lock the mutex of the currently set ParseSessionData and thereby ensure
 * only one ParseSession can operate on a given CXTranslationUnit stored therein.
 */
class KDEVCLANGDUCHAIN_EXPORT ParseSession
{
public:
    /**
     * @return a unique identifier for Clang documents.
     */
    static KDevelop::IndexedString languageString();

    /**
     * Initialize a parse session with the given data and, if that data is valid, lock its mutex.
     */
    ParseSession(ParseSessionData::Ptr data);
    /**
     * Unlocks the mutex of the currently set ParseSessionData.
     */
    ~ParseSession();

    /**
     * Unlocks the mutex of the currently set ParseSessionData, and instead acquire the lock in @p data.
     */
    void setData(ParseSessionData::Ptr data);
    ParseSessionData::Ptr data() const;

    /**
     * @return the URL of this session
     */
    KDevelop::IndexedString url() const;

    QList<KDevelop::ProblemPointer> problemsForFile(CXFile file) const;

    CXTranslationUnit unit() const;

    CXFile file() const;

    bool reparse(const QByteArray& contents,
                 const KDevelop::Path::List& includes = {}, const QHash<QString, QString>& defines = {});

    using TopAstNode = CXTranslationUnit;

private:
    ParseSessionData::Ptr d;

};

#endif // PARSESESSION_H
