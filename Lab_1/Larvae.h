#ifndef LARVAE_H_
#define LARVAE_H_

#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <stdbool.h>



typedef enum
{
	kEgg = 1,
	kLarva,
	kPupa
} Stage;


char stages[3][10] = {
{ "Egg" },
{ "Larva" },
{ "Pupa" }
};


typedef struct 
{
	int larvae_id;
	char breed[40];
	int age;
	Stage stage;
	bool isFertilized;
} Larvae;

typedef struct 
{
	Larvae larvae;
	// Record number in slave file
	int next_record_adress;
	bool isValid;
} LarvaeRecord;

const int size_of_larvae_record = sizeof(int) + 40 * sizeof(char) + sizeof(int) +
sizeof(Stage) + sizeof(bool) + sizeof(int) + sizeof(bool);

void WriteLarvaeToFile(LarvaeRecord lrecord, FILE* file)
{
	fwrite((char*)&lrecord.larvae.larvae_id, sizeof(int), 1, file);
	fwrite(lrecord.larvae.breed, 40 * sizeof(char), 1, file);
	fwrite((char*)&lrecord.larvae.age, sizeof(int), 1, file);
	fwrite((char*)&lrecord.larvae.stage, sizeof(Stage), 1, file);
	fwrite((char*)&lrecord.larvae.isFertilized, sizeof(bool), 1, file);

	fwrite((char*)&lrecord.next_record_adress, sizeof(int), 1, file);
	fwrite((char*)&lrecord.isValid, sizeof(bool), 1, file);
}

void ReadLarvaeFromFile(FILE* file, LarvaeRecord* lrecord)
{
	fread((char*)&lrecord->larvae.larvae_id, sizeof(int), 1, file);
	fread(lrecord->larvae.breed, 40 * sizeof(char), 1, file);
	fread((char*)&lrecord->larvae.age, sizeof(int), 1, file);
	fread((char*)&lrecord->larvae.stage, sizeof(Stage), 1, file);
	fread((char*)&lrecord->larvae.isFertilized, sizeof(bool), 1, file);

	fread((char*)&lrecord->next_record_adress, sizeof(int), 1, file);
	fread((char*)&lrecord->isValid, sizeof(bool), 1, file);
}

void PrintLarvae(Larvae larvae)
{
	printf("---------------------------------------------------\n");
	printf("Larvae ID:	%i\n", larvae.larvae_id);
	if (strlen(larvae.breed) != 0)
		printf("Breed:		%s\n", larvae.breed);
	printf("Age:		%i days\n", larvae.age);
	printf("Stage:		%s\n", stages[abs((int)larvae.stage % 4 - 1)]);
	char* true_false = malloc(10);
	if (larvae.isFertilized)
		strcpy(true_false, "True");
	else
		strcpy(true_false, "False");
	printf("Is fertilized:  %s\n", true_false);
	printf("---------------------------------------------------\n");
}
#endif // !LARVAE_H_
