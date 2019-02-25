#include <iostream>
#include <iomanip>
#include "Sudoku.h"

// ---------------------------------------------------
// construction / destruction / assignment
// ---------------------------------------------------

Sudoku::Sudoku() : data(new tile_t[81]) {}
Sudoku::~Sudoku()
{
	delete[] data;
}

Sudoku::Sudoku(const Sudoku &other) : data(new tile_t[81])
{
	for (int i(0); i < 81; ++i)
		data[i] = other.data[i];
}
Sudoku::Sudoku(Sudoku &&other)
{
	data = other.data;
	other.data = nullptr;
}

Sudoku& Sudoku::operator=(const Sudoku &other)
{
	for (int i(0); i < 81; ++i)
		data[i] = other.data[i];

	return *this;
}
Sudoku& Sudoku::operator=(Sudoku &&other)
{
	tile_t *temp(data);
	data = other.data;
	other.data = temp;

	return *this;
}

// ---------------------------------------------------
// access methods
// ---------------------------------------------------

tile_t& Sudoku::operator()(int row, int col)
{
	return data[row * 9 + col];
}
tile_t Sudoku::operator()(int row, int col) const
{
	return data[row * 9 + col];
}

int Sudoku::countComplete() const
{
	int count(0);

	for (int i(0); i < 81; ++i)
		if (data[i] != 0) ++count;

	return count;
}
int Sudoku::countIncomplete() const
{
	int count(0);

	for (int i(0); i < 81; ++i)
		if (data[i] == 0) ++count;

	return count;
}

void Sudoku::clear()
{
	for (int i(0); i < 81; ++i) data[i] = 0;
}

bool Sudoku::strictEquals(const Sudoku &other) const
{
	for (int i(0); i < 81; ++i) if (data[i] != other.data[i]) return false;

	return true;
}
bool Sudoku::fuzzyEquals(const Sudoku &other) const
{
	for (int i(0); i < 81; ++i) if (data[i] != 0 && other.data[i] != 0 && data[i] != other.data[i]) return false;
	
	return true;
}

bool Sudoku::completed() const
{
	for (int i(0); i < 81; ++i) if (data[i] == 0) return false;

	return true;
}
bool Sudoku::valid() const
{
	const Sudoku &board(*this);

	for (int row(0); row < 9; ++row) for (int col(0); col < 9; ++col)
	{
		for (tile_t mark(1); mark <= 9; ++mark)
		{
			int count;
			
			// count row
			count = 0;
			for (int i(0); i < 9; ++i) if (board(row, i) == mark) ++count;
			if (count > 1) return false;

			// count column
			count = 0;
			for (int i(0); i < 9; ++i) if (board(i, col) == mark) ++count;
			if (count > 1) return false;

			// count block
			count = 0;
			int blockRow(row / 3), blockCol(col / 3);
			for (int _r(0); _r < 3; ++_r) for (int _c(0); _c < 3; ++_c)
			{
				int r(blockRow * 3 + _r), c(blockCol * 3 + _c);
				if (board(r, c) == mark) ++count;
			}
			if (count > 1) return false;
		}
	}

	return true;
}

// ---------------------------------------------------
// solving methods
// ---------------------------------------------------

// container class for notes, offering functions for adding/removing notes without having duplicates
class notes_t
{
public:
	tile_t notes[9];
	int length;

	notes_t();
	notes_t(const notes_t &other);

	bool contains(tile_t note) const;

	void add(tile_t note);
	void remove(tile_t note);
	void removeAt(int index);

	void add(const notes_t &notes);
	void remove(const notes_t &notes);

	void clear();
	void set(tile_t note);

	tile_t& operator[](int index);
	tile_t operator[](int index) const;
};

notes_t::notes_t() : length(0) {}
notes_t::notes_t(const notes_t &other) : length(other.length)
{
	for (int i(0); i < other.length; ++i)
		notes[i] = other.notes[i];
}

bool notes_t::contains(tile_t note) const
{
	for (int i(0); i < length; ++i)
		if (notes[i] == note) return true;

	return false;
}

void notes_t::add(tile_t note)
{
	if (contains(note)) return;
	notes[length++] = note;
}
void notes_t::remove(tile_t note)
{
	for(int i(0); i < length; ++i)
		if (notes[i] == note)
		{
			notes[i] = notes[--length];
			return;
		}
}
void notes_t::removeAt(int index)
{
	notes[index] = notes[--length];
}

