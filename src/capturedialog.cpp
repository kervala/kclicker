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
#include "capturedialog.h"
#include "moc_capturedialog.cpp"
#include "utils.h"

CaptureDialog::CaptureDialog(QWidget *parent):QDialog(parent)
{
	setupUi(this);

	// disable OK button
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	QStandardItemModel *model = new QStandardItemModel(0, 1, this);

	// add windows list with icons to listview model
	createWindowsList(model);

	windowsListView->setModel(model);

	int rowHeight = windowsListView->sizeHintForRow(0) + 1;
	windowsListView->setMinimumHeight(qMin(6, model->rowCount()) * rowHeight);

	connect(windowsListView, SIGNAL(pressed(const QModelIndex &)), this, SLOT(enableButton(const QModelIndex &)));
	connect(windowsListView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(validateButton(const QModelIndex &)));
}

void CaptureDialog::enableButton(const QModelIndex &index)
{
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

	m_window = windowsListView->model()->data(index, Qt::UserRole).value<Window>();
}

void CaptureDialog::validateButton(const QModelIndex &index)
{
	enableButton(index);

	accept();
}