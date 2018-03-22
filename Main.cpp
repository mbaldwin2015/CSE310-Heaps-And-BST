#define SYMBOL_LENGTH 20 // Maximum length of a variable name
#define INT 0 // Integer type
#define CHAR 1 // Character string type
#define BST 2 // Binary search tree type
#define STRTERM ’\0’ // String termination character

#include <iostream>
#include <math.h>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

using namespace std;

struct heapEntry{
	int blockSize; // Size of free block in bytes
	int offset; // Start address of free block given as offset into memory block
};

struct symbolTableEntry{
	char symbol[20]; // Variable identifier
	int type; // The type of the variable: one of INT, CHAR, or BST
	int offset; // Start address of variable in memory given as offset
	int length; // Size in bytes of to store variable
};

struct bstNode{
	int key; // A node of a binary search tree ordered on this key value
	int left; // Left child
	int right; // Right child
};

void myMalloc(bstNode*, heapEntry*, symbolTableEntry*, unsigned char*, string, string, unsigned long, int, int, unsigned char, int);
void myFree(bstNode*&, heapEntry*&, symbolTableEntry*, unsigned char*, string, int, int&, int&);
void map(symbolTableEntry*, unsigned char*, int, int);
bool is_hex_notation(string&);
void add(symbolTableEntry*, unsigned char*, string, string, int);
void strcat(symbolTableEntry*, unsigned char*, string, string, int);
void print(symbolTableEntry*, unsigned char*, string, int);
void compact(heapEntry*&, int&);
void insert(bstNode*&, heapEntry*, symbolTableEntry*, unsigned char*, int&, int, int, unsigned long, string);

// MAIN FUNCTION
int main() {
	int k, t, c, n, heapArrSize=1, bstArrSize=1, inSize;
	unsigned long inValue;
	string command, type, varName, inValueStr, addVal1, addVal2, inSymbol;
	unsigned char inCharacter;
	heapEntry *heap;
	symbolTableEntry *symTable;
	bstNode *bst;

	cin >> k;
	cin >> t;
	cin >> c;

	n = (int)pow(2, k);
	unsigned char *memblock = new unsigned char[n];
	for (int i = 0; i < n; i++) memblock[i] = ' ';
	heap = new heapEntry[heapArrSize];
	heap[0].blockSize = n;
	heap[0].offset = 0;
	symTable = new symbolTableEntry[t];
	bst = new bstNode[bstArrSize];

	for (int i = 0; i < t; i++) symTable[i].type = -1; // to make it work with linux because linux instantiates to 0

	for (int i = 0; i < c; i++) { // read in all the commands
		cin >> command;
		cout << "COMMAND: " << command << endl;
		if (command == "allocate") { // allocate command
			if (heap[0].blockSize == 0) {
				cout << "Error: Insufficient memory to allocate variable.";
				exit(0);
			}
			cin >> type;

			if (type == "INT") {
				cin >> varName;
				cin >> inValue;
				
				myMalloc(bst, heap, symTable, memblock, type, varName, inValue, 4, t, ' ', heapArrSize);
			}
			else if (type == "CHAR") {
				cin >> varName;
				cin >> inSize;
				cin.ignore(2);
				cin >> inCharacter;
				cin.ignore();

				myMalloc(bst, heap, symTable, memblock, type, varName, inValue, inSize, t, inCharacter, heapArrSize);
			}
			else {
				cin >> varName;
				cin >> inValue;

				myMalloc(bst, heap, symTable, memblock, type, varName, inValue, 12, t, ' ', heapArrSize);
			}
		}
		else if (command == "add") { // add command
			cin >> addVal1;
			cin >> addVal2;

			add(symTable, memblock, addVal1, addVal2, t);
		}
		else if (command == "print") { // print command
			cin >> inSymbol;

			print(symTable, memblock, inSymbol, t);
		}
		else if (command == "strcat") { // strcat command
			cin >> inSymbol;
			cin >> addVal1;

			if (addVal1[0] == '\"') addVal1 = addVal1[1];

			strcat(symTable, memblock, inSymbol, addVal1, t);
		}
		else if (command == "free") { // free command
			cin >> inSymbol;

			myFree(bst, heap, symTable, memblock, inSymbol, t, heapArrSize, bstArrSize);
		}
		else if (command == "compact") { // compact command
			compact(heap, heapArrSize);
		}
		else if (command == "map") { // map command
			map(symTable, memblock, n, t);
		}
		else if (command == "insert") {
			cin >> varName;
			cin >> inValue;

			insert(bst, heap, symTable, memblock, bstArrSize, t, 12, inValue, varName);
		}
		else cout << "Command not recognized.";
	}

	delete[] memblock;
	delete[] heap;
	delete[] symTable;
	delete[] bst;

	return 0;
}

