//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#include "noderegisters.h"
#include <queue>
#include <QTimer>
#include "cyphalpp.hpp"
#include <uavcan/_register/Access_1_0.qt.hpp>
#include <uavcan/_register/List_1_0.qt.hpp>
#include <uavcan/_register/Value_1_0.qt.hpp>

Q_DECLARE_METATYPE(uavcan::_register::Value_1_0)

namespace cyphalpp {
namespace qt {

using List = uavcan::_register::List::Service_1_0;
using Access = uavcan::_register::Access::Service_1_0;
using Name = uavcan::_register::Name_1_0;
using Value = uavcan::_register::Value_1_0;

struct RegisterPrivate{
    Name name;
    Access::Response value{};
    RegisterPrivate(Name n):name(n){}
};

class NodeRegistersPrivate{
    Q_DECLARE_PUBLIC(NodeRegisters)
    NodeRegisters * const q_ptr;
    CyphalUdp &uc;
    uint16_t node_id;
    std::shared_ptr<CyphalUdp::ServiceCaller<List > > list{nullptr};
    std::shared_ptr<CyphalUdp::ServiceCaller<Access> > access{nullptr};
    std::vector<RegisterPrivate> values{};
    struct AccessRq{
        int index;
        Access::Request rq;
        AccessRq(int i, Access::Request r):index(i), rq(r){}
    };
    std::queue<AccessRq> accessQueue{};
    bool accessInProgress;
    NodeRegistersPrivate(NodeRegisters * const q, CyphalUdp &uc_, uint16_t node_id_)
        :q_ptr(q), uc(uc_), node_id(node_id_){
        auto timer = new QTimer(q);
        timer->setInterval(1000);
        timer->setSingleShot(false);
        QObject::connect(timer, &QTimer::timeout, [this](){
            restartAccesses();
        });
        list = uc.prepareServiceCalls<List>([this](const TransferMetadata&, const List::Response& rsp){
            if(rsp.name.name.empty()){
                restartAccesses();
                return;
            }
            Q_Q(NodeRegisters);
            auto newIndex = values.size();
            q->beginInsertRows(QModelIndex{}, newIndex, newIndex);
            values.emplace_back(rsp.name);
            q->endInsertRows();
            {
                Access::Request rq{};
                rq.name = rsp.name;
                rq.value.set_empty();
                accessQueue.emplace(newIndex, rq);
            }
            List::Request rq{};
            rq.index = newIndex+1;
            list->call(node_id, rq);
        },
        [this](){
            Q_Q(NodeRegisters);
            auto timer = new QTimer(q);
            timer->setSingleShot(true);
            QObject::connect(timer, &QTimer::timeout, [this, timer]{
                List::Request rq{};
                rq.index = values.size();
                list->call(node_id, rq);
                timer->deleteLater();
            });
            timer->start(100);
        });
        access = uc.prepareServiceCalls<Access>([this](const TransferMetadata&, const Access::Response& rsp){
            if(accessQueue.empty()) return;
            accessInProgress = false;
            auto i = accessQueue.front().index;
            auto& v = values.at(i);
            v.value = rsp;
            Q_Q(NodeRegisters);
            q->dataChanged(q->createIndex(i, 1), q->createIndex(i, 3));
            qDebug() << "REQ:" << accessQueue.front().rq;
            qDebug() << "RESP:"  << rsp;
            accessQueue.pop();
            restartAccesses();
        },[this](){
            accessInProgress = false;
            restartAccesses();
        });
        List::Request rq{};
        rq.index = 0;
        list->call(node_id, rq);
        timer->start();
    }
    void restartAccesses(){
        if(accessInProgress) return;
        if(accessQueue.empty()) return;
        accessInProgress = true;
        access->call(node_id, accessQueue.front().rq);
    }
};


NodeRegisters::NodeRegisters(CyphalUdp &uc,uint16_t node_id, QObject *parent)
    : QAbstractTableModel{parent}, d_ptr(new NodeRegistersPrivate{this, uc, node_id})
{
}

NodeRegisters::~NodeRegisters()
{
}

int NodeRegisters::rowCount(const QModelIndex &parent) const
{
    Q_D(const NodeRegisters);
    if(parent.isValid()) return 0;
    return d->values.size();
}

int NodeRegisters::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid()) return 0;
    return 4;
}

