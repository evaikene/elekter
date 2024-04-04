#pragma once

#ifndef EL_NORDPOOL_H
#  define EL_NORDPOOL_H

#include "common.h"

#include <QObject>
#include <QVector>

QT_BEGIN_NAMESPACE
    class QNetworkAccessManager;
    class QNetworkReply;
QT_END_NAMESPACE

namespace El {

class App;

/// Class that queries Nord Pool prices over the network
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
    /// @param[in] region Price region
    /// @param[in] start_h Start time (hours since the EPOCH)
    /// @param[in] end_h End time (hours since the EPOCH)
    /// @return Price blocks with NordPool prices
    /// @throws El::Exception on errors
    auto get_prices(QString const &region, int start_h, int end_h) -> PriceBlocks const;

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
