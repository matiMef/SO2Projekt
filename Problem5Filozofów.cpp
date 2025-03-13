#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>

//consant values
constexpr int NUM_PHILOSOPHERS = 100;
constexpr int NUM_PERMITS = (NUM_PHILOSOPHERS % 2 == 0) ? (NUM_PHILOSOPHERS / 2) : (NUM_PHILOSOPHERS / 2 - 1);

std::atomic<bool> running = true;

//mutexes and semaphores
//forks are represented by mutexes
std::vector<std::mutex> forks(NUM_PHILOSOPHERS);
//forks_locked is used to check if forks are locked
std::vector<bool> forks_locked(NUM_PHILOSOPHERS,false);
//eating_permit is used to limit number of philosophers eating at the same time
std::counting_semaphore<NUM_PERMITS> eating_permit(NUM_PERMITS);
//cout_mutex is used to synchronize output
std::mutex cout_mutex;
//forks_locked_mutex is used to synchronize access to forks_locked
std::mutex forks_locked_mutex;

//functions

//acquire_forks is used to acquire forks for philosopher, they only eat if they can acquire both forks, else they think
bool acquire_forks(int id) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> think_time(1, 15); 
    int thinking_duration = think_time(gen);

    eating_permit.acquire();
    int first = std::min(id, (id + 1) % NUM_PHILOSOPHERS);
    int second = std::max(id, (id + 1) % NUM_PHILOSOPHERS);

    if (forks_locked[second]==false && forks_locked[first]==false) {
        {
            std::lock_guard<std::mutex> lock(forks_locked_mutex);
            forks_locked[first] = true;
            forks_locked[second] = true;
        }

        forks[second].lock();
        forks[first].lock();

        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Philosopher " << id << " eats for 5 seconds." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return true;
    }
    else {
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Philosopher " << id << " thinks for " << thinking_duration << " seconds." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(thinking_duration));
        eating_permit.release();
		return false;
    }
}

//put_fork is used to put forks back on the table
//after eathing philosophers start to think
void put_fork(int id) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> think_time(1, 15);
    int thinking_duration = think_time(gen);

    int first = std::min(id, (id + 1) % NUM_PHILOSOPHERS);
    int second = std::max(id, (id + 1) % NUM_PHILOSOPHERS);

    forks[second].unlock();
    forks[first].unlock();

    {
        std::lock_guard<std::mutex> lock(forks_locked_mutex);
        forks_locked[first] = false;
        forks_locked[second] = false;
    }

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Philosopher " << id << " finished eating." << std::endl;
    }
    eating_permit.release();

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Philosopher " << id << " thinks for " << thinking_duration << " seconds." << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(thinking_duration));
}

//philosopher is a function that simulates philosopher's behaviour
//every philosepher thinks for some time and then tries to acquire forks
//if they can acquire both forks they eat for 5 seconds
//if they can't acquire forks they think again
//philosophers run until running is set to false
//after that they finish their actions and simulation ends
//philosophers are identified by their id
void philosopher(int id) {
	//random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> think_time(1, 15);
	
    //philosopher's loop
    while (running) {
        bool result;
        int thinking_duration = think_time(gen);
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Philosopher " << id << " thinks for " << thinking_duration << " seconds." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(thinking_duration));

		//acquire forks
		//if philosopher can't acquire forks they think again
		//if philosopher can acquire forks they eat and then put forks back on the table
        result = acquire_forks(id);
        if (result) {
            put_fork(id);
        }
    }
}

int main() {
	//create philosophers threads
    std::vector<std::thread> philosophers;

	//create threads for philosophers
	//philosophers are identified by their id
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosophers.emplace_back(philosopher, i);
    }

	//run simulation for 60 seconds
    std::this_thread::sleep_for(std::chrono::seconds(60));

	//philosophers finish their actions and simulation ends
    running = false;

	//join all threads
	//wait for all threads to finish
    for (auto& ph : philosophers) {
        if (ph.joinable()) {
            ph.join();
        }
    }

    std::cout << "Simulation completed." << std::endl;
    return 0;
}


