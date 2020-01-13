#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace Columbus
{

	class thread_pool
	{
	private:
		std::vector<std::thread> _workers;
		std::queue<std::function<void()>> _tasks;
		std::mutex _queue_mutex;
		std::condition_variable _condition;
		bool _stop;

		template <typename T>
		void enqueue(T& t) {
			std::unique_lock<std::mutex> l(_queue_mutex);
			_tasks.push(t);
		}

	public:
		thread_pool(size_t threads) : _stop(false) {
			for (size_t i = 0; i < threads; i++)
			{
				_workers.emplace_back([this]() {
					std::function<void()> task;
					while (!_stop) {
						{
							std::unique_lock<std::mutex> lock(_queue_mutex);
							_condition.wait(lock, [this]() { return _stop || !_tasks.empty(); });
							if (_stop && _tasks.empty()) return;
							task = std::move(_tasks.front());
							_tasks.pop();
						}
						task();
					}
				});
			}
		}

		template <typename F, typename...Args>
		auto submit(F&& f, Args&&... args)->std::future<decltype(f(args...))> {
			auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
			auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
			auto wrapper_func = [task_ptr]() {
				(*task_ptr)();
			};
			enqueue(wrapper_func);
			_condition.notify_one();
			return task_ptr->get_future();
		}

		~thread_pool() {
			{
				std::unique_lock<std::mutex> l(_queue_mutex);
				_stop = true;
			}
			_condition.notify_all();
			for (auto& worker : _workers)
				worker.join();
		}
	};

}
