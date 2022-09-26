#define _CRT_SECURE_NO_DEPRECATE
#include<stdio.h>
#include<assert.h>
#include<Windows.h>

#include "Queen.h"
#include"Larvae.h"
#include"IndexFileRecord.h"

// Default file mode
#define FILE_MODE "rb+"

#define INDEX_FILE "Queen.ind"
#define MASTER_FILE "Queen.fl"
#define SLAVE_FILE "Larvae.fl"



// Returns array of index file records
IndexFileRecord* ReadIndexFile(FILE* index_file, int &records_count)
{
	fseek(index_file, 0, SEEK_END);
	records_count = ftell(index_file) / sizeof(IndexFileRecord);
	fseek(index_file, 0, SEEK_SET);

	IndexFileRecord* irecords = new IndexFileRecord[records_count];
	fread((char*)irecords, sizeof(IndexFileRecord), records_count, index_file);

	assert(!ferror(index_file));

	return irecords;
}

void WriteIndexFile(FILE* index_file, IndexFileRecord* records, int records_count)
{
	index_file = freopen(INDEX_FILE, "wb", index_file);
	fwrite((char*)records, sizeof(IndexFileRecord), records_count, index_file);
	fflush(index_file);
	index_file = freopen(INDEX_FILE, FILE_MODE, index_file);
}

// Returns address of the queen in irecords array
int BinarySearchOfTheQueen(int queen_id, IndexFileRecord* irecords, int records_count)
{
	int low = 0;
	int high = records_count - 1;
	while (low <= high)
	{
		int mid = (low + high) / 2;
		if (irecords[mid].key > queen_id)
			high = mid - 1;
		else
			if (irecords[mid].key < queen_id)
				low = mid + 1;
			else
			{
				if (irecords[mid].isValid == true) {
					return mid;
				}
				else
				{
					low = mid + 1;
					continue;
				}
			}
	}
	return -1;
	//for (int i = 0; i < records_count; i++)
	//{
	//	if (irecords[i].key == queen_id && irecords[i].isValid)
	//		return i;
	//}
	//return -1;
}

// Finds record with the queen in the index file 
int GetAddressInIndexFile(int queen_id, FILE* index_file)
{
	int records_number;
	IndexFileRecord* irecords = ReadIndexFile(index_file, records_number);

	int address = BinarySearchOfTheQueen(queen_id, irecords, records_number);
	delete[] irecords;
	return address;
}

// Returns queen's address in master file
int Get_M(int queen_id, FILE* index_file)
{
	int index_file_address = GetAddressInIndexFile(queen_id, index_file);

	if (index_file_address == -1)
		return -1;

	IndexFileRecord irecord;
	fseek(index_file, index_file_address * sizeof(IndexFileRecord), SEEK_SET);
	fread((char*)&irecord, sizeof(IndexFileRecord), 1, index_file);
	
	assert(irecord.key == queen_id);

	return irecord.address;
}

bool CorrectMasterFile(int old_larvae_address, int new_larvae_address, FILE* master_file)
{
	fseek(master_file, 0, SEEK_SET);
	int qrecord_address = 0;
	QueenRecord qrecord;
	while (!feof(master_file) && !ferror(master_file))
	{
		ReadFromFile(master_file, qrecord);

		if (qrecord.first_larvae_record_address == old_larvae_address)
		{
			qrecord.first_larvae_record_address = new_larvae_address;
			fseek(master_file, qrecord_address * size_of_queen_record, SEEK_SET);
			WriteToFile(qrecord, master_file);
			return true;
		}
		qrecord_address++;
	}
	return false;
}

