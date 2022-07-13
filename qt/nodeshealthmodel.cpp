//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#include "nodeshealthmodel.h"
#include "cyphalpp.hpp"
#include "values_enum.hpp"
#include <uavcan/node/Heartbeat_1_0.hpp>
#include <uavcan/node/GetInfo_1_0.hpp>
#include <QTimer>
#include <optional>
#include <unordered_map>
#include <QTimer>


namespace cyphalpp {
namespace qt{
namespace utils {

using namespace uavcan::node;

using GetInfo = GetInfo::Service_1_0;

static const  EnumValue<Health_1_0> healthValues{
    {Health_1_0::NOMINAL, "NOMINAL", "The component is functioning properly (nominal)."},
    {Health_1_0::ADVISORY, "ADVISORY", "A critical parameter went out of range or the component encountered a minor failure that does not prevent the subsystem from performing any of its real-time functions."},
    {Health_1_0::CAUTION, "CAUTION", "The component encountered a major failure and is performing in a degraded mode or outside of its designed limitations."},
    {Health_1_0::WARNING, "WARNING", "The component suffered a fatal malfunction and is unable to perform its intended function."},
};

static const EnumValue<Mode_1_0> modeValues{
    {Mode_1_0::OPERATIONAL, "OPERATIONAL", "Normal operating mode."},
    {Mode_1_0::INITIALIZATION, "INITIALIZATION", "Initialization is in progress; this mode is entered immediately after startup."},
    {Mode_1_0::MAINTENANCE, "MAINTENANCE", "E.g., calibration, self-test, etc."},
    {Mode_1_0::SOFTWARE_UPDATE, "SOFTWARE_UPDATE", "New software/firmware is being loaded or the bootloader is running."},
};

static QString to_string(const Version_1_0& v){ return QStringLiteral("%1.%2").arg(v.major).arg(v.minor); }

struct NodeData{
    std::optional<uavcan::node::Heartbeat_1_0> heartbeat{};
    GetInfo::Response info{};
    QPointer<QTimer> timeout{nullptr};
    bool dataFetching{false};
    bool dataFetched{false};
};

using NodeDataGetter = QVariant(*)(const NodeData& d);
struct NodeDataField{
    QString name;
    QString description;
    NodeDataGetter get;
    NodeDataGetter getEdit;
    NodeDataGetter getTooltip;
    enum class Field {
        Heartbeat, Info
    } field;
};
static const NodeDataField fields[] = {
    {
        "NAME",
        "Human-readable non-empty ASCII node name. An empty name is not permitted."
        "The name shall not be changed while the node is running."
        "Allowed characters are: a-z (lowercase ASCII letters) 0-9 (decimal digits) . (dot) - (dash) _ (underscore)."
        "Node name is a reversed Internet domain name (like Java packages), e.g. \"com.manufacturer.project.product\".",
        [](const NodeData& d)->QVariant{ return QString::fromUtf8(reinterpret_cast<const char*>(d.info.name.data()), d.info.name.size()); },
        [](const NodeData& d)->QVariant{ return QString::fromUtf8(reinterpret_cast<const char*>(d.info.name.data()), d.info.name.size()); },
        [](const NodeData&)->QVariant{ return QVariant{};},
        NodeDataField::Field::Info
    },
    {
        "uptime",
        "[second] The uptime seconds counter should never overflow. "
        "The counter will reach the upper limit in ~136 years,  upon which time it should stay at "
        "0xFFFFFFFF until the node is restarted. Other nodes may detect that a remote node has "
        "restarted when this value leaps backwards.",
        [](const NodeData& d)->QVariant{ if(d.heartbeat){ return QStringLiteral("%1 s").arg(d.heartbeat->uptime);} else return "OFFLINE"; },
        [](const NodeData& d)->QVariant{ if(d.heartbeat){ return d.heartbeat->uptime;} else return QVariant{}; },
        [](const NodeData&)->QVariant{ return fields[0].description; },
        NodeDataField::Field::Heartbeat
    },
    {
        "health",
        "The abstract health status of this node.",
        [](const NodeData& d)->QVariant{ if(d.heartbeat){return healthValues.getName(d.heartbeat->health);} else return "OFFLINE"; },
        [](const NodeData& d)->QVariant{ if(d.heartbeat){ return static_cast<uint>(d.heartbeat->health.value);} else return QVariant{}; },
        [](const NodeData& d)->QVariant{ if(d.heartbeat){return healthValues.getDescription(d.heartbeat->health);} else return "OFFLINE"; },
        NodeDataField::Field::Heartbeat
    },
    {
        "mode",
        "The abstract operating mode of the publishing node. This field indicates the general "
        "level of readiness that can be further elaborated on a per-activity "
        "basis using various specialized interfaces.",
        [](const NodeData& d)->QVariant{ if(d.heartbeat){return modeValues.getName(d.heartbeat->mode);} else return "OFFLINE"; },
        [](const NodeData& d)->QVariant{ if(d.heartbeat){ return static_cast<uint>(d.heartbeat->mode.value);} else return QVariant{}; },
        [](const NodeData& d)->QVariant{ if(d.heartbeat){return modeValues.getDescription(d.heartbeat->mode);} else return "OFFLINE"; },
        NodeDataField::Field::Heartbeat
    },
    {
        "VSSC",
        "Optional, vendor-specific node status code, e.g. a fault code or a status bitmask.",
        [](const NodeData& d)->QVariant{ if(d.heartbeat){return int(d.heartbeat->vendor_specific_status_code);} else return "OFFLINE"; },
        [](const NodeData& d)->QVariant{ if(d.heartbeat){ return static_cast<uint>(d.heartbeat->vendor_specific_status_code);} else return QVariant{}; },
        [](const NodeData&)->QVariant{ return fields[3].description; },
        NodeDataField::Field::Heartbeat
    },
    {
        "PROT",
        "The UAVCAN protocol version implemented on this node, both major and minor. "
        "Not to be changed while the node is running.",
        [](const NodeData& d)->QVariant{ return to_string(d.info.protocol_version); },
        [](const NodeData& d)->QVariant{ return static_cast<uint>(d.info.protocol_version.major << 8 | d.info.protocol_version.minor); },
        [](const NodeData&)->QVariant{ return QVariant{};},
        NodeDataField::Field::Info
    },
    {
        "HW",
        "",
        [](const NodeData& d)->QVariant{ return to_string(d.info.hardware_version); },
        [](const NodeData& d)->QVariant{ return static_cast<uint>(d.info.hardware_version.major << 8 | d.info.hardware_version.minor); },
        [](const NodeData&)->QVariant{ return QVariant{};},
        NodeDataField::Field::Info
    },
    {
        "SW",
        "The version information shall not be changed while the node is running. "
        "The correct hardware version shall be reported at all times, excepting software-only nodes, "
        "in which case it should be set to zeros. If the node is equipped with a UAVCAN-capable "
        "bootloader, the bootloader should report the software version of the installed application, "
        "if there is any; if no application is found, zeros should be reported.",
        [](const NodeData& d)->QVariant{ return to_string(d.info.software_version); },
        [](const NodeData& d)->QVariant{ return static_cast<uint>(d.info.software_version.major << 8 | d.info.software_version.minor); },
        [](const NodeData&)->QVariant{ return QVariant{};},
        NodeDataField::Field::Info
    },
    {
        "SW_VCS",
        "A version control system (VCS) revision number or hash. "
        "Not to be changed while the node is running. For example, this field can be used for reporting "
        "the short git commit hash of the current software revision. Set to zero if not used.",
        [](const NodeData& d)->QVariant{ return QString::number(d.info.software_vcs_revision_id, 16); },
        [](const NodeData& d)->QVariant{ return QVariant::fromValue(d.info.software_vcs_revision_id); },
        [](const NodeData&)->QVariant{ return QVariant{};},
        NodeDataField::Field::Info
    },
    {
        "unique_id",
        "The unique-ID (UID) is a 128-bit long sequence that is likely to be globally unique per node. "
        "The vendor shall ensure that the probability of a collision with any other node UID globally is negligibly low. "
        "UID is defined once per hardware unit and should never be changed. "
        "All zeros is not a valid UID. If the node is equipped with a UAVCAN-capable bootloader, "
        "the bootloader shall use the same UID.",
        [](const NodeData& d)->QVariant{
            QStringList ret;
            for(auto& c: d.info.unique_id){
                ret.append(QString::number(c, 16));
            }
            return ret.join("");
        },
        nullptr,
        [](const NodeData&)->QVariant{ return QVariant{};},
        NodeDataField::Field::Info
    },
    {
        "CRC",
        "The value of an arbitrary hash function applied to the software image. Not to be changed while the node is"
        "running."
        "This field can be used to detect whether the software or firmware running on the node is an exact"
        "same version as a certain specific revision. This field provides a very strong identity guarantee,"
        "unlike the version fields above, which can be the same for different builds of the software."
        "As can be seen from its definition, this field is optional."
        "The exact hash function and the methods of its application are implementation-defined."
        "However, implementations are recommended to adhere to the following guidelines, fully or partially:"
        "  - The hash function should be CRC-64-WE."
        "  - The hash function should be applied to the entire application image padded to 8 bytes."
        "  - If the computed image CRC is stored within the software image itself, the value of"
        "    the hash function becomes ill-defined, because it becomes recursively dependent on itself."
        "    In order to circumvent this issue, while computing or checking the CRC, its value stored"
        "    within the image should be zeroed out.",
        [](const NodeData& d)->QVariant{
            if(d.info.software_image_crc.size() > 0){
                return QString::number(d.info.software_image_crc[0], 16);
            }
            return QVariant{};
        },
        nullptr,
        [](const NodeData&)->QVariant{ return QVariant{};},
        NodeDataField::Field::Info
    },
    {
        "COA",
        "The certificate of authenticity (COA) of the node, 222 bytes max, optional. This field can be used for"
        "reporting digital signatures (e.g., RSA-1776, or ECDSA if a higher degree of cryptographic strength is desired)."
        "Leave empty if not used. Not to be changed while the node is running.",
        [](const NodeData& d)->QVariant{
            QStringList ret;
            for(auto& c: d.info.certificate_of_authenticity){
                ret.append(QString::number(c, 16));
            }
            return ret.join("");
        },
        nullptr,
        [](const NodeData&)->QVariant{ return QVariant{};},
        NodeDataField::Field::Info
    }
};
constexpr static size_t colcount = (std::end(fields)- std::begin(fields)) + 1;

class NodesHealthModelPrivate{
public:
    std::vector<uint16_t> nodes{};
    std::unordered_map<uint16_t, NodeData> nodeData{};
};

NodesHealthModel::NodesHealthModel(CyphalUdp &uc, QObject *parent)
    : QAbstractTableModel{parent}, d_ptr(new NodesHealthModelPrivate{})
{
    auto updateRow =[this](int row, NodeDataField::Field f){
        size_t c = 1;
        for(const auto& d:fields){
            if(d.field == f){
                auto idx = createIndex(row, c);
                dataChanged(idx, idx);
            }
            ++c;
        }
    };
    auto getData = uc.prepareServiceCalls<GetInfo>(
        [this, updateRow](const TransferMetadata& tm, const GetInfo::Response& rsp){
            Q_D(NodesHealthModel);
            auto node = tm.data.node_id;
            auto nodePos = std::find(d->nodes.begin(), d->nodes.end(), node);
            if(d->nodes.end() == nodePos) return;
            auto dataPos = d->nodeData.find(node);
            if(d->nodeData.end() == dataPos) return;
            auto& nodeInfo = d->nodeData.at(node);
            nodeInfo.dataFetching = false;
            nodeInfo.dataFetched = true;
            nodeInfo.info = rsp;
            updateRow(nodePos - d->nodes.begin(), NodeDataField::Field::Info);
        },[]{});
    uc.subscribeMessage<Heartbeat_1_0>([this, getData, updateRow](const TransferMetadata& tm, const Heartbeat_1_0& hb){
        Q_D(NodesHealthModel);
        auto node = tm.data.node_id;
        auto nodePos = std::find(d->nodes.begin(), d->nodes.end(), node);
        if(d->nodes.end() == nodePos){
            beginInsertRows(QModelIndex{}, d->nodes.size(), d->nodes.size());
            d->nodes.emplace_back(node);
            nodePos = --d->nodes.end();
            d->nodeData.emplace(node, NodeData{hb, GetInfo::Response{}});
            endInsertRows();
        }else{
            auto dataPos = d->nodeData.find(node);
            if(d->nodeData.end() == dataPos) return;
            d->nodeData.at(node).heartbeat = hb;
        }
        updateRow(nodePos - d->nodes.begin(), NodeDataField::Field::Heartbeat);
        auto& nd= d->nodeData.at(node);
        if(not nd.dataFetched){
            if(not nd.dataFetching){
                nd.dataFetching = true;
                (*getData)(node, GetInfo::Request{});
            }
        }
        if(not nd.timeout){
            nd.timeout = new QTimer(this);
            nd.timeout->setSingleShot(false);
            nd.timeout->setInterval(Heartbeat_1_0::OFFLINE_TIMEOUT * 1000);
            connect(nd.timeout, &QTimer::timeout, [this, node, updateRow](){
                Q_D(NodesHealthModel);
                auto nodePos = std::find(d->nodes.begin(), d->nodes.end(), node);
                if(d->nodes.end() == nodePos) return;
                auto dataPos = d->nodeData.find(node);
                if(d->nodeData.end() == dataPos) return;
                auto& nodeInfo = d->nodeData.at(node);
                nodeInfo.heartbeat = std::nullopt;
                nodeInfo.dataFetched = false;
                updateRow(nodePos - d->nodes.begin(), NodeDataField::Field::Heartbeat);
            });
            nd.timeout->start();
        }else{
            nd.timeout->stop();
            nd.timeout->start();
        }
    });
}

NodesHealthModel::~NodesHealthModel()
{

}

int NodesHealthModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const NodesHealthModel);
    if(parent.isValid()) return 0;
    return d->nodes.size();
}

int NodesHealthModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid()) return 0;
    return static_cast<int>(colcount);
}

