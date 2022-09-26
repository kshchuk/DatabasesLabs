#ifndef LARVAE_H_
#define LARVAE_H_

#include <stdio.h>

enum Stage
{
	kEgg = 1,
	kLarva,
	kPupa
};

const char** stages = new const char* [3] {
	{"Egg"},
	{ "Larva" },
	{ "Pupa" }
};

struct Larvae
{
	int larvae_id;
	char breed[40];
	int age;
	Stage stage;
	bool isFertilized;
};

struct LarvaeRecord
{
	Larvae larvae;
	// Record number in slave file
	int next_record_adress;
	bool isValid;
};

const int size_of_larvae_record = sizeof(int) + 40 * sizeof(char) + sizeof(int) +
sizeof(Stage) + sizeof(bool) + sizeof(int) + sizeof(bool);

void WriteToFile(LarvaeRecord lrecord, FILE* file)
{
	fwrite((char*)&lrecord.larvae.larvae_id, sizeof(int), 1, file);
	fwrite(lrecord.larvae.breed, 40 * sizeof(char), 1, file);
	fwrite((char*)&lrecord.larvae.age, sizeof(int), 1, file);
	fwrite((char*)&lrecord.larvae.stage, sizeof(Stage), 1, file);
	fwrite((char*)&lrecord.larvae.isFertilized, sizeof(bool), 1, file);

	fwrite((char*)&lrecord.next_record_adress, sizeof(int), 1, file);
	fwrite((char*)&lrecord.isValid, sizeof(bool), 1, file);
}

void ReadFromFile(FILE* file, LarvaeRecord& lrecord)
{
	fread((char*)&lrecord.larvae.larvae_id, sizeof(int), 1, file);
	fread(lrecord.larvae.breed, 40 * sizeof(char), 1, file);
	fread((char*)&lrecord.larvae.age, sizeof(int), 1, file);
	fread((char*)&lrecord.larvae.stage, sizeof(Stage), 1, file);
	fread((char*)&lrecord.larvae.isFertilized, sizeof(bool), 1, file);

	fread((char*)&lrecord.next_record_adress, sizeof(int), 1, file);
	fread((char*)&lrecord.isValid, sizeof(bool), 1, file);
}

void PrintLarvae(Larvae larvae)
{
	printf("---------------------------------------------------\n");
	printf("Larvae ID:	%i\n", larvae.larvae_id);
	printf("Breed:		%s\n", larvae.breed);
	printf("Age:		%i days\n", larvae.age);
	printf("Stage:		%s\n", stages[abs((int)larvae.stage - 1)]);
	char* true_false = new char[5];
	if (larvae.isFertilized)
		strcpy(true_false, "True");
	else
		strcpy(true_false, "False");
	printf("Is fertilized:  %s\n", true_false);
	printf("---------------------------------------------------\n");
}
#endif // !LARVAE_H_