bool CorrectSlaveFile(int old_larvae_address, int new_larvae_address, FILE* slave_file)
{
	fseek(slave_file, 0, SEEK_SET);
	int lrecord_address = 0;
	LarvaeRecord lrecord;
	while (!feof(slave_file) && !ferror(slave_file))
	{
		ReadFromFile(slave_file, lrecord);

		if (lrecord.next_record_adress == old_larvae_address)
		{
			lrecord.next_record_adress = new_larvae_address;
			fseek(slave_file, lrecord_address * size_of_larvae_record, SEEK_SET);
			WriteToFile(lrecord, slave_file);
			return true;
		}
		lrecord_address++;
	}
	return false;
}

bool CorrectIndexFile(int old_qrecord_address, int new_qrecord_address,
	IndexFileRecord* irecords, int irecord_count)
{
	for (int i = 0; i < irecord_count; i++)
	{
		if (irecords[i].address == old_qrecord_address)
		{
			irecords[i].address = new_qrecord_address;
			return true;
		}
	}
	return false;
}

QueenRecord GetQueenRecord(int queen_address, FILE* master_file)
{
	if (queen_address == -1)
	{
		printf("\nERROR: Element does not exist\n");
		return QueenRecord{};
	}

	QueenRecord qrecord;
	fseek(master_file, queen_address * size_of_queen_record, SEEK_SET);
	ReadFromFile(master_file, qrecord);

	return qrecord;
}

int Get_S(int larvae_id, int queen_id,
	FILE* index_file, FILE* master_file, FILE* slave_file)
{
	int queen_record_address = Get_M(queen_id, index_file);
	QueenRecord qrecord = GetQueenRecord(queen_record_address, master_file);

	// Search larvae record in slave file
	LarvaeRecord lrecord{};
	int cur_record_address = qrecord.first_larvae_record_address;
	int prev_record_address = 0;
	while ((lrecord.larvae.larvae_id != larvae_id || lrecord.isValid == false)
		&& cur_record_address != -1 && !feof(slave_file) && !ferror(slave_file))
	{
		prev_record_address = cur_record_address;
		fseek(slave_file, cur_record_address * size_of_larvae_record, SEEK_SET);
		ReadFromFile(slave_file, lrecord);
		cur_record_address = lrecord.next_record_adress;
	}

	if (lrecord.larvae.larvae_id == larvae_id)
		return prev_record_address;
	else
		return -1;
}

LarvaeRecord GetLarvaeRecord(int larvae_address, FILE* slave_file)
{
	if (larvae_address == -1)
	{
		printf("\nERROR: Element does not exist\n");
		return LarvaeRecord{};
;	}

	LarvaeRecord lrecord;
	fseek(slave_file, larvae_address * size_of_larvae_record, SEEK_SET);
	ReadFromFile(slave_file, lrecord);

	return lrecord;
}

void Del_S(int larvae_id, int queen_id,
	FILE* index_file, FILE* master_file, FILE* slave_file)
{
	int larvae_address = Get_S(larvae_id, queen_id,
		index_file, master_file, slave_file);

	if (larvae_address == -1) return;

	LarvaeRecord lrecord;
	fseek(slave_file, larvae_address * size_of_larvae_record, SEEK_SET);
	ReadFromFile(slave_file, lrecord);
	
	lrecord.isValid = false;
	fseek(slave_file, larvae_address * size_of_larvae_record, SEEK_SET);
	WriteToFile(lrecord, slave_file);
}

