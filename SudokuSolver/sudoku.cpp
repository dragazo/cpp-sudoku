#include <iostream>
#include <iomanip>
#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstddef>

#include "Sudoku.h"

// ----------- //

// -- query -- //

// ----------- //

bool sudoku::valid() const
{
	// for each board position
	for (std::size_t row = 0; row < 9; ++row) for (std::size_t col = 0; col < 9; ++col)
	{
		// for each number
		for (tile_t mark = 1; mark <= 9; ++mark)
		{
			// count occurrences in this row
			std::size_t count = 0;
			for (std::size_t i = 0; i < 9; ++i) if ((*this)(row, i) == mark) ++count;
			if (count > 1) return false; // if there's more than 1 it's illegal

			// count occurrences in this column
			count = 0;
			for (std::size_t i = 0; i < 9; ++i) if ((*this)(i, col) == mark) ++count;
			if (count > 1) return false; // if there's more than 1 it's illegal

			// ount occurrences in this block
			count = 0;
			std::size_t blockRow = row / 3;
			std::size_t blockCol = col / 3;
			for (std::size_t _r = 0; _r < 3; ++_r) for (std::size_t _c = 0; _c < 3; ++_c) if ((*this)(blockRow * 3 + _r, blockCol * 3 + _c) == mark) ++count;
			if (count > 1) return false; // if there's more than 1 it's illegal
		}
	}

	return true;
}

// ---------------------- //

// -- basic note logic -- //

// ---------------------- //

// type to use for keeping notes
typedef std::bitset<9 + 1> notes_t;

// type to use for keeping notes for the entire board
typedef notes_t boardnotes_t[9][9];

// returns true iff the given note is valid for the specified position using the most basic rules of validity.
bool notevalid(const sudoku &board, std::size_t row, std::size_t col, tile_t note)
{
	// iterate over each row and column of current position
	// if any equal potential note, potential note is invalid
	for (std::size_t i = 0; i < 9; ++i)
		if (board(i, col) == note || board(row, i) == note) return false;

	// get the row and column of the 3x3 block we're in
	std::size_t blockRow = row / 3;
	std::size_t blockCol = col / 3;

	// iterate over each position in the block
	// if any equal potential note, potential note is invalid
	for (std::size_t _row = 0; _row < 3; ++_row) for (std::size_t _col = 0; _col < 3; ++_col)
		if (board(blockRow * 3 + _row, blockCol * 3 + _col) == note) return false;

	// otherwise, note is valid under basic rules
	return true;
}
// populates notes_out with all the valid notes at the specified row and column.
// returns false if an error is found in the board (i.e. false implies the board invalid.
bool getnotes(const sudoku &board, std::size_t row, std::size_t col, notes_t &notes_out)
{
	// clear out the notes
	notes_out.reset();

	// if tile is blank, generate the notes list
	if (board(row, col) == 0)
	{
		// set all the flags for valid values
		for (tile_t sub = 1; sub <= 9; ++sub)
			if (notevalid(board, row, col, sub)) notes_out.set(sub);

		// if this tile has no valid values, fail
		if (notes_out.none()) return false;
	}
	// otherwise it has one note - its actual value
	else notes_out.set(board(row, col));

	// no errors
	return true;
}
// calls getnotes() for every position in the board and stores the results in boardnotes_out.
// returns false if an error is found in the board (i.e. false implies the board is invalid).
bool getnotes_all(const sudoku &board, boardnotes_t boardnotes_out)
{
	// get the notes for every position on the board - if any of them fail the whole thing fails
	for (std::size_t row = 0; row < 9; ++row) for (std::size_t col = 0; col < 9; ++col)
		if (!getnotes(board, row, col, boardnotes_out[row][col])) return false;

	// no errors
	return true;
}

// --------------------------------------- //

// -- advanced note reduction utilities -- //

// --------------------------------------- //

void print_orthogonality(boardnotes_t notes)
{
	for (std::size_t i = 0; i < 9; ++i)
	{
		for (std::size_t j = 0; j < 9; ++j) std::cout << notes[i][j].count() << ' ';
		std::cout << '\n';
	}
}
void print_group(notes_t *group[9])
{
	for (std::size_t i = 0; i < 9; ++i) std::cout << std::hex << group[i]->to_ulong() << std::dec << '\n';
}

