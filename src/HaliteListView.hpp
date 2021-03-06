
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HAL_MAIN_LISTVIEW_BEGIN	 		30000
#define ID_LVM_DOWNLOAD_FOLDER          HAL_MAIN_LISTVIEW_BEGIN + 1
#define ID_LVM_RECHECK					HAL_MAIN_LISTVIEW_BEGIN + 2
#define ID_LVM_EDIT_FOLDERS				HAL_MAIN_LISTVIEW_BEGIN + 3
#define HAL_LISTVIEW_CONFIRMDELETE		HAL_MAIN_LISTVIEW_BEGIN + 4
#define HAL_LVM_QUEUE_VIEW				HAL_MAIN_LISTVIEW_BEGIN + 5
#define HAL_AUTO_MANAGED				HAL_MAIN_LISTVIEW_BEGIN + 6
#define HAL_UNMANAGED					HAL_MAIN_LISTVIEW_BEGIN + 7
#define HAL_MANAGED_DOWNLOADING		HAL_MAIN_LISTVIEW_BEGIN + 8
#define HAL_MANAGED_SEEDING			HAL_MAIN_LISTVIEW_BEGIN + 9
#define HAL_QUEUE_MOVE_TOP				HAL_MAIN_LISTVIEW_BEGIN + 10
#define HAL_QUEUE_MOVE_UP				HAL_MAIN_LISTVIEW_BEGIN + 11
#define HAL_QUEUE_MOVE_DOWN			HAL_MAIN_LISTVIEW_BEGIN + 12
#define HAL_QUEUE_MOVE_BOTTOM			HAL_MAIN_LISTVIEW_BEGIN + 13
#define ID_LVM_TOGGLE_SUPERSEEDING		HAL_MAIN_LISTVIEW_BEGIN + 14


#ifndef RC_INVOKED

#include "Halite.hpp"

#include <boost/array.hpp>

#include "global/string_conv.hpp"

#include "halIni.hpp"
#include "HaliteListViewCtrl.hpp"
#include "HaliteSortListViewCtrl.hpp"

class HaliteWindow;

template<typename Iter>
hal::uuid item_hash(Iter& i)
{
	std::wstringstream ss(i->text(hal::torrent_details::uuid_e-hal::torrent_details::name_e));
	hal::uuid id;

	ss >> id;

	return id;
}	

class HaliteListViewCtrl :
	public CHaliteSortListViewCtrl<HaliteListViewCtrl, hal::uuid>,
	public WTL::CCustomDraw<HaliteListViewCtrl>,
	private hal::IniBase<HaliteListViewCtrl>,
	private boost::noncopyable
{
public:
	typedef HaliteListViewCtrl this_class_t;
	typedef hal::IniBase<this_class_t> ini_class_t;
	typedef CHaliteSortListViewCtrl<this_class_t, hal::uuid> list_class_t;
	typedef WTL::CCustomDraw<this_class_t> ownDrawClass;

	friend class list_class_t;
	
	class ex_list_value_type : public list_class_t::list_value_type
	{
	public:
		ex_list_value_type(const list_class_t::list_value_type& l);

		hal::uuid hash() const;	
	};

	typedef ex_list_value_type list_value_type;

public:
	enum { 
		LISTVIEW_ID_MENU = HAL_LISTVIEW_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_LISTVIEW_COLUMNS,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_LISTVIEW_DEFAULTS
	};

	HaliteListViewCtrl(HaliteWindow& HalWindow);

	BEGIN_MSG_MAP_EX(HaliteListViewCtrl)
		try
		{
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER(ID__LVM_PAUSE, OnPause)
		COMMAND_ID_HANDLER(ID_LVM_STOP, OnStop)
		COMMAND_ID_HANDLER(ID_LVM_RESUME, OnResume)
		COMMAND_ID_HANDLER(ID_LVM_RECHECK, OnRecheck)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_T, OnRemove)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_TD, OnRemoveWipeFiles)
		COMMAND_ID_HANDLER(ID_LVM_DOWNLOAD_FOLDER, OnDownloadFolder)
		COMMAND_ID_HANDLER(ID_LVM_EDIT_FOLDERS, OnEditFolders)
		COMMAND_ID_HANDLER(ID_LVM_TOGGLE_SUPERSEEDING, OnToggleSuperseeding)

		COMMAND_ID_HANDLER(HAL_AUTO_MANAGED, OnSetManaged)
		COMMAND_ID_HANDLER(HAL_UNMANAGED, OnSetUnmanaged)
		COMMAND_RANGE_HANDLER(HAL_QUEUE_MOVE_TOP, HAL_QUEUE_MOVE_BOTTOM, OnAdjustQueuePosition)
		COMMAND_ID_HANDLER(HAL_LVM_QUEUE_VIEW, OnQueueView)

		REFLECTED_NOTIFY_CODE_HANDLER(SLVN_SORTCHANGED, OnSortChanged)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
//		REFLECTED_NOTIFY_CODE_HANDLER(LVN_BEGINLABELEDIT, OnBeginLabelEdit)
//		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ENDLABELEDIT, OnEndLabelEdit)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in HaliteWindow MSG_MAP")

	CHAIN_MSG_MAP_ALT(ownDrawClass, 1)
		CHAIN_MSG_MAP(list_class_t)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void OnShowWindow(UINT, INT);
	void OnDestroy();
	void SaveSettings();
	void uiUpdate(const hal::torrent_details_manager& allTorrents); 

	DWORD OnPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCD);
	DWORD OnItemPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCD);

	LRESULT OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRecheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveFocused(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDownloadFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditFolders(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnToggleSuperseeding(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnSetManaged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSetUnmanaged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAdjustQueuePosition(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnQueueView(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnGetDispInfo(int, LPNMHDR pnmh, BOOL&);
	LRESULT OnSortChanged(int, LPNMHDR pnmh, BOOL&);
	LRESULT OnBeginLabelEdit(int i, LPNMHDR pnmh, BOOL&);
	LRESULT OnEndLabelEdit(int i, LPNMHDR pnmh, BOOL&);

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 4:			
			ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<list_class_t>(*this));
			ar & make_nvp("queue_view", queue_view_);
		
		break;
			
		case 3:
		case 2:
		case 1:
		default:
			assert(false);
		}		
	}

	void erase_torrent_name(const hal::uuid& id);
	bool sort_list_comparison(list_class_data_t l, list_class_data_t r, size_t index, bool ascending);
	void remove_to_bin(const hal::uuid& id, hal::fs::wpath root, boost::shared_ptr<std::vector<std::wstring> > files);

private:
	bool editing_lock_;
	void queue_view_mode();
	bool queue_view_;
	
	enum { NumberOfColumns_s = 25 };
	std::vector<const hal::torrent_details_ptr> data_elements_;
	
	HaliteWindow& halite_window_;
};

BOOST_CLASS_VERSION(HaliteListViewCtrl, 4)

typedef HaliteListViewCtrl::SelectionManager ListViewManager;

#endif // RC_INVOKED
