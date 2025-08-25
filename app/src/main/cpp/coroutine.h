//
// Created by admin on 2025/7/31.
//

#include <iostream>
#include <coroutine>
#include <thread>
#include <chrono>
#include "log.h"

// 模拟一个异步操作
struct task {
    struct promise_type; // 前向声明 promise_type
    using handle_type = std::coroutine_handle<promise_type>;

    task(handle_type h) : coro(h) {}
    ~task() { coro.destroy(); }

    handle_type coro;
};

// promise_type 用于定义协程的返回值类型和生命周期管理
struct task::promise_type {
    task get_return_object() {
        return task{handle_type::from_promise(*this)};
    }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {} // 协程无返回值
    void unhandled_exception() {
        std::exit(1); // 异常处理
    }
};

// 协程函数
task example() {
    LOGI("协程开始");
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟异步操作
    co_return;
}

int test() {
    auto t = example(); // 调用协程
    t.coro.resume();
    // 协程执行到此会暂停，等待返回
    std::this_thread::sleep_for(std::chrono::seconds(2)); // 等待协程结束
    LOGI("结束执行协程");
    return 0;
}