// applies tuple reduction logic to the specified group.
// group - an array of pointers to the 9 notes objects that represent the group (i.e. row, col, or block).
// did_something - output flag that is set to true if the call to this function makes a change.
// returns false if the reduction results in an invalid board state (e.g. tile with no valid values).
bool tuple_reduce(notes_t *group[9], bool &did_something)
{
	// for every combination of 1 to 8 group members
	for (std::size_t combination = (1 << 9) - 2; combination > 0; --combination)
	{
		notes_t notes_union;  // the union of notes in this combination
		std::size_t member_count = 0; // the number of group members in this combination

		// for each group member in this combination
		for (std::size_t i = 0; i < 9; ++i) if ((combination >> i) & 1)
		{
			notes_union |= *group[i]; // add this member's notes to the notes union
			++member_count; // inc the number of members in this combination
		}

		// if the notes union has the same number of possibles as there are members in this combination, it's a tuple
		if (notes_union.count() == member_count)
		{
			// now we know it's a tuple, so we can eliminate the notes union choices from all group members not in this combination
			for (std::size_t i = 0; i < 9; ++i) if (!((combination >> i) & 1))
			{
				// update the did something flag - becomes true if this position has anything in common with the notes union
				did_something = did_something || (*group[i] & notes_union).any();

				// eliminate the notes union from this position - if the result has no valid values, this is an invalid board state - fail
				if ((*group[i] &= ~notes_union).none()) return false;
			}
		}
	}

	// no errors
	return true;
}

// applies unique note reduction logic to the specified group - returns true iff a change was made.
// group - an array of pointers to the 9 notes objects that represent the group (i.e. row, col, or block).
// did_something - output flag that is set to true if the call to this function makes a change.
// returns false if the reduction results in an invalid board state (e.g. tile with no valid values).
bool unique_reduce(notes_t *group[9], bool &did_something)
{
	// for each possible note value
	for (std::size_t note = 1; note <= 9; ++note)
	{
		std::size_t first, second; // first and second

		// find the first tile in the group that has it
		for (first = 0; first < 9 && !group[first]->test(note); ++first);

		// if first does not exist then no tile in this group can be legally be note - ie. invalid board
		if (first >= 9) return false;

		// otherwise, find the second tile in the group that has it
		for (second = first + 1; second < 9 && !group[second]->test(note); ++second);

		// if second does not exist that means first is the only one that can be note
		if (first < 9 && second >= 9)
		{
			// set this as the value for first
			group[first]->reset();
			group[first]->set(note);
		}
	}

	// no errors
	return true;
}
// applies reduction logic to the specified notes group.
// group - an array of pointers to the 9 notes objects that represents the group (i.e. row, col, or block) (order doesn't matter).
// did_something - output flag that is set to true if the call to this function makes a change.
// returns false if the reduction results in an invalid board state (e.g. tile with no valid values).
bool reduce(notes_t *group[9], bool &did_something)
{
	// perform all types of supported reductions - any failure is an error for us as well (i.e. invalid board state).
	return tuple_reduce(group, did_something) && unique_reduce(group, did_something);
}

// applies group reduction to the entire board.
// boardNotes - the (already filled out) notes array for the entire board.
// returns false if the reduction results in an invalid board state (e.g. tile with no valid values).
bool reduce(boardnotes_t boardNotes)
{
	notes_t *group[9];  // the group array to use
	bool did_something; // flag used to check if we did anything during an iteration

	// repeat reduction logic until there is no change
	do
	{
		did_something = false; // so far we haven't done anything this iteration

		// reduce all the rows
		for (std::size_t row = 0; row < 9; ++row)
		{
			for (std::size_t i = 0; i < 9; ++i) group[i] = &boardNotes[row][i];
			if (!reduce(group, did_something)) return false;
		}

		// reduce all the cols
		for (std::size_t col = 0; col < 9; ++col)
		{
			for (std::size_t i = 0; i < 9; ++i) group[i] = &boardNotes[i][col];
			if (!reduce(group, did_something)) return false;
		}

		// reduce all the blocks
		for (std::size_t blockRow = 0; blockRow < 3; ++blockRow) for (std::size_t blockCol = 0; blockCol < 3; ++blockCol)
		{
			for (std::size_t _row = 0; _row < 3; ++_row) for (std::size_t _col = 0; _col < 3; ++_col)
			{
				group[3 * _row + _col] = &boardNotes[blockRow * 3 + _row][blockCol * 3 + _col];
			}
			if (!reduce(group, did_something)) return false;
		}
	}
	while (did_something);

	// no errors
	return true;
}

