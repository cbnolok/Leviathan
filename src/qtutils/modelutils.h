#ifndef MODELUTILS_H
#define MODELUTILS_H

#include <QStringList>
#include <QModelIndex>


class QAbstractItemModel;
class QDirModel;
class CheckableProxyModel;

class ModelUtils
{
public:
    static QStringList extractStringsFromModel(QAbstractItemModel *model, const QModelIndex &parent, bool appendParent = true);
    static QStringList extractStringsFromCheckableProxyModel(CheckableProxyModel *model, const QModelIndex &proxyParent, bool appendParent = true, bool extractCheckedOnly = true);
    static QStringList extractPathsFromCheckableProxyModelSourcedQDirModel(CheckableProxyModel *model, const QModelIndex &proxyParent, bool extractCheckedOnly = true);
    static void resetCheckedStateCheckableProxyModel(CheckableProxyModel *model, bool value, const QModelIndex &proxyParent = QModelIndex());
};


#endif // MODELUTILS_H
