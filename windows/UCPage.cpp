/*
 * Copyright (C) 2001-2017 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdafx.h"
#include "UCPage.h"
#include "CommandDlg.h"

PropPage::TextItem UCPage::texts[] =
{
	{ IDC_MOVE_UP, ResourceManager::MOVE_UP },
	{ IDC_MOVE_DOWN, ResourceManager::MOVE_DOWN },
	{ IDC_ADD_MENU, ResourceManager::ADD },
	{ IDC_CHANGE_MENU, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE_MENU, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item UCPage::items[] =
{
	{ 0, 0, PropPage::T_END }
};

LRESULT UCPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	PropPage::read(*this, items);
	
	CRect rc;
	
	ctrlCommands.Attach(GetDlgItem(IDC_MENU_ITEMS));
	ctrlCommands.GetClientRect(rc);
	
	ctrlCommands.InsertColumn(0, CTSTRING(SETTINGS_NAME), LVCFMT_LEFT, rc.Width() / 4, 0);
	ctrlCommands.InsertColumn(1, CTSTRING(SETTINGS_COMMAND), LVCFMT_LEFT, rc.Width() * 2 / 4, 1);
	ctrlCommands.InsertColumn(2, CTSTRING(HUB), LVCFMT_LEFT, rc.Width() / 4, 2);
	SET_EXTENDENT_LIST_VIEW_STYLE(ctrlCommands);
#ifdef USE_SET_LIST_COLOR_IN_SETTINGS
	SET_LIST_COLOR_IN_SETTING(ctrlCommands);
#endif
	
	// Do specialized reading here
	UserCommand::List lst = FavoriteManager::getInstance()->getUserCommands();
	auto cnt = ctrlCommands.GetItemCount();
	for (auto i = lst.cbegin(); i != lst.cend(); ++i)
	{
		const UserCommand& uc = *i;
		if (!uc.isSet(UserCommand::FLAG_NOSAVE))
		{
			addEntry(uc, cnt++);
		}
	}
	
	return TRUE;
}

LRESULT UCPage::onAddMenu(WORD, WORD, HWND, BOOL&)
{
	CommandDlg dlg;
	
	if (dlg.DoModal() == IDOK)
	{
		addEntry(FavoriteManager::addUserCommand(dlg.type, dlg.ctx,
		                                         0, Text::fromT(dlg.name), Text::fromT(dlg.command), "", Text::fromT(dlg.hub)), ctrlCommands.GetItemCount());
	}
	return 0;
}

LRESULT UCPage::onChangeMenu(WORD, WORD, HWND, BOOL&)
{
	if (ctrlCommands.GetSelectedCount() == 1)
	{
		int sel = ctrlCommands.GetSelectedIndex();
		UserCommand uc;
		FavoriteManager::getUserCommand(ctrlCommands.GetItemData(sel), uc);
		
		CommandDlg dlg;
		dlg.type = uc.getType();
		dlg.ctx = uc.getCtx();
		dlg.name = Text::toT(uc.getName());
		dlg.command = Text::toT(uc.getCommand());
		dlg.hub = Text::toT(uc.getHub());
		
		if (dlg.DoModal() == IDOK)
		{
			if (dlg.type == UserCommand::TYPE_SEPARATOR)
				ctrlCommands.SetItemText(sel, 0, CTSTRING(SEPARATOR));
			else
				ctrlCommands.SetItemText(sel, 0, dlg.name.c_str());
			ctrlCommands.SetItemText(sel, 1, dlg.command.c_str());
			ctrlCommands.SetItemText(sel, 2, dlg.hub.c_str());
			uc.setName(Text::fromT(dlg.name));
			uc.setCommand(Text::fromT(dlg.command));
			uc.setHub(Text::fromT(dlg.hub));
			uc.setType(dlg.type);
			uc.setCtx(dlg.ctx);
			FavoriteManager::updateUserCommand(uc);
		}
	}
	return 0;
}

LRESULT UCPage::onRemoveMenu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (ctrlCommands.GetSelectedCount() == 1)
	{
		int i = ctrlCommands.GetNextItem(-1, LVNI_SELECTED);
		FavoriteManager::removeUserCommandCID(ctrlCommands.GetItemData(i));
		ctrlCommands.DeleteItem(i);
	}
	return 0;
}

LRESULT UCPage::onMoveUp(WORD, WORD, HWND, BOOL&)
{
	int i = ctrlCommands.GetSelectedIndex();
	if (i != -1 && i != 0)
	{
		int n = ctrlCommands.GetItemData(i);
		FavoriteManager::moveUserCommand(n, -1);
		CLockRedraw<> l_lock_draw(ctrlCommands);
		ctrlCommands.DeleteItem(i);
		UserCommand uc;
		FavoriteManager::getUserCommand(n, uc);
		addEntry(uc, i - 1);
		ctrlCommands.SelectItem(i - 1);
		ctrlCommands.EnsureVisible(i - 1, FALSE);
	}
	return 0;
}

LRESULT UCPage::onMoveDown(WORD, WORD, HWND, BOOL&)
{
	int i = ctrlCommands.GetSelectedIndex();
	if (i != -1 && i != (ctrlCommands.GetItemCount() - 1))
	{
		int n = ctrlCommands.GetItemData(i);
		FavoriteManager::moveUserCommand(n, 1);
		CLockRedraw<> l_lock_draw(ctrlCommands);
		ctrlCommands.DeleteItem(i);
		UserCommand uc;
		FavoriteManager::getUserCommand(n, uc);
		addEntry(uc, i + 1);
		ctrlCommands.SelectItem(i + 1);
		ctrlCommands.EnsureVisible(i + 1, FALSE);
	}
	return 0;
}

LRESULT UCPage::onItemchangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	const NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
	const bool l_is_enabled = (lv->uNewState & LVIS_FOCUSED);
	::EnableWindow(GetDlgItem(IDC_MOVE_UP), l_is_enabled);
	::EnableWindow(GetDlgItem(IDC_MOVE_DOWN), l_is_enabled);
	::EnableWindow(GetDlgItem(IDC_CHANGE_MENU), l_is_enabled);
	::EnableWindow(GetDlgItem(IDC_REMOVE_MENU), l_is_enabled);
	return 0;
}

LRESULT UCPage::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	switch (kd->wVKey)
	{
		case VK_INSERT:
			PostMessage(WM_COMMAND, IDC_ADD_MENU, 0);
			break;
		case VK_DELETE:
			PostMessage(WM_COMMAND, IDC_REMOVE_MENU, 0);
			break;
		default:
			bHandled = FALSE;
	}
	return 0;
}

LRESULT UCPage::onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	
	if (item->iItem >= 0)
	{
		PostMessage(WM_COMMAND, IDC_CHANGE_MENU, 0);
	}
	else if (item->iItem == -1)
	{
		PostMessage(WM_COMMAND, IDC_ADD_MENU, 0);
	}
	
	return 0;
}

void UCPage::addEntry(const UserCommand& uc, int pos)
{
	TStringList lst;
	if (uc.getType() == UserCommand::TYPE_SEPARATOR)
		lst.push_back(TSTRING(SEPARATOR));
	else
		lst.push_back(Text::toT(uc.getName()));
	lst.push_back(Text::toT(uc.getCommand()));
	lst.push_back(Text::toT(uc.getHub()));
	ctrlCommands.insert(pos, lst, 0, (LPARAM)uc.getId());
}

void UCPage::write()
{
	PropPage::write(*this, items);
}

/**
 * @file
 * $Id: UCPage.cpp,v 1.13 2006/06/15 20:14:15 bigmuscle Exp $
 */
