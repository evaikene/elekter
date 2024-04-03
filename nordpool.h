#pragma once

#ifndef NORDPOOL_H
#  define NORDPOOL_H

#include "common.h"

#include <QObject>
#include <QVector>

QT_BEGIN_NAMESPACE
    class QDateTime;
    class QNetworkAccessManager;
    class QNetworkReply;
QT_END_NAMESPACE

namespace El {

class App;

/// Class that queries NordPool prices over the network
class NordPool : public QObject {
    Q_OBJECT

public:

    /// Ctor
    /// @param[in] app The application instance
    /// @param[in] parent Optional parent
    NordPool(App const &app, QObject *parent = nullptr);

    /// Dtor
    ~NordPool() override;

    /// Request NordPool prices
    /// @param[in] start Start time
    /// @param[in] end End time
    /// @return Price blocks with NordPool prices
    /// @throws El::Exception on errors
    auto get_prices(QDateTime const &start, QDateTime const &end) -> PriceBlocks const;

private:

    /// Application instance
    App const &_app;

    /// Network access manager
    QNetworkAccessManager *_manager = nullptr;

    /// Flag indicating that network request is finished
    bool _done = false;

};

} // namespace El

#endif