QVariant NodesHealthModel::data(const QModelIndex &index, int role) const
{
    Q_D(const NodesHealthModel);
    const auto ri = index.row();
    if( ri < 0){ return QVariant{}; }
    const size_t r = ri;
    if(r > d->nodes.size()){ return QVariant{}; }
    const auto ci = index.column();
    if(ci < 0){ return QVariant{}; }
    const size_t c = ci;
    if(c > colcount){ return QVariant{}; }
    auto node_id = d->nodes[r];
    auto dp = d->nodeData.find(node_id);
    if(dp == d->nodeData.end()){ return QVariant{}; }
    if(role == Qt::DisplayRole){
        if(c == 0){
            return QString::number(node_id);
        }
        return fields[c - 1].get(dp->second);
    }
    if(role == Qt::EditRole){
        if(c == 0){
            return node_id;
        }
        auto getter = fields[c - 1].getEdit;
        if(getter == nullptr){
            getter = fields[c - 1].get;
        }
        return getter(dp->second);
    }
    if(role == Qt::ToolTipRole){
        if(c == 0){
            return QVariant{};
        }
        return fields[c - 1].getTooltip(dp->second);
    }
    return QVariant{};
}

QVariant NodesHealthModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical){
        return QAbstractTableModel::headerData(section, orientation, role);
    }else{
        const auto ci = section;
        if(ci < 0){ return QVariant{}; }
        const size_t c = ci;
        if(c > colcount){ return QVariant{}; }
        if(role==Qt::DisplayRole){
            if(c == 0){
                return "Node ID";
            }
            return fields[c - 1].name;
        }
        if(role==Qt::ToolTipRole){
            if(c >= 1) return fields[c - 1].name;
        }
    }
    return QVariant{};
}



} // namespace utils
} // namespace qt
} // namespace cyphalpp
