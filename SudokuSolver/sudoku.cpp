#include <iostream>
#include <iomanip>
#include <algorithm>
#include <bitset>

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

// --------------------- //

// -- solving helpers -- //

// --------------------- //

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
}
// calls getnotes() for every position in the board and stores the results in boardnotes_out.
void getnotes_all(const sudoku &board, boardnotes_t boardnotes_out)
{
	for (int row = 0; row < 9; ++row) for (int col = 0; col < 9; ++col)
		getnotes(board, row, col, boardnotes_out[row][col]);
}

// removes the tuple entries from the specified notes.
// notes - the notes object representing the current position in question.
// others - pointers to the 8 other notes objects representing the other tiles in a group (i.e. row, col, or block).
// returns true iff a change was made to notes.
void elucidate_group(notes_t &notes, const notes_t *const others[8])
{
	// loop through each other group member (or until notes is empty)
	for (int i = 0; i < 8 && notes.any(); ++i)
	{
		// if this group member has no notes, we can skip it
		if (others[i]->none()) continue;

		// count the number of group members that are equal to this group member (including itself)
		int count = 1;
		for (int j = 0; j < 8; ++j)
			if (i != j && *others[i] == *others[j]) ++count;

		// if there are as many equal members as the number of notes in each, it is a tuple (and thus can be excluded from this tile)
		if (count == others[i]->count()) notes &= ~*others[i];
	}

	// for each possible value for the current position
	for (int i = 1; i <= 9; ++i)
	{
		// only considering actual possibilities
		if (notes.test(i))
		{
			bool canSet = true; // marks if we can set this notes object to one possibility

			// if any of the other members contains this note we can't set the current location
			for (int j = 0; j < 8; ++j) if (others[j]->test(i))
			{
				canSet = false;
				break;
			}

			// otherwise no other tile in the group can be i, so it has to be here
			if (canSet)
			{
				notes.reset();
				notes.set(i);
				break;
			}
		}
	}
}
// returns an array of all the valid notes (removing tuples), saving the number of valid notes in count_out
void getnotesAdv(const sudoku &board, int row, int col, const boardnotes_t boardNotes, notes_t &notes_out)
{
	// clone the notes list from the raw notes table
	notes_out = boardNotes[row][col];

	const notes_t *otherNotes[8];
	int index;

	// get row
	index = 0;
	for (int i = 0; i < 9; ++i) if (i != col) otherNotes[index++] = &boardNotes[row][i];
	elucidate_group(notes_out, otherNotes);

	// get column
	index = 0;
	for (int i = 0; i < 9; ++i) if (i != row) otherNotes[index++] = &boardNotes[i][col];
	elucidate_group(notes_out, otherNotes);

	// get block
	index = 0;
	int blockRow = row / 3;
	int blockCol = col / 3;
	for (int _row = 0; _row < 3; ++_row) for (int _col = 0; _col < 3; ++_col)
	{
		int r = blockRow * 3 + _row;
		int c = blockCol * 3 + _col;
		if (r != row || c != col) otherNotes[index++] = &boardNotes[r][c];
	}
	elucidate_group(notes_out, otherNotes);
}

// removes the specified note from the list of possible raw notes
void removeNote(int row, int col, tile_t note, boardnotes_t boardNotes)
{
	// remove from row
	for (int i = 0; i < 9; ++i) if(i != col)
		boardNotes[row][i].reset(note);

	// remove from column
	for (int i = 0; i < 9; ++i) if(i != row)
		boardNotes[i][col].reset(note);

	// remove from block
	int blockRow(row / 3), blockCol(col / 3);
	for (int _row = 0; _row < 3; ++_row) for (int _col = 0; _col < 3; ++_col)
	{
		int r(blockRow * 3 + _row), c(blockCol * 3 + _col);
		if (r != row || c != col) boardNotes[r][c].reset(note);
	}

	// clear all notes from the tile we're on
	boardNotes[row][col].reset();
}

// solves the tiles that can be solved assuredly (only 1 valid possible value)
bool solveSure(sudoku &board)
{
	// store notes for the whole board
	boardnotes_t boardNotes;
	getnotes_all(board, boardNotes);

	bool didone, completed;
	do
	{
		didone = false;
		completed = true;

		// iterate over each row and col position
		for (int row = 0; row < 9; ++row) for (int col = 0; col < 9; ++col)
		{
			// if position is already known, skip it
			if (board(row, col) != 0) continue;

			// get simplified notes for this tile
			notes_t notes;
			getnotesAdv(board, row, col, boardNotes, notes);

			// if there are notes, the board is in an invalid state (further solving is pointless)
			if (notes.none()) return false;

			// if only 1 note, it has to be that
			if (notes.count() == 1)
			{
				// wind val up to the actual note value to use
				int val;
				for (val = 0; !notes.test(val); ++val);

				// then select that value
				board(row, col) = val;
				didone = true;

				// remove the note from neighbor tiles
				removeNote(row, col, val, boardNotes);
			}
			// otherwise all we can say is board is incomplete
			else completed = false;
		}
	}
	// do solve passes while one was done and it's not completed
	while (didone && !completed);

	return completed;
}
// attempts to solve the board via guessing
bool solveGuess(sudoku &board)
{
	// get the notes for the entire board
	boardnotes_t boardNotes;
	getnotes_all(board, boardNotes);
	
	// iterate throuh each tile on the board
	for (int row = 0; row < 9; ++row) for (int col = 0; col < 9; ++col)
	{
		// if tile not empty, skip
		if (board(row, col) != 0) continue;

		// get the adv notes for this tile
		notes_t notes;
		getnotesAdv(board, row, col, boardNotes, notes);

		// if there are no valid notes, a previous step was invalid (skip further work)
		if (notes.none()) return false;

		// iterate through each adv note
		for (int i = 0; i < 9; ++i)
		{
			// only consider actual possibilities
			if (notes.test(i))
			{
				// clone the board and make the change to the clone
				sudoku clone(board);
				clone(row, col) = i;

				// apply the sure solver, and recurse if needed
				// if successful, move the clone into board and return success
				if (solveSure(clone) || solveGuess(clone))
				{
					board = std::move(clone);
					return true;
				}
			}
		}
	}

	// otherwise was unsuccessful
	return false;
}

bool sudoku::solve(bool deep)
{
	return solveSure(*this) || deep && solveGuess(*this);
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
