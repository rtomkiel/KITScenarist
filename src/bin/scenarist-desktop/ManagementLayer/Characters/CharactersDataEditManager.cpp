#include "CharactersDataEditManager.h"

#include <Domain/Character.h>

#include <DataLayer/DataStorageLayer/StorageFacade.h>
#include <DataLayer/DataStorageLayer/CharacterStorage.h>

#include <UserInterfaceLayer/Characters/CharactersDataEdit/CharactersDataEdit.h>

using ManagementLayer::CharactersDataEditManager;
using UserInterface::CharactersDataEdit;


CharactersDataEditManager::CharactersDataEditManager(QObject *_parent, QWidget* _parentWidget) :
	QObject(_parent),
	m_editor(new CharactersDataEdit(_parentWidget))
{
	initView();
	initConnections();

	clean();
}

QWidget* CharactersDataEditManager::view() const
{
	return m_editor;
}

void CharactersDataEditManager::clean()
{
	m_editor->clean();
	m_editor->setEnabled(false);
}

void CharactersDataEditManager::editCharacter(Domain::Character* _character)
{
	clean();

	m_character = _character;

	if (m_character != nullptr) {
		m_editor->setEnabled(true);
		m_editor->setName(m_character->name());
		m_editor->setRealName(m_character->realName());
		m_editor->setDescription(m_character->description());
		m_editor->setPhotos(m_character->photos());
	} else {
		clean();
	}
}

void CharactersDataEditManager::setCommentOnly(bool _isCommentOnly)
{
	m_editor->setCommentOnly(_isCommentOnly);
}

void CharactersDataEditManager::aboutSave()
{
	if (!m_editor->name().isEmpty()
		&& m_character != nullptr) {
		//
		// Сохраним предыдущее название персонажа
		//
		QString previousName = m_character->name();

		//
		// Установим новые значения
		//
		m_character->setName(m_editor->name());
		m_character->setRealName(m_editor->realName());
		m_character->setDescription(m_editor->description());
		m_character->setPhotos(m_editor->photos());

		//
		// Уведомим об изменении названия персонажа
		//
		if (previousName != m_character->name()) {
			//
			// Обновим заголовок в панели с данными
			//
			m_editor->setName(m_character->name());
			//
			// Пусть модель знает, что данные изменились
			//
			DataStorageLayer::StorageFacade::characterStorage()->all()->itemChanged(m_character);
			//
			// Отправляем сигнал, чтобы обновить имена персонажей в тексте сценария
			//
			emit characterNameChanged(previousName, m_character->name());
		}

		emit characterChanged();
	}
}

void CharactersDataEditManager::initView()
{
}

void CharactersDataEditManager::initConnections()
{
	connect(m_editor, SIGNAL(saveCharacter()), this, SLOT(aboutSave()));
}