QVariant NodeRegisters::data(const QModelIndex &index, int role) const
{
    if(not index.isValid()) return QVariant{};
    auto c = index.column();
    if(c <0 or c >= 4) return QVariant{};
    Q_D(const NodeRegisters);
    if(index.row() <0) return QVariant{};
    size_t r = index.row();
    if(r >= d->values.size()) return QVariant{};
    auto& v = d->values[r];
    if(role == Qt::DisplayRole){
        switch(c){
        case 0: return QString::fromUtf8(reinterpret_cast<const char*>(v.name.name.data()), v.name.name.size());
        case 1: return toString(v.value.value);
        case 2: return v.value._mutable?"YES":"NO";
        case 3: return v.value.persistent?"YES":"NO";
        }
    }
    if(role == Qt::EditRole and c == 1){
        return toVariant(v.value.value);
    }
    if(role == Qt::UserRole and c == 1){
        return toMetaType(v.value.value);
    }
    return QVariant{};
}

Qt::ItemFlags NodeRegisters::flags(const QModelIndex &index) const
{
    auto ret = QAbstractTableModel::flags(index);
    if(index.column() == 1){
        ret.setFlag(Qt::ItemIsEditable);
    }
    return ret;
}

QVariant NodeRegisters::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical) return QVariant{};
    if(role == Qt::DisplayRole){
        if(section == 0){
            return "NAME";
        }
        if(section == 1){
            return "VALUE";
        }
        if(section == 2){
            return "MUT";
        }
        if(section == 3){
            return "PERS";
        }
    }
    return QVariant{};
}

bool NodeRegisters::setData(const QModelIndex &index, const QVariant &v, int role)
{
    if(role != Qt::EditRole) return false;
    if(not index.isValid()) return false;
    auto c = index.column();
    if(c != 1) return false;
    Q_D(NodeRegisters);
    if(index.row() <0) return false;
    size_t r = index.row();
    if(r >= d->values.size()) return false;
    auto& val = d->values.at(r);
    if(not val.value._mutable) return false;
    Value nv = val.value.value;
    if(not fromVariant(nv, v)) return false;
    d->accessQueue.emplace(r, Access::Request{val.name, nv});
    d->restartAccesses();
    return true;
}


template<typename T>
QString implToString(const T& val){
    QStringList r;
    for (auto i: val->value){
        r.append(QString::number(i));
    }
    return QStringLiteral("[ %1 ]").arg(r.join(", "));
}

QString toString(const uavcan::_register::Value_1_0 &v)
{
    if(v.is_empty()){return "<<EMPTY>>";}
    if(v.is_string()){
        auto val = v.get_string_if();
        return QString::fromUtf8(reinterpret_cast<const char*>(val->value.data()), val->value.size());
    }
    if(v.is_unstructured()){
        auto val = v.get_unstructured_if();
        QStringList ru;
        for (auto i: val->value){
            ru.append(QString::number(i, 16));
        }
        return ru.join(", ");
    }
    if(v.is_bit()){
        auto val = v.get_bit_if();
        QStringList rb;
        for (auto i: val->value){
            rb.append(i? "T": "F");
        }
        return rb.join(", ");
    }
    if(v.is_integer64()){ return implToString(v.get_integer64_if()); }
    if(v.is_integer32()){ return implToString(v.get_integer32_if()); }
    if(v.is_integer16()){ return implToString(v.get_integer16_if()); }
    if(v.is_integer8()){ return implToString(v.get_integer8_if()); }
    if(v.is_natural64()){ return implToString(v.get_natural64_if()); }
    if(v.is_natural32()){ return implToString(v.get_natural32_if()); }
    if(v.is_natural16()){ return implToString(v.get_natural16_if()); }
    if(v.is_natural8()){ return implToString(v.get_natural8_if()); }
    if(v.is_real64()){ return implToString(v.get_real64_if()); }
    if(v.is_real32()){ return implToString(v.get_real32_if()); }
    if(v.is_real16()){ return implToString(v.get_real16_if()); }
    return QString{};
}

template<typename T>
static int implMetaTypeId(){
    return qMetaTypeId<typename decltype(std::declval<T>()->value)::value_type>();
}

int toMetaType(const uavcan::_register::Value_1_0 &v)
{
    if(v.is_empty()){return QVariant::Type::Invalid;}
    if(v.is_string()){ return QVariant::String; }
    if(v.is_unstructured()){ return implMetaTypeId<decltype(v.get_unstructured_if())>();}
    if(v.is_bit()){return implMetaTypeId<decltype(v.get_bit_if())>();}
    if(v.is_integer64()){ return implMetaTypeId<decltype(v.get_integer64_if())>(); }
    if(v.is_integer32()){ return implMetaTypeId<decltype(v.get_integer32_if())>(); }
    if(v.is_integer16()){ return implMetaTypeId<decltype(v.get_integer16_if())>(); }
    if(v.is_integer8()){ return implMetaTypeId<decltype(v.get_integer8_if())>(); }
    if(v.is_natural64()){ return implMetaTypeId<decltype(v.get_natural64_if())>(); }
    if(v.is_natural32()){ return implMetaTypeId<decltype(v.get_natural32_if())>(); }
    if(v.is_natural16()){ return implMetaTypeId<decltype(v.get_natural16_if())>(); }
    if(v.is_natural8()){ return implMetaTypeId<decltype(v.get_natural8_if())>(); }
    if(v.is_real64()){ return implMetaTypeId<decltype(v.get_real64_if())>(); }
    if(v.is_real32()){ return implMetaTypeId<decltype(v.get_real32_if())>(); }
    if(v.is_real16()){ return implMetaTypeId<decltype(v.get_real16_if())>(); }
    return QVariant::Type::Invalid;
}