void myMalloc(bstNode* bst, heapEntry* heap, symbolTableEntry* symTable, unsigned char* memblock, string type, string varName, unsigned long inValue, int inSize, int t, unsigned char inCharacter, int heapArrSize) {
	int index, sum = 0, full = 1, shift = (inSize - 1);

	while (inSize % 4 != 0) inSize++;

	for (unsigned int i = 0; i < varName.size(); i++) sum += varName[i]; // sum ASCII characters of var name
	index = sum % t; // get index for hash table

	while (symTable[index].type >= 0 && symTable[index].type <= 2) { // linear probing
		index++;
		if (index == t) index = 0;
		if (full == t) {
			cout << "Error: Hash table full, exiting." << endl;
			exit(0);
		}
		full++;
	}

	// fill the new symbol table entry
	strcpy(symTable[index].symbol, varName.c_str());
	if (type == "INT") symTable[index].type = INT;
	else if (type == "CHAR") symTable[index].type = CHAR;
	else symTable[index].type = BST;
	symTable[index].offset = heap[0].offset;
	symTable[index].length = inSize;

	// Adding values to the memory block
	if (type == "INT") {
		for (int i = heap[0].offset; i < (heap[0].offset+inSize); i++) {
			memblock[i] = (inValue >> (shift * 8)) & 0xFF;
			shift--;
		}
	}
	else if (type == "CHAR") {
		memblock[heap[0].offset] = inCharacter;
		memblock[heap[0].offset + 1] = '\0';
	}
	else if (type == "BST") {
		for (int i = heap[0].offset; i < (heap[0].offset + 4); i++) {
			memblock[i] = (inValue >> (shift * 8)) & 0xFF;
			shift--;
		}
		for (int i = heap[0].offset + 4; i < (heap[0].offset + 12); i++) {
			memblock[i] = (-1 >> (shift * 8)) & 0xFF;
			shift--;
		}

		bst[0].key = inValue;
		bst[0].left = -1;
		bst[0].right = -1;
	}
	else cout << "TYPE NOT RECOGNIZED\n";

	// Adjust free space in heap
	heap[0].blockSize -= inSize;
	heap[0].offset += inSize;
	// RUN MAX-HEAPIFY
}

void myFree(bstNode*& bst, heapEntry*& heap, symbolTableEntry* symTable, unsigned char* memblock, string inSymbol, int t, int& heapArrSize, int& bstArrSize) {
	int key, offset, blockSize;
	heapArrSize++;
	heapEntry* temp = new heapEntry[heapArrSize];

	for (int i = 0; i < heapArrSize-1; i++) temp[i] = heap[i];

	for (int i = 0; i < t; i++) if (string(symTable[i].symbol) == inSymbol) key = i;

	blockSize = symTable[key].length;
	offset = symTable[key].offset;

	strcpy(symTable[key].symbol, " ");
	symTable[key].length = -10000;
	symTable[key].offset = -10000;
	symTable[key].type = -10000;

	for (int i = offset; i < (blockSize+offset); i++) memblock[i] = ' ';

	temp[heapArrSize - 1].blockSize = blockSize;
	temp[heapArrSize - 1].offset = offset;

	// Grow heap
	delete[] heap;
	heap = new heapEntry[heapArrSize];
	for (int i = 0; i < heapArrSize; i++) heap[i] = temp[i];
	delete[] temp;
	// MAX-HEAPIFY
}

void map(symbolTableEntry* symTable, unsigned char* memblock, int n, int t) {
	int index = 0, pass = 0, minIndex = 0, minVal = 0;
	bool* found = new bool[t];
	for (int i = 0; i < t; i++) found[i] = false;

	cout << "\n" << "Memory Map: " << endl;

	for (int j = 0; j < t; j++) {
		if (symTable[j].type >= 0 && symTable[j].type <= 2) {
			found[j] = true;
		}
	}

	while (index < n) {
		int i = pass * 16;

		while (i < ((pass + 1) * 16) && i < n) {
			cout << i << "\t";
			i++;
		}
		cout << endl;

		while (index < i) {
			if (memblock[index] == ' ') cout << " \t";
			else if (isalpha(memblock[index]) || ispunct(memblock[index])) cout << memblock[index] << "\t";
			else printf("%02x\t", (unsigned char)memblock[index]);
			index++;
		}
		cout << endl << endl;
		pass++;
	}
	delete[] found;
}

