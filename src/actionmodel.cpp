/*
 *  kClicker is a tool to click automatically
 *  Copyright (C) 2017-2022  Cedric OCHS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common.h"
#include "actionmodel.h"

struct SMagicHeader
{
	union
	{
		char str[5];
		quint32 num;
	};
};

const static QString s_actionHeader("action=");
const static QString s_windowHeader("window=");

SMagicHeader s_header = { "ACFK" };

// version 1:
// - initial version
//
// version 2:
// - added duration to a click
//
// version 3:
// - added window name
//
// version 4:
// - added action types
// - added Repeat action type
//
// version 5:
// - renamed delay to delayMax
// - added delayMin
//
// version 6:
// - added name

quint32 s_version = 6;

ActionModel::ActionModel(QObject* parent) : QAbstractTableModel(parent), m_startFrom(0)
{
}

ActionModel::~ActionModel()
{
}

int ActionModel::rowCount(const QModelIndex &/* parent */) const
{
	return m_actions.size();
}

int ActionModel::columnCount(const QModelIndex &/* parent */) const
{
	// name, type, original position, delay, duration, last position, count
	return ActionColumnLast;
}

QVariant ActionModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		switch (index.column())
		{
			case ActionColumnName: return m_actions[index.row()].name;
			case ActionColumnType: return typeToInt(m_actions[index.row()].type);
			case ActionColumnOriginalPosition: return m_actions[index.row()].originalPosition;
			case ActionColumnDelayMin: return m_actions[index.row()].delayMin;
			case ActionColumnDelayMax: return m_actions[index.row()].delayMax;
			case ActionColumnDuration: return m_actions[index.row()].duration;
			case ActionColumnLastPosition: return m_actions[index.row()].lastPosition;
			case ActionColumnOriginalCount: return m_actions[index.row()].originalCount;
			case ActionColumnLastCount: return m_actions[index.row()].lastCount;
		}
	}
	
	return QVariant();
}

bool ActionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
		if (!checkIndex(index)) return false;
#endif

		// save value
		switch (index.column())
		{
			case ActionColumnName: m_actions[index.row()].name = value.toString(); break;
			case ActionColumnType: m_actions[index.row()].type = typeFromInt(value.toInt()); break;
			case ActionColumnOriginalPosition: m_actions[index.row()].originalPosition = value.toPoint(); break;
			case ActionColumnDelayMin: m_actions[index.row()].delayMin = value.toInt(); break;
			case ActionColumnDelayMax: m_actions[index.row()].delayMax = value.toInt(); break;
			case ActionColumnDuration: m_actions[index.row()].duration = value.toInt(); break;
			case ActionColumnLastPosition: m_actions[index.row()].lastPosition = value.toPoint(); break;
			case ActionColumnOriginalCount: m_actions[index.row()].originalCount = value.toInt(); break;
			case ActionColumnLastCount: m_actions[index.row()].lastCount = value.toInt(); break;
			default: return false;
		}

		return true;

	}

	return false;
}

Qt::ItemFlags ActionModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = Qt::ItemIsDropEnabled | QAbstractTableModel::flags(index);

	if (index.isValid()) flags |= Qt::ItemIsEditable | Qt::ItemIsDragEnabled;

	return flags;
}

bool ActionModel::insertRows(int position, int rows, const QModelIndex& /* parent */)
{
	bool insertAtTheEnd = position == -1;

	if (position == -1) position = rowCount();

	beginInsertRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; ++row)
	{
		Action action;
		action.name = tr("Action #%1").arg(rowCount() + 1);
		action.type = Action::Type::Click;
		action.originalPosition = QPoint(0, 0);
		action.delayMin = 30;
		action.delayMax = 150;
		action.lastPosition = QPoint(0, 0);

		if (insertAtTheEnd)
		{
			m_actions.push_back(action);
		}
		else
		{
			m_actions.insert(row + position, action);
		}
	}

	endInsertRows();
	return true;
}

bool ActionModel::removeRows(int position, int rows, const QModelIndex& /* parent */)
{
	beginRemoveRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; ++row)
	{
		m_actions.removeAt(position + row);

		if (m_startFrom == row)
		{
			m_startFrom = 0;
		}
	}

	endRemoveRows();
	return true;
}

Qt::DropActions ActionModel::supportedDropActions() const
{
	return Qt::MoveAction | Qt::CopyAction;
}

QStringList ActionModel::mimeTypes() const
{
	QStringList types;
	types << "application/x-autoclicker";
	return types;
}

QMimeData* ActionModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData* mimeData = new QMimeData();
	QByteArray encodedData;

	QDataStream stream(&encodedData, QIODevice::WriteOnly);

	foreach(const QModelIndex &index, indexes)
	{
		if (index.isValid())
		{
			stream << m_actions[index.row()];
		}
	}

	mimeData->setData("application/x-autoclicker", encodedData);

	return mimeData;
}

bool ActionModel::dropMimeData(const QMimeData* data, Qt::DropAction actionType, int row, int /* column */, const QModelIndex& /* parent */)
{
	if (!data->hasFormat("application/x-autoclicker")) return false;

	if (actionType == Qt::IgnoreAction) return true;

	bool insertAtTheEnd = row == -1;

	if (row == -1) row = rowCount();

	QByteArray encodedData = data->data("application/x-autoclicker");
	QDataStream stream(&encodedData, QIODevice::ReadOnly);

	Action action;

	stream >> action;

	beginInsertRows(QModelIndex(), row, row);

	if (insertAtTheEnd)
	{
		m_actions.push_back(action);
	}
	else
	{
		m_actions.insert(row, action);
	}

	endInsertRows();

	return true;
}

