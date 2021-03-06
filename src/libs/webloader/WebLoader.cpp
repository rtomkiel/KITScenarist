#include "WebLoader.h"

#include <QtNetwork/QNetworkCookieJar>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QHttpMultiPart>
#include <QtCore/QTimer>
#include <QEventLoop>

namespace {
	/**
	 * @brief Не все сайты передают суммарный размер загружаемой страницы,
	 *		  поэтому для отображения прогресса загрузки используется
	 *		  заранее заданное число (средний размер веб-страницы)
	 */
	const int POSSIBLE_RECIEVED_MAX_FILE_SIZE = 120000;

	/**
	 * @brief Максимальное время ожидания ответа от серева при синхронном запросе
	 */
	const int SYNC_LOAD_TIMEOUT = 60000;

	/**
	 * @brief Преобразовать ошибку в читаемый вид
	 */
	static QString networkErrorToString(QNetworkReply::NetworkError networkError) {
		QString result;
		switch (networkError) {
			case QNetworkReply::ConnectionRefusedError: result = "the remote server refused the connection (the server is not accepting requests)"; break;
			case QNetworkReply::RemoteHostClosedError: result = "the remote server closed the connection prematurely, before the entire reply was received and processed"; break;
			case QNetworkReply::HostNotFoundError: result = "the remote host name was not found (invalid hostname)"; break;
			case QNetworkReply::TimeoutError: result = "the connection to the remote server timed out"; break;
			case QNetworkReply::OperationCanceledError: result = "the operation was canceled via calls to abort() or close() before it was finished."; break;
			case QNetworkReply::SslHandshakeFailedError: result = "the SSL/TLS handshake failed and the encrypted channel could not be established. The sslErrors() signal should have been emitted."; break;
			case QNetworkReply::TemporaryNetworkFailureError: result = "the connection was broken due to disconnection from the network, however the system has initiated roaming to another access point. The request should be resubmitted and will be processed as soon as the connection is re-established."; break;
			case QNetworkReply::NetworkSessionFailedError: result = "the connection was broken due to disconnection from the network or failure to start the network."; break;
			case QNetworkReply::BackgroundRequestNotAllowedError: result = "the background request is not currently allowed due to platform policy."; break;
			case QNetworkReply::ProxyConnectionRefusedError: result = "the connection to the proxy server was refused (the proxy server is not accepting requests)"; break;
			case QNetworkReply::ProxyConnectionClosedError: result = "the proxy server closed the connection prematurely, before the entire reply was received and processed"; break;
			case QNetworkReply::ProxyNotFoundError: result = "the proxy host name was not found (invalid proxy hostname)"; break;
			case QNetworkReply::ProxyTimeoutError: result = "the connection to the proxy timed out or the proxy did not reply in time to the request sent"; break;
			case QNetworkReply::ProxyAuthenticationRequiredError: result = "the proxy requires authentication in order to honour the request but did not accept any credentials offered (if any)"; break;
			case QNetworkReply::ContentAccessDenied: result = "the access to the remote content was denied (similar to HTTP error 401)"; break;
			case QNetworkReply::ContentOperationNotPermittedError: result = "the operation requested on the remote content is not permitted"; break;
			case QNetworkReply::ContentNotFoundError: result = "the remote content was not found at the server (similar to HTTP error 404)"; break;
			case QNetworkReply::AuthenticationRequiredError: result = "the remote server requires authentication to serve the content but the credentials provided were not accepted (if any)"; break;
			case QNetworkReply::ContentReSendError: result = "the request needed to be sent again, but this failed for example because the upload data could not be read a second time."; break;
			case QNetworkReply::ContentConflictError: result = "the request could not be completed due to a conflict with the current state of the resource."; break;
			case QNetworkReply::ContentGoneError: result = "the requested resource is no longer available at the server."; break;
			case QNetworkReply::InternalServerError: result = "the server encountered an unexpected condition which prevented it from fulfilling the request."; break;
			case QNetworkReply::OperationNotImplementedError: result = "the server does not support the functionality required to fulfill the request."; break;
			case QNetworkReply::ServiceUnavailableError: result = "the server is unable to handle the request at this time."; break;
			case QNetworkReply::ProtocolUnknownError: result = "the Network Access API cannot honor the request because the protocol is not known"; break;
			case QNetworkReply::ProtocolInvalidOperationError: result = "the requested operation is invalid for this protocol"; break;
			case QNetworkReply::UnknownNetworkError: result = "an unknown network-related error was detected"; break;
			case QNetworkReply::UnknownProxyError: result = "an unknown proxy-related error was detected"; break;
			case QNetworkReply::UnknownContentError: result = "an unknown error related to the remote content was detected"; break;
			case QNetworkReply::ProtocolFailure: result = "a breakdown in protocol was detected (parsing error, invalid or unexpected responses, etc.)"; break;
			case QNetworkReply::UnknownServerError: result = "an unknown error related to the server response was detected"; break;
			case QNetworkReply::NoError: result = "No error"; break;
		}

		return result;
	}
}