void add(symbolTableEntry* symTable, unsigned char* memblock, string addVal1, string addVal2, int t) {
	int key1, key2=t+1, total, val1=0, val2=0, shift=3;

	for (int i = 0; i < t; i++) if (string(symTable[i].symbol) == addVal1) key1 = i;

	for (int i = 0; i < t; i++) if (string(symTable[i].symbol) == addVal2) key2 = i;

	for (int i = symTable[key1].offset; i < (symTable[key1].offset + 4); i++) val1 += (int)memblock[i];

	if (key2 > t) {
		istringstream(addVal2) >> val2;
		total = val1 + val2;
	}
	else {
		for (int i = symTable[key2].offset; i < (symTable[key2].offset + 4); i++) val2 += (int)memblock[i];
		total = val1 + val2;
	}

	for (int i = symTable[key1].offset; i < (symTable[key1].offset + 4); i++) {
		memblock[i] = (total >> (shift * 8)) & 0xFF;
		shift--;
	}
}

void strcat(symbolTableEntry* symTable, unsigned char* memblock, string inSymbol, string addVal1, int t) {
	int key1 = t + 1, key2 = t + 1, shift = 3;

	for (int i = 0; i < t; i++) if (string(symTable[i].symbol) == inSymbol) key1 = i;

	for (int i = 0; i < t; i++) if (string(symTable[i].symbol) == addVal1) key2 = i;

	int k = symTable[key1].offset;

	while (memblock[k] != '\0' && k < (symTable[key1].offset + symTable[key1].length)) k++;

	if (k < (symTable[key1].offset + symTable[key1].length)) {
		if (key2 > t) { 
			memblock[k] = addVal1[0]; 
			if (k+1 < (symTable[key1].offset + symTable[key1].length)) memblock[k+1] = '\0';
		}
		else
			for (int j = symTable[key2].offset; j < (symTable[key2].offset + symTable[key2].length); j++)
				if (memblock[j] != '\0' && k < (symTable[key1].offset + symTable[key1].length) && memblock[j] != ' ') {
					memblock[k] = memblock[j];
					k++;
					if (k < (symTable[key1].offset + symTable[key1].length)) memblock[k] = '\0';
				}

	}
	else cout << "No space in memory.";
}

void print(symbolTableEntry* symTable, unsigned char* memblock, string inSymbol, int t) {
	int key1 = 0, val1 = 0;

	for (int i = 0; i < t; i++) if (string(symTable[i].symbol) == inSymbol) key1 = i;

	int k = symTable[key1].offset;

	if (symTable[key1].type == 0) {
		for (int i = symTable[key1].offset; i < (symTable[key1].offset + 4); i++) val1 += (int)memblock[i];
		cout << "Var " << inSymbol << " contains: " << val1 << endl;
	}
	else {
		cout << "Var " << inSymbol << " contains: ";
		while (memblock[k] != '\0' && k < (symTable[key1].offset + symTable[key1].length)) {
			cout << memblock[k];
			k++;
		}
		cout << endl;
	}
}

void compact(heapEntry*& heap, int& heapArrSize) {
	cout << "Free blocks before compact" << endl;
	for (int i = 0; i < heapArrSize; i++) cout << "Offset: " << heap[i].offset << " " << "Size: " << heap[i].blockSize << " " << endl;
	cout << endl;
	
	int key, size = heapArrSize;
	int* adjacent = new int[heapArrSize];
	bool* adjKey = new bool[heapArrSize];

	for (int i = 0; i < heapArrSize; i++) {
		key = heap[i].offset + heap[i].blockSize;
		for (int j = 0; j < heapArrSize; j++) {
			if (key == heap[j].offset) {
				adjKey[i] = true;
				adjacent[i] = j;
				heapArrSize--;
			}
		}
	}

	heapEntry* temp = new heapEntry[heapArrSize];
	key = heapArrSize - 1;

	for (int i = 0; i < size; i++) {
		if (adjKey[i] == true) {
			temp[key].offset = heap[i].offset;
			temp[key].blockSize = heap[i].blockSize + heap[adjacent[i]].blockSize;
			key--;
		}
	}

	for (int i = 0; i < size; i++) {
		if (!(heap[i].offset >= temp[key].offset && heap[i].offset <= temp[key].blockSize)) {
			temp[key].offset = heap[i].offset;
			temp[key].blockSize = heap[i].blockSize;
			key--;
		}
	}

	// Shrink heap
	delete[] heap;
	heap = new heapEntry[heapArrSize];
	for (int i = 0; i < heapArrSize; i++) heap[i] = temp[i];

	cout << "Free blocks after compact" << endl;
	for (int i = 0; i < heapArrSize; i++) cout << "Offset: " << heap[i].offset << " " << "Size: " << heap[i].blockSize << " " << endl;
	cout << endl;

	delete[] temp;
	delete[] adjacent;
	delete[] adjKey;
}