void Del_M(int queen_id, FILE* index_file, FILE* master_file, FILE* slave_file)
{
	int queen_address = Get_M(queen_id, index_file);

	if (queen_address == -1) return;
	
	// Invalidate record in master file
	QueenRecord qrecord;
	fseek(master_file, queen_address * size_of_queen_record, SEEK_SET);
	ReadFromFile(master_file, qrecord);

	qrecord.isValid = false;
	fseek(master_file, queen_address * size_of_queen_record, SEEK_SET);
	WriteToFile(qrecord, master_file);

	// Invalidate record in index file
	int index_file_record_address = GetAddressInIndexFile(queen_id, index_file);
	IndexFileRecord irecord;
	fseek(index_file, index_file_record_address * sizeof(IndexFileRecord), SEEK_SET);
	fread((char*)&irecord, sizeof(IndexFileRecord), 1, index_file);

	irecord.isValid = false;
	fseek(index_file, index_file_record_address * sizeof(IndexFileRecord), SEEK_SET);
	fwrite((char*)&irecord, sizeof(IndexFileRecord), 1, index_file);

	// Invalidate all slave records
	LarvaeRecord lrecord;
	int cur_lrecord_address = qrecord.first_larvae_record_address;
	while (cur_lrecord_address != -1) 
	{
		fseek(slave_file, cur_lrecord_address * size_of_larvae_record, SEEK_SET);
		ReadFromFile(slave_file, lrecord);
		// Queen is deleted - del_s cannot be used
		lrecord.isValid = false;
		fseek(slave_file, cur_lrecord_address * size_of_larvae_record, SEEK_SET);
		WriteToFile(lrecord, slave_file);
		cur_lrecord_address = lrecord.next_record_adress;
	}
}

void GarbageCollector(FILE* index_file, FILE* master_file, FILE* slave_file,
	const char* index_filename, const char* master_filename, const char* slave_filename)
{
	// Read valid records from index file into RAM
	int non_valid_irecords_count;
	IndexFileRecord* irecords = ReadIndexFile(index_file, non_valid_irecords_count);
	IndexFileRecord* valid_irecords = new IndexFileRecord[non_valid_irecords_count];
	int cur_valid_irecord = 0;
	for (int i = 0; i < non_valid_irecords_count; i++)
	{
		if (irecords[i].isValid)
			valid_irecords[cur_valid_irecord++] = irecords[i];
	}
	int valid_irecords_count = cur_valid_irecord;
	delete irecords;


	// Get rid of garbage in slave file
	FILE* temp = fopen("temp.fl", "wb");
	LarvaeRecord lrecord;
	int old_larvae_address = 0;
	int	new_larvae_address = 0;
	fseek(slave_file, 0, SEEK_SET);
	int pos = getc(slave_file);
	while (!feof(slave_file) && !ferror(slave_file) && pos != EOF)
	{
		fseek(slave_file, -1, SEEK_CUR);

		ReadFromFile(slave_file, lrecord);

		if (lrecord.isValid)
		{
			WriteToFile(lrecord, temp);

			if (old_larvae_address != new_larvae_address)
			{
				if (!CorrectMasterFile(old_larvae_address, new_larvae_address,
						master_file))
				{
					CorrectSlaveFile(old_larvae_address, new_larvae_address,
						slave_file);
				}
			}

			new_larvae_address++;
		}

		old_larvae_address++;
		pos = getc(slave_file);
	}


	// Replace slave file
	fflush(slave_file);
	fclose(slave_file);
	fflush(temp);
	fclose(temp);
	remove(slave_filename);
	rename("temp.fl", slave_filename);
	slave_file = fopen(slave_filename, FILE_MODE);


	// Get rid of garbage in master file
	QueenRecord qrecord;
	temp = fopen("temp.fl", "wb");
	int old_qrecord_address = 0;
	int new_qrecord_address = 0;
	fseek(master_file, 0, SEEK_SET);
	pos = getc(master_file);
	while (!feof(master_file) && !ferror(master_file) && pos != EOF)
	{
		fseek(master_file, -1, SEEK_CUR);

		ReadFromFile(master_file, qrecord);

		if (qrecord.isValid)
		{
			WriteToFile(qrecord, temp);

			if (old_qrecord_address != new_qrecord_address)
				CorrectIndexFile(old_qrecord_address, new_qrecord_address,
					valid_irecords, valid_irecords_count);

			new_qrecord_address++;
		}

		old_qrecord_address++;
		pos = getc(master_file);
	}


	// Replace master file
	fclose(master_file);
	fflush(temp);
	fclose(temp);
	remove(master_filename);
	rename("temp.fl", master_filename);
	master_file = fopen(master_filename, FILE_MODE);


	// Update index file
	WriteIndexFile(index_file, valid_irecords, valid_irecords_count);
	delete[] valid_irecords;
}