WebLoader::WebLoader(QObject* _parent, QNetworkCookieJar* _jar) :
	QThread(_parent),
	m_networkManager(0),
	m_cookieJar(_jar),
	m_request(new WebRequest),
	m_requestMethod(Undefined),
	m_isNeedRedirect(true)
{
}

WebLoader::~WebLoader()
{
	if ( m_request )
		delete m_request;
	if ( m_networkManager )
		m_networkManager->deleteLater();//delete m_networkManager;//
}

void WebLoader::setCookieJar( QNetworkCookieJar *jar )
{
	if ( m_cookieJar != jar )
		m_cookieJar = jar;
}

void WebLoader::setRequestMethod( WebLoader::RequestMethod method )
{
	if ( m_requestMethod != method )
		m_requestMethod = method;
}

void WebLoader::clearRequestAttributes()
{
	m_request->clearAttributes();
}

void WebLoader::addRequestAttribute( QString name, QVariant value )
{
	m_request->addAttribute( name, value );
}

void WebLoader::addRequestAttributeFile( QString name, QString filePath )
{
	m_request->addAttributeFile( name, filePath );
}

void WebLoader::loadAsync( QUrl urlToLoad, QUrl referer )
{
	m_request->setUrlToLoad( urlToLoad );
	m_request->setUrlReferer  ( referer );

	start();
}

QByteArray WebLoader::loadSync(QUrl urlToLoad, QUrl referer)
{
	m_request->setUrlToLoad(urlToLoad);
	m_request->setUrlReferer(referer);

	QEventLoop loop;
	connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
	QTimer::singleShot(SYNC_LOAD_TIMEOUT, &loop, SLOT(quit()));
	start();
	loop.exec();

	return m_downloadedData;
}

QUrl WebLoader::url() const
{
	return m_request->urlToLoad();
}


//*****************************************************************************
// Внутренняя реализация класса

void WebLoader::run()
{
	initNetworkManager();

	do
	{
		//! Начало загрузки страницы m_request->url()
		emit uploadProgress(0);
		emit downloadProgress(0);

		QNetworkReply *reply = 0;

		switch (m_requestMethod) {

			default:
			case WebLoader::Get: {
				const QNetworkRequest request = this->m_request->networkRequest();
				reply = m_networkManager->get(request);
				break;
			}

			case WebLoader::Post: {
				const QNetworkRequest networkRequest = m_request->networkRequest(true);
				const QByteArray data = m_request->multiPartData();
				reply = m_networkManager->post(networkRequest, data);
				break;
			}

		} // switch

		connect(reply, SIGNAL(uploadProgress(qint64,qint64)),
				this, SLOT(uploadProgress(qint64,qint64)));
		connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
				this, SLOT(downloadProgress(qint64,qint64)));
		connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
				this, SLOT(downloadError(QNetworkReply::NetworkError)));
		connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
				this, SLOT(downloadSslErrors(QList<QSslError>)));
		connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
				reply, SLOT(ignoreSslErrors()));

		//
		// Таймер для прерыванию работы через TIMEOUT_MS секунд
		//
		const int TIMEOUT_MS = 20000;
		QTimer timeoutTimer;
		connect(&timeoutTimer, SIGNAL(timeout()), this, SLOT(quit()));
		timeoutTimer.setSingleShot(true);
		timeoutTimer.start(TIMEOUT_MS);

		//
		// Входим в поток обработки событий, ожидая завершения отработки networkManager'а
		//
		exec();

		//
		// Если ответ получен, останавливаем таймер
		//
		if (timeoutTimer.isActive()) {
			timeoutTimer.stop();
		}
		//
		// ... а если загрузка прервалась по таймеру, освобождаем ресурсы и закрываем соединение
		//
		else {
			reply->abort();
		}

	} while (m_isNeedRedirect);

	emit downloadComplete(m_downloadedData);
	emit downloadComplete(QString(m_downloadedData));
}

