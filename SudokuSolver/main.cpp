#include <iostream>
#include <chrono>

#include "sudoku.h"

//#define DIAGNOSTICS

int main(int argsc, const char *args[])
{
#ifdef DIAGNOSTICS
	using namespace std::chrono;
	constexpr int reps(2000);

	Sudoku boards[reps];
	std::cin >> boards[0];

	std::cout << boards[0].countComplete() << " / 81:\n" << boards[0] << '\n';

	int deep(argsc == 2 && strcmp(args[1], "deep") == 0);

	for (int i(1); i < reps; ++i) boards[i] = boards[0];

	high_resolution_clock::time_point start(high_resolution_clock::now());

	for (int i(0); i < reps; ++i)
		boards[i].solve(deep);

	high_resolution_clock::duration duration(high_resolution_clock::now() - start);
	
	std::cout << boards[0].countComplete() << " / 81:\n" << boards[0];

	long long elapsed(duration_cast<microseconds>(duration).count());
	long long average(elapsed / reps);

	std::cout << "\nTime Elapsed: " << elapsed << " us";
	std::cout << "\nAverage Time over " << reps << " cycles: " << average << " us";
#else

	sudoku board;
	std::cout << "enter board (0 for unknown):\n";
	std::cin >> board;

	std::cout << board.countComplete() << " / 81:\n" << board << std::endl;

	board.solve(argsc == 2 && strcmp(args[1], "deep") == 0);

	std::cout << board.countComplete() << " / 81:\n" << board;

#endif
}