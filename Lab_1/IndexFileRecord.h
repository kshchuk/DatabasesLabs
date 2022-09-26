struct IndexFileRecord
{
	int key;
	// Record number in master file
	int address;
	bool isValid;
};