#ifndef QUEEN_H_

#define QUEEN_H_

#include <stdio.h>

struct Queen 
{
	int queen_id;
	char breed[40];
	int age;
	bool reproduce_ability;
};

struct QueenRecord
{
	Queen queen;
	// Record address in slave file
	int first_larvae_record_address;
	bool isValid;
};

const int size_of_queen_record = (sizeof(int) + 40 * sizeof(char) + sizeof(int) +
	sizeof(bool) + sizeof(int) + sizeof(bool));

void WriteToFile(QueenRecord qrecord, FILE* file)
{
	fwrite((char*)&qrecord.queen.queen_id, sizeof(int), 1, file);
	fwrite(qrecord.queen.breed, 40 * sizeof(char), 1, file);
	fwrite((char*)&qrecord.queen.age, sizeof(int), 1, file);
	fwrite((char*)&qrecord.queen.reproduce_ability, sizeof(bool), 1, file);

	fwrite((char*)&qrecord.first_larvae_record_address, sizeof(int), 1, file);
	fwrite((char*)&qrecord.isValid, sizeof(bool), 1, file);		
}

void  ReadFromFile(FILE* file, QueenRecord& qrecord)
{
	fread((char*)&qrecord.queen.queen_id, sizeof(int), 1, file);
	fread(qrecord.queen.breed, 40 * sizeof(char), 1, file);
	fread((char*)&qrecord.queen.age, sizeof(int), 1, file);
	fread((char*)&qrecord.queen.reproduce_ability, sizeof(bool), 1, file);

	fread((char*)&qrecord.first_larvae_record_address, sizeof(int), 1, file);
	fread((char*)&qrecord.isValid, sizeof(bool), 1, file);
}

void PrintQueen(Queen queen)
{
	printf("---------------------------------------------------\n");
	printf("Queen ID:	%i\n", queen.queen_id);
	printf("Breed:		%s\n", queen.breed);
	printf("Age:		%i days\n", queen.age);
	char* true_false = new char[5];
	if (queen.reproduce_ability)
		strcpy(true_false, "True");
	else
		strcpy(true_false, "False");
	printf("Reproduce ability: %s\n", true_false);
	printf("---------------------------------------------------\n");
}
#endif // !QUEEN_H_