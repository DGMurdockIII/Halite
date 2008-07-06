
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"
#include "Halite.hpp"

#include "ConfigOptions.hpp"
#include "ProgressDialog.hpp"

void SecurityOptions::onFilterImport(UINT, int, HWND hWnd)
{
	CSSFileDialog dlgOpen(TRUE, NULL, NULL, OFN_HIDEREADONLY, L"eMule ipfilter.dat. (*.dat)|*.dat|", m_hWnd);
	
	if (dlgOpen.DoModal() == IDOK) 
	{
		ProgressDialog progDlg(L"Importing IP filters...", bind(
			&hal::bit::ip_filter_import_dat, &hal::bittorrent(), path(hal::to_utf8(dlgOpen.m_ofn.lpstrFile)), _1, true));
		progDlg.DoModal();
	}
}	

BOOL GeneralOptions::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	lang_list_.Attach(GetDlgItem(HAL_LANGLIST));
	lang_list_.AddString(L"English");
	lang_list_.SetCurSel(0);
	
	if (hal::fs::is_directory(hal::app().exe_path().branch_path()))
	{
		hal::fs::wdirectory_iterator end_iter;
		
		for (hal::fs::wdirectory_iterator dir_itr(hal::app().exe_path().branch_path());
				dir_itr != end_iter; ++dir_itr )
		{
			if (hal::fs::is_regular(dir_itr->status()))
			{
				hal::xp::wsregex rex = hal::xp::wsregex::compile(L"[\\s\\w\\(\\)_-]+\\.[dD][lL]{2}");
				hal::xp::wsmatch what;

				if(hal::xp::regex_match(dir_itr->path().leaf(), what, rex))
				{
					HMODULE hMod = ::LoadLibraryEx(dir_itr->path().string().c_str(), 0, LOAD_LIBRARY_AS_DATAFILE);

					// The upper size limit ain't nice, but at least it's safe from buffer overflow
					const int buffer_size = 512;
					boost::array<wchar_t, buffer_size> buffer;
					int length = ::LoadString(hMod, HALITE_LANGUAGE, buffer.elems, buffer_size);
					
					if (length)
					{
						wstring lang_name(buffer.elems);
						lang_map_[lang_name] = dir_itr->path().leaf();
						int index = lang_list_.AddString(lang_name.c_str());
						
						if (dir_itr->path().leaf() == halite().dll_) 
							lang_list_.SetCurSel(index);
					}
					::FreeLibrary(hMod);
				}
			}
		}
	}
	
	autosizeClass::CtrlsArrange();
	return DoDataExchange(false);
}