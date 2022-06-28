//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#ifndef QT_DSDL_SUPPORT_HPP
#define QT_DSDL_SUPPORT_HPP
#include <QtDebug>
#include <array>
#include <vector>

template<typename T, size_t N>
QDebug operator <<(QDebug stream, const std::array<T,N>& arr){
    stream << "[";
    for(const auto& element: arr){
        stream << element;
    }
    stream << "]";
    return stream;
}

#endif // QT_DSDL_SUPPORT_HPP
