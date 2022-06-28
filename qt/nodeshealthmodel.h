//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#ifndef NODESHEALTHMODEL_H
#define NODESHEALTHMODEL_H

#include <QPointer>
#include <QModelIndex>
#include <QAbstractTableModel>

class QTimer;

namespace cyphalpp {
class CyphalUdp;

namespace qt {

class NodesHealthModelPrivate;
/**
 * @brief The NodesHealthModel class is a read-only model, populated from current network
 * with currently online nodes.
 */
class NodesHealthModel : public QAbstractTableModel
{
    Q_DECLARE_PRIVATE(NodesHealthModel);
public:
    explicit NodesHealthModel(CyphalUdp &uc, QObject *parent = nullptr);
    virtual ~NodesHealthModel() override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    QScopedPointer<NodesHealthModelPrivate> const d_ptr;
};


} // namespace qt
} // namespace cyphalpp
#endif // NODESHEALTHMODEL_H
