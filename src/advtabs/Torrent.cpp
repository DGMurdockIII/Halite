
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/array.hpp>

#include "../HaliteWindow.hpp"
#include "../HaliteListViewCtrl.hpp"

#include "Torrent.hpp"

LRESULT AdvTorrentDialog::onInitDialog(HWND, LPARAM)
{
	dlg_base_class_t::InitializeHalDialogBase();	
	
	m_prog.Attach(GetDlgItem(TORRENTPROG));
	m_prog.SetRange(0, 100);	
	
	totalConnections_.Attach(GetDlgItem(HAL_EDITNCD));
	uploadConnections_.Attach(GetDlgItem(HAL_EDITNCU));
	downloadRate_.Attach(GetDlgItem(HAL_EDITTLD));
	uploadRate_.Attach(GetDlgItem(HAL_EDITTLU));
//	ratio_.Attach(GetDlgItem(HAL_EDITRATIO));
	
	totalConnections_ = -1;
	uploadConnections_ = -1;
	downloadRate_ = -1;
	uploadRate_ = -1;	
//	ratio_ = -1;	
	
	DoDataExchange(false);
	return 0;
}

AdvTorrentDialog::CWindowMapStruct* AdvTorrentDialog::GetWindowMap()
{
#define TORRENT_LIMITS_LAYOUT \
	WMB_HEAD(WMB_COL(_exp|20), WMB_COL(_exp|30), WMB_COL(_exp|20), WMB_COL(_exp|30)), \
		WMB_ROW(10,	HAL_TL,	_r, _r, _r), \
		WMB_ROW(11,	HAL_TLD, HAL_EDITTLD, HAL_TLU, HAL_EDITTLU), \
		WMB_ROW(10,	HAL_NC,	_r, _r, _r), \
		WMB_ROW(11,	HAL_NCD, HAL_EDITNCD, HAL_NCU, HAL_EDITNCU), \
		WMB_ROW(11,	HAL_RATIOESTATIC, _r, _r, HAL_EDITRATIO), \
	WMB_END()

#define TORRENT_STATUS_LAYOUT \
	WMB_HEAD(WMB_COL(45), WMB_COLNOMIN(_exp|150), WMB_COL(_eq|0), WMB_COL(_exp|100)), \
		WMB_ROW(10,	HAL_NAME_STATUS_LABEL, HAL_NAME_STATUS, _r, _r), \
		WMB_ROW(10,	HAL_PEERS_LABEL, HAL_PEERS, HAL_SEEDS_LABEL, HAL_SEEDS), \
		WMB_ROW(10,	HAL_TRANSFERED_LABEL, HAL_TRANSFERED, HAL_OVERHEAD_LABEL, HAL_OVERHEAD), \
		WMB_ROW(10,	HAL_REMAINING_LABEL, HAL_REMAINING, HAL_ETA_LABEL, HAL_ETA), \
		WMB_ROW(10,	HAL_RATE_LABEL, HAL_RATE, HAL_RATIO_LABEL, HAL_RATIO), \
	WMB_END()
	
#define TORRENT_REANNOUNCE_LAYOUT \
	WMB_HEAD(WMB_COL(50), WMB_COLNOMIN(_exp)), \
		WMB_ROW(10,	HAL_UPDATESTAT, HAL_UPDATE), \
	WMB_END()	

	BEGIN_WINDOW_MAP_INLINE(AdvTorrentDialog, 6, 6, 3, 3)
		WMB_HEAD(WMB_COL(_gap), WMB_COL(_exp), WMB_COL(120), WMB_COL(_gap)), 
			WMB_ROW(_gap|3,	HAL_GROUP_TORRENT, _r, _r, _r), 
			WMB_ROW(_auto,	_d, TORRENT_STATUS_LAYOUT, TORRENT_LIMITS_LAYOUT), 
			WMB_ROW(_auto,	_d, TORRENTPROG, _r), 
			WMB_ROW(_gap,	_d), 
			WMB_ROW(_gap|3,	HAL_GROUP_TRACKER, _r, _r, _r), 
			WMB_ROW(_auto,	_d, HAL_TRACKER, TORRENT_REANNOUNCE_LAYOUT), 
			WMB_ROW(_gap,	_d), 
		WMB_END() 
	END_WINDOW_MAP_INLINE()	
}

void AdvTorrentDialog::onClose()
{
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

LRESULT AdvTorrentDialog::OnHalEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hal::bit::torrent t = hal::bittorrent().get(focused_torrent()->uuid()))
	{
		t.set_rate_limits(downloadRate_, uploadRate_);
		t.set_connection_limits(totalConnections_, uploadConnections_);
	}
	
	return 0;
}

