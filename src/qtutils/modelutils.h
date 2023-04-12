#ifndef MODELUTILS_H
#define MODELUTILS_H

#include <QStringList>
#include <QModelIndex>


// ModelUtils class holding and ordering the functions.
class QAbstractItemModel;
class CheckableProxyModel;
class QFileSystemModel;
class QTreeView;

namespace ModelUtils
{
    namespace Abstract
    {
        QStringList extractStrings(QAbstractItemModel *model, const QModelIndex &parent, bool appendParent = true);
    };

    namespace CheckableProxy
    {
        QStringList extractStrings(CheckableProxyModel *model, const QModelIndex &proxyParent, bool appendParent = true, bool extractCheckedOnly = true);
        void resetCheckedState(CheckableProxyModel *model, bool value, QModelIndex proxyParent = QModelIndex());

        namespace FileSystem
        {
            QStringList extractCheckedFilesPath(QFileSystemModel *model_base, CheckableProxyModel *model_proxy, const QModelIndex &proxyParent, bool extractCheckedOnly = true);
            void checkChildren(QFileSystemModel *model_base, CheckableProxyModel *model_proxy, QTreeView *view, QString const& folder);
        };
    };
};


#endif // MODELUTILS_H
