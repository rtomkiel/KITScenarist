#include "ScenarioNavigatorManager.h"

#include <BusinessLayer/ScenarioDocument/ScenarioModel.h>

#include <DataLayer/DataStorageLayer/StorageFacade.h>
#include <DataLayer/DataStorageLayer/SettingsStorage.h>

#include <UserInterfaceLayer/Scenario/ScenarioNavigator/ScenarioNavigator.h>
#include <UserInterfaceLayer/Scenario/ScenarioItemDialog/ScenarioItemDialog.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

using ManagementLayer::ScenarioNavigatorManager;
using BusinessLogic::ScenarioModel;
using BusinessLogic::ScenarioModelFiltered;
using UserInterface::ScenarioNavigator;
using UserInterface::ScenarioItemDialog;


ScenarioNavigatorManager::ScenarioNavigatorManager(QObject *_parent, QWidget* _parentWidget, bool _isDraft) :
	QObject(_parent),
	m_scenarioModel(0),
	m_scenarioModelProxy(new ScenarioModelFiltered(this)),
	m_navigator(new ScenarioNavigator(_parentWidget)),
	m_addItemDialog(new ScenarioItemDialog(m_navigator)),
	m_isDraft(_isDraft)
{
	initView();
	initConnections();
	reloadNavigatorSettings();
}

QWidget* ScenarioNavigatorManager::view() const
{
	return m_navigator;
}

void ScenarioNavigatorManager::setNavigationModel(ScenarioModel* _model)
{
	disconnectModel();
	m_scenarioModel = _model;
	m_scenarioModelProxy->setSourceModel(m_scenarioModel);
	m_navigator->setModel(m_scenarioModelProxy);
	connectModel();

	if (m_scenarioModel != 0) {
		aboutModelUpdated();
	}
}

void ScenarioNavigatorManager::reloadNavigatorSettings()
{
	//
	// Сбросим представление
	//
	m_navigator->resetView();
	m_navigator->setShowSceneNumber(
				DataStorageLayer::StorageFacade::settingsStorage()->value(
					"navigator/show-scenes-numbers",
					DataStorageLayer::SettingsStorage::ApplicationSettings)
				.toInt()
				);
	m_navigator->setShowSceneTitle(
				DataStorageLayer::StorageFacade::settingsStorage()->value(
					"navigator/show-scene-title",
					DataStorageLayer::SettingsStorage::ApplicationSettings)
				.toInt()
				);
	m_navigator->setShowSceneDescription(
				DataStorageLayer::StorageFacade::settingsStorage()->value(
					"navigator/show-scene-description",
					DataStorageLayer::SettingsStorage::ApplicationSettings)
				.toInt()
				);
	m_navigator->setSceneDescriptionIsSceneText(
				DataStorageLayer::StorageFacade::settingsStorage()->value(
					"navigator/scene-description-is-scene-text",
					DataStorageLayer::SettingsStorage::ApplicationSettings)
				.toInt()
				);
	m_navigator->setSceneDescriptionHeight(
				DataStorageLayer::StorageFacade::settingsStorage()->value(
					"navigator/scene-description-height",
					DataStorageLayer::SettingsStorage::ApplicationSettings)
				.toInt()
				);
}

void ScenarioNavigatorManager::setCurrentIndex(const QModelIndex& _index)
{
	m_navigator->setCurrentIndex(m_scenarioModelProxy->mapFromSource(_index));
}

void ScenarioNavigatorManager::clearSelection()
{
	m_navigator->clearSelection();
}

void ScenarioNavigatorManager::setDraftVisible(bool _visible)
{
	m_navigator->setDraftVisible(_visible);
}

void ScenarioNavigatorManager::setNoteVisible(bool _visible)
{
	m_navigator->setNoteVisible(_visible);
}

void ScenarioNavigatorManager::setCommentOnly(bool _isCommentOnly)
{
	m_navigator->setCommentOnly(_isCommentOnly);
}

