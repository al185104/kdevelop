/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "qmakemanager.h"
#include <QList>
#include <QVector>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QHash>

#include <kurl.h>
#include <kio/job.h>
#include <kprocess.h>
#include <kdebug.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <interfaces/iruncontroller.h>
#include <language/duchain/indexedstring.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include "iqmakebuilder.h"
#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kpluginloader.h>
#include <kdebug.h>
#include <project/projectmodel.h>

#include "qmakemodelitems.h"
#include "qmakeprojectfile.h"
#include "qmakecache.h"
#include "qmakemkspecs.h"
#include <QAction>
#include <KIcon>
#include "qmakejob.h"
#include <KDirWatch>
#include <interfaces/iprojectcontroller.h>

using namespace KDevelop;

//BEGIN Helpers

QMakeFolderItem* findQMakeFolderParent(ProjectBaseItem* item) {
    QMakeFolderItem* p = 0;
    while (!p && item) {
        p = dynamic_cast<QMakeFolderItem*>( item );
        item = item->parent();
    }
    return p;
}

//END Helpers

K_PLUGIN_FACTORY(QMakeSupportFactory, registerPlugin<QMakeProjectManager>(); )
K_EXPORT_PLUGIN(QMakeSupportFactory(KAboutData(
    "kdevqmakemanager","kdevqmake", ki18n("QMake Manager"), "0.1",
    ki18n("Support for managing QMake projects"), KAboutData::License_GPL)))

QMakeProjectManager* QMakeProjectManager::m_self = 0;

QMakeProjectManager* QMakeProjectManager::self()
{
    return m_self;
}

QMakeProjectManager::QMakeProjectManager( QObject* parent, const QVariantList& )
        : AbstractFileManagerPlugin( QMakeSupportFactory::componentData(), parent ),
          IBuildSystemManager(),
          m_builder(0),
          m_runQmake(0)
{
    Q_ASSERT(!m_self);
    m_self = this;

    KDEV_USE_EXTENSION_INTERFACE( IBuildSystemManager )
    IPlugin* i = core()->pluginController()->pluginForExtension( "org.kdevelop.IQMakeBuilder" );
    Q_ASSERT(i);
    m_builder = i->extension<IQMakeBuilder>();
    Q_ASSERT(m_builder);

    connect(this, SIGNAL(folderAdded(KDevelop::ProjectFolderItem*)),
            this, SLOT(slotFolderAdded(KDevelop::ProjectFolderItem*)));

    m_runQmake = new QAction(KIcon("qtlogo"), i18n("Run QMake"), this);
    connect(m_runQmake, SIGNAL(triggered(bool)),
            this, SLOT(slotRunQMake()));
}

QMakeProjectManager::~QMakeProjectManager()
{
    m_self = 0;
}

IProjectFileManager::Features QMakeProjectManager::features() const
{
    return Features(Folders | Targets | Files);
}

bool QMakeProjectManager::isValid( const KUrl& url, const bool isFolder, IProject* /*project*/ ) const
{
    // TODO: filter setup

    QString name = url.fileName();
    const QStringList invalidFolders = QStringList() << ".kdev4" << ".svn" << ".git" << "CVS";
    if (isFolder && invalidFolders.contains( name )) {
        return false;
    } else if (!isFolder && (name.startsWith("Makefile") || name.endsWith(".o")
                          || name.startsWith("moc_") || name.endsWith(".moc")
                          || name.endsWith(".so") || name.contains(".so.")))
    {
        return false;
    } else {
        return true;
    }
}

KUrl QMakeProjectManager::buildDirectory(ProjectBaseItem* item) const
{
    ///TODO: support includes by some other parent or sibling in a different file-tree-branch
    QMakeFolderItem* qmakeItem = findQMakeFolderParent(item);
    if ( qmakeItem ) {
        if (!item->parent()) {
            return qmakeItem->projectFiles().first()->buildDirectory();
        }
        foreach ( QMakeProjectFile* pro, qmakeItem->projectFiles() ) {
            if ( pro->hasSubProject( item->url().toLocalFile() ) ) {
                return pro->buildDirectory();
            }
        }
    }

    return KUrl();
}

ProjectFolderItem* QMakeProjectManager::createFolderItem( IProject* project, const KUrl& url,
                                                          ProjectBaseItem* parent )
{
    if ( !parent ) {
        return projectRootItem( project, url );
    } else if (ProjectFolderItem* buildFolder = buildFolderItem( project, url, parent )) {
        // child folder in a qmake folder
        return buildFolder;
    } else {
        return AbstractFileManagerPlugin::createFolderItem( project, url, parent );
    }
}