void Update_M(Queen queen,
	FILE* index_file, FILE* master_file)
{
	int queen_address = Get_M(queen.queen_id, index_file);
	if (queen_address == -1)
		return;
	QueenRecord qrecord = GetQueenRecord(queen_address, master_file);

	qrecord.queen = queen;
	fseek(master_file, queen_address * size_of_queen_record, SEEK_SET);
	WriteToFile(qrecord, master_file);
}

void Update_S(Larvae larvae, int queen_id,
	FILE* index_file, FILE* master_file, FILE* slave_file)
{
	int larvae_address = Get_S(larvae.larvae_id, queen_id,
		index_file, master_file, slave_file);
	if (larvae_address == -1)
		return;
	LarvaeRecord lrecord = GetLarvaeRecord(larvae_address, slave_file);

	lrecord.larvae = larvae;
	fseek(slave_file, larvae_address * size_of_larvae_record, SEEK_SET);
	WriteToFile(lrecord, slave_file);
}

void Insert_M(Queen queen, FILE* index_file, FILE* master_file)
{
	// Write in index file
	int queens_count;
	IndexFileRecord* irecords = ReadIndexFile(index_file, queens_count);
	index_file = freopen(INDEX_FILE, "wb", index_file);

	if (queens_count == 0)
	{
		IndexFileRecord new_irecord{ queen.queen_id, 0, true };
		fwrite((char*)&new_irecord, sizeof(IndexFileRecord), 1, index_file);
	}
	else
	{
		for (int i = 0; i < queens_count; i++)
		{
			if (irecords[i].key > queen.queen_id)
			{
				IndexFileRecord new_irecord{ queen.queen_id, queens_count, true };
				fwrite((char*)&new_irecord, sizeof(IndexFileRecord), 1, index_file);
				while (i < queens_count)
				{
					fwrite((char*)&irecords[i++], sizeof(IndexFileRecord), 1, index_file);
				}
				break;
			}
			else
			{
				fwrite((char*)&irecords[i], sizeof(IndexFileRecord), 1, index_file);

				if (i + 1 == queens_count) // Last record
				{
					IndexFileRecord new_irecord{ queen.queen_id, queens_count, true };
					fwrite((char*)&new_irecord, sizeof(IndexFileRecord), 1, index_file);
				}
			}
		}
	}
	delete[] irecords;
	fflush(index_file);
	index_file = freopen(INDEX_FILE, FILE_MODE, index_file);

	// Append master file
	master_file = freopen(MASTER_FILE, "ab", master_file);
	QueenRecord qrecord{ queen, -1, true };
	WriteToFile(qrecord, master_file);
	fflush(master_file);
	master_file = freopen(MASTER_FILE, FILE_MODE, master_file);
}

void Insert_S(Larvae larvae, int queen_id,
	FILE* index_file, FILE* master_file, FILE* slave_file)
{
	int queen_address = Get_M(queen_id, index_file);
	if (queen_address == -1) 
		return;

	QueenRecord qrecord;
	fseek(master_file, queen_address * size_of_queen_record, SEEK_SET);
	ReadFromFile(master_file, qrecord);

	LarvaeRecord lrecord{ larvae, qrecord.first_larvae_record_address, true };
	slave_file = freopen(SLAVE_FILE, "ab", slave_file);
	WriteToFile(lrecord, slave_file);
	slave_file = freopen(SLAVE_FILE, FILE_MODE, slave_file);

	fseek(slave_file, 0, SEEK_END);
	int slave_records_count = ftell(slave_file) / size_of_larvae_record;

	qrecord.first_larvae_record_address = slave_records_count - 1;
	fseek(master_file, queen_address * size_of_queen_record, SEEK_SET);
	WriteToFile(qrecord, master_file);
}





