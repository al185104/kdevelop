#ifndef KROSSIUICONTROLLER_H
#define KROSSIUICONTROLLER_H

#include<QtCore/QVariant>

//This is file has been generated by xmltokross, you should not edit this file but the files used to generate it.

namespace KDevelop { class IToolViewFactory; }
namespace KDevelop { class IUiController; }
namespace Handlers
{
	QVariant _kDevelopIToolViewFactoryHandler(void* type);
	QVariant kDevelopIToolViewFactoryHandler(KDevelop::IToolViewFactory* type) { return _kDevelopIToolViewFactoryHandler((void*) type); }
	QVariant kDevelopIToolViewFactoryHandler(const KDevelop::IToolViewFactory* type) { return _kDevelopIToolViewFactoryHandler((void*) type); }

	QVariant _kDevelopIUiControllerHandler(void* type);
	QVariant kDevelopIUiControllerHandler(KDevelop::IUiController* type) { return _kDevelopIUiControllerHandler((void*) type); }
	QVariant kDevelopIUiControllerHandler(const KDevelop::IUiController* type) { return _kDevelopIUiControllerHandler((void*) type); }

}

#endif