Action ActionModel::getAction(int row) const
{
	return m_actions[row];
}

void ActionModel::setAction(int row, const Action& action)
{
	m_actions[row] = action;

	emit dataChanged(index(row, 0), index(row, ActionColumnLast-1), { Qt::DisplayRole, Qt::EditRole });
}

QString ActionModel::getWindowTitle() const
{
	return m_windowTitle;
}

void ActionModel::setWindowTitle(const QString& title)
{
	m_windowTitle = title;
}

QString ActionModel::getName() const
{
	return m_name;
}

void ActionModel::setName(const QString& name)
{
	m_name = name;
}

int ActionModel::getStartFrom() const
{
	return m_startFrom;
}

void ActionModel::setStartFrom(int startFrom)
{
	m_startFrom = startFrom;
}

void ActionModel::reset()
{
	m_windowTitle.clear();
	m_filename.clear();
	m_name.clear();

	m_startFrom = 0;

	beginResetModel();

	m_actions.clear();

	endResetModel();
}

void ActionModel::resetCount()
{
	for (int i = 0; i < m_actions.size(); ++i)
	{
		Action& action = m_actions[i];

		if (action.type == Action::Type::Repeat)
		{
			// reset repeat count
			action.lastCount = action.originalCount;
		}
	}
}

bool ActionModel::load(const QString& filename)
{
	if (filename.isEmpty()) return false;

	QFile file(filename);

	if (!file.open(QIODevice::ReadOnly)) return false;

	QDataStream stream(&file);

	// Read and check the header
	SMagicHeader header;

	stream >> header.num;

	if (header.num != s_header.num) return false;

	// Read the version
	quint32 version;
	stream >> version;

	if (version > s_version) return false;

	// define version for items and other serialized objects
	stream.device()->setProperty("version", version);

#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0))
	stream.setVersion(QDataStream::Qt_5_4);
#else
	stream.setVersion(QDataStream::Qt_5_6);
#endif

	beginResetModel();

	// actions
	stream >> m_actions;

	endResetModel();

	// deserialize window name
	if (version >= 3)
	{
		stream >> m_windowTitle;
	}
	else
	{
		// clear any previous window name
		m_windowTitle.clear();
	}

	// deserialize name
	if (version >= 6)
	{
		stream >> m_name;
	}
	else
	{
		// use filename
		m_name = QFileInfo(filename).baseName();
	}

	m_filename = filename;

	return true;
}

bool ActionModel::save(const QString& filename)
{
	if (filename.isEmpty()) return false;

	QFile file(filename);

	if (!file.open(QIODevice::WriteOnly)) return false;

	QDataStream stream(&file);

	// Write a header with a "magic number" and a version
	stream << s_header.num;
	stream << s_version;

#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0))
	stream.setVersion(QDataStream::Qt_5_4);
#else
	stream.setVersion(QDataStream::Qt_5_6);
#endif

	stream << m_actions;

	// serialize window title
	stream << m_windowTitle;

	// serialize name
	if (m_name.isEmpty())
	{
		// use filename
		stream << QFileInfo(filename).baseName();
	}
	else
	{
		stream << m_name;
	}

	m_filename = filename;

	return true;
}

bool ActionModel::loadText(const QString& filename)
{
	if (filename.isEmpty()) return false;

	QSettings settings(filename, QSettings::IniFormat);

	settings.beginGroup("Common");

	m_name = settings.value("Name").toString();
	m_windowTitle = settings.value("WindowTitle").toString();

	int actionsCount = settings.value("ActionsCount").toInt();

	settings.endGroup();

	beginResetModel();

	m_actions.clear();

	for(int i = 0; i < actionsCount; ++i)
	{
		settings.beginGroup(QString("Action_%1").arg(i));

		Action action;
		action.readFromSettings(settings);

		settings.endGroup();

		m_actions << action;
	}

	endResetModel();

	return true;
}

bool ActionModel::saveText(const QString& filename)
{
	if (filename.isEmpty()) return false;

	QSettings settings(filename, QSettings::IniFormat);

	if (!settings.isWritable()) return false;

	settings.beginGroup("Common");

	settings.setValue("Name", m_name);
	settings.setValue("WindowTitle", m_windowTitle);

	settings.setValue("ActionsCount", m_actions.size());

	settings.endGroup();

	int actionCount = 0;

	for (const Action &action: m_actions)
	{
		settings.beginGroup(QString("Action_%1").arg(actionCount++));

		action.writeToSettings(settings);

		settings.endGroup();
	}

	return true;
}

bool ActionModel::updateSpotsPosition(const QPoint& offset)
{
	for (int i = 0; i < m_actions.size(); ++i)
	{
		Action &action = m_actions[i];

		action.originalPosition -= offset;
		action.lastPosition = action.originalPosition;
	}

	return true;
}

QString ActionModel::getFilename() const
{
	return m_filename;
}

ActionModel* ActionModel::clone(QObject *parent) const
{
	ActionModel* res = new ActionModel(parent);

	res->m_actions = m_actions;
	res->m_name = m_name;
	res->m_windowTitle = m_windowTitle;
	res->m_filename = m_filename;
	res->m_startFrom = m_startFrom;

	return res;
}
