
#include "stdAfx.hpp"
#include <boost/array.hpp>

CAppModule _Module;

wstring mbstowcs(const string &str) 
{
	size_t len=::mbstowcs(NULL, str.c_str(), str.length());
	boost::scoped_array<wchar_t> buf(new wchar_t[len]);

	len=::mbstowcs(buf.get(), str.c_str(), str.length());
	if(len==static_cast<size_t>(-1)) 
		throw std::runtime_error("mbstowcs(): invalid multi-byte character");

	return wstring(buf.get(), len);
}

string wcstombs(const wstring &str) 
{
	size_t len=::wcstombs(NULL, str.c_str(), 0);
	boost::scoped_array<char> buf(new char[len]);

	len=::wcstombs(buf.get(), str.c_str(), len);
	if(len==static_cast<size_t>(-1)) 
		throw std::runtime_error("wcstombs(): unable to convert character");

	return string(buf.get(), len);
}

wstring GlobalModule::loadResString(UINT uID)
{
	const int buffer_size = 512;
	boost::array<wchar_t, buffer_size> buffer;
	::LoadString(_Module.GetResourceInstance(), uID, buffer.elems, buffer_size);
	
	return wstring(buffer.elems);
}

GlobalModule& globalModule()
{
	static GlobalModule module;
	return module;
}
