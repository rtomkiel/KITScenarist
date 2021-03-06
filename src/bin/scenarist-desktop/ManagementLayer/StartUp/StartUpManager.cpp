#include "StartUpManager.h"

#include <ManagementLayer/Project/ProjectsManager.h>

#include <UserInterfaceLayer/StartUp/StartUpView.h>
#include <UserInterfaceLayer/StartUp/LoginDialog.h>
#include <UserInterfaceLayer/StartUp/CrashReportDialog.h>

#include <DataLayer/DataStorageLayer/StorageFacade.h>
#include <DataLayer/DataStorageLayer/SettingsStorage.h>

#include <3rd_party/Helpers/PasswordStorage.h>

#include <WebLoader.h>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QMutableMapIterator>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStandardPaths>
#include <QTimer>

using ManagementLayer::StartUpManager;
using ManagementLayer::ProjectsManager;
using DataStorageLayer::StorageFacade;
using DataStorageLayer::SettingsStorage;
using UserInterface::StartUpView;
using UserInterface::LoginDialog;
using UserInterface::CrashReportDialog;


StartUpManager::StartUpManager(QObject *_parent, QWidget* _parentWidget) :
	QObject(_parent),
	m_view(new StartUpView(_parentWidget))
{
	initData();
	initConnections();

	//
	// Проверим наличие отчётов об ошибках в работе программы
	//
	QTimer::singleShot(0, this, &StartUpManager::checkCrashReports);
	//
	// Проверяем наличие новой версии уже после старта программы
	//
	QTimer::singleShot(0, this, &StartUpManager::checkNewVersion);
}

QWidget* StartUpManager::view() const
{
	return m_view;
}

void StartUpManager::aboutUserLogged()
{
	const bool isLogged = true;
	m_view->setUserLogged(isLogged, m_userName);
}

void StartUpManager::aboutRetryLogin(const QString& _error)
{
	//
	// Показать диалог авторизации с заданной ошибкой
	//
	LoginDialog loginDialog(m_view);
	loginDialog.setUserName(m_userName);
	loginDialog.setPassword(m_password);
	loginDialog.setError(_error);
	if (loginDialog.exec() == QLightBoxDialog::Accepted) {
		m_userName = loginDialog.userName();
		m_password = loginDialog.password();
		emit loginRequested(m_userName, m_password);
	}
}

void StartUpManager::aboutUserUnlogged()
{
	const bool isLogged = false;
	m_view->setUserLogged(isLogged);
}

void StartUpManager::setRecentProjects(QAbstractItemModel* _model)
{
	m_view->setRecentProjects(_model);
}

void StartUpManager::setRemoteProjects(QAbstractItemModel* _model)
{
	m_view->setRemoteProjects(_model);
}

void StartUpManager::aboutLoginClicked()
{
	//
	// Показать диалог авторизации
	//
	LoginDialog loginDialog(m_view);
	if (loginDialog.exec() == QLightBoxDialog::Accepted) {
		m_userName = loginDialog.userName();
		m_password = loginDialog.password();
		emit loginRequested(m_userName, m_password);
	}
}

void StartUpManager::aboutLoadUpdatesInfo(QNetworkReply* _reply)
{
	if (_reply != 0) {
		QString updatesPageData = _reply->readAll().simplified();

		//
		// Извлекаем все версии и формируем ссылку на последнюю из них
		//
#ifdef Q_OS_WIN
		QRegularExpression rx_updateFiner("scenarist-setup-(\\d+.\\d+.\\d+).exe");
#elif defined Q_OS_LINUX
		QRegularExpression rx_updateFiner("scenarist-setup-(\\d+.\\d+.\\d+)_i386.deb");
#elif defined Q_OS_MAC
		QRegularExpression rx_updateFiner("scenarist-setup-(\\d+.\\d+.\\d+).dmg");
#endif

		QRegularExpressionMatch match = rx_updateFiner.match(updatesPageData);
		QList<QString> versions;
		while (match.hasMatch()) {
			versions.append(match.captured(1));
			match = rx_updateFiner.match(updatesPageData, match.capturedEnd(1));
		}

		//
		// Если версии найдены
		//
		if (versions.count() > 0) {
			//
			// Сортируем
			//
			qSort(versions);

			//
			// Извлекаем последнюю версию
			//
			QString maxVersion = versions.last();

			//
			// Если она больше текущей версии программы, выводим информацию
			//
			if (QApplication::applicationVersion() < maxVersion) {
				QString localeSuffix;
				if (QLocale().language() == QLocale::English) {
					localeSuffix = "_en";
				} else if (QLocale().language() == QLocale::Spanish) {
					localeSuffix = "_es";
				} else if (QLocale().language() == QLocale::French) {
					localeSuffix = "_fr";
				}
				QString updateInfo =
						tr("Released version %1 ").arg(maxVersion)
#ifdef Q_OS_WIN
						+ "<a href=\"https://kitscenarist.ru/downloads/windows/scenarist-setup-" + maxVersion + ".exe\" "
#elif defined Q_OS_LINUX
#ifdef Q_PROCESSOR_X86_64
						+ "<a href=\"https://kitscenarist.ru/downloads/linux/scenarist-setup-" + maxVersion + localeSuffix + "_amd64.deb\" "
#else
						+ "<a href=\"https://kitscenarist.ru/downloads/linux/scenarist-setup-" + maxVersion + localeSuffix + "_i386.deb\" "
#endif
#elif defined Q_OS_MAC
						+ "<a href=\"https://kitscenarist.ru/downloads/mac/scenarist-setup-" + maxVersion + localeSuffix + ".dmg\" "
#endif
						+ "style=\"color:#2b78da;\">" + tr("download") + "</a> "
						+ tr("or") + " <a href=\"https://kitscenarist.ru/history.html\" "
						+ "style=\"color:#2b78da;\">" + tr("read more") + "</a>.";
				m_view->setUpdateInfo(updateInfo);
			}
		}
	}
}