void notes_t::add(const notes_t &notes)
{
	for (int i(0); i < notes.length; ++i)
		add(notes.notes[i]);
}
void notes_t::remove(const notes_t &notes)
{
	for (int i(0); i < notes.length; ++i)
		remove(notes.notes[i]);
}

void notes_t::clear()
{
	length = 0;
}
void notes_t::set(tile_t note)
{
	length = 1;
	notes[0] = note;
}

tile_t& notes_t::operator[](int index)
{
	return notes[index];
}
tile_t notes_t::operator[](int index) const
{
	return notes[index];
}

bool operator==(const notes_t &a, const notes_t &b)
{
	if (a.length != b.length) return false;
	for (int i(0); i < a.length; ++i) if (!b.contains(a[i])) return false;

	return true;
}
bool operator!=(const notes_t &a, const notes_t &b)
{
	return !(a == b);
}

typedef notes_t boardNotes_t[9][9];

// copies the niotes in a into b
void copyNotes(const boardNotes_t a, boardNotes_t b)
{
	for (int row(0); row < 9; ++row) for (int col(0); col < 9; ++col)
		b[row][col] = a[row][col];
}

// returns true iff the given note is valid for the specified position
bool notevalid(const Sudoku &board, int row, int col, tile_t note)
{
	// iterate over each row and column of current position
	// if any equal potential note, potential note is invalid
	for (int i(0); i < 9; ++i)
		if (board(i, col) == note || board(row, i) == note) return false;

	// get the row and column of the 3x3 block we're in
	int blockRow(row / 3), blockCol(col / 3);

	// iterate over each position in the block
	// if any equal potential note, potential note is invalid
	for (int _row(0); _row < 3; ++_row) for (int _col(0); _col < 3; ++_col)
		if (board(blockRow * 3 + _row, blockCol * 3 + _col) == note) return false;

	// otherwise, note is valid
	return true;
}

// populates the notes and count with the valid notes at the specified row and column
void getnotes(const Sudoku &board, int row, int col, notes_t &notes_out)
{
	// clear out the notes
	notes_out.clear();

	// if tile is blank, generate the notes list
	if (board(row, col) == 0)
		for (tile_t sub(1); sub <= 9; ++sub)
			if (notevalid(board, row, col, sub)) notes_out.notes[notes_out.length++] = sub;
}
// gets the boardNotes for the specified board
void getnotesAll(const Sudoku &board, boardNotes_t boardNotes_out)
{
	for (int row(0); row < 9; ++row) for (int col(0); col < 9; ++col)
		getnotes(board, row, col, boardNotes_out[row][col]);
}

// removes the tuple entries from the specified notes and chnges count_out accordingly
void elucidateGroup(notes_t &notes_out, const notes_t *const otherNotes[8])
{
	// loop through each otherNotes (or until notes_out is empty)
	for (int i(0); i < 8 && notes_out.length > 0; ++i)
	{
		// if this otherNotes has no notes, we can skip it
		if (otherNotes[i]->length == 0) continue;

		// count the number of otherNotes that are equal to this otherNote
		int count(1);
		for (int j(0); j < 8; ++j)
			if (i != j && *otherNotes[i] == *otherNotes[j]) ++count;

		// if the number of tiles with equal notes is equal to its number of notes, it is a tuple (and thus can be excluded)
		if (count == otherNotes[i]->length) notes_out.remove(*otherNotes[i]);
	}

	for (int i(0); i < notes_out.length; ++i)
	{
		bool canSet(true);
		for (int j(0); j < 8; ++j) if (otherNotes[j]->contains(notes_out[i]))
		{
			canSet = false;
			break;
		}

		if (canSet)
		{
			notes_out.set(notes_out[i]);
			break;
		}
	}
}
// returns an array of all the valid notes (removing tuples), saving the number of valid notes in count_out
void getnotesAdv(const Sudoku &board, int row, int col, const boardNotes_t boardNotes, notes_t &notes_out)
{
	// clone the notes list from the raw notes table
	notes_out = boardNotes[row][col];

	const notes_t *otherNotes[8];
	int index;

	// get row
	index = 0;
	for (int i(0); i < 9; ++i) if (i != col) otherNotes[index++] = &boardNotes[row][i];
	elucidateGroup(notes_out, otherNotes);

	// get column
	index = 0;
	for (int i(0); i < 9; ++i) if (i != row) otherNotes[index++] = &boardNotes[i][col];
	elucidateGroup(notes_out, otherNotes);

	// get block
	index = 0;
	int blockRow(row / 3), blockCol(col / 3);
	for (int _row(0); _row < 3; ++_row) for (int _col(0); _col < 3; ++_col)
	{
		int r(blockRow * 3 + _row), c(blockCol * 3 + _col);
		if (r != row || c != col) otherNotes[index++] = &boardNotes[r][c];
	}
	elucidateGroup(notes_out, otherNotes);
}