void AdvTorrentDialog::focusChanged(const hal::torrent_details_ptr pT)
{
	std::pair<float, float> tranLimit(-1.0f, -1.0f);
	std::pair<int, int> connLimit(-1, -1);
//	float ratio = 0;

	if (hal::bit::torrent t = hal::bittorrent().get(focused_torrent()))
	{
		tranLimit = t.rate_limits();
		connLimit = t.connection_limits();
//		ratio = t.ratio;
		
		::EnableWindow(GetDlgItem(HAL_EDITTLD), true);
		::EnableWindow(GetDlgItem(HAL_EDITTLU), true);
		::EnableWindow(GetDlgItem(HAL_EDITNCD), true);
		::EnableWindow(GetDlgItem(HAL_EDITNCU), true);
//		::EnableWindow(GetDlgItem(HAL_EDITRATIO), true);
	}
	else
	{
		SetDlgItemText(HAL_NAME_STATUS, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_PEERS, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_SEEDS, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_TRANSFERED, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_OVERHEAD, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_REMAINING, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_ETA, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_RATE, hal::app().res_wstr(HAL_NA).c_str());
//		SetDlgItemText(HAL_RATIO, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_TRACKER, hal::app().res_wstr(HAL_NA).c_str());
		SetDlgItemText(HAL_UPDATE, hal::app().res_wstr(HAL_NA).c_str());
		
		m_prog.SetPos(0);
		
		::EnableWindow(GetDlgItem(HAL_EDITTLD), false);
		::EnableWindow(GetDlgItem(HAL_EDITTLU), false);
		::EnableWindow(GetDlgItem(HAL_EDITNCD), false);
		::EnableWindow(GetDlgItem(HAL_EDITNCU), false);
//		::EnableWindow(GetDlgItem(HAL_EDITRATIO), false);
	}
	
	totalConnections_ = connLimit.first;
	uploadConnections_ = connLimit.second;
	downloadRate_ = tranLimit.first;
	uploadRate_ = tranLimit.second;
//	ratio_ = ratio;

	DoDataExchange(false);	
}

void AdvTorrentDialog::uiUpdate(const hal::torrent_details_manager& tD)
{	
	if (hal::torrent_details_ptr torrent = tD.focused_torrent()) 	
	{			
		uiUpdateSingle(torrent);	
	}
}

void AdvTorrentDialog::uiUpdateSingle(const hal::torrent_details_ptr& torrent)
{	
	if (torrent) 	
	{
/*		HAL_NAME_STATUS		"Name: %1%, %2%."
		HAL_SECOND				"Peers %1% (%2%).		Seeds %3% (%4%)."
		HAL_TRANSFERED			"Transfered (Overhead): %1$.2fMB (%2$.2fMB) Down, %3$.2fMB (%4$.2fMB) Up."
		HAL_REMAINING			"Remaining: %1$.2fMB of %2$.2fMB, ETA %3%."
		HAL_RATE				"Downloading at %1$.2fkb/s, Uploading at %2$.2fkb/s, Ratio %3$.2f."
*/	
		SetDlgItemInfo(HAL_NAME_STATUS, 
			hal::wform(hal::app().res_wstr(HAL_NAME_STATUS)) 
				% torrent->name()
				% torrent->state());

		SetDlgItemInfo(HAL_PEERS,
			hal::wform(L"%1% (%2%)")
				% torrent->peers_connected()
				% torrent->peers());

		SetDlgItemInfo(HAL_SEEDS,
			hal::wform(L"%1% (%2%)")
				% torrent->seeds_connected()
				% torrent->seeds());

		SetDlgItemInfo(HAL_TRANSFERED,
			hal::wform(hal::app().res_wstr(HAL_TRANSFERED)) 
				% (static_cast<float>(torrent->total_payload_downloaded())/(1024*1024))
				% (static_cast<float>(torrent->total_payload_uploaded())/(1024*1024)));

		SetDlgItemInfo(HAL_OVERHEAD,
			hal::wform(L"%1$.2fMB - %2$.2fMB") 
				% (static_cast<float>(torrent->total_downloaded() - torrent->total_payload_downloaded())/(1024*1024))
				% (static_cast<float>(torrent->total_uploaded() - torrent->total_payload_uploaded())/(1024*1024)));

		SetDlgItemInfo(HAL_REMAINING,
			hal::wform(hal::app().res_wstr(HAL_REMAINING))
				% (static_cast<float>(torrent->total_wanted()-torrent->total_wanted_done())/(1024*1024))
				% (static_cast<float>(torrent->total_wanted())/(1024*1024)));
		
		wstring eta = hal::app().res_wstr(HAL_INF);	
		if (!torrent->estimated_time_left().is_special())
			eta = hal::from_utf8(boost::posix_time::to_simple_string(torrent->estimated_time_left()));
		
		SetDlgItemInfo(HAL_ETA, eta);
			
		SetDlgItemInfo(HAL_RATE,
			hal::wform(hal::app().res_wstr(HAL_RATE))
				% (torrent->speed().first/1024)
				% (torrent->speed().second/1024));
				
		float ratio = (torrent->total_payload_downloaded()) 
			? static_cast<float>(torrent->total_payload_uploaded())
				/ static_cast<float>(torrent->total_payload_downloaded())
			: 0;
		
		SetDlgItemInfo(HAL_RATIO, 
			hal::wform(L"%1$.2f") % ratio);		
		
		m_prog.SetPos(static_cast<int>(torrent->completion()*100));	
		
		SetDlgItemText(HAL_TRACKER, torrent->current_tracker().c_str());
		
		if (!torrent->update_tracker_in().is_special())
		{
			SetDlgItemText(HAL_UPDATE,	
				(hal::from_utf8(boost::posix_time::to_simple_string(torrent->update_tracker_in())).c_str()));
		}
		else SetDlgItemText(HAL_UPDATE, hal::app().res_wstr(HAL_NA).c_str());		
	}
}

void AdvTorrentDialog::uiUpdateMultiple(const hal::torrent_details_vec& torrents)
{}

void AdvTorrentDialog::uiUpdateNone()
{}