// User interface




void AddQueen(FILE* index_file, FILE* master_file)
{
	system("cls");
	Queen queen;
	printf("Type ID:     ");
	scanf("%i", &queen.queen_id);
	printf("Type breed:  ");
	scanf("%s", queen.breed);
	printf("Type age:    ");
	scanf("%i", &queen.age);
	printf("Type reproduce ability ('1' - true, '0' - false): \n");
	int true_false;
	scanf("%i", &true_false);
	queen.reproduce_ability = (bool)true_false;

	Insert_M(queen, index_file, master_file);
}

void GetQueen(FILE* index_file, FILE* master_file, FILE* slave_file)
{
	system("cls");

	int queen_id;
	printf("Type ID:	");
	scanf("%i", &queen_id);

	int queen_address = Get_M(queen_id, index_file);
	QueenRecord qrecord = GetQueenRecord(queen_address, master_file);
	PrintQueen(qrecord.queen);
}

void RemoveQueen(FILE* index_file, FILE* master_file, FILE* slave_file)
{
	system("cls");

	int queen_id;
	printf("Type ID:	");
	scanf("%i", &queen_id);

	Del_M(queen_id, index_file, master_file, slave_file);
}

void UpdateQueen(FILE* index_file, FILE* master_file)
{
	system("cls");

	int queen_id;
	printf("Type ID:	");
	scanf("%i", &queen_id);

	int queen_address = Get_M(queen_id, index_file);
	QueenRecord qrecord = GetQueenRecord(queen_address, master_file);
	
	enum EditingMenu
	{
		kBreed = 1,
		kAge,
		kReproduceAbility,
		kUpdate = 0
	};

	while (true)
	{
		printf("\nWhat to edit:\n");
		printf("\n	1 - Breed");
		printf("\n	2 - Age");
		printf("\n	3 - Reproduce ability");
		printf("\n	0 - Update\n");

		EditingMenu ans;
		scanf("%i", &ans);
		switch (ans)
		{
		case kBreed:
			printf("\nEnter new breed:	");
			scanf("%s", qrecord.queen.breed);
			break;
		case kAge:
			printf("Enter new age:	");
			scanf("%i", &qrecord.queen.age);
			break;
		case kReproduceAbility:
			printf("Enter new reproduce ability (1 - true, 0 - false):	");
			int true_false;
			scanf("%i", &true_false);
			qrecord.queen.reproduce_ability = (bool)true_false;
			break;
		case kUpdate:
			Update_M(qrecord.queen, index_file, master_file);
			return;
		default:
			continue;
		}
	}
}

void AddLarvae(FILE* index_file, FILE* master_file, FILE* slave_file)
{
	system("cls");

	int queen_id;
	printf("Type Queen ID:   ");
	scanf("%i", &queen_id);

	Larvae larvae;
	printf("Type Larvae ID:  ");
	scanf("%i", &larvae.larvae_id);
	printf("Type breed:      ");
	scanf("%s", larvae.breed);
	printf("Type age:        ");
	scanf("%i", &larvae.age);

	printf("\n	Stages:      \n");
	printf("1 - %s\n", stages[0]);
	printf("2 - %s\n", stages[1]);
	printf("3 - %s\n", stages[2]);

	printf("\nType stage:    ");
	scanf("%i", &larvae.stage);
	printf("Is larvae fertilized? (1 - true, 0 - false):	\n");
	int true_false;
	scanf("%i", &true_false);
	larvae.isFertilized = (bool)true_false;

	Insert_S(larvae, queen_id, index_file, master_file, slave_file);
}

