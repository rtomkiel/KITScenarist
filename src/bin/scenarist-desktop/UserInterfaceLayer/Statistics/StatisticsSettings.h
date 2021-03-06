#ifndef STATISTICSSETTINGS_H
#define STATISTICSSETTINGS_H

#include <BusinessLayer/Statistics/Reports/AbstractReport.h>

#include <QStackedWidget>

class QAbstractItemModel;
class QSortFilterProxyModel;

namespace Ui {
	class StatisticsSettings;
}


namespace UserInterface
{
	/**
	 * @brief Класс виджета с настройками отчётов/графиков
	 */
	class StatisticsSettings : public QStackedWidget
	{
		Q_OBJECT

	public:
		explicit StatisticsSettings(QWidget *parent = 0);
		~StatisticsSettings();

		/**
		 * @brief Задать персонажей
		 */
		void setCharacters(QAbstractItemModel* _characters);

		/**
		 * @brief Получить параметры отчётов
		 */
		const BusinessLogic::StatisticsParameters& settings() const;

	signals:
		/**
		 * @brief Изменились настройки
		 */
		void settingsChanged();

	private:
		/**
		 * @brief Настроить представление
		 */
		void initView();

		/**
		 * @brief Настроить соединения для формы
		 */
		void initConnections();

	private:
		Ui::StatisticsSettings* m_ui;

		/**
		 * @brief Отсортированная модель персонажей
		 */
		QSortFilterProxyModel* m_charactersModel;

		/**
		 * @brief Параметры отчёта
		 */
		mutable BusinessLogic::StatisticsParameters m_settings;
	};
}

#endif // STATISTICSSETTINGS_H
