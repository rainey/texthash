//Author: Shawn 'sintaxerror' Rainey
//Do what you want with it.

#include <fstream>
#include <iostream>
#include <string.h>
#include <cmath>
#include "hr_time.h"

using namespace std;

const int numEntries = 50000;
const int numLookups = 10000;

unsigned int caseInsensitiveHash(const char  *str);

const int tableSize = 0x3FFFF;
//Simple chained bucket entries
class tableEntry{
	const char* word;
	int value;
	tableEntry* next;
public:
	tableEntry(): word(0), value(0), next(0) {
	}

	//Returns the length of this bucket chain after insertion
	//or 0 if duplicate
	int insert(const char* word, const int value)
	{
		tableEntry* insertPoint = this;
		int returnVal = 1;
		
		//Until we reach the last 
		while(insertPoint->word != 0)
		{
			//String already in table.  Replace value
			if(!strcmp(this->word, word))
			{
				this->value = value;
				return 0;
			}

			//make a new blank tableEntry as the last one
			if(0 == insertPoint->next)
			{
				insertPoint->next = new tableEntry();
				returnVal++;
			}

			insertPoint = insertPoint->next;
		}
		//insertPoint->word = new char[strlen(word)+1];
		insertPoint->word = word;
		//strcpy(insertPoint->word, word);
		insertPoint->value = value;
		return returnVal;
	}

	//Return value mapped to given string
	//or -1 if not found.
	int valueOf(const char* string)
	{
		if(0 == this->word) return -1;
		int returnVal = 0;
		tableEntry* foundPoint = this;
		while(returnVal != -1)
		{
			if(!strcmp(foundPoint->word, string))
			{
				returnVal = foundPoint->value;
				break;
			}
			else
			{
				if(foundPoint->next == 0)
				{
					returnVal = -1;
				}
				else
				{
					foundPoint = foundPoint->next;
				}
			}
		}
		return returnVal;
	}

	~tableEntry()
	{
		//delete[] word;
		delete next;	
		//word = 0;
		//value = 0;
		//next = 0;
	}
} hashTable[tableSize];

//Simple hash for single words, returns a number 0 through (tableLength-1)
unsigned int caseInsensitiveHash(const char* str)
{
	//Length of a minimal representation of a letter: 5 bits
	static const int sOfLetter = 5;

	//Size of the hash in bits
	//Note: 21 is not an arbitrary value.  This length produces the least
	//amount of collisions for the given input.
	static const int sOfInt = 21;//(int)(log(tableSize)/log(2));

	int offset = 0;
	
	unsigned int hash = 0;//0x55555555;
	unsigned int previousOffset = offset;
	unsigned const int maxStartOffset = sOfInt - sOfLetter;
	
	while(*str != 0)
	{
		hash ^= (unsigned(toupper(*str++))
				- 64)
				<< offset;

		offset+=sOfLetter;
		
		if(offset >= sOfInt-sOfLetter)
		{
			offset = (++previousOffset % (maxStartOffset));
		}
	}
	
	if((hash & tableSize) == tableSize)
		hash = tableSize-1;
		
	return (hash & tableSize);
}

inline int AddWord(const char * text, int value)
{
	return hashTable[caseInsensitiveHash(text)].insert(text, value);
}

inline int GetValue(const char * text)
{
	return hashTable[caseInsensitiveHash(text)].valueOf(text);
}

//Reads a whole file into the contents buffer.
//Allocates a new contents array using new
void readFile(const char *const fname, char * &contents, long &size)
{
	//Probably not the right thing to do
	if(contents != 0) return;

	filebuf *fbuf;
	ifstream inReader(fname);
	fbuf = inReader.rdbuf();
	size = fbuf->pubseekoff(0, ios::end, ios::in);
	fbuf->pubseekpos(0, ios::in);

	contents = new char[size];
	fbuf->sgetn(contents, size);
	inReader.close();
}

int main()
{
	char* buffer[2] = {0, 0};
	long size50, size10;

	CStopWatch s;
	readFile("50000words.txt", buffer[0], size50);
	readFile("10000words.txt", buffer[1], size10);

	
	char *entryList[numEntries],
		*lookupList[numLookups];
	int entryVals[numEntries],
		lookupVals[numLookups];

	const char delim[] = ", \n\r";
	
	int i = 0;
	entryList[0] = strtok(buffer[0], delim);
	
	//cout << entryList[0] << endl;	//Tetraheism
	
	while(entryList[i] != NULL && i < numEntries)
	{
		entryVals[i] = atoi(strtok(NULL, delim));
		entryList[++i] = strtok(NULL, delim);
	}
	
	//cout << (void*)entryList[0] << endl;	//0x7c0020
	//cout << entryList[1] << endl;	//Chrisoms
	
	
	lookupList[0] = strtok(buffer[1], delim);
	
	//cout << (void*)entryList[0] << endl;	//0x7c0020
	//cout << entryList[1] << endl;	//Chrisoms
	
	i = 0;
	while(lookupList[i] != NULL && i < numLookups)
	{
		lookupList[++i] = strtok(NULL, delim);
	}
	
	entryList[0] = buffer[0];
	//cout << (void*)entryList[0] << endl;	//0x8a8b2f  <-- WTF strtok
	//cout << entryList[1] << endl;	//Chrisoms
	
	int numCollisions = 0;
	
	//timer code goes here
	i = 0;
	while(entryList[i] != NULL && i < numEntries)
	{
		//Collided if AddWord returns > 1
		numCollisions += (AddWord(entryList[i], entryVals[i]) > 1? 1:0);
		
		//entryList[i] = 0;
		++i;
	}
	
	//DONE LOADING HERE - Start the timer!
	s.startTimer();
	i = 0;
	int sum = 0;
	while(lookupList[i] != NULL && i < numLookups)
	{
		sum += lookupVals[i] = GetValue(lookupList[i]);
		++i;
	}
	
	//DONE TIMING
	s.stopTimer();
	i=0;
	while(lookupList[i] != NULL && i < numLookups)
	{
		//if(lookupVals[i] == -1)
		//{
		//	cout << entryList[0] << " " << entryVals[0] << endl;
		//	cout << lookupList[i] <<  caseInsensitiveHash(lookupList[i]) << endl;
		//	cout << hashTable[caseInsensitiveHash(lookupList[i])].valueOf(lookupList[i]) << endl;
		//} 
		//cout << lookupList[i] << " " << lookupVals[i] << endl;
		lookupList[i] = 0;
		++i;
	}
	
	cout << "Number of collisions: " << numCollisions << endl;
	cout << s.getElapsedTime() << endl;
	//Should be 496637501
	cout << sum << endl;
	delete [] buffer[0];
	delete [] buffer[1];
	return 0;
}
