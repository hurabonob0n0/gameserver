#include <iostream>
#include <chrono>
#include <thread>
using namespace std;
using namespace chrono;

constexpr int T_SIZE = 100000000;
short rand_arr[T_SIZE];

int main()
{
	for (int i = 0; i < T_SIZE; ++i) rand_arr[i] = rand() - 16384;
	int sum = 0;
	auto start_t = high_resolution_clock::now();
	for (int i = 0; i < T_SIZE; ++i) sum += abs(rand_arr[i]);
	auto du = high_resolution_clock::now() - start_t;
	cout << "[abs] Time " << duration_cast<milliseconds>(du).count() << " ms\n";
	cout << "Result : " << sum << endl;
	sum = 0;
	start_t = high_resolution_clock::now();
	for (int i = 0; i < T_SIZE; ++i) sum += abs2(rand_arr[i]);
	du = high_resolution_clock::now() - start_t;
	cout << "[abs2] Time " << duration_cast<milliseconds>(du).count() << " ms\n";
	cout << "Result : " << sum << endl;
}
