#include <iostream>
#include <chrono>
#include <cstdlib>

#include "sudoku.h"

#define DIAGNOSTICS

int main(int argsc, const char *args[])
{
#ifdef DIAGNOSTICS
	constexpr int reps = 100;

	sudoku boards[reps];
	std::cin >> boards[0];

	std::cout << boards[0].countComplete() << " / 81:\n" << boards[0] << '\n';

	for (int i = 0; i < reps; ++i) boards[i] = boards[0];

	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < reps; ++i) if (!boards[i].solve()) { std::cerr << "SOLVE FAILURE!!\n"; std::abort(); }

	auto duration = std::chrono::high_resolution_clock::now() - start;
	
	std::cout << boards[0].countComplete() << " / 81:\n" << boards[0];

	long long elapsed = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
	long long average = elapsed / reps;

	std::cout << "\nTime Elapsed: " << elapsed << " us";
	std::cout << "\nAverage Time over " << reps << " cycles: " << average << " us\n";
#else

	sudoku board;
	std::cout << "enter board (0 for unknown):\n";
	std::cin >> board;

	std::cout << board.countComplete() << " / 81:\n" << board << std::endl;

	board.solve();

	std::cout << board.countComplete() << " / 81:\n" << board << '\n';

#endif
}