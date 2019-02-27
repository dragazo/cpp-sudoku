#ifndef SUDOKU_H
#define SUDOKU_H

#include <iostream>
#include <array>
#include <algorithm>
#include <type_traits>
#include <cstdint>

typedef std::int_fast8_t tile_t; // type to use for representing tile values

class sudoku
{
private: // -- data -- //

	std::array<tile_t, 81> data; // the board data - flattened 9x9 array

public: // -- ctor / dtor / asgn -- //

	sudoku() : data{} {}

	sudoku(const sudoku&) = default;
	sudoku &operator=(const sudoku&) = default;

public: // -- board access -- //

	// gets the value at the specified position.
	tile_t& operator()(int row, int col) { return data[row * 9 + col]; }
	tile_t operator()(int row, int col) const { return data[row * 9 + col]; }

	// sets every tile to zero (blank).
	void clear() { std::fill(data.begin(), data.end(), (tile_t)0); }

public: // -- query -- //

	// counts the number of non-blank tiles.
	auto countComplete() const { return std::count_if(data.begin(), data.end(), [](tile_t v) { return v != (tile_t)0; }); }
	// counts the number of blank tiles.
	auto countIncomplete() const { return std::extent<decltype(data)>::value - countComplete(); }

	// returns true iff every position in each board is the same.
	bool strictEquals(const sudoku &other) const { return data == other.data; }
	// returns true iff every position in each board is the same (but ignores blank tiles in either board).
	bool fuzzyEquals(const sudoku &other) const
	{
		return std::mismatch(data.begin(), data.end(), other.data.begin(), [](tile_t a, tile_t b) { return a == (tile_t)0 || b == (tile_t)0 || a == b; }).first == data.end();
	}

	// returns true if the board contains no blank tiles
	bool filled() const { return std::none_of(data.begin(), data.end(), [](tile_t v) { return v == (tile_t)0; }); }
	
public: // -- solving -- //

	// returns true iff the board in its current state is valid
	bool valid() const;

	// attempts to solve the board - returns true if successful
	bool solve();

public: // -- io functions -- //

	friend std::ostream& operator<<(std::ostream &ostr, const sudoku &board);
	friend std::istream& operator>>(std::istream &ostr, sudoku &board);
};

#endif
