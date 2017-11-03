#include "modelutils.h"
#include <QAbstractItemModel>
#include <QDirModel>
#include "checkableproxymodel.h"


// To use with a tree-style model.
QStringList ModelUtils::extractStringsFromModel(QAbstractItemModel *model, const QModelIndex &parent, bool appendParent)
{
    QStringList retVal;

    for(int i = 0; i < model->rowCount(parent); ++i)
    {
        QModelIndex idx = model->index(i, 0, parent);
        if(!idx.isValid())
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
        QStringList recursiveList = extractStringsFromModel(model, idx, appendParent);
        if (!recursiveList.isEmpty())
            retVal << recursiveList;
    }

    return retVal;
}

// To use with a tree-style model.
QStringList ModelUtils::extractStringsFromCheckableProxyModel(CheckableProxyModel *model, const QModelIndex &proxyParent, bool appendParent, bool extractCheckedOnly)
{
    // It isn't suitable for directory trees (QFileSystemModel), since on Windows the drive letter is followed by other text, and because also
    //  the directories are checked and included in the returned list.
    QStringList retVal;

    for (int i = 0; i < model->rowCount(proxyParent); ++i)
    {
        QModelIndex idx = model->index(i, 0, proxyParent);
        if(!idx.isValid())
            continue;

        bool checked = (idx.data(Qt::CheckStateRole)==Qt::Checked);
        if ( (extractCheckedOnly && checked) || !extractCheckedOnly )
        {
            QString str(idx.data(Qt::DisplayRole).toString());
            if (appendParent)
            {
                QModelIndex parentOfParent = proxyParent;
                while (parentOfParent.isValid())
                {
                    str.prepend(parentOfParent.data(Qt::DisplayRole).toString() + '/');
                    parentOfParent = parentOfParent.parent();
                }
            }
            retVal << str;
        }

        // Check if this node has children elements, if affirmative, append them to the list.
        QStringList recursiveList = extractStringsFromCheckableProxyModel(model, idx, appendParent, extractCheckedOnly);
        if (!recursiveList.isEmpty())
            retVal << recursiveList;
    }

    return retVal;
}


QStringList ModelUtils::extractPathsFromCheckableProxyModelSourcedQDirModel(CheckableProxyModel *model, const QModelIndex &proxyParent, bool extractCheckedOnly)
{
    QStringList retVal;
    QDirModel *sourceModel = static_cast<QDirModel*>(model->sourceModel());   // if source model is wrong this will crash...

    for (int i = 0; i < model->rowCount(proxyParent); ++i)
    {
        const QModelIndex idx = model->index(i, 0, proxyParent);
        if(!idx.isValid())
            continue;
        const QModelIndex sourceIdx = model->mapToSource(idx);

        int role = idx.data(Qt::CheckStateRole).toInt();
        bool checked = (role==Qt::Checked) || (role==Qt::PartiallyChecked);
        if ( (extractCheckedOnly && checked) || !extractCheckedOnly )
        {
            if (!sourceModel->isDir(sourceIdx))
                retVal << sourceModel->filePath(sourceIdx);
            else
            {
                // Check if this node has children elements, if affirmative, append them to the list.
                QStringList recursiveList = extractPathsFromCheckableProxyModelSourcedQDirModel(model, idx, extractCheckedOnly);
                if (!recursiveList.isEmpty())
                    retVal << recursiveList;
            }
        }
    }

    return retVal;
}

void ModelUtils::resetCheckedStateCheckableProxyModel(CheckableProxyModel *model, bool value, const QModelIndex &proxyParent)
{
    for (int i = 0; i < model->rowCount(proxyParent); ++i)
    {
        const QModelIndex idx = model->index(i, 0, proxyParent);
        if(!idx.isValid())
            continue;

        model->setData(idx, value ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);

        const QModelIndex childIdx = idx.child(0,0);
        if (childIdx.isValid())
            resetCheckedStateCheckableProxyModel(model, value, childIdx);
    }
}

