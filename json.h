#pragma once

#ifndef EL_JSON_H_INCLUDED
#  define EL_JSON_H_INCLUDED

#include "common.h"

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QByteArray)
QT_FORWARD_DECLARE_CLASS(QDateTime)
QT_FORWARD_DECLARE_CLASS(QString)

namespace El {

/// Helper class for parsing JSON documents with Nord Pool price records
class Json {
public:

    /// Parses the JSON document and returns a JSON class instance with prices
    /// @param[in] json JSON document
    /// @param[in] region price region
    /// @param[in] end end time for the requested prices
    /// @return Json class instance with prices
    /// @throws Exception on errors
    static auto from_json(QByteArray const &json, QString const &region, QDateTime const &end) -> Json;

    /// Dtor
    ~Json() = default;

    /// Returns price blocks
    auto prices() const noexcept -> auto const & { return _prices; }

private:

    /// price blocks
    PriceBlocks _prices;

    /// Private ctor
    Json();

    /// Parses the JSON document
    /// @param[in] region region
    /// @param[in] json JSON document
    /// @param[in] end end time for the requested prices
    /// @throws Exception on errors
    void parse(QString const &region, QByteArray const &json, QDateTime const &end);

};

} // namespace El

#endif