void GetLarvae(FILE* index_file, FILE* master_file, FILE* slave_file)
{
	system("cls");

	int queen_id;
	printf("Type Queen ID:	");
	scanf("%i", &queen_id);

	int larvae_id;
	printf("Type Larvae ID:	");
	scanf("%i", &larvae_id);

	int larvae_address = Get_S(larvae_id, queen_id, 
		index_file, master_file, slave_file);
	LarvaeRecord lrecord = GetLarvaeRecord(larvae_address, slave_file);
	PrintLarvae(lrecord.larvae);
}

void RemoveLarvae(FILE* index_file, FILE* master_file, FILE* slave_file)
{
	system("cls");

	int queen_id;
	printf("Type Queen ID:	");
	scanf("%i", &queen_id);

	int larvae_id;
	printf("Type Larvae ID:	");
	scanf("%i", &larvae_id);

	Del_S(larvae_id, queen_id,
		index_file, master_file, slave_file);
}

void UpdateLarvae(FILE* index_file, FILE* master_file, FILE* slave_file)
{
	system("cls");

	int queen_id;
	printf("Type Queen ID:	");
	scanf("%i", &queen_id);

	int larvae_id;
	printf("Type Larvae ID:	");
	scanf("%i", &larvae_id);

	int larvae_address = Get_S(larvae_id, queen_id,
		index_file, master_file, slave_file);
	LarvaeRecord lrecord = GetLarvaeRecord(larvae_address, slave_file);
	
	enum EditingMenu
	{
		kBreed = 1,
		kAge,
		kStage,
		kFertilization,
		kUpdate = 0
	};


	while (true)
	{
		printf("\nWhat to edit:\n");
		printf("\n	1 - Breed");
		printf("\n	2 - Age");
		printf("\n	3 - Stage");
		printf("\n	4 - Fertilization");
		printf("\n	0 - Update\n");

		EditingMenu ans;
		scanf("%i", &ans);
		switch (ans)
		{
		case kBreed:
			printf("\nEnter new breed:	");
			scanf("%s", lrecord.larvae.breed);
			break;
		case kAge:
			printf("\nEnter new age:	");
			scanf("%i", &lrecord.larvae.age);
			break;
		case kStage:
			printf("\n	Stages: \n");
			printf("1 - %s\n", stages[0]);
			printf("2 - %s\n", stages[1]);
			printf("3 - %s\n", stages[2]);

			printf("\nEnter new stage:		");
			scanf("%i", &lrecord.larvae.stage);
			break;
		case kFertilization:
			printf("\nIs larvae fertilized? (1 - true, 0 - false):	");
			int true_false;
			scanf("%i", &true_false);
			lrecord.larvae.isFertilized = (bool)true_false;
			break;
		case kUpdate:
			Update_S(lrecord.larvae, queen_id,
				index_file, master_file, slave_file);
			return;
		default:
			continue;
		}
	}
}

void PrintIndexFile(FILE* index_file)
{
	fseek(index_file, 0, SEEK_SET);
	IndexFileRecord irecord;
	printf("\n\n\nKey | Address in master file | Validity\n\n");
	int pos = getc(index_file);
	while (!feof(index_file) && !ferror(index_file) && pos != EOF)
	{
		fseek(index_file, -1, SEEK_CUR);

		fread((char*)&irecord, sizeof(IndexFileRecord), 1, index_file);
		char* true_false = new char[7];
		irecord.isValid ? strcpy(true_false, "True") : strcpy(true_false, "False");
		printf("\n%i           %i               %s\n", irecord.key, irecord.address, true_false);

		pos = getc(index_file);
	}
}

void PrintMasterFile(FILE* master_file)
{
	fseek(master_file, 0, SEEK_SET);
	QueenRecord qrecord;
	printf("\n\n\nQueen ID | Address of the first slave record | Validity\n\n");
	int pos = getc(master_file);
	while (!feof(master_file) && !ferror(master_file) && pos != EOF)
	{
		fseek(master_file, -1, SEEK_CUR);

		ReadFromFile(master_file, qrecord);
		char* true_false = new char[7];
		qrecord.isValid ? strcpy(true_false, "True") : strcpy(true_false, "False");
		printf("\n%i                %i                          %s\n",
			qrecord.queen.queen_id, qrecord.first_larvae_record_address, true_false);

		pos = getc(master_file);
	}
}

