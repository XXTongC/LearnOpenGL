#include "Logger.h"
#include <iostream>

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
	// Open log file
	std::ofstream logFile{ "log.txt" };
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