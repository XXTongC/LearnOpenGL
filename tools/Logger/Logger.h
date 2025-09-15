#pragma once
#include <mutex>
#include <queue>
#include <string>
#include <fstream>

class Logger
{
public:
	Logger();
	Logger(const Logger& src) = delete;
	Logger& operator=(const Logger& rhs) = delete;
	void log(std::string entry);
	virtual ~Logger();
private:
	void processEntries();
	void processEntriesHelper(std::queue<std::string>& queue, std::ofstream& ofs) const;
	std::mutex m_mutex;
	std::condition_variable m_condVar;
	std::queue<std::string> m_queue;
	std::thread m_thread;
	bool m_exit{ false };
};

void loggerMessge(Logger& logger,const std::string& message);
