
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <boost/format.hpp>
#include <boost/array.hpp>

#include "stdAfx.hpp"
#include "HaliteDialog.hpp"
#include "HaliteWindow.hpp"
#include "HaliteListView.hpp"
#include "halEvent.hpp"

HaliteDialog::HaliteDialog(HaliteWindow& halWindow) :
	dlg_base_class_t(halWindow)
{}

LRESULT HaliteDialog::OnInitDialog(HWND, LPARAM)
{

	m_prog.Attach(GetDlgItem(TORRENTPROG));
	m_prog.SetRange(0, 100);
	
	m_list.SubclassWindow(GetDlgItem(LISTPEERS));
	
	totalConnections_.Attach(GetDlgItem(HAL_EDITNCD));
	uploadConnections_.Attach(GetDlgItem(HAL_EDITNCU));
	downloadRate_.Attach(GetDlgItem(HAL_EDITTLD));
	uploadRate_.Attach(GetDlgItem(HAL_EDITTLU));
	
	totalConnections_ = -1;
	uploadConnections_ = -1;
	downloadRate_ = -1;
	uploadRate_ = -1;	
	
	DoDataExchange(false);

	return 0;
}

HaliteDialog::CWindowMapStruct* HaliteDialog::GetWindowMap()
{
#define TORRENT_LIMITS_LAYOUT \
	WMB_HEAD(WMB_COL(_exp|20), WMB_COL(_exp|30), WMB_COL(_exp|20), WMB_COL(_exp|30)), \
		WMB_ROW(10,	HAL_TL,	_r, _r, _r), \
		WMB_ROW(11,	HAL_TLD, HAL_EDITTLD, HAL_TLU, HAL_EDITTLU), \
		WMB_ROW(10,	HAL_NC,	_r, _r, _r), \
		WMB_ROW(11,	HAL_NCD, HAL_EDITNCD, HAL_NCU, HAL_EDITNCU), \
	WMB_END()

#define TORRENT_STATUS_LAYOUT \
	WMB_HEAD(WMB_COL(45), WMB_COLNOMIN(_exp|100), WMB_COL(_eq|0), WMB_COL(_exp|200)), \
		WMB_ROW(_gap), \
		WMB_ROW(_auto,	HAL_NAME_STATIC, HAL_NAME, _r, _r), \
		WMB_ROW(_auto,	HAL_TRACKER_STATIC, HAL_TRACKER, _r, _r), \
		WMB_ROW(_auto,	HAL_STATUS_STATIC, HAL_STATUS, _r, _r), \
		WMB_ROW(_auto,	HAL_TIME_STATIC, HAL_AVAIL, HAL_COMPLETED_STATIC, HAL_COMPLETE), \
	WMB_END()
	
#define TORRENT_BUTTON_LAYOUT \
	WMB_HEAD(WMB_COL(_exp)), \
		WMB_ROW(_gap), \
		WMB_ROWMINNOMAX(_exp, 13, BTNPAUSE), \
		WMB_ROWMINNOMAX(_exp, 13, BTNREANNOUNCE), \
		WMB_ROWMINNOMAX(_exp, 13, BTNREMOVE), \
	WMB_END()	

	BEGIN_WINDOW_MAP_INLINE(HaliteDialog, 6, 0, 3, 3)
		WMB_HEAD(WMB_COL(_gap), WMB_COL(_exp), WMB_COL(120), WMB_COL(60), WMB_COL(_gap)), 
			WMB_ROW(_gap,	HAL_DETAILS_GROUP, _r, _r, _r, _r), 
			WMB_ROW(_auto,	_d, TORRENT_STATUS_LAYOUT, TORRENT_LIMITS_LAYOUT, TORRENT_BUTTON_LAYOUT), 
			WMB_ROW(_gap, _d),
			WMB_ROWMIN(_auto, 8, _d, TORRENTPROG, _r, _r), 
			WMB_ROW(_gap, _d), 
			WMB_ROWNOMAX(_exp, _d, LISTPEERS, _r, _r), 
			WMB_ROW(_gap, _d), 
			WMB_ROW(_gap|3), 
		WMB_END() 
	END_WINDOW_MAP_INLINE()	
}

void HaliteDialog::saveStatus()
{
	m_list.saveSettings();
}

