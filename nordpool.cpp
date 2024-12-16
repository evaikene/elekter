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

#include <fmt/format.h>

namespace El {

// -----------------------------------------------------------------------------

NordPool::NordPool(App const &app, QObject *parent)
    : QObject(parent)
    , _app(app)
{}

NordPool::~NordPool() = default;

auto NordPool::get_prices(QString const &region, int start_h, int end_h) -> PriceBlocks
{
    constexpr char const *URL = "https://dashboard.elering.ee";

    auto const start = to_datetime(start_h);
    auto const end   = to_datetime(end_h);

    fmt::print("Küsin võrgust Nord Pool hindasid perioodile {} ... {}\n", start, end);

    // create the network access manager if needed
    if (_manager == nullptr) {
        _manager = new QNetworkAccessManager{this};
        connect(_manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *) { _done = true; });
    }

    // prepare the request
    auto const query = QStringLiteral(u"%1/api/nps/price?start=%2&end=%3")
                           .arg(URL,
                                start.toUTC().toString("yyyy-MM-ddThh\'\%3A\'mm\'\%3A\'ss.zzzZ"),
                                end.toUTC().toString("yyyy-MM-ddThh\'\%3A\'mm\'\%3A\'ss.zzzZ"));
    if (_app.args().verbose()) {
        fmt::print("GET {}\n", query);
    }

    QNetworkRequest rqst{};
    rqst.setUrl(QUrl{query});
    rqst.setRawHeader("accept", "*/*");

    _done       = false;
    auto *reply = _manager->get(rqst);
    if (reply == nullptr) {
        throw Exception{"võrgupäring ebaõnnestus"};
    }

    // wait for the results
    constexpr int MAX_TIME_MS = 5000;
    if (!App::wait_for(_done, MAX_TIME_MS)) {
        throw Exception{"võrgupäring aegus"};
    }

    // check for errors
    if (reply->error() != QNetworkReply::NoError) {
        throw Exception{fmt::format("võrgupäring ebaõnnestus: {}", reply->errorString())};
    }

    auto prices = Json::from_json(reply->readAll(), region);

    // delete the reply
    reply->deleteLater();

    return prices.prices();
}

} // namespace El