void ScenarioNavigatorManager::aboutAddItem(const QModelIndex& _index)
{
	m_addItemDialog->clear();

	//
	// Если пользователь действительно хочет добавить элемент
	//
	if (m_addItemDialog->exec() == QLightBoxDialog::Accepted) {
		const int itemType = m_addItemDialog->itemType();
		const QString header = m_addItemDialog->header();
		const QColor color = m_addItemDialog->color();
		const QString description = m_addItemDialog->description();

		//
		// Если задан заголовок
		//
		if (!header.isEmpty()) {
			emit addItem(m_scenarioModelProxy->mapToSource(_index), itemType, header, color, description);
		}
	}
}

void ScenarioNavigatorManager::aboutRemoveItems(const QModelIndexList& _indexes)
{
	QModelIndexList removeIndexes;
	foreach (const QModelIndex& index, _indexes) {
		removeIndexes.append(m_scenarioModelProxy->mapToSource(index));
	}
	emit removeItems(removeIndexes);
}

void ScenarioNavigatorManager::aboutSetItemColors(const QModelIndex& _index, const QString& _colors)
{
	emit setItemColors(m_scenarioModelProxy->mapToSource(_index), _colors);
}

void ScenarioNavigatorManager::aboutChangeItemType(const QModelIndex& _index, int _type)
{
	emit changeItemTypeRequested(m_scenarioModelProxy->mapToSource(_index), _type);
}

void ScenarioNavigatorManager::aboutSceneChoosed(const QModelIndex& _index)
{
	emit sceneChoosed(m_scenarioModelProxy->mapToSource(_index));
}

void ScenarioNavigatorManager::aboutModelUpdated()
{
	m_navigator->setScenesCount(m_scenarioModel->scenesCount());
}

void ScenarioNavigatorManager::initView()
{
	if (!m_isDraft) {
		m_navigator->setObjectName("scenarioNavigator");
	} else {
		m_navigator->setObjectName("scenarioDraftNavigator");
	}
	m_navigator->setIsDraft(m_isDraft);
}

void ScenarioNavigatorManager::initConnections()
{
	connectModel();

	connect(m_navigator, SIGNAL(addItem(QModelIndex)), this, SLOT(aboutAddItem(QModelIndex)));
	connect(m_navigator, SIGNAL(removeItems(QModelIndexList)), this, SLOT(aboutRemoveItems(QModelIndexList)));
	connect(m_navigator, SIGNAL(setItemColors(QModelIndex,QString)), this, SLOT(aboutSetItemColors(QModelIndex,QString)));
	connect(m_navigator, &ScenarioNavigator::changeItemTypeRequested, this, &ScenarioNavigatorManager::aboutChangeItemType);
	connect(m_navigator, SIGNAL(showHideDraft()), this, SIGNAL(showHideDraft()));
	connect(m_navigator, SIGNAL(showHideNote()), this, SIGNAL(showHideNote()));
	connect(m_navigator, SIGNAL(sceneChoosed(QModelIndex)), this, SLOT(aboutSceneChoosed(QModelIndex)));
	connect(m_navigator, SIGNAL(undoRequest()), this, SIGNAL(undoRequest()));
	connect(m_navigator, SIGNAL(redoRequest()), this, SIGNAL(redoRequest()));
}

void ScenarioNavigatorManager::connectModel()
{
	if (m_scenarioModel != 0) {
		connect(m_scenarioModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(aboutModelUpdated()));
		connect(m_scenarioModel, SIGNAL(mimeDropped(int)), this, SIGNAL(sceneChoosed(int)));
	}
}

void ScenarioNavigatorManager::disconnectModel()
{
	if (m_scenarioModel != 0) {
		disconnect(m_scenarioModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(aboutModelUpdated()));
		disconnect(m_scenarioModel, SIGNAL(mimeDropped(int)), this, SIGNAL(sceneChoosed(int)));
	}
}
