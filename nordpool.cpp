#include "nordpool.h"

#include "app.h"
#include "args.h"
#include "common.h"
#include "json.h"

#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringLiteral>
#include <QUrl>

#include <QtCore/qnamespace.h>
#include <fmt/format.h>

namespace El {

// -----------------------------------------------------------------------------

NordPool::NordPool(App const &app, QObject *parent)
    : QObject(parent)
    , _app(app)
{}

NordPool::~NordPool() = default;

auto NordPool::get_prices(QString const &region, int start_h, int end_h) -> PriceBlocks const
{
    constexpr char const *URL = "https://dashboard.elering.ee";

    auto const start = to_datetime(start_h);
    auto const end = to_datetime(end_h);

    fmt::print("Requesting Nord Pool prices from {} for time period {} ... {}\n", URL, start, end);

    // create the network access manager if needed
    if (!_manager) {
        _manager = new QNetworkAccessManager{this};
        connect(_manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *) {
            _done = true;
        });
    }

    // prepare the request
    auto const query = QString{"%1/api/nps/price?start=%2&end=%3"}.arg(
        URL,
        start.toUTC().toString("yyyy-MM-ddThh\'\%3A\'mm\'\%3A\'ss.zzzZ"),
        end.toUTC().toString("yyyy-MM-ddThh\'\%3A\'mm\'\%3A\'ss.zzzZ"));
    fmt::print("query: {}\n", query);

    QNetworkRequest rqst{};
    rqst.setUrl(QUrl{query});
    rqst.setRawHeader("accept", "*/*");

    _done       = false;
    auto *reply = _manager->get(rqst);
    if (!reply) {
        throw Exception{"Network request failed"};
    }

    // wait for the results
    if (!_app.wait_for(_done, 5000)) {
        throw Exception{"Timed out waiting for response"};
    }

    // check for errors
    if (reply->error() != QNetworkReply::NoError) {
        throw Exception{fmt::format("Network request failed: {}", reply->errorString())};
    }

    auto prices = Json::from_json(reply->readAll(), region);

    // delete the reply
    reply->deleteLater();

    return prices.prices();
}

} // namespace El
