/*
 *  kdAmn is a deviantART Messaging Network client
 *  Copyright (C) 2013-2015  Cedric OCHS
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
#include "spotmodel.h"

struct SMagicHeader
{
	union
	{
		char str[5];
		quint32 num;
	};
};

SMagicHeader s_header = { "ACFK" };
quint32 s_version = 1;

SpotModel::SpotModel(QObject* parent) : QAbstractTableModel(parent)
{
}

SpotModel::~SpotModel()
{
}

int SpotModel::rowCount(const QModelIndex &parent) const
{
	return m_spots.size();
}

int SpotModel::columnCount(const QModelIndex &parent) const
{
	// name, original position, delay, last position
	return 4;
}

QVariant SpotModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		switch (index.column())
		{
			case 0: return m_spots[index.row()].name;
			case 1: return m_spots[index.row()].originalPosition;
			case 2: return m_spots[index.row()].delay;
			case 3: return m_spots[index.row()].lastPosition;
		}
	}

	return QVariant();
}

bool SpotModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::EditRole)
	{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
		if (!checkIndex(index)) return false;
#endif

		// save value
		switch (index.column())
		{
			case 0: m_spots[index.row()].name = value.toString(); break;
			case 1: m_spots[index.row()].originalPosition = value.toPoint(); break;
			case 2: m_spots[index.row()].delay = value.toInt(); break;
			case 3: m_spots[index.row()].lastPosition = value.toPoint(); break;
			default: return false;
		}

		return true;

	}

	return false;
}

Qt::ItemFlags SpotModel::flags(const QModelIndex &index) const
{
	return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool SpotModel::insertRows(int position, int rows, const QModelIndex& parent)
{
	bool insertAtTheEnd = position == -1;

	if (position == -1) position = rowCount();

	beginInsertRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; ++row)
	{
		Spot spot;
		spot.name = tr("Spot #%1").arg(rowCount() + 1);
		spot.originalPosition = QPoint(0, 0);
		spot.delay = 150;
		spot.lastPosition = QPoint(0, 0);

		if (insertAtTheEnd)
		{
			m_spots.push_back(spot);
		}
		else
		{
			m_spots.insert(row + position, spot);
		}
	}

	endInsertRows();
	return true;
}

bool SpotModel::removeRows(int position, int rows, const QModelIndex& parent)
{
	beginRemoveRows(QModelIndex(), position, position + rows - 1);

	for (int row = 0; row < rows; ++row)
	{
		m_spots.removeAt(position + row);
	}

	endRemoveRows();
	return true;
}

Spot SpotModel::getSpot(int row) const
{
	return m_spots[row];
}

void SpotModel::setSpot(int row, const Spot& spot)
{
	m_spots[row] = spot;
}

void SpotModel::reset()
{
	beginResetModel();

	m_spots.clear();

	endResetModel();
}

bool SpotModel::load(const QString& filename)
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

	// spots
	stream >> m_spots;

	endResetModel();

	return true;
}

bool SpotModel::save(const QString& filename) const
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

	stream << m_spots;

	return true;
}