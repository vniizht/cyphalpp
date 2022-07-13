//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#ifndef NODEREGISTERS_H
#define NODEREGISTERS_H

#include <QAbstractTableModel>

namespace uavcan {
namespace _register {
class Value_1_0;
} // namespace _register
} // namespace uavcan


namespace cyphalpp {
class CyphalUdp;

namespace qt{
namespace utils {

QString toString(const uavcan::_register::Value_1_0& v);
QVariant toVariant(const uavcan::_register::Value_1_0& v);
bool fromVariant(uavcan::_register::Value_1_0& v, const QVariant& newVal);
int toMetaType(const uavcan::_register::Value_1_0 &v);

class NodeRegistersPrivate;
class NodeRegisters : public QAbstractTableModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(NodeRegisters)
public:
    explicit NodeRegisters(CyphalUdp& uc, uint16_t node_id, QObject *parent = nullptr);
    virtual ~NodeRegisters() override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant& v, int role = Qt::DisplayRole) override;
private:
    QScopedPointer<NodeRegistersPrivate> const d_ptr;
};

} // namespace utils
} // namespace qt
} // namespace cyphalpp
#endif // NODEREGISTERS_H