ProjectFolderItem* QMakeProjectManager::projectRootItem( IProject* project, const KUrl& url )
{
    QFileInfo fi( url.toLocalFile() );
    QDir dir( url.toLocalFile() );
    QStringList l = dir.entryList( QStringList() << "*.pro" );

    QString projectfile;

    if( l.count() && l.indexOf( project->name() + ".pro") != -1 )
        projectfile = project->name() + ".pro";
    if( l.isEmpty() || ( l.count() && l.indexOf( fi.baseName() + ".pro" ) != -1 ) )
    {
        projectfile = fi.baseName() + ".pro";
    }else
    {
        projectfile = l.first();
    }

    KUrl projecturl = url;
    projecturl.adjustPath( KUrl::AddTrailingSlash );
    projecturl.setFileName( projectfile );
    QHash<QString,QString> qmvars = queryQMake( project );
    QMakeMkSpecs* mkspecs = new QMakeMkSpecs( findBasicMkSpec( qmvars["QMAKE_MKSPECS"] ), qmvars );
    mkspecs->read();
    QMakeCache* cache = findQMakeCache( projecturl.toLocalFile() );
    if( cache ) {
        cache->setMkSpecs( mkspecs );
        cache->read();
    }
    QMakeProjectFile* scope = new QMakeProjectFile( projecturl.toLocalFile() );
    scope->setMkSpecs( mkspecs );
    if( cache ) {
        scope->setQMakeCache( cache );
    }
    scope->read();
    kDebug(9024) << "top-level scope with variables:" << scope->variables();
    QMakeFolderItem* item = new QMakeFolderItem( project, project->folder() );
    item->addProjectFile(scope);
    return item;
}

ProjectFolderItem* QMakeProjectManager::buildFolderItem( IProject* project, const KUrl& url,
                                                         ProjectBaseItem* parent )
{
    // find .pro or .pri files in dir
    QDir dir(url.toLocalFile());
    QStringList projectFiles = dir.entryList(QStringList() << "*.pro" << "*.pri", QDir::Files);
    if ( projectFiles.isEmpty() ) {
        return 0;
    }

    QMakeFolderItem* folderItem = new QMakeFolderItem(project, url, parent);

    //TODO: included by not-parent file (in a nother file-tree-branch).
    QMakeFolderItem* qmakeParent = findQMakeFolderParent(parent);
    Q_ASSERT(qmakeParent);

    foreach( const QString& file, projectFiles ) {
        const QString absFile = dir.absoluteFilePath(file);

        //TODO: multiple includes by different .pro's
        QMakeProjectFile* parentPro = 0;
        foreach( QMakeProjectFile* p, qmakeParent->projectFiles() ) {
            if (p->hasSubProject(absFile)) {
                parentPro = p;
                break;
            }
        }
        if (!parentPro && file.endsWith(".pri")) {
            continue;
        }
        kDebug(9024) << "add project file:" << absFile;
        if (parentPro) {
            kDebug(9024) << "parent:" << parentPro->absoluteFile();
        } else {
            kDebug(9024) << "no parent, assume project root";
        }

        QMakeProjectFile* qmscope = new QMakeProjectFile( absFile );

        const QFileInfo info( absFile );
        const QDir d = info.dir();
        ///TODO: cleanup
        if ( parentPro) {
            // subdir
            if( d.exists(".qmake.cache") ) {
                QMakeCache* cache = new QMakeCache( d.canonicalPath()+"/.qmake.cache" );
                cache->setMkSpecs( parentPro->mkSpecs() );
                cache->read();
                qmscope->setQMakeCache( cache );
            } else {
                qmscope->setQMakeCache( parentPro->qmakeCache() );
            }

            qmscope->setMkSpecs( parentPro->mkSpecs() );
        } else {
            // new project
            QMakeFolderItem* root = dynamic_cast<QMakeFolderItem*>( project->projectItem() );
            Q_ASSERT(root);
            qmscope->setMkSpecs( root->projectFiles().first()->mkSpecs() );
            if( root->projectFiles().first()->qmakeCache() ) {
                qmscope->setQMakeCache( root->projectFiles().first()->qmakeCache() );
            }
        }

        if( qmscope->read() ) {
            //TODO: only on read?
            folderItem->addProjectFile( qmscope );
        } else {
            delete qmscope;
            return 0;
        }
    }

    return folderItem;
}

void QMakeProjectManager::slotFolderAdded( ProjectFolderItem* folder )
{
    QMakeFolderItem* qmakeParent = dynamic_cast<QMakeFolderItem*>( folder );
    if ( !qmakeParent ) {
        return;
    }

    kDebug(9024) << "adding targets for" << folder->url();
    foreach( QMakeProjectFile* pro, qmakeParent->projectFiles() ) {
        foreach( const QString& s, pro->targets() ) {
            kDebug(9024) << "adding target:" << s;
            QMakeTargetItem* target = new QMakeTargetItem( folder->project(), s, folder );
            foreach( const KUrl& u, pro->filesForTarget(s) ) {
                new ProjectFileItem( folder->project(), u, target );
                ///TODO: signal?
            }
        }
    }
}

ProjectFolderItem* QMakeProjectManager::import( IProject* project )
{
    KUrl dirName = project->folder();
    if( !dirName.isLocalFile() )
    {
        //FIXME turn this into a real warning
        kWarning(9025) << "not a local file. QMake support doesn't handle remote projects";
        return 0;
    }

    ProjectFolderItem* ret = AbstractFileManagerPlugin::import( project );

    connect(projectWatcher(project), SIGNAL(dirty(QString)),
            this, SLOT(slotDirty(QString)));

    return ret;
}

