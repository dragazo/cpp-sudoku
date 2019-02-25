#ifndef UTIL_H
#define UTIL_H

// returns true iff the array contains at least one element equal to the specified item
template<typename T, typename L, typename O>
bool contains(const T *arr, L len, const O &item)
{
	for (L i(0); i < len; ++i)
		if (arr[i] == item) return true;

	return false;
}
template<typename T, typename L, typename O>
bool containsAny(const T *arr, L len, const O *items, L itemsLen)
{
	for (L i(0); i < itemsLen; ++i)
		if (contains(arr, len, items[i])) return true;

	return false;
}
template<typename T, typename L, typename O>
bool containsAll(const T *arr, L len, const O *items, L itemsLen)
{
	for (L i(0); i < itemsLen; ++i)
		if (!contains(arr, len, items[i])) return false;

	return true;
}

// returns the index of the first element of the array that is equal to the specified item, or -1 if not found
template<typename T, typename L, typename O>
L index(const T *arr, L len, const O &item)
{
	for (L i(0); i < len; ++i)
		if (arr[i] == item) return i;

	return -1;
}

// finds and returns a reference to the first array element equal to the specified item
// throws (const char*) - element not found
template<typename T, typename L, typename O>
T& find(T *arr, L len, const O &item)
{
	for (L i(0); i < len; ++i)
		if (arr[i] == item) return arr[i];

	throw "item not found in array";
}

// resizes the array, copying the items that can be stored into the new array (calls delete[] on old array)
template<typename T, typename L>
void resize(T *&arr, L len, L newLen)
{
	T *newArr(new T[newLen]);

	L iMax(len < newLen ? len : newLen);
	for (L i(0); i < iMax; ++i)
		newArr[i] = arr[i];

	delete[] arr;
	arr = newArr;
}

// removes all occurences of array neg from the specified array (does not preserve order)
template<typename T, typename L, typename O>
void remove(T arr[], L &arrLen, const O neg[], L negLen)
{
	for (L i(0); i < arrLen; ++i)
		for (L j(0); j < negLen; ++j)
			if (arr[i] == neg[j])
			{
				arr[i] = arr[arrLen - 1];
				--arrLen;

				return;
			}
}
// removes all occurences of neg from the specified array (does not preserve order)
template<typename T, typename L, typename O>
void remove(T arr[], L &arrLen, const O neg)
{
	for (L i(0); i < arrLen; ++i)
		if (arr[i] == neg)
		{
			arr[i] = arr[arrLen - 1];
			--arrLen;

			return;
		}
}
// removes all occurences in array neg from the specified array (preserves order)
template<typename T, typename L, typename O>
void removeOrdered(T *arr, L &arrLen, const O *neg, L negLen)
{
	for (L i(0); i < arrLen; ++i)
		for (L j(0); j < negLen; ++j)
			if (arr[i] == neg[j])
			{
				for (L k(i + 1); k < arrLen; ++k)
					arr[k - 1] = arr[k];
				--arrLen;

				return;
			}
}
// removes all occurences of neg from the specified array (preserves order)
template<typename T, typename L, typename O>
void removeOrdered(T *arr, L &arrLen, const O neg)
{
	for (L i(0); i < arrLen; ++i)
		if (arr[i] == neg)
		{
			for (L k(i + 1); k < arrLen; ++k)
				arr[k - 1] = arr[k];
			--arrLen;

			return;
		}
}

// removes all occurences of array neg from the specified array (does not preserve order)
template<typename T, typename L, typename O>
void removeAll(T arr[], L &arrLen, const O neg[], L negLen)
{
	for (L i(0); i < arrLen; ++i)
		for (L j(0); j < negLen; ++j)
			if (arr[i] == neg[j])
			{
				arr[i] = arr[arrLen - 1];
				--arrLen;
			}
}
// removes all occurences of neg from the specified array (does not preserve order)
template<typename T, typename L, typename O>
void removeAll(T arr[], L &arrLen, const O neg)
{
	for(L i(0); i < arrLen; ++i)
		if (arr[i] == neg)
		{
			arr[i] = arr[arrLen - 1];
			--arrLen;
		}
}
// removes all occurences in array neg from the specified array (preserves order)
template<typename T, typename L, typename O>
void removeAllOrdered(T *arr, L &arrLen, const O *neg, L negLen)
{
	for (L i(0); i < arrLen; ++i)
		for(L j(0); j < negLen; ++j)
			if (arr[i] == neg[j])
			{
				for (L k(i + 1); k < arrLen; ++k)
					arr[k - 1] = arr[k];
				--arrLen;
			}
}
// removes all occurences of neg from the specified array (preserves order)
template<typename T, typename L, typename O>
void removeAllOrdered(T *arr, L &arrLen, const O neg)
{
	for (L i(0); i < arrLen; ++i)
		if (arr[i] == neg)
		{
			for (L k(i + 1); k < arrLen; ++k)
				arr[k - 1] = arr[k];
			--arrLen;
		}
}

#endif
