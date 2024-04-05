#pragma once

#ifndef EL_JSON_H
#  define EL_JSON_H

#include "common.h"

#include <QByteArray>
#include <QString>
#include <QStringLiteral>
#include <QVector>

namespace El {

/// Helper class for parsing JSON documents with Nord Pool price records
class Json {
public:

    /// Parses the JSON document and returns a JSON class instance with prices
    /// @param[in] json JSON document
    /// @param[in] region Optional region (defaults to "ee")
    /// @return Json class instance with prices
    /// @throws Exception on errors
    static Json from_json(QByteArray const &json, QString const &region = QStringLiteral(u"ee"));

    /// Dtor
    ~Json();

    /// Returns price blocks
    inline auto const &prices() const noexcept { return _prices; }

private:

    /// price blocks
    PriceBlocks _prices;

    /// Private ctor
    Json();

    /// Parses the JSON document
    /// @param[in] region region
    /// @param[in] json JSON document
    /// @throws Exception on errors
    void parse(QString const &region, QByteArray const &json);

};

} // namespace El

#endif
