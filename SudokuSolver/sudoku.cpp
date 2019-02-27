#include <iostream>
#include <iomanip>
#include <algorithm>
#include <bitset>
#include <cassert>
#include <queue>

#include "Sudoku.h"

// ----------- //

// -- query -- //

// ----------- //

bool sudoku::valid() const
{
	const sudoku &board(*this);

	// for each board position
	for (int row = 0; row < 9; ++row) for (int col = 0; col < 9; ++col)
	{
		// for each number
		for (tile_t mark = 1; mark <= 9; ++mark)
		{
			// count occurrences in this row
			int count = 0;
			for (int i = 0; i < 9; ++i) if (board(row, i) == mark) ++count;
			if (count > 1) return false; // if there's more than 1 it's illegal

			// count occurrences in this column
			count = 0;
			for (int i = 0; i < 9; ++i) if (board(i, col) == mark) ++count;
			if (count > 1) return false; // if there's more than 1 it's illegal

			// ount occurrences in this block
			count = 0;
			int blockRow = row / 3;
			int blockCol = col / 3;
			for (int _r = 0; _r < 3; ++_r) for (int _c = 0; _c < 3; ++_c) if (board(blockRow * 3 + _r, blockCol * 3 + _c) == mark) ++count;
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
bool notevalid(const sudoku &board, int row, int col, tile_t note)
{
	// iterate over each row and column of current position
	// if any equal potential note, potential note is invalid
	for (int i = 0; i < 9; ++i)
		if (board(i, col) == note || board(row, i) == note) return false;

	// get the row and column of the 3x3 block we're in
	int blockRow = row / 3;
	int blockCol = col / 3;

	// iterate over each position in the block
	// if any equal potential note, potential note is invalid
	for (int _row = 0; _row < 3; ++_row) for (int _col = 0; _col < 3; ++_col)
		if (board(blockRow * 3 + _row, blockCol * 3 + _col) == note) return false;

	// otherwise, note is valid
	return true;
}
// populates notes_out with all the valid notes at the specified row and column.
// if the board position is already known (i.e. nonzero), the result is empty.
void getnotes(const sudoku &board, int row, int col, notes_t &notes_out)
{
	// clear out the notes
	notes_out.reset();

	// if tile is blank, generate the notes list
	if (board(row, col) == 0)
	{
		for (tile_t sub = 1; sub <= 9; ++sub)
			if (notevalid(board, row, col, sub)) notes_out.set(sub);
	}
	// otherwise it has one note - its actual value
	else notes_out.set(board(row, col));
}
// calls getnotes() for every position in the board and stores the results in boardnotes_out.
void getnotes_all(const sudoku &board, boardnotes_t boardnotes_out)
{
	for (int row = 0; row < 9; ++row) for (int col = 0; col < 9; ++col)
		getnotes(board, row, col, boardnotes_out[row][col]);
}

// --------------------------------------- //

// -- advanced note reduction utilities -- //

// --------------------------------------- //

void print_orthogonality(boardnotes_t notes)
{
	for (int i = 0; i < 9; ++i)
	{
		for (int j = 0; j < 9; ++j)
		{
			std::cout << notes[i][j].count() << ' ';
		}
		std::cout << '\n';
	}
}
void print_group(notes_t *group[9])
{
	for (int i = 0; i < 9; ++i) std::cout << std::hex << group[i]->to_ulong() << std::dec << '\n';
}

// applies reduction logic to the specified notes group.
// group - an array of pointers to the 9 notes objects that represents the group (i.e. row, col, or block) (order doesn't matter).
// returns true if any reductions were successful.
bool reduce(notes_t *group[9])
{
	bool did_something = false; // flags if we did anything - defaults to false

	// for every combination of 1 to 8 group members
	for (int combination = (1 << 9) - 2; combination > 0; --combination)
	{
		notes_t notes_union;  // the union of notes in this combination
		int member_count = 0; // the number of group members in this combination

		// for each group member in this combination
		for (int i = 0; i < 9; ++i) if ((combination >> i) & 1)
		{
			notes_union |= *group[i]; // add this member's notes to the notes union
			++member_count; // inc the number of members in this combination
		}

		// if the notes union has the same number of possibles as there are members in this combination, it's a tuple
		if (notes_union.count() == member_count)
		{
			// now we know it's a tuple, so we can eliminate the notes union choices from all group members not in this combination
			for (int i = 0; i < 9; ++i) if (!((combination >> i) & 1))
			{
				// update the did something flag - becomes true if this position has anything in common with the notes union
				did_something = did_something || (*group[i] & notes_union).any();

				// eliminate the notes union from this position
				*group[i] &= ~notes_union;
			}
		}
	}

	// for each tile in the group with more than 1 choice (as individuals now)
	for (int i = 0; i < 9; ++i) if (group[i]->count() > 1)
	{
		// for each possible value for this tile
		for (int j = 1; j <= 9; ++j) if (group[i]->test(j))
		{
			bool unique = true; // mark the note value as unique to begin with

			// look through all the group members and see if any of them can also be that value
			for (int k = 0; k < 9; ++k) if (i != k && group[k]->test(j))
			{
				// in this case the value's not unique
				unique = false;
				break;
			}

			// if the note was unique, we know this position has to be that
			if (unique)
			{
				// update the did something flag
				did_something = true;

				// set the unique note as the only option for this tile
				group[i]->reset();
				group[i]->set(j);

				break; // go on to the next tile
			}
		}
	}

	// return the did something flag
	return did_something;
}
// applies group reduction to every group on the board.
// returns true if a change was made.
bool reduce(boardnotes_t boardNotes)
{
	notes_t *group[9]; // the group array to use
	bool did_something = false; // flag that marks if we did something - defaults to false

	// reduce all the rows
	for (int row = 0; row < 9; ++row)
	{
		for (int i = 0; i < 9; ++i) group[i] = &boardNotes[row][i];
		if (reduce(group)) did_something = true;
	}

	// reduce all the cols
	for (int col = 0; col < 9; ++col)
	{
		for (int i = 0; i < 9; ++i) group[i] = &boardNotes[i][col];
		if (reduce(group)) did_something = true;
	}

	// reduce all the blocks
	for (int blockRow = 0; blockRow < 3; ++blockRow) for (int blockCol = 0; blockCol < 3; ++blockCol)
	{
		for (int _row = 0; _row < 3; ++_row) for (int _col = 0; _col < 3; ++_col)
		{
			group[3 * _row + _col] = &boardNotes[blockRow * 3 + _row][blockCol * 3 + _col];
		}
		if (reduce(group)) did_something = true;
	}

	// return the did something flag
	return did_something;
}

// --------------------- //

// -- solving helpers -- //

// --------------------- //

// represents a guess entry for the guess queue
struct guess_entry
{
	int row;
	int col;
	notes_t notes;

	// comparison operator - arranges them in descending number of possible notes
	friend bool operator<(const guess_entry &a, const guess_entry &b) { return a.notes.count() > b.notes.count(); }
};

// given a notes set, returns the lowest set gues bit (i.e. the lowest ordinal possible value).
// if no bits are set, returns 0.
int lowest_set_guess(notes_t n)
{
	// return the lowest set guess
	for (int i = 1; i <= 9; ++i)
		if (n.test(i)) return i;

	// otherwise no bits are set, return 0
	return 0;
}

bool sudoku::solve()
{
	sudoku &board = *this; // alias the board for convenience

	boardnotes_t boardNotes;  // notes for the entire board
	std::priority_queue<guess_entry> guess_queue; // the sorted queue of guesses to make - top is the guess entry with the least number of possible values

	// populate basic notes and repeat reduction logic until there is no change
	getnotes_all(board, boardNotes);
	while (reduce(boardNotes));
	
	// iterate through each tile on the board
	for (int row = 0; row < 9; ++row) for (int col = 0; col < 9; ++col)
	{
		// get the number of notes for this tile
		auto note_count = boardNotes[row][col].count();

		// if there's more than 1, add it to the guess queue
		if (note_count > 1) guess_queue.push({ row, col, boardNotes[row][col] });
		// if there's exactly one, we know the value for this position - fill it in
		else if (note_count == 1) board(row, col) = lowest_set_guess(boardNotes[row][col]);
		// otherwise this board is impossible - return failure
		else return false;
	}

	// now we need to handle all the guesses
	for (; !guess_queue.empty(); guess_queue.pop())
	{
		// get the top guess entry (one with the fewest possible values)
		guess_entry entry = guess_queue.top();

		// for each of the possible guesses
		for (int i = 1; i <= 9; ++i) if (entry.notes.test(i))
		{
			// record the previous board state
			sudoku cpy = board;

			// make the change to this position (make the guess)
			board(entry.row, entry.col) = i;

			// recurse - if that succeeds return true
			if (board.solve()) return true;

			// otherwise undo all the changes that did and continue searching
			board = std::move(cpy);
		}

		// if we get to this point we've exhausted all possible guesses for this tile and nothing worked - fail
		return false;
	}

	// if we get here we have no more guesses to make - return if we succeeded or not
	return board.filled() && board.valid();
}

// ------------------ //

// -- io functions -- //

// ------------------ //

std::ostream& operator<<(std::ostream &ostr, const sudoku &board)
{
	for (int i = 0; i < 9; ++i)
	{
		for (int j = 0; j < 9; ++j)
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
