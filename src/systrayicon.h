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

#ifndef SYSTRAYICON_H
#define SYSTRAYICON_H

#include <QtWidgets/QtWidgets>

class SystrayIcon : public QObject
{
	Q_OBJECT

public:
	enum SystrayStatus
	{
		StatusUndefined,
		StatusNormal,
		StatusClick
	};

	typedef QMap<QString, SystrayStatus> SystrayStatuses;
	typedef SystrayStatuses::iterator SystrayStatusesIterator;
	typedef SystrayStatuses::const_iterator SystrayStatusesConstIterator;

	enum SystrayAction
	{
		ActionNone,
		ActionUpdate
	};

	SystrayIcon(QWidget* parent);
	virtual ~SystrayIcon();

	static SystrayIcon* getInstance() { return s_instance; }

	SystrayStatus getStatus() const;
	void setStatus(SystrayStatus status);

	void displayMessage(const QString &message, SystrayAction action);
	void update();

signals:
	void requestMinimize();
	void requestRestore();
	void requestClose();
	void requestAction(SystrayIcon::SystrayAction action);

public slots:
	void onMessageClicked();
	void onTrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
	bool create();
	bool release();

	void updateStatus();

	static SystrayIcon* s_instance;

	SystrayStatus m_status;
	QSystemTrayIcon *m_icon;
	SystrayAction m_action;

	QAction *m_minimizeAction;
	QAction *m_restoreAction;
	QAction *m_quitAction;
};

#endif