void StartUpManager::initData()
{
	//
	// Загрузим имя пользователя и пароль из настроек
	//
	m_userName =
			StorageFacade::settingsStorage()->value(
				"application/user-name",
				SettingsStorage::ApplicationSettings);
	m_password =
			PasswordStorage::load(
				StorageFacade::settingsStorage()->value(
					"application/password",
					SettingsStorage::ApplicationSettings),
				m_userName
				);
}

void StartUpManager::initConnections()
{
	connect(m_view, SIGNAL(loginClicked()), this, SLOT(aboutLoginClicked()));
	connect(m_view, SIGNAL(logoutClicked()), this, SIGNAL(logoutRequested()));
	connect(m_view, SIGNAL(createProjectClicked()), this, SIGNAL(createProjectRequested()));
	connect(m_view, SIGNAL(openProjectClicked()), this, SIGNAL(openProjectRequested()));
	connect(m_view, SIGNAL(helpClicked()), this, SIGNAL(helpRequested()));

	connect(m_view, SIGNAL(openRecentProjectClicked(QModelIndex)),
			this, SIGNAL(openRecentProjectRequested(QModelIndex)));
	connect(m_view, SIGNAL(openRemoteProjectClicked(QModelIndex)),
			this, SIGNAL(openRemoteProjectRequested(QModelIndex)));
	connect(m_view, SIGNAL(refreshProjects()), this, SIGNAL(refreshProjectsRequested()));
}

void StartUpManager::checkCrashReports()
{
	const QString SENDED = "sended";
	const QString IGNORED = "ignored";

	//
	// Настроим отлавливание ошибок
	//
	QString appDataFolderPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	QString crashReportsFolderPath = appDataFolderPath + QDir::separator() + "CrashReports";
	bool hasUnhandledReports = false;
	QString unhandledReportPath;
	for (const QFileInfo& crashReportFileInfo : QDir(crashReportsFolderPath).entryInfoList(QDir::Files)) {
		if (crashReportFileInfo.suffix() != SENDED
			&& crashReportFileInfo.suffix() != IGNORED) {
			hasUnhandledReports = true;
			unhandledReportPath = crashReportFileInfo.filePath();
			break;
		}
	}

	//
	// Если есть необработанные отчёты, показать диалог
	//
	if (hasUnhandledReports) {
		CrashReportDialog dialog(m_view);
		QString handledReportPath = unhandledReportPath;
		if (dialog.exec() == CrashReportDialog::Accepted) {
			dialog.showProgress();
			//
			// Отправляем
			//
			WebLoader loader;
			loader.setRequestMethod(WebLoader::Post);
			loader.addRequestAttribute("email", dialog.email());
			loader.addRequestAttribute("message", dialog.message());
			loader.addRequestAttributeFile("report", unhandledReportPath);
			loader.loadSync(QUrl("https://kitscenarist.ru/api/app/feedback/"));

			//
			// Помечаем отчёт, как отправленный
			//
			handledReportPath += "." + SENDED;
		}
		//
		// Помечаем отчёт, как проигнорированный
		//
		else {
			handledReportPath += "." + IGNORED;
		}
		QFile::rename(unhandledReportPath, handledReportPath);
	}
}

void StartUpManager::checkNewVersion()
{
	QNetworkAccessManager* manager = new QNetworkAccessManager(this);
	connect(manager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(aboutLoadUpdatesInfo(QNetworkReply*)));

	//
	// Сформируем uuid для приложения, по которому будем идентифицировать данного пользователя
	//
	QString uuid
			= DataStorageLayer::StorageFacade::settingsStorage()->value(
				  "application/uuid", DataStorageLayer::SettingsStorage::ApplicationSettings);
	DataStorageLayer::StorageFacade::settingsStorage()->setValue(
		"application/uuid", uuid, DataStorageLayer::SettingsStorage::ApplicationSettings);

	//
	// Построим ссылку, чтобы учитывать запрос на проверку обновлений
	//
	QString url = QString("https://kitscenarist.ru/api/app/updates/");

	url.append("?system_type=");
	url.append(
#ifdef Q_OS_WIN
				"windows"
#elif defined Q_OS_LINUX
				"linux"
#elif defined Q_OS_MAC
				"mac"
#else
				QSysInfo::kernelType()
#endif
				);

	url.append("&system_name=");
	url.append(QSysInfo::prettyProductName().toUtf8().toPercentEncoding());

	url.append("&uuid=");
	url.append(uuid);

	url.append("&application_version=");
	url.append(QApplication::applicationVersion());

	QNetworkRequest request = QNetworkRequest(QUrl(url));
	manager->get(request);
}