void HaliteDialog::OnClose()
{
	saveStatus();
	
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

void HaliteDialog::OnPause(UINT, int, HWND)
{
	if (hal::torrent_details_ptr torrent = hal::bittorrent::Instance().torrentDetails().focused_torrent()) 
	{
		string torrentName = hal::to_utf8(torrent->name());
		
		if (!hal::bittorrent::Instance().is_torrent_active(torrentName))
		{
			SetDlgItemText(BTNPAUSE,L"Pause");
			hal::bittorrent::Instance().resume_torrent(torrentName);
		}
		else
		{
			SetDlgItemText(BTNPAUSE,L"Resume");
			hal::bittorrent::Instance().pause_torrent(torrentName);
		}
		
		requestUiUpdate();
	}
}

void HaliteDialog::OnReannounce(UINT, int, HWND)
{
	if (hal::torrent_details_ptr torrent = hal::bittorrent::Instance().torrentDetails().focused_torrent()) 
		hal::bittorrent::Instance().reannounce_torrent(torrent->name());
}

void HaliteDialog::OnRemove(UINT, int, HWND)
{
	if (hal::torrent_details_ptr torrent = hal::bittorrent::Instance().torrentDetails().focused_torrent()) 
	{
		hal::bittorrent::Instance().remove_torrent(torrent->name());
		torrentsList().erase_torrent_name(torrent->name());
	}
}

LRESULT HaliteDialog::OnHalEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hal::bit::torrent t = hal::bittorrent::Instance().get(focused_torrent()))
	{
		t.rate_limits = std::pair<float, float>(downloadRate_, uploadRate_);
		t.connection_limits = std::pair<int, int>(totalConnections_, uploadConnections_);
	}
	
	return 0;
}

LRESULT HaliteDialog::OnCltColor(HDC hDC, HWND hWnd)
{	
	::SetTextColor(hDC, RGB(255, 0, 255)); 
	
	return (LRESULT)::GetCurrentObject(hDC, OBJ_BRUSH);
}

bool HaliteDialog::DialogListView::sort_list_comparison(std::wstring l, std::wstring r, size_t index, bool ascending)
{
	hal::peer_details_vec::optional_type pdl = peer_details_.find_peer(l);
	hal::peer_details_vec::optional_type pdr = peer_details_.find_peer(r);

	if (pdl && pdr) 
		return hal::hal_details_ptr_compare(pdl, pdr, index, ascending);
	else
		return false;
}

LRESULT HaliteDialog::DialogListView::OnGetDispInfo(int, LPNMHDR pnmh, BOOL&)
{	
	hal::mutex_t::scoped_lock l(list_class_t::mutex_);

	NMLVDISPINFO* pdi = (NMLVDISPINFO*)pnmh;

	HAL_DEV_SORT_MSG(hal::wform(L"OnGetDispInfo index = %1% size = %2%") % pdi->item.iItem % peer_details_.size());

	if (pdi->item.iItem >= 0 && peer_details_.size() >= numeric_cast<unsigned>(pdi->item.iItem)) 
	{	

	hal::peer_details_vec::optional_type pd = peer_details_.find_peer(key_from_index(pdi->item.iItem));

	if (pd && pdi->item.mask & LVIF_TEXT)
	{
		wstring str = pd->to_wstring(pdi->item.iSubItem);
		
		size_t len = str.copy(pdi->item.pszText, min(pdi->item.cchTextMax - 1, static_cast<int>(str.size())));
		pdi->item.pszText[len] = '\0';
	}

	}
	
	return 0;
}

void HaliteDialog::DialogListView::uiUpdate(const hal::torrent_details_manager& tD) 
{	
	if (hal::try_update_lock<list_class_t> lock = hal::try_update_lock<list_class_t>(this)) 
	{		
		selection_from_listview();
		
		peer_details_.clear();
		
		foreach (const std::wstring filename, tD.selected_names())
		{
			if (const hal::torrent_details_ptr t = tD.get(filename))
				std::copy(t->get_peer_details().begin(), t->get_peer_details().end(), 
					std::inserter(peer_details_, peer_details_.begin()));
		}
		
		std::set<std::wstring> ip_set;
		foreach (hal::peer_detail& pd,  peer_details_)
			ip_set.insert(pd.ip_address);
		
		erase_based_on_set(ip_set, true);

		if (IsSortOnce() || AutoSort())
		{
			if (GetSecondarySortColumn() != -1)
			{
				int index = GetColumnSortType(GetSecondarySortColumn());					
				if (index > WTL::LVCOLSORT_LAST)
					sort(index - (WTL::LVCOLSORT_LAST+1+hal::peer_detail::ip_address_e), IsSecondarySortDescending());
			}

			if (GetSortColumn() != -1)
			{		
				int index = GetColumnSortType(GetSortColumn());				
				if (index > WTL::LVCOLSORT_LAST)
					sort(index - (WTL::LVCOLSORT_LAST+1+hal::peer_detail::ip_address_e), IsSortDescending());
			}
		}
		
		set_keys(ip_set);		
		InvalidateRect(NULL,true);
	}
}

