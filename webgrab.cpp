#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>
#include <cstdlib>

enum class CommandType {
	Download,
	Quit
};

struct Command {
	CommandType type;
	std::string url;
};

class CommandQueue {
private:
	std::queue<Command> queue_;
	std::mutex mutex_;
	std::condition_variable cv_;
	bool quitting_ = false;

public:
	void push(Command cmd) {
		{
			std::lock_guard<std::mutex> lock(mutex_);
			if (quitting_) return;
			queue_.push(std::move(cmd));
		}
		cv_.notify_one();
	}

	bool pop(Command& cmd) {
		std::unique_lock<std::mutex> lock(mutex_);
		cv_.wait(lock, [&]() { return !queue_.empty() || quitting_; });

		if (queue_.empty())
			return false;

		cmd = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	void shutdown() {
		{
			std::lock_guard<std::mutex> lock(mutex_);
			quitting_ = true;
		}
		cv_.notify_all();
	}
};

std::atomic<bool> running = true;
CommandQueue cmdQueue;

void workerThread(int id) {
	while (running) {
		Command cmd;
		if (!cmdQueue.pop(cmd))
			break;

		if (cmd.type == CommandType::Download) {
			std::cout << "[Worker " << id << "] Downloading: " << cmd.url << std::endl;

			std::string command = "curl -O " + cmd.url;
			int result = std::system(command.c_str());

			if (result != 0)
				std::cerr << "[Worker " << id <<"] Downloading error: " << cmd.url << std::endl;
		}
	}

	std::cout << "[Worker " << id << "] Ending thread.\n";
}

void commandInputLoop() {
	std::string line;
	while (running) {
		std::cout << "> ";
		if (!std::getline(std::cin, line))
			break;

		if (line.starts_with("download ")) {
			std::string url = line.substr(9);
			cmdQueue.push(Command{ CommandType::Download, url });
		}
		else if(line == "quit") {
			running = false;
			cmdQueue.shutdown();
			break;
		}
		else {
			std::cout << "Unknown command.\n";
		}
	}
}

int main() {
	const int numWorkers = 3;
	std::vector<std::thread> workers;

	for (int i = 0; i < numWorkers; ++i) {
		workers.emplace_back(workerThread, i + 1);
	}

	commandInputLoop();

	for (auto& th : workers) {
		if (th.joinable())
			th.join();
	}

	std::cout << "Service terminated.\n";
	return 0;
}