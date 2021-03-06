#ifndef SCALABLEWRAPPER_H
#define SCALABLEWRAPPER_H

#include <QGraphicsView>
#include <QVariant>

class QGestureEvent;
class SpellCheckTextEdit;


/**
 * @brief Обёртка над редактором текста, чтобы сделать его масштабируемым
 */
class ScalableWrapper : public QGraphicsView
{
	Q_OBJECT

public:
	explicit ScalableWrapper(SpellCheckTextEdit* _editor, QWidget* _parent = 0);

	/**
	 * @brief Получить редактор, вокруг которого построена обёртка
	 */
	SpellCheckTextEdit* editor() const;

	/**
	 * @brief Установить коэффициент масштабирование
	 */
	void setZoomRange(qreal _zoomRange);

	/**
	 * @brief Дополнительные вещи для корректной работы в андройде
	 */
	/** @{ */
	QVariant inputMethodQuery(Qt::InputMethodQuery _query) const;
	Q_INVOKABLE QVariant inputMethodQuery(Qt::InputMethodQuery _query, QVariant _argument) const;
	/** @} */

public slots:
	/**
	 * @brief Увеличить масштаб
	 */
	void zoomIn();

	/**
	 * @brief Уменьшить масштаб
	 */
	void zoomOut();

signals:
	/**
	 * @brief Изменился коэффициент масштабирования
	 */
	void zoomRangeChanged(qreal) const;

protected:
	/**
	 * @brief Переопределяем для обработки жестов
	 */
	bool event(QEvent* _event);

	/**
	 * @brief Переопределяется для реализации увеличения/уменьшения
	 */
	void wheelEvent(QWheelEvent* _event);

	/**
	 * @brief Обрабатываем жест увеличения масштаба
	 */
	void gestureEvent(QGestureEvent* _event);

	/**
	 * @brief Переопределяется для отлавливания контекстного меню текстового редактора
	 */
	bool eventFilter(QObject* _object, QEvent* _event);

private:
	/**
	 * @brief Включить/выключить синхронизацию полос прокрутки между редактором и представлением
	 */
	void setupScrollingSynchronization(bool _needSync);

	/**
	 * @brief Пересчитать размер редактора текста
	 */
	void updateTextEditSize();

	/**
	 * @brief Собственно масштабирование представления текстового редактора
	 */
	void scaleTextEdit();

private:
	/**
	 * @brief Сцена в которой будет позиционироваться редактор
	 */
	QGraphicsScene* m_scene;

	/**
	 * @brief Указатель на сам редактор
	 */
	SpellCheckTextEdit* m_editor;

	/**
	 * @brief Графическое представление редактора
	 */
	QGraphicsProxyWidget* m_editorProxy;

	/**
	 * @brief Коэффициент масштабирования
	 */
	qreal m_zoomRange;

	/**
	 * @brief Инерционный тормоз масштабирования при помощи жестов
	 */
	int m_gestureZoomInertionBreak;

	/**
	 * @brief Вспомогательный элемент, посредством которого настраивается размер полос прокрутки
	 */
	QGraphicsRectItem* m_rect;

	/**
	 * @brief Нужно ли обновить положение полос прокрутки после события изменения размера
	 */
	bool m_needUpdateScrollValues;

	/**
	 * @brief Положения полос прокрутки зафиксированные перед событием изменения размера
	 */
	int m_vbarScrollValue, m_hbarScrollValue;
};

#endif // SCALABLEWRAPPER_H
