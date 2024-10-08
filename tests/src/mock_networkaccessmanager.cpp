/*
 * Strawberry Music Player
 * This file was part of Clementine.
 * Copyright 2010, David Sansome <me@davidsansome.com>
 *
 * Strawberry is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Strawberry is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Strawberry.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mock_networkaccessmanager.h"

#include <algorithm>

#include <QtGlobal>
#include <QMap>
#include <QByteArray>
#include <QString>
#include <QIODevice>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QtDebug>

using std::min;

using ::testing::MakeMatcher;
using ::testing::Matcher;
using ::testing::MatcherInterface;
using ::testing::MatchResultListener;
using ::testing::Return;

class RequestForUrlMatcher : public MatcherInterface<const QNetworkRequest&> {
 public:
  RequestForUrlMatcher(const QString& contains, const QMap<QString, QString> &expected_params)
      : contains_(contains), expected_params_(expected_params) {}

  ~RequestForUrlMatcher() override = default;

  virtual bool Matches(const QNetworkRequest& req) const {
    const QUrl& url = req.url();

    if (!url.toString().contains(contains_)) {
      return false;
    }

    QUrlQuery url_query(url);
    for (QMap<QString, QString>::const_iterator it = expected_params_.constBegin(); it != expected_params_.constEnd(); ++it) {
      if (!url_query.hasQueryItem(it.key()) || url_query.queryItemValue(it.key()) != it.value()) {
        return false;
      }
    }
    return true;
  }

  bool MatchAndExplain(const QNetworkRequest& req, MatchResultListener* listener) const override {
    *listener << "which is " << req.url().toString().toUtf8().constData();
    return Matches(req);
  }

  void DescribeTo(::std::ostream* os) const override {
    *os << "matches url";
  }

 private:
  QString contains_;
  QMap<QString, QString> expected_params_;

  Q_DISABLE_COPY(RequestForUrlMatcher)

};

inline Matcher<const QNetworkRequest&> RequestForUrl(const QString &contains, const QMap<QString, QString> &params) {
  return MakeMatcher(new RequestForUrlMatcher(contains, params));
}

MockNetworkReply* MockNetworkAccessManager::ExpectGet(const QString &contains, const QMap<QString, QString> &expected_params, int status, const QByteArray &data) {

  MockNetworkReply* reply = new MockNetworkReply(data);
  reply->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, status);

  EXPECT_CALL(*this, createRequest(GetOperation, RequestForUrl(contains, expected_params), nullptr)). WillOnce(Return(reply));

  return reply;

}

MockNetworkReply::MockNetworkReply(QObject *parent)
    : QNetworkReply(parent), data_(nullptr), pos_(0) {
}

MockNetworkReply::MockNetworkReply(const QByteArray &data, QObject *parent)
    : QNetworkReply(parent),
      data_(data),
      pos_(0) {
}

void MockNetworkReply::SetData(const QByteArray &data) {
  data_ = data;
  pos_ = 0;
}

qint64 MockNetworkReply::readData(char *data, qint64 size) {

  if (data_.size() == pos_) {
    return -1;
  }
  qint64 bytes_to_read = min(data_.size() - pos_, size);
  memcpy(data, data_.constData() + pos_, bytes_to_read);
  pos_ += bytes_to_read;
  return bytes_to_read;

}

qint64 MockNetworkReply::writeData(const char*, qint64) {

  ADD_FAILURE() << "Something tried to write to a QNetworkReply";
  return -1;

}

void MockNetworkReply::Done() {

  setOpenMode(QIODevice::ReadOnly);
  Q_EMIT finished();

}

void MockNetworkReply::setAttribute(QNetworkRequest::Attribute code, const QVariant &value) {
  QNetworkReply::setAttribute(code, value);
}
