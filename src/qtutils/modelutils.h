#ifndef MODELUTILS_H
#define MODELUTILS_H

#include <QStringList>
#include <QModelIndex>


// ModelUtils class holding and ordering the functions.
class QAbstractItemModel;
class CheckableProxyModel;
class QFileSystemModel;
class QTreeView;

class ModelUtils
{
public:
    class Abstract
    {
        static QStringList extractStrings(QAbstractItemModel *model, const QModelIndex &parent, bool appendParent = true);
    };

    class CheckableProxy
    {
    public:
        static QStringList extractStrings(CheckableProxyModel *model, const QModelIndex &proxyParent, bool appendParent = true, bool extractCheckedOnly = true);
        static void resetCheckedState(CheckableProxyModel *model, bool value, QModelIndex proxyParent = QModelIndex());

        class FileSystem
        {
        public:
            static QStringList extractCheckedFilesPath(QFileSystemModel *model_base, CheckableProxyModel *model_proxy, const QModelIndex &proxyParent, bool extractCheckedOnly = true);
            static void checkChildren(QFileSystemModel *model_base, CheckableProxyModel *model_proxy, QTreeView *view, QString const& folder);
        };
    };

    /*
    class FileSystem
    {
    public:

    };
    */
};


#endif // MODELUTILS_H
