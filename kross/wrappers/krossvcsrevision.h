#ifndef KROSSVCSREVISION_H
#define KROSSVCSREVISION_H

#include<QtCore/QVariant>

//This is file has been generated by xmltokross, you should not edit this file but the files used to generate it.

namespace KDevelop { class VcsRevision; }
namespace Handlers
{
	QVariant _kDevelopVcsRevisionHandler(void* type);
	QVariant kDevelopVcsRevisionHandler(KDevelop::VcsRevision* type) { return _kDevelopVcsRevisionHandler((void*) type); }
	QVariant kDevelopVcsRevisionHandler(const KDevelop::VcsRevision* type) { return _kDevelopVcsRevisionHandler((void*) type); }

}

#endif