// removes the specified note from the list of possible raw notes
void removeNote(int row, int col, tile_t note, boardNotes_t boardNotes)
{
	// remove from row
	for (int i(0); i < 9; ++i) if(i != col)
		boardNotes[row][i].remove(note);

	// remove from column
	for (int i(0); i < 9; ++i) if(i != row)
		boardNotes[i][col].remove(note);

	// remove from block
	int blockRow(row / 3), blockCol(col / 3);
	for (int _row(0); _row < 3; ++_row) for (int _col(0); _col < 3; ++_col)
	{
		int r(blockRow * 3 + _row), c(blockCol * 3 + _col);
		if (r != row || c != col) boardNotes[r][c].remove(note);
	}

	// clear all notes from the tile we're on
	boardNotes[row][col].clear();
}

// solves the tiles that can be solved assuredly (only 1 valid possible value)
bool solveSure(Sudoku &board)
{
	// store notes for the whole board
	boardNotes_t boardNotes;
	getnotesAll(board, boardNotes);

	bool didone, completed;
	do
	{
		didone = false;
		completed = true;

		// iterate over each row and col position
		for (int row(0); row < 9; ++row) for (int col(0); col < 9; ++col)
		{
			// if position is already known, skip it
			if (board(row, col) != 0) continue;

			// get simplified notes for this tile
			notes_t notes;
			getnotesAdv(board, row, col, boardNotes, notes);

			// if there are notes, the board is in an invalid state (further solving is pointless)
			if (notes.length == 0) return false;

			// if only 1 note, it has to be that
			if (notes.length == 1)
			{
				board(row, col) = notes.notes[0];
				didone = true;

				// remove the note from neighbor tiles
				removeNote(row, col, notes.notes[0], boardNotes);
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
bool solveGuess(Sudoku &board)
{
	// get the notes for the entire board
	boardNotes_t boardNotes;
	getnotesAll(board, boardNotes);
	
	// iterate throuh each tile on the board
	for (int row(0); row < 9; ++row) for (int col(0); col < 9; ++col)
	{
		// if tile not empty, skip
		if (board(row, col) != 0) continue;

		// get the adv notes for this tile
		notes_t notes;
		getnotesAdv(board, row, col, boardNotes, notes);

		// if there are no valid notes, a previous step was invalid (skip further work)
		if (notes.length == 0) return false;

		// iterate through each adv note
		for (int i(0); i < notes.length; ++i)
		{
			// clone the board and make the change to the clone
			Sudoku clone(board);
			clone(row, col) = notes[i];

			// apply the sure solver, and recurse if needed
			// if successful, move the clone into board and return success
			if (solveSure(clone) || solveGuess(clone))
			{
				board = std::move(clone);
				return true;
			}
		}
	}

	// otherwise was unsuccessful
	return false;
}

bool Sudoku::solve(bool deep)
{
	if (solveSure(*this)) return true;
	if (deep && solveGuess(*this)) return true;

	return false;
}

// ---------------------------------------------------
// io functions
// ---------------------------------------------------

std::ostream& operator<<(std::ostream &ostr, const Sudoku &board)
{
	for (int i(0); i < 9; ++i)
	{
		for (int j(0); j < 9; ++j)
			ostr << board.data[i * 9 + j] << ' ';

		ostr << '\n';
	}

	return ostr;
}
std::istream& operator>>(std::istream &istr, Sudoku &board)
{
	for (int i(0); i < 81; ++i)
		istr >> board.data[i];

	return istr;
}
