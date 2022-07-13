//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#ifndef FILESERVER_HPP
#define FILESERVER_HPP

#include <QObject>
#include <QDir>

namespace cyphalpp {

class CyphalUdp;

namespace qt{
namespace utils {

class FileServerPrivate;

class FileServer : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(FileServer)
public:
    explicit FileServer(CyphalUdp& uc, QObject *parent = nullptr);
    ~FileServer() override;
    QDir folder() const;
signals:
private:
    QScopedPointer<FileServerPrivate> const d_ptr;
};

} // namespace utils
} // namespace qt
} // namespace cyphalpp
#endif // FILESERVER_HPP
