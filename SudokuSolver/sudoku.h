#ifndef SUDOKU_H
#define SUDOKU_H

#include <iostream>

typedef short tile_t;

class Sudoku
{
private:
	tile_t *data;

public:
	// ---------------------------------------------------
	// construction / destruction / assignment
	// ---------------------------------------------------

	Sudoku();
	~Sudoku();

	Sudoku(const Sudoku &other);
	Sudoku(Sudoku &&other);

	Sudoku& operator=(const Sudoku &other);
	Sudoku& operator=(Sudoku &&other);

	// ---------------------------------------------------
	// access methods
	// ---------------------------------------------------

	// gets a reference to the tile at the specified row and column
	tile_t& operator()(int row, int col);
	// gets the value of the tile at the specified row and column
	tile_t operator()(int row, int col) const;

	// counts the number of non-blank tiles
	int countComplete() const;
	// counts the number of blank tiles
	int countIncomplete() const;

	// sets every tile to zero (blank)
	void clear();

	// returns true iff every tile in other is the same as in this board
	bool strictEquals(const Sudoku &other) const;
	// returns true iff every non-blank tile pair in other and this board is the same
	bool fuzzyEquals(const Sudoku &other) const;

	// returns true if the board contains no blank tiles
	bool completed() const;
	// returns true iff the board in its current state is valid
	bool valid() const;

	// ---------------------------------------------------
	// solving methods
	// ---------------------------------------------------

	// attempts to solve the board - returns true if successful
	// setting deep to true applies the recursive solver (much slower)
	bool solve(bool deep);

	// ---------------------------------------------------
	// io functions
	// ---------------------------------------------------

	friend std::ostream& operator<<(std::ostream &ostr, const Sudoku &board);
	friend std::istream& operator>>(std::istream &ostr, Sudoku &board);
};

#endif
