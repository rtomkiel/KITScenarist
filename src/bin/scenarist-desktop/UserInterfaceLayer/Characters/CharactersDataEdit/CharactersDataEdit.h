#ifndef CHARACTERSDATAEDIT_H
#define CHARACTERSDATAEDIT_H

#include <QWidget>

namespace Ui {
	class CharactersDataEdit;
}


namespace UserInterface
{
	/**
	 * @brief Редактор данных персонажа
	 */
	class CharactersDataEdit : public QWidget
	{
		Q_OBJECT

	public:
		explicit CharactersDataEdit(QWidget *parent = 0);
		~CharactersDataEdit();

		/**
		 * @brief Очистить все поля
		 */
		void clean();

		/**
		 * @brief Установить имя персонажа
		 */
		void setName(const QString& _name);

		/**
		 * @brief Получить имя персонажа
		 */
		QString name() const;

		/**
		 * @brief Установить настоящее имя персонажа
		 */
		void setRealName(const QString& _realName);

		/**
		 * @brief Получить настоящее имя персонажа
		 */
		QString realName() const;

		/**
		 * @brief Установить описание локации
		 */
		void setDescription(const QString& _description);

		/**
		 * @brief Получить описание локации
		 */
		QString description() const;

		/**
		 * @brief Установить фотографии
		 */
		void setPhotos(const QList<QPixmap>& _photos);

		/**
		 * @brief Получить список фотографий
		 */
		QList<QPixmap> photos() const;

		/**
		 * @brief Установить режим работы со сценарием
		 */
		void setCommentOnly(bool _isCommentOnly);

	signals:
		/**
		 * @brief Сохранить
		 */
		void saveCharacter();

	private slots:
		/**
		 * @brief Персонаж изменён
		 */
		void aboutCharacterChanged();

		/**
		 * @brief Добавить фотографию
		 */
		void aboutAddPhoto();

	private:
		/**
		 * @brief Настроить представление
		 */
		void initView();

		/**
		 * @brief Настроить соединения для формы
		 */
		void initConnections();
		void removeConnections();

		/**
		 * @brief Настроить внешний вид
		 */
		void initStyleSheet();

	private:
		Ui::CharactersDataEdit *ui;

		/**
		 * @brief Исходное полное имя
		 */
		QString m_sourceRealName;

		/**
		 * @brief Исходное описание
		 */
		QString m_sourceDescription;

		/**
		 * @brief Исходные фотографии
		 */
		QList<QPixmap> m_sourcePhotos;
	};
}

#endif // CHARACTERSDATAEDIT_H