void PrintSlaveFile(FILE* slave_file)
{
	fseek(slave_file, 0, SEEK_SET);
	LarvaeRecord lrecord;
	printf("\n\n\Larvae ID | Address of the next slave record | Validity\n\n");
	int pos = getc(slave_file);
	while (!feof(slave_file) && !ferror(slave_file) && pos != EOF)
	{
		fseek(slave_file, -1, SEEK_CUR);

		ReadFromFile(slave_file, lrecord);
		char* true_false = new char[7];
		lrecord.isValid ? strcpy(true_false, "True") : strcpy(true_false, "False");
		printf("\n%i                %i                          %s\n",
			lrecord.larvae.larvae_id, lrecord.next_record_adress, true_false);

		pos = getc(slave_file);
	}
}

void Menu(FILE* index_file, FILE* master_file, FILE* slave_file)
{
	enum MainMenu
	{
		kAddQueen = 1,
		kGetQueen,
		kRemoveQueen,
		kUpdateQueen,
		kAddLarvae,
		kGetLarvae,
		kRemoveLarvae,
		kUpdateLarvae,
		kCleanUpGarbage,
		kPrintIndexFile,
		kPrintMasterFile,
		kPrintSlaveFile,
		kExit = 0
	};
	while (true) {
		printf("\n\n	1 - Add Queen\n");
		printf("	2 - Get Queen\n");
		printf("	3 - Remove Queen\n");
		printf("	4 - Update Queen\n");
		printf("	5 - Add Larvae\n");
		printf("	6 - Get Larvae\n");
		printf("	7 - Remove Larvae\n");
		printf("	8 - Update Larvae\n");
		printf("	9 - Clean up the garbage\n");
		printf("\nDebugging tools:\n");
		printf("	10 - Print index file\n");
		printf("	11 - Print master file\n");
		printf("	12 - Print slave file\n\n");
		printf("    0 - Exit\n\n");

		MainMenu ans;
		scanf("%i", &ans);
		switch (ans)
		{
		case kAddQueen:
			AddQueen(index_file, master_file);
			break;
		case kGetQueen:
			GetQueen(index_file, master_file, slave_file);
			break;
		case kRemoveQueen:
			RemoveQueen(index_file, master_file, slave_file);
			break;
		case kUpdateQueen:
			UpdateQueen(index_file, master_file);
			break;
		case kAddLarvae:
			AddLarvae(index_file, master_file, slave_file);
			break;
		case kGetLarvae:
			GetLarvae(index_file, master_file, slave_file);
			break;
		case kRemoveLarvae:
			RemoveLarvae(index_file, master_file, slave_file);
			break;
		case kUpdateLarvae:
			UpdateLarvae(index_file, master_file, slave_file);
			break;
		case kCleanUpGarbage:
			GarbageCollector(index_file, master_file, slave_file,
				INDEX_FILE, MASTER_FILE, SLAVE_FILE);
			break;
		case kPrintIndexFile:
			PrintIndexFile(index_file);
			break;
		case kPrintMasterFile:
			PrintMasterFile(master_file);
			break;
		case kPrintSlaveFile:
			PrintSlaveFile(slave_file);
			break;
		case kExit:
			fclose(index_file);
			fclose(master_file);
			fclose(slave_file);
			exit(0);
			break;
		default:
			continue;
		}
	}
}

int main()
{
	FILE* index_file = fopen("Queen.ind", FILE_MODE);
	FILE* master_file = fopen("Queen.fl", FILE_MODE);
	FILE* slave_file = fopen("Larvae.fl", FILE_MODE);

	Menu(index_file, master_file, slave_file);

}
