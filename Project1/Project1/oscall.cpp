#include <iostream>
#include <chrono>
#include <thread>
using namespace std;
using namespace chrono;
int main()
{
	volatile long long tmp = 0;
	auto start = high_resolution_clock::now();
	for (int j = 0; j <= 10000000; ++j) {
		tmp += j;
		//this_thread::yield(); //호출하고 안하고 엄청난 차이가 난다. 이 줄이 실행되면 운영체제가 호출되는 거다.
	}
	auto duration = high_resolution_clock::now() - start;
	cout << "Time " << duration_cast<milliseconds>(duration).count();
	cout << " msec\n";
	cout << "RESULT " << tmp << endl;
}