void insert(bstNode*& bst, heapEntry* heap, symbolTableEntry*symTable, unsigned char* memblock, int& bstArrSize, int t, int inSize, unsigned long inValue, string varName) {
	bool added = false;
	int j = 0, k = 0, offsetVal, shift, origOffset;
	unsigned long rootVal = 0;
	bstArrSize++;
	bstNode* temp = new bstNode[bstArrSize];

	while (string(symTable[j].symbol) != varName) j++;

	offsetVal = symTable[j].offset;
	origOffset = offsetVal;

	for (int i = offsetVal; i < (offsetVal + 4); i++) rootVal += (int)memblock[i];

	while (bst[k].key != rootVal) k++;

	while (!added) {
		if (inValue < rootVal) {
			if (bst[k].left == -1) {
				added = true;

				bst[k].left = inValue;

				temp[bstArrSize - 1].key = inValue;
				temp[bstArrSize - 1].left = -1;
				temp[bstArrSize - 1].right = -1;

				shift = 3;
				for (int i = symTable[j].length; i < (symTable[j].length + 4); i++) {
					memblock[i] = (inValue >> (shift * 8)) & 0xFF;
					shift--;
				}
				shift = 7;
				for (int i = symTable[j].length + 4; i < (symTable[j].length + 12); i++) {
					memblock[i] = (-1 >> (shift * 8)) & 0xFF;
					shift--;
				}

				shift = 3;
				for (int i = offsetVal + 4; i < offsetVal + 8; i++) {
					memblock[i] = (symTable[j].length >> (shift * 8)) & 0xFF;
					shift--;
				}

				symTable[j].length += 12;
			}
			else {
				offsetVal = 0;
				for (int i = symTable[j].offset + 4; i < (symTable[j].offset + 8); i++) offsetVal += (int)memblock[i]; // get new offsetVal

				symTable[j].offset = offsetVal;

				rootVal = 0;
				for (int i = offsetVal; i < (offsetVal + 4); i++) rootVal += (int)memblock[i]; // get new rootVal

				k = 0;
				while (bst[k].key != rootVal) k++;
			}
		}
		else if (inValue > rootVal) {
			if (bst[k].right == -1) {
				added = true;

				bst[k].right = inValue;

				temp[bstArrSize - 1].key = inValue;
				temp[bstArrSize - 1].left = -1;
				temp[bstArrSize - 1].right = -1;

				shift = 3;
				for (int i = symTable[j].length; i < (symTable[j].length + 4); i++) {
					memblock[i] = (inValue >> (shift * 8)) & 0xFF;
					shift--;
				}
				shift = 7;
				for (int i = symTable[j].length + 4; i < (symTable[j].length + 12); i++) {
					memblock[i] = (-1 >> (shift * 8)) & 0xFF;
					shift--;
				}

				shift = 3;
				for (int i = offsetVal + 8; i < offsetVal + 12; i++) {
					memblock[i] = (symTable[j].length >> (shift * 8)) & 0xFF;
					shift--;
				}

				symTable[j].length += 12;
			}
			else {
				offsetVal = 0;
				for (int i = symTable[j].offset + 8; i < (symTable[j].offset + 12); i++) offsetVal += (int)memblock[i]; // get new offsetVal

				symTable[j].offset = offsetVal;

				rootVal = 0;
				for (int i = offsetVal; i < (offsetVal + 4); i++) rootVal += (int)memblock[i]; // get new rootVal

				k = 0;
				while (bst[k].key != rootVal) k++;
			}
		}
		else cout << "Value " << inValue << " already added." << endl;
	}

	symTable[j].offset = origOffset;

	for (int i = 0; i < bstArrSize - 1; i++) temp[i] = bst[i];

	// Grow bst
	delete[] bst;
	bst = new bstNode[bstArrSize];
	for (int i = 0; i < bstArrSize; i++) bst[i] = temp[i];
	delete[] temp;

	heap[0].blockSize -= inSize;
	heap[0].offset += inSize;
}