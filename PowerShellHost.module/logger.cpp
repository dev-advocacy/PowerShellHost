#include "pch.h"
#include "logger.h"


// Define application-specific severity levels
enum severity_level
{
	normal,
	warning,
	error,
	max
};

static std::array<std::wstring, severity_level::max> _SeverityLevelText =
{
	L"normal",
	L"warning",
	L"error"
};

severity_level GetSeverityLevel(const std::wstring& key)
{
	// Do not log errors when retrieving the configuration item, since we are initializing the logger...
	std::wstring logFileLevel = L"normal";

	for (size_t pos = 0; pos < _SeverityLevelText.size(); pos++)
	{	// Parse configuration item.
		if (_wcsicmp(_SeverityLevelText[pos].c_str(), logFileLevel.c_str()) == 0)
		{	// Found the matching severity level.
			return static_cast<severity_level>(pos);
		}
	}

	return severity_level::error; // Return default if config file entry is missing or incorrect.
}


BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", logging::trivial::severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(processid, "ProcessID", logging::attributes::current_process_id::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(thread_id, "ThreadID", attrs::current_thread_id::value_type)


/// <summary>
/// Set the logging framework
///          debug mode   : output to the console application, Visual Studio OutputWindow, Text file, path = %temp%\HostPowerShell
/// 		 release mode :	output to the console application, output to eventlog, Text file, path = %temp%\HostPowerShell
/// </summary>
/// <param name="path">the log path</param>
/// <returns>error is it failed</returns>
std::error_code  GetLog(fs::path& pFileName)
{
	std::wstring		pFileNameNoEx;
	wchar_t				buffer[MAX_PATH * sizeof(wchar_t)];
	wchar_t				wszPath[MAX_PATH * sizeof(wchar_t)];
	std::error_code		error;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, wszPath)))
	{
		GetModuleFileName(NULL, buffer, MAX_PATH);
		std::wstring::size_type pos = std::wstring(buffer).find_last_of(_T("\\/"));
		std::wstring::size_type pos1 = std::wstring(buffer).find_last_of(_T("."));

		if (pos != 0 && pos1 != 0 && pos1 > pos)
		{
			pFileName = wszPath;
			pFileNameNoEx = std::wstring(buffer).substr(pos + 1, pos1 - pos - 1);
			pFileName.append(COMPFOLDER);
			fs::path appPath = pFileName;
			pFileName.append(pFileNameNoEx);

			if (!fs::is_directory(pFileName) && !fs::is_directory(appPath))
			{
				if (!fs::create_directory(appPath))
				{
					return(std::error_code(GetLastError(), std::system_category()));
				}
				if (!fs::create_directory(pFileName))
				{
					return(std::error_code(GetLastError(), std::system_category()));
				}
			}
			pFileName.append(pFileNameNoEx);
		}
		else
		{
			error = std::error_code(ERROR_PATH_NOT_FOUND, std::system_category());
		}
	}
	else
	{
		error = std::error_code(ERROR_PATH_NOT_FOUND, std::system_category());
	}
	return error;
}

/// <summary>
/// Boosts the log global logger initialize.
/// </summary>
/// <param name="">The .</param>
/// <param name="">The .</param>
/// <returns></returns>
BOOST_LOG_GLOBAL_LOGGER_INIT(logger, src::severity_logger_mt)
{
	src::severity_logger_mt<boost::log::trivial::severity_level> logger;
	fs::path				pFileName;

	// add attributes
	logger.add_attribute("LineID", attrs::counter<unsigned int>(1));				// lines are sequentially numbered
	logger.add_attribute("TimeStamp", attrs::local_clock());						// each log line gets a timestamp

	typedef sinks::synchronous_sink<sinks::text_file_backend> TextSink;						// file text sink typedef
	typedef sinks::synchronous_sink<sinks::debug_output_backend> outputdebugstring_sink;	// OutPutDebugString typedef

	typedef logging::ipc::reliable_message_queue queue_t;
	typedef sinks::text_ipc_message_queue_backend< queue_t > backend_t;
	typedef sinks::synchronous_sink< backend_t > sink_t;

	if (!GetLog(pFileName))
	{
		boost::shared_ptr<sinks::text_file_backend> filetextbackend = boost::make_shared<sinks::text_file_backend>(
			keywords::file_name = pFileName.generic_string() + "sign_%Y-%m-%d_%H-%M-%S.%N.log",
			keywords::rotation_size = 10 * 1024 * 1024,
			keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
			keywords::min_free_space = 30 * 1024 * 1024);

		logging::formatter formatter = expr::stream
			<< std::setw(7) << std::setfill('0') << line_id << std::setfill(' ') << " | "
			<< expr::format_date_time(timestamp, "%Y-%m-%d, %H:%M:%S.%f") << " "
			<< "[" << logging::trivial::severity << "]"
			<< " - " << expr::smessage;

		logging::formatter formatterout = expr::stream
			<< std::setw(7) << std::setfill('0') << line_id << std::setfill(' ') << " | "
			<< expr::format_date_time(timestamp, "%Y-%m-%d, %H:%M:%S.%f") << " "
			<< "[" << logging::trivial::severity << "]"
			<< " - " << expr::smessage << std::endl;

		logging::formatter formatterconsole = expr::stream
			<< expr::smessage << std::endl;

		// Create a sink that is associated with the interprocess message queue
		// named "ipc_message_queue".
		boost::shared_ptr< sink_t > sink = boost::make_shared< sink_t >
		(
				keywords::name = logging::ipc::object_name(logging::ipc::object_name::user, "WebView_ipc_message_queue"),
				keywords::open_mode = logging::open_mode::open_or_create,
				keywords::capacity = 256,
				keywords::block_size = 1024,
				keywords::overflow_policy = queue_t::fail_on_overflow
		);

		// Set the formatter
		sink->set_formatter(expr::stream << "[" << timestamp << "] [" << processid << ":" << thread_id << "] " << expr::smessage);

		logging::core::get()->add_sink(sink);
		// Add the commonly used attributes, including TimeStamp, ProcessID and ThreadID
		logging::add_common_attributes();

		filetextbackend->auto_flush(true);

		// create the file sink
		boost::shared_ptr<TextSink> filetextsink(new TextSink(filetextbackend));
		filetextsink->set_formatter(formatter);
		logging::core::get()->add_sink(filetextsink);


//#if _DEBUG	
		boost::shared_ptr<outputdebugstring_sink> output_sink(new outputdebugstring_sink());
		output_sink->set_formatter(formatterout);
		logging::core::get()->add_sink(output_sink);
//#endif // _DEBUG

		auto consoleSink = logging::add_console_log(std::cout);
		consoleSink->set_formatter(formatterconsole);



//#ifndef _DEBUG // we tracing only error and fatal on release mode
//		consoleSink->set_filter(severity >= severity_level::error);
//		filetextsink->set_filter(severity >= GetSeverityLevel(L"logfilelevel"));
//#endif // DEBUG

	}
	return logger;
}