// --------------------- //

// -- solving helpers -- //

// --------------------- //

// given a notes set, returns the lowest set gues bit (i.e. the lowest ordinal possible value).
// if no bits are set, returns 0.
std::size_t lowest_set_guess(notes_t n)
{
	// return the lowest set guess
	for (std::size_t i = 1; i <= 9; ++i)
		if (n.test(i)) return i;

	// otherwise no bits are set, return 0
	return 0;
}

bool sudoku::solve()
{
	boardnotes_t boardNotes; // notes for the entire board

	std::size_t best_guess_row, best_guess_col; // row and col of best guess
	std::size_t best_guess_count = 0xbad;       // number of possible values in the best guess - 0xbad indicates no current best guess

	// populate basic notes and apply reduction logic - if either finds that the board is invalid, fail immediately
	if (!getnotes_all(*this, boardNotes) || !reduce(boardNotes)) return false;

	// iterate through each tile on the board
	for (std::size_t row = 0; row < 9; ++row) for (std::size_t col = 0; col < 9; ++col)
	{
		// get the number of notes for this tile
		std::size_t note_count = boardNotes[row][col].count();

		// if there's more than 1, we might need to guess here
		if (note_count > 1)
		{
			// if this position has fewer possible than the previous best, use it instead
			if (note_count < best_guess_count)
			{
				best_guess_row = row;
				best_guess_col = col;

				best_guess_count = note_count;
			}
		}
		// if there's exactly one, we know the value for this position - fill it in
		else if (note_count == 1) (*this)(row, col) = (tile_t)lowest_set_guess(boardNotes[row][col]);
		// otherwise this board is impossible - return failure
		else return false;
	}

	// if we need to guess, do that logic now that everything we know is filled in
	if (best_guess_count != 0xbad)
	{
		// get the best guess 
		notes_t best = boardNotes[best_guess_row][best_guess_col];

		// for each of the possible guesses
		for (std::size_t i = 1; i <= 9; ++i) if (best.test(i))
		{
			// record the previous board state
			sudoku cpy = *this;

			// make the change to this position (make the guess)
			(*this)(best_guess_row, best_guess_col) = (tile_t)i;

			// recurse - if that succeeds return true
			if (solve()) return true;

			// otherwise undo all the changes that did and continue searching
			*this = std::move(cpy);
		}

		// if we get to this point we've exhausted all possible guesses for this tile and nothing worked - fail
		return false;
	}

	// if we already filled in everything and had nothing to guess on - return if we succeeded or not
	return filled() && valid();
}

// ------------------ //

// -- io functions -- //

// ------------------ //

std::ostream& operator<<(std::ostream &ostr, const sudoku &board)
{
	for (std::size_t i = 0; i < 9; ++i)
	{
		for (std::size_t j = 0; j < 9; ++j)
		{
			ostr << (int)board(i, j); // output as int (because tile_t might be char, which has different symantics)

			if (j % 3 == 2) ostr << "   "; else ostr << ' ';
		}

		if (i % 3 == 2) ostr << "\n\n"; else ostr << '\n';
	}

	return ostr;
}
std::istream& operator>>(std::istream &istr, sudoku &board)
{
	int val; // temporary for reading values (because tile_t might be char, which has different symantics)
	for (auto &i : board.data)
	{
		istr >> val;
		i = val;
	}
	return istr;
}
