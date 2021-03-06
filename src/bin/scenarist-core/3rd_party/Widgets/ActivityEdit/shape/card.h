#ifndef CARD_SHAPE_H
#define CARD_SHAPE_H

#include "resizableshape.h"


/**
 * @brief Карточка сценария
 */
class CardShape : public ResizableShape
{
	Q_OBJECT

public:
	/**
	 * @brief Тип карточки
	 */
	enum CardType {
		TypeUndefined,
		TypeScene,
		TypeScenesGroup,
		TypeFolder,
		TypeScenario
	};

	static const int DEFAULT_WIDTH = 210;
	static const int DEFAULT_HEIGHT = 100;

public:
	CardShape(QGraphicsItem* _parent = NULL);
	CardShape(const QString& _uuid, CardType _type, const QString& _title,
		const QString& _description, const QString& _colors, const QPointF& _pos, QGraphicsItem* _parent = NULL);

	/**
	 * @brief Идентификатор
	 */
	/** @{ */
	void setUuid(const QString& _uuid);
	QString uuid() const;
	/** @} */

	/**
	 * @brief Тип
	 */
	/** @{ */
	void setCardType(CardType _type);
	CardType cardType() const;
	/** @} */

	/**
	 * @brief Заголовок
	 */
	/** @{ */
	void setTitle(const QString& _title);
	QString title() const;
	/** @} */

	/**
	 * @brief Описание
	 */
	/** @{ */
	void setDescription(const QString& _description);
	QString description() const;
	/** @} */

	/**
	 * @brief Цвета
	 */
	/** @{ */
	void setColors(const QString& _colors);
	QString colors() const;
	/** @} */

	/**
	 * @brief Установить состояние вложения дочернего элемента
	 */
	/** @{ */
	void setOnInstertionState(bool _on);
	bool isOnInstertionState() const;
	/** @} */

	/**
	 * @brief Уровень вложенности
	 */
	int depthLevel() const;

	/**
	 * @brief Является ли карточка предком заданного элемента
	 */
	bool isGrandParentOf(QGraphicsItem* _item) const;

	/**
	 * @brief Подогнать размер фигуры (если он меньше минимального, то увеличить).
	 */
	void adjustSize() override;

protected:
	/**
	 * @brief Рисуем карточку
	 */
	void paint(QPainter* _painter, const QStyleOptionGraphicsItem* _option, QWidget* _widget) override;

private:
	/**
	 * @brief Идентификатор
	 */
	QString m_uuid;

	/**
	 * @brief Тип карточки
	 */
	CardType m_cardType;

	/**
	 * @brief Заголовок карточки
	 */
	QString m_title;

	/**
	 * @brief Описание карточки
	 */
	QString m_description;

	/**
	 * @brief Цвета карточки
	 */
	QString m_colors;

	/**
	 * @brief Карточка находится в состоянии вложения дочернего элемента
	 */
	bool m_isOnInsertionState;
};

#endif // CARD_SHAPE_H
