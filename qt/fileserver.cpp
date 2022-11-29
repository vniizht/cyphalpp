//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#include "fileserver.hpp"
#include <QStandardPaths>
#include <QtGlobal>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <array>
#include <uavcan/file/List_0_2.qt.hpp>
#include <uavcan/file/Read_1_1.qt.hpp>
#include <uavcan/file/GetInfo_0_2.qt.hpp>
#include <cyphalpp.hpp>

using namespace uavcan::file;

namespace cyphalpp {
namespace qt{
namespace utils {
class FileServerPrivate{
    Q_DECLARE_PUBLIC(FileServer)
    FileServer* const q_ptr;
    CyphalUdp& uc_;
    QDir folder=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    FileServerPrivate(FileServer* const q, CyphalUdp& uc ): q_ptr(q), uc_(uc){}

};

FileServer::FileServer(CyphalUdp &uc, QObject *parent)
    : QObject{parent}, d_ptr(new FileServerPrivate(this, uc))
{
    Q_D(FileServer);
    uc.subscribeServiceRequest<List::Service_0_2>([this](const TransferMetadata&, const List::Request_0_2& req) -> List::Response_0_2{
        Q_D(FileServer);
        List::Response_0_2 ret;
        QDir files(d->folder);
        {
            auto& pth = req.directory_path.path;
            if(not files.cd(QString::fromUtf8(reinterpret_cast<const char*>(pth.data()), pth.size()))){
                return ret;
            }
        }
        auto l = files.entryList(QDir::Filter::NoDotAndDotDot | QDir::Filter::AllEntries, QDir::SortFlag::Name);
        int i = req.entry_index;
        if(i < l.size()){
            auto r = l.at(i).toUtf8();
            ret.entry_base_name.path.resize(r.size());
            std::copy_n(r.data(), r.size(), ret.entry_base_name.path.begin());
        }
        return ret;
    });
    uc.subscribeServiceRequest<GetInfo::Service_0_2>([this](const TransferMetadata&, const GetInfo::Request_0_2& req) -> GetInfo::Response_0_2{
        GetInfo::Response_0_2 ret;
        Q_D(FileServer);
        QFileInfo f(d->folder.filePath(QString::fromUtf8(reinterpret_cast<const char*>(req.path.path.data()), req.path.path.size())));
        if(not f.exists()){
            ret._error.value = Error_1_0::NOT_FOUND;
            return ret;
        }
        ret._error.value = Error_1_0::OK;
        ret.size = f.size();
        ret.unix_timestamp_of_last_modification = f.lastModified().toSecsSinceEpoch();
        ret.is_file_not_directory = f.isFile();
        ret.is_link = f.isSymLink();
        ret.is_readable = f.isReadable();
        ret.is_writeable = f.isWritable();
        return ret;
    });
    uc.subscribeServiceRequest<Read::Service_1_1>([this](const TransferMetadata&, const Read::Request_1_1& req) -> Read::Response_1_1{
        Read::Response_1_1 ret;
        Q_D(FileServer);
        QFile f(d->folder.filePath(QString::fromUtf8(reinterpret_cast<const char*>(req.path.path.data()), req.path.path.size())));
        if(not f.exists()){
            ret._error.value = Error_1_0::NOT_FOUND;
            return ret;
        }
        if(not f.open(QFile::ReadOnly)){
            ret._error.value = Error_1_0::IO_ERROR;
            return ret;
        }
        if(not f.seek(req.offset)){
            ret._error.value = Error_1_0::INVALID_VALUE;
            return ret;
        }
        ret.data.value.resize(256);
        auto readLen = f.read(reinterpret_cast<char*>(ret.data.value.data()), 256);
        if(readLen >= 0){
            ret.data.value.resize(readLen);
            ret._error.value = Error_1_0::OK;
        }else{
            ret.data.value.resize(0);
            ret._error.value = Error_1_0::IO_ERROR;
        }
        return ret;
    });
    qWarning("folder=%s",qPrintable(d->folder.path()));

    QFile* file=new QFile(d->folder.filePath("test.txt"));
    if (!file->exists()) {
        // create the folder, if necessary
        QDir* dir=new QDir(d->folder);
        if (!dir->exists()) {
            qWarning("creating new folder");
            dir->mkpath(".");
        }
        qWarning("creating new file");
        file->open(QIODevice::WriteOnly);
        file->write("Hello World");
        file->close();
    }
    if (file->exists()) {
        qWarning("file exists");
        file->open(QIODevice::ReadOnly);
        QByteArray data=file->readAll();
        qWarning("file data=%s",data.constData());
        file->close();

    }
}

FileServer::~FileServer()
{
}

QDir FileServer::folder() const
{
    Q_D(const FileServer);
    return d->folder.path();
}

} // namespace utils
} // namespace qt
} // namespace cyphalpp

