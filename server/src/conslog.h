#pragma once
#pragma warning(disable:4996)

#include <string>
#include <iostream>

#define Seprtr		   Console::Informator::Print(Separator, false);
#define Emptyc	       Console::Informator::Empty();
#define Printc(msg)    Console::Informator::Print(msg)
#define Logc(msg,inf)  Console::Informator::Log(msg, inf)
#define InfoLogc(msg)  Console::Informator::InfoLog(msg)
#define ErrorLogc(msg) Console::Informator::ErrorLog(msg)
#define Success		   InfoLogc("Success.")
#define Separator	   "===================================================================="

namespace Console
{
	class Informator
	{
		public:
			static void Print(std::string msg, bool date = true)
			{
				tm* now = CreateTimeNow();
				std::string datestr = date ? ("[" + GetTimeString(*now) + "]") : "";
				std::cout << datestr 
					      << msg << std::endl;
			}
			static void Log(std::string msg, std::string informator)
			{
				tm* now = CreateTimeNow();
				std::cout << "[" << GetTimeString(*now) << "]"
						  << " " << informator << ": "
						  << msg << std::endl;
			}
			static inline void InfoLog(std::string msg)
			{
				Log(msg,"INFO");
			}
			static inline void ErrorLog(std::string msg)
			{
				Log(msg, "ERROR");
			}
			static inline void Empty()
			{
				std::cout << std::endl;
			}
		private:
			static tm* CreateTimeNow(bool utc = false)
			{
				time_t raw;
				time(&raw);
				tm* now = utc? gmtime(&raw) : localtime(&raw);
				return now;
			}
			static std::string GetTimeString(tm timenow)
			{
				return //GetDayString(timenow.tm_wday) + " " + GetMonthString(timenow.tm_mon) + "; " +
					 std::to_string(timenow.tm_hour) + ":" + std::to_string(timenow.tm_min) + ":" + std::to_string(timenow.tm_sec);
			}
			static std::string GetMonthString(int month)
			{
				month %= 12;
				switch (month)
				{
					case 0: return "Jan";
					case 1: return "Feb";
					case 2: return "Mar";
					case 3: return "Apr";
					case 4: return "May";
					case 5: return "Jun";
					case 6: return "Jul";
					case 7: return "Aug";
					case 8: return "Sep";
					case 9: return "Oct";
					case 10: return "Nov";
					case 11: return "Dec";
				}
			}
			static std::string GetDayString(int day)
			{
				day %= 7;
				switch (day)
				{
					case 0: return "Mon";
					case 1: return "Tue";
					case 2: return "Wed";
					case 3: return "Thu";
					case 4: return "Feb";
					case 5: return "Sat";
					case 6: return "Sun";
				}
			}
	};
}