LRESULT HaliteDialog::DialogListView::OnSortChanged(int, LPNMHDR pnmh, BOOL&)
{
	
	return 0;
}

void HaliteDialog::focusChanged(const hal::torrent_details_ptr pT)
{
	std::pair<float, float> tranLimit(-1.0, -1.0);
	std::pair<int, int> connLimit(-1, -1);
	
	if (hal::bit::torrent t = hal::bittorrent::Instance().get(focused_torrent()))
	{
		tranLimit = t.rate_limits;
		connLimit = t.connection_limits;
		
		if (!t.is_active)
			SetDlgItemText(BTNPAUSE, L"Resume");
		else		
			SetDlgItemText(BTNPAUSE, L"Pause");
		
		::EnableWindow(GetDlgItem(BTNPAUSE), true);
		::EnableWindow(GetDlgItem(BTNREANNOUNCE), true);
		::EnableWindow(GetDlgItem(BTNREMOVE), true);
		
		::EnableWindow(GetDlgItem(HAL_EDITTLD), true);
		::EnableWindow(GetDlgItem(HAL_EDITTLU), true);
		::EnableWindow(GetDlgItem(HAL_EDITNCD), true);
		::EnableWindow(GetDlgItem(HAL_EDITNCU), true);
	}
	else
	{
		SetDlgItemText(HAL_NAME, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_TRACKER, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_STATUS, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_AVAIL, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_COMPLETE, hal::app().res_wstr(HAL_NA).c_str());
		
		SetDlgItemText(BTNPAUSE, L"Pause");		
		m_prog.SetPos(0);
		
		::EnableWindow(GetDlgItem(BTNPAUSE), false);
		::EnableWindow(GetDlgItem(BTNREANNOUNCE), false);
		::EnableWindow(GetDlgItem(BTNREMOVE), false);
		
		::EnableWindow(GetDlgItem(HAL_EDITTLD), false);
		::EnableWindow(GetDlgItem(HAL_EDITTLU), false);
		::EnableWindow(GetDlgItem(HAL_EDITNCD), false);
		::EnableWindow(GetDlgItem(HAL_EDITNCU), false);
	}
	
	totalConnections_ = connLimit.first;
	uploadConnections_ = connLimit.second;
	downloadRate_ = tranLimit.first;
	uploadRate_ = tranLimit.second;
		
	DoDataExchange(false);
}

void HaliteDialog::uiUpdate(const hal::torrent_details_manager& tD)
{	
	std::pair<float, float> tranLimit(-1.0, -1.0);
	std::pair<int, int> connLimit(-1, -1);
	
	if (hal::torrent_details_ptr torrent = tD.focused_torrent()) 	
	{	
		string torrent_name = hal::to_utf8(torrent->name());
		
		SetDlgItemText(HAL_NAME, torrent->name().c_str());
		SetDlgItemText(HAL_TRACKER, torrent->current_tracker().c_str());
		SetDlgItemText(HAL_STATUS, torrent->state().c_str());
		m_prog.SetPos(static_cast<int>(torrent->completion()*100));
		
		if (!torrent->estimated_time_left().is_special())
		{
			SetDlgItemText(HAL_AVAIL,
				(hal::from_utf8(boost::posix_time::to_simple_string(
					torrent->estimated_time_left())).c_str()));
		}
		else
		{
			SetDlgItemText(HAL_AVAIL, hal::app().res_wstr(HAL_INF).c_str());		
		}
		
		SetDlgItemText(HAL_COMPLETE,
			(hal::wform(L"%1$.2fmb of %2$.2fmb") 
				% (static_cast<float>(torrent->total_wanted_done())/(1024*1024))
				% (static_cast<float>(torrent->total_wanted())/(1024*1024))
			).str().c_str());
		
		m_list.uiUpdate(tD);
	}
}