#include "modelutils.h"
#include <QAbstractItemModel>
#include <QFileSystemModel>
#include <QTreeView>
#include "checkableproxymodel.h"

#include <QDebug>


// To use with a tree-style model.
auto ModelUtils::Abstract::
extractStrings(QAbstractItemModel *model, const QModelIndex &parent, bool appendParent)
-> QStringList
{
    QStringList retVal;

    for(int i = 0; i < model->rowCount(parent); ++i)
    {
        const QModelIndex idx = model->index(i, 0, parent);
        if (!idx.isValid())
            continue;

        QString str(idx.data(Qt::DisplayRole).toString());
        if (appendParent)
        {
            QModelIndex parentOfParent = parent;
            while (parentOfParent.isValid())
            {
                str.prepend(parentOfParent.data(Qt::DisplayRole).toString() + '/');
                parentOfParent = parentOfParent.parent();
            }
        }
        retVal << str;

        // Check if this node has children elements, if affirmative, append them to the list.
        QStringList recursiveList = Abstract::extractStrings(model, idx, appendParent);
        if (!recursiveList.isEmpty())
            retVal << recursiveList;
    }

    return retVal;
}

// To use with a tree-style model.
auto ModelUtils::CheckableProxy::
extractStrings(CheckableProxyModel *model, const QModelIndex &folderIdxProxy, bool appendParent, bool extractCheckedOnly)
-> QStringList
{
    // It isn't suitable for directory trees (QFileSystemModel), since on Windows the drive letter is followed by other text, and because also
    //  the directories are checked and included in the returned list.
    QStringList retVal;

    for (int i = 0; i < model->rowCount(folderIdxProxy); ++i)
    {
        const QModelIndex idx = model->index(i, 0, folderIdxProxy);
        if(!idx.isValid())
            continue;

        bool checked = (idx.data(Qt::CheckStateRole)==Qt::Checked);
        if ( (extractCheckedOnly && checked) || !extractCheckedOnly )
        {
            QString str(idx.data(Qt::DisplayRole).toString());
            if (appendParent)
            {
                QModelIndex parentOfParent = folderIdxProxy;
                while (parentOfParent.isValid())
                {
                    str.prepend(parentOfParent.data(Qt::DisplayRole).toString() + '/');
                    parentOfParent = parentOfParent.parent();
                }
            }
            retVal << str;
        }

        // Check if this node has children elements, if affirmative, append them to the list.
        QStringList recursiveList = CheckableProxy::extractStrings(model, idx, appendParent, extractCheckedOnly);
        if (!recursiveList.isEmpty())
            retVal << recursiveList;
    }

    return retVal;
}


auto ModelUtils::CheckableProxy::FileSystem::
extractCheckedFilesPath(QFileSystemModel *model_base, CheckableProxyModel *model_proxy, const QModelIndex &folderIdxProxy, bool extractCheckedOnly)
-> QStringList
{
    QStringList retVal;
    for (int i = 0; i < model_proxy->rowCount(folderIdxProxy); ++i)
    {
        const QModelIndex idx = model_proxy->index(i, 0, folderIdxProxy);
        if(!idx.isValid())
            continue;
        const QModelIndex sourceIdx = model_proxy->mapToSource(idx);

        const int role = idx.data(Qt::CheckStateRole).toInt();
        const bool checked = (role==Qt::Checked) || (role==Qt::PartiallyChecked);
        if ( (extractCheckedOnly && checked) || !extractCheckedOnly )
        {
            if (!model_base->isDir(sourceIdx))
            {
                retVal << model_base->filePath(sourceIdx);
            }
            else
            {
                if (role==Qt::Checked)
                    retVal << model_base->filePath(sourceIdx);

                // Check if this node has children elements, if affirmative, append them to the list.
                QStringList recursiveList = CheckableProxy::FileSystem::extractCheckedFilesPath(model_base, model_proxy, idx, extractCheckedOnly);
                if (!recursiveList.isEmpty())
                    retVal << recursiveList;
            }
        }
    }

    return retVal;
}

auto ModelUtils::CheckableProxy::FileSystem::
checkChildren(QFileSystemModel *model_base, CheckableProxyModel *model_proxy, QTreeView *view, QString const& folder)
-> void
{
    qDebug() << "entro";
    if (!model_base || !model_proxy || !view)
        return;
    if (folder.isEmpty())
        return;

    QModelIndex viewRootIdx     = view->rootIndex();
    if (!viewRootIdx.isValid())
        return;
    QModelIndex folderIdxBase   = model_base->index(folder);
    if (!folderIdxBase.isValid())
        return;
    QModelIndex folderIdxProxy  = model_proxy->mapFromSource(folderIdxBase);
    if (!folderIdxProxy.isValid())
        return;

    qDebug() << "inizio";
    for (int baseRow = 0; baseRow < model_proxy->rowCount(folderIdxProxy); ++baseRow)
    {
        qDebug() << 1;
        const QModelIndex elemIdxProxy = model_proxy->index(baseRow, 0, folderIdxProxy);
        //const QModelIndex elemIdxProxy = folderIdxProxy.siblingAtRow(baseRow);
        if (!elemIdxProxy.isValid())
            continue;

        qDebug() << 2;
        const int state = model_proxy->data(elemIdxProxy, Qt::CheckStateRole).toInt();
        const QModelIndex elemIdxSource = model_proxy->mapToSource(elemIdxProxy);
        if (!model_base->isDir(elemIdxSource))
        {
            qDebug() << 3;
            // I'm a child file
            if (folderIdxProxy == viewRootIdx)
                continue; //check only files in subfolders

            qDebug() << elemIdxSource.data() << " " << elemIdxProxy.data(Qt::CheckStateRole);

            if (state != Qt::Checked)
                model_proxy->setData(elemIdxProxy, Qt::Checked, Qt::CheckStateRole);

            continue;
        }

        qDebug() << 4;
        // I'm a child folder.
        if (state != Qt::Checked)
            continue;

        view->expand(elemIdxProxy);
    }
}

auto ModelUtils::CheckableProxy::
resetCheckedState(CheckableProxyModel *model, bool value, QModelIndex folderIdxProxy)
-> void
{
    if (!folderIdxProxy.isValid())
        folderIdxProxy = model->index(0,0,QModelIndex());
    Q_ASSERT(folderIdxProxy.isValid());

    for (int i = 0; i < model->rowCount(folderIdxProxy); ++i)
    {
        const QModelIndex idx = model->index(i, 0, folderIdxProxy);
        if (!idx.isValid())
            continue;

        model->setData(idx, value ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);

        const QAbstractItemModel *itemModel = idx.model();
        if (!itemModel)
            continue;
        const QModelIndex childIdx = itemModel->index(0,0, idx);
        if (childIdx.isValid())
            CheckableProxy::resetCheckedState(model, value, childIdx);
    }
}