void WebLoader::uploadProgress( qint64 uploadedBytes, qint64 totalBytes )
{
	//! отправлено [uploaded] байт из [total]
	if (totalBytes > 0)
		emit uploadProgress( ((float)uploadedBytes / totalBytes) * 100 );
}

void WebLoader::downloadProgress( qint64 recievedBytes, qint64 totalBytes )
{
	//! загружено [recieved] байт из [total]
	// не все сайты передают суммарный размер загружаемой страницы,
	// поэтому для отображения прогресса загрузки используется
	// заранее заданное число (средний размер веб-страницы)
	if (totalBytes < 0)
		totalBytes = POSSIBLE_RECIEVED_MAX_FILE_SIZE;
	emit downloadProgress( ((float)recievedBytes / totalBytes) * 100 );
}

void WebLoader::downloadComplete( QNetworkReply * reply )
{
	//! Завершена загрузка страницы [m_request->url()]

	// требуется ли редирект?
	if ( !reply->header( QNetworkRequest::LocationHeader ).isNull() ) {
		//! Осуществляется редирект по ссылке [redirectUrl]
		// Referer'ом становится ссылка по хоторой был осуществлен запрос
		QUrl refererUrl = m_request->urlToLoad();
		m_request->setUrlReferer( refererUrl );
		// Получаем ссылку для загрузки из заголовка ответа [Loacation]
		QUrl redirectUrl = reply->header( QNetworkRequest::LocationHeader ).toUrl();
		m_request->setUrlToLoad( redirectUrl );
		setRequestMethod( WebLoader::Get ); // Редирект всегда методом Get
		m_isNeedRedirect = true;
	} else {
		//! Загружены данные [reply->bytesAvailable()]
		qint64 downloadedDataSize = reply->bytesAvailable();
		QByteArray downloadedData = reply->read( downloadedDataSize );
		m_downloadedData = downloadedData;
		reply->deleteLater();
		m_isNeedRedirect = false;
	}

	if (!isRunning()) {
		wait(100);
	}
	quit(); // прерываем цикл обработки событий потока ( возвращаемся в run() )
}

void WebLoader::downloadError( QNetworkReply::NetworkError networkError )
{
	switch ( networkError ) {

		case QNetworkReply::NoError:
			m_lastError.clear();
			m_lastErrorDetails.clear();
			break;
		default:
			m_lastError =
					tr("Sorry, we have some error while loading. Error is: %1")
					.arg(::networkErrorToString(networkError));
			emit error(lastError());
			break;

	}
}

void WebLoader::downloadSslErrors(const QList<QSslError>& _errors)
{
	QString fullError;
	foreach (const QSslError& error, _errors) {
		if (!fullError.isEmpty()) {
			fullError.append("\n");
		}
		fullError.append(error.errorString());
	}

	m_lastErrorDetails = fullError;
}


//*****************************************************************************
// Методы доступа к данным класса, а так же вспомогательные
// методы для работы с данными класса

void WebLoader::initNetworkManager()
{
	//
	// Создаём загрузчика, если нужно
	//
	if (m_networkManager == 0) {
		m_networkManager = new QNetworkAccessManager;
	}

	//
	// Настраиваем куки
	//
	if (m_cookieJar != 0) {
		m_networkManager->setCookieJar(m_cookieJar);
		m_cookieJar->setParent(0);
	}

	//
	// Оключаем от предыдущих соединений
	//
	m_networkManager->disconnect();
	//
	// Настраиваем новое соединение
	//
	connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(downloadComplete(QNetworkReply*)));
}

QString WebLoader::lastError() const
{
	return m_lastError;
}

QString WebLoader::lastErrorDetails() const
{
	return m_lastErrorDetails;
}
