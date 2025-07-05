#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

namespace utils {
	namespace string {

		inline std::string ConvertWideToANSI(const std::wstring& wstr)
		{
			int count = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
			std::string str(count, 0);
			WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
			return str;
		}

		inline std::wstring ConvertAnsiToWide(const std::string& str)
		{
			int count = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), NULL, 0);
			std::wstring wstr(count, 0);
			MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), &wstr[0], count);
			return wstr;
		}

		inline std::string ConvertWideToUtf8(const std::wstring& wstr)
		{
			int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
			std::string str(count, 0);
			WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
			return str;
		}

		inline std::wstring ConvertUtf8ToWide(const std::string& str)
		{
			int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
			std::wstring wstr(count, 0);
			MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstr[0], count);
			return wstr;
		}

		inline void ltrim(std::string& str) {
			str.erase(0, str.find_first_not_of(" \n\r\t"));
		}

		inline void rtrim(std::string& str) {
			str.erase(str.find_last_not_of(" \n\r\t") + 1);
		}

		inline void trim(std::string& str) {
			ltrim(str);
			rtrim(str);
		}

		inline void ltrim(std::wstring& wstr) {
			wstr.erase(0, wstr.find_first_not_of(L" \n\r\t"));
		}

		inline void rtrim(std::wstring& wstr) {
			wstr.erase(wstr.find_last_not_of(L" \n\r\t") + 1);
		}

		inline void trim(std::wstring& wstr) {
			ltrim(wstr);
			rtrim(wstr);
		}
	} // string

	namespace process {
		inline void raise_process_priority(void) {
			SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
		}

		inline void lower_process_priority(void) {
			SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		}
	} // process
} // utils