void QMakeProjectManager::slotDirty(const QString& path)
{
    if (!path.endsWith(".pro") && !path.endsWith(".pri")) {
        return;
    }

    QFileInfo info(path);
    if (!info.isFile()) {
        return;
    }

    const KUrl url(path);
    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
    Q_ASSERT(project);

    foreach(ProjectFolderItem* folder, project->foldersForUrl(url.upUrl())) {
        if (QMakeFolderItem* qmakeFolder = dynamic_cast<QMakeFolderItem*>( folder )) {
            foreach(QMakeProjectFile* pro, qmakeFolder->projectFiles()) {
                if (pro->absoluteFile() == path) {
                    //TODO: children
                    //TODO: cache added
                    kDebug() << "reloading" << pro << path;
                    pro->read();
                }
            }
            return;
        }
    }
}

QList<ProjectTargetItem*> QMakeProjectManager::targets(ProjectFolderItem* item) const
{
    Q_UNUSED(item)
    return QList<ProjectTargetItem*>();
}

IProjectBuilder* QMakeProjectManager::builder(ProjectFolderItem*) const
{
    Q_ASSERT(m_builder);
    return m_builder;
}

KUrl::List QMakeProjectManager::includeDirectories(ProjectBaseItem* item) const
{
    KUrl::List list;
    QMakeFolderItem* folder = findQMakeFolderParent(item);

    if ( folder ) {
        foreach( QMakeProjectFile* pro, folder->projectFiles() ) {
            if (pro->files().contains(item->url())) {
                foreach(const KUrl& url, pro->includeDirectories()) {
                    Q_ASSERT(url.isValid());
                    if (!list.contains(url)) {
                        list << url;
                    }
                }
            }
        }
//         kDebug(9024) << "include dirs for" << item->url() << ":" << list;
    }
    return list;
}

QString QMakeProjectManager::findBasicMkSpec( const QString& mkspecdir ) const
{
    QFileInfo fi( mkspecdir+"/default/qmake.conf" );
    if( !fi.exists() )
        return QString();

    return fi.absoluteFilePath();
}

QHash<QString,QString> QMakeProjectManager::queryQMake( IProject* project ) const
{
    if( !project->folder().isLocalFile() || !m_builder )
        return QHash<QString,QString>();

    QHash<QString,QString> hash;
    KProcess p;
    QStringList queryVariables;
    queryVariables << "QMAKE_MKSPECS" << "QMAKE_VERSION" <<
            "QT_INSTALL_BINS" << "QT_INSTALL_CONFIGURATION" <<
            "QT_INSTALL_DATA" << "QT_INSTALL_DEMOS" << "QT_INSTALL_DOCS" <<
            "QT_INSTALL_EXAMPLES" << "QT_INSTALL_HEADERS" <<
            "QT_INSTALL_LIBS" << "QT_INSTALL_PLUGINS" << "QT_INSTALL_PREFIX" <<
            "QT_INSTALL_TRANSLATIONS" << "QT_VERSION";
    foreach( const QString& var, queryVariables)
    {
        p.clearProgram();
        p.setOutputChannelMode( KProcess::OnlyStdoutChannel );
        p.setWorkingDirectory( project->folder().toLocalFile() );
        //To be implemented when there's an API to fetch Env from Project
        //p.setEnv();
        p << m_builder->qmakeBinary( project ) << "-query" << var;
        p.execute();
        QString result = QString::fromLocal8Bit( p.readAllStandardOutput() ).trimmed();
        if( result != "**Unknown**")
            hash[var] = result;
    }
    kDebug(9024) << "Ran qmake (" << m_builder->qmakeBinary( project ) << "), found:" << hash;
    return hash;
}

QMakeCache* QMakeProjectManager::findQMakeCache( const QString& projectfile ) const
{
    QDir curdir( QFileInfo( projectfile ).canonicalPath() );
    while( !curdir.exists(".qmake.cache") && !curdir.isRoot() )
    {
        curdir.cdUp();
    }

    if( curdir.exists(".qmake.cache") )
    {
        return new QMakeCache( curdir.canonicalPath()+"/.qmake.cache" );
    }
    return 0;
}

ContextMenuExtension QMakeProjectManager::contextMenuExtension( Context* context )
{
    ContextMenuExtension ext;

    if ( context->hasType( Context::ProjectItemContext ) ) {
        ProjectItemContext* pic = dynamic_cast<ProjectItemContext*>( context );
        Q_ASSERT(pic);
        if ( pic->items().isEmpty() ) {
            return ext;
        }

        m_actionItem = dynamic_cast<QMakeFolderItem*>( pic->items().first() );
        if ( m_actionItem ) {
            ext.addAction( ContextMenuExtension::ProjectGroup, m_runQmake );
        }
    }

    return ext;
}

void QMakeProjectManager::slotRunQMake()
{
    Q_ASSERT(m_actionItem);

    ///TODO: support shadow builds
    QMakeJob* job = new QMakeJob( m_actionItem->url().toLocalFile(), this );

    KDevelop::ICore::self()->runController()->registerJob( job );
}

#include "qmakemanager.moc"