template<typename T>
static QVariant implToVariant(T* val){
    QVariantList r;
    for (typename decltype(T::value)::value_type i: val->value){
        r.append(QVariant::fromValue(i));
    }
    return r;
}

QVariant toVariant(const uavcan::_register::Value_1_0 &v)
{
    if(v.is_empty()){return QVariant{};}
    if(v.is_string()){
        auto val = v.get_string_if();
        return QString::fromUtf8(reinterpret_cast<const char*>(val->value.data()), val->value.size());
    }
    if(v.is_unstructured()){ return implToVariant(v.get_unstructured_if());}
    if(v.is_bit()){return implToVariant(v.get_bit_if());}
    if(v.is_integer64()){ return implToVariant(v.get_integer64_if()); }
    if(v.is_integer32()){ return implToVariant(v.get_integer32_if()); }
    if(v.is_integer16()){ return implToVariant(v.get_integer16_if()); }
    if(v.is_integer8()){ return implToVariant(v.get_integer8_if()); }
    if(v.is_natural64()){ return implToVariant(v.get_natural64_if()); }
    if(v.is_natural32()){ return implToVariant(v.get_natural32_if()); }
    if(v.is_natural16()){ return implToVariant(v.get_natural16_if()); }
    if(v.is_natural8()){ return implToVariant(v.get_natural8_if()); }
    if(v.is_real64()){ return implToVariant(v.get_real64_if()); }
    if(v.is_real32()){ return implToVariant(v.get_real32_if()); }
    if(v.is_real16()){ return implToVariant(v.get_real16_if()); }
    return QVariant{};
}


template<typename T>
bool implFromVariant(T val, const QVariant &newVal){
    using vt = typename  decltype(val->value)::value_type;
    if(newVal.type() != QVariant::List){ return false; }
    auto l = newVal.toList();
    val->value.resize(l.size());
    std::transform(l.begin(), l.end(), val->value.begin(), [](const QVariant& nv){
        auto mt = qMetaTypeId<vt>();
        if(nv.userType() == mt){
            QVariant nvc = nv;
            nvc.convert(mt);
            return nv.value<vt>();
        }
        return nv.value<vt>();
    });
    return true;
}

bool fromVariant(uavcan::_register::Value_1_0 &v, const QVariant &newVal)
{
    if(v.is_empty()){return false;}
    if(v.is_string()){
        auto val = v.get_string_if();
        if(newVal.type() != QVariant::String){
            return false;
        }
        auto nv = newVal.toString().toUtf8();
        val->value.resize(nv.size());
        std::copy(nv.begin(), nv.end(), val->value.begin());
        return true;
    }
    if(v.is_unstructured()){ return implFromVariant(v.get_unstructured_if(), newVal);}
    if(v.is_bit()){ return implFromVariant(v.get_bit_if(), newVal); }
    if(v.is_integer64()){ return implFromVariant(v.get_integer64_if(), newVal); }
    if(v.is_integer32()){ return implFromVariant(v.get_integer32_if(), newVal); }
    if(v.is_integer16()){ return implFromVariant(v.get_integer16_if(), newVal); }
    if(v.is_integer8()){ return implFromVariant(v.get_integer8_if(), newVal); }
    if(v.is_natural64()){ return implFromVariant(v.get_natural64_if(), newVal); }
    if(v.is_natural32()){ return implFromVariant(v.get_natural32_if(), newVal); }
    if(v.is_natural16()){ return implFromVariant(v.get_natural16_if(), newVal); }
    if(v.is_natural8()){ return implFromVariant(v.get_natural8_if(), newVal); }
    if(v.is_real64()){ return implFromVariant(v.get_real64_if(), newVal); }
    if(v.is_real32()){ return implFromVariant(v.get_real32_if(), newVal); }
    if(v.is_real16()){ return implFromVariant(v.get_real16_if(), newVal); }
    return false;
}

} // namespace qt
} // namespace cyphalpp
