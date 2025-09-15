#include "Logger.h"
#include <iostream>
#include <chrono>
#include <format>

Logger::Logger()
{
	m_thread = std::thread{ &Logger::processEntries,this };
}

Logger::~Logger()
{
	{
		std::unique_lock lock{ m_mutex };
		m_exit = true;
	}
	m_condVar.notify_all();
	m_thread.join();
}


void Logger::log(std::string entry)
{
	// Lock mutex and add entry to the queue
	std::unique_lock lock{ m_mutex };
	m_queue.push(std::move(entry));
	// Notify condition variable to wake up thread
	m_condVar.notify_all();
}

void Logger::processEntries()
{
	auto time = std::chrono::system_clock::now();
	std::chrono::zoned_time zone_time{ std::chrono::locate_zone("UTC"),std::chrono::clock_cast<std::chrono::system_clock>(time) };
	std::string time_str = std::format("{0:%Y}_{0:%m}_{0:%d}_{0:%OH}_{0:%M}_{0:%S}", zone_time.get_sys_time() + std::chrono::hours(8));
	// Open log file
	std::ofstream logFile{ "Log/log_" + time_str + ".txt"};
	if (logFile.fail())
	{
		std::cerr << "Failed to open logfile.\n"<<std::endl;
		return;
	}

	// Create a lock for m_mutex, but do not yet acquire a lock on it
	std::unique_lock lock{ m_mutex,std::defer_lock };
	// Start processing loop
	while (true)
	{
		lock.lock();

		if (!m_exit)
		{
			// wait for a notification if we don't have to exit
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			m_condVar.wait(lock);
		}
		else
		{
			processEntriesHelper(m_queue, logFile);
			break;
		}

		std::queue < std::string > localQueue;
		localQueue.swap(m_queue);

		lock.unlock();

		processEntriesHelper(localQueue, logFile);
	}
}

void Logger::processEntriesHelper(std::queue<std::string>& queue, std::ofstream& ofs) const
{
	while (!queue.empty())
	{
		ofs << queue.front() << std::endl;
		queue.pop();
	}
}

void loggerMessge(Logger& logger,const std::string& message)
{
	auto time = std::chrono::system_clock::now();
	std::chrono::zoned_time zone_time{std::chrono::locate_zone("UTC"),std::chrono::clock_cast<std::chrono::system_clock>(time)};
	std::string time_str = std::format("{0:%Y}-{0:%m}-{0:%d} {0:%OH}:{0:%M}:{0:%S}", zone_time.get_sys_time() + std::chrono::hours(8));
	logger.log(time_str + " : " + message);
}
