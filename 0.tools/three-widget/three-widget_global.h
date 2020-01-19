#ifndef THREEWIDGETGLOBAL_H
#define THREEWIDGETGLOBAL_H
#include <QtCore/qglobal.h>

#if defined(TW_LIBRARY)
#  define TW_LIBRARY Q_DECL_EXPORT
#else
#  define TW_LIBRARY Q_DECL_IMPORT
#endif

#endif // THREEWIDGETGLOBAL_H
