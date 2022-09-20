#include <cstdio>
#include <windows.h>
#include <winnt.h>
#include <fileapi.h>
#include <WinBase.h>
#include <time.h>
#include <inttypes.h>

#include "skCrypt.h"

#pragma warning(disable : 4996)

int main(int argc, char** argv);
int randomize_file(const char* path, uint32_t added_file_size);

int main(int argc, char** argv)
{
	printf(skCrypt("\nMade by Baitinq.\n\n"));

	if (argc < 3)
	{
		printf(skCrypt("BAD USAGE!\tUsage: disguise_file.exe <file_name> <added file size in bytes>\n"));
		system(skCrypt("pause"));

		return 1;
	}

	char executable_path[MAX_PATH];
	strncpy(executable_path, argv[1], sizeof(executable_path));

	const uint32_t added_size = atoi(argv[2]);

	srand(time(NULL));

	printf(skCrypt("Disguising your file...\n"));

	int ret = randomize_file(executable_path, added_size);
	if (ret < 0)
		printf(skCrypt("Failed to disguise your file. (%d)\n"), -ret);

	printf(skCrypt("Disguised your file succesfully!\n"));

	system(skCrypt("pause"));

	return ret;
}

int randomize_file(const char* path, uint32_t added_file_size)
{
	HANDLE file = CreateFile(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == NULL || file == INVALID_HANDLE_VALUE)
	{
		//printf("BAD FILE!\n");
		return -1;
	}

	PIMAGE_DOS_HEADER dosHeader;
	PIMAGE_NT_HEADERS ntHeader;
	PIMAGE_FILE_HEADER header;

	HANDLE hMapObject = CreateFileMapping(file, NULL, PAGE_READWRITE, 0, GetFileSize(file, NULL) + added_file_size, NULL);
	if (hMapObject == NULL || hMapObject == INVALID_HANDLE_VALUE)
	{
		//printf("BAD mapping!\n");
		return -2;
	}

	void* mapped_file = MapViewOfFile(hMapObject, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

	dosHeader = (PIMAGE_DOS_HEADER)mapped_file;
	if (dosHeader == NULL || dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		//printf("bad dosheader!\n");
		return -3;
	}

	ntHeader = (PIMAGE_NT_HEADERS)((uint8_t*)dosHeader + dosHeader->e_lfanew);
	if(ntHeader == NULL || ntHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		//printf("bad ntheader!\n");
		return -4;
	}

	header = &ntHeader->FileHeader;

	//printf("Timestamp b4: %lu\n", header->TimeDateStamp);

	//printf("Timestamp after: %lu\n", header->TimeDateStamp);

	//printf("setting modified...\n");

	FILETIME fileTime;
	GetSystemTimeAsFileTime(&fileTime);

	FILE_BASIC_INFO b {0};
	b.ChangeTime = *(LARGE_INTEGER*)&fileTime;
	b.CreationTime = *(LARGE_INTEGER*)&fileTime;
	b.LastAccessTime = *(LARGE_INTEGER*)&fileTime;
	b.LastWriteTime = *(LARGE_INTEGER*)&fileTime;
	b.FileAttributes = 0;
	if (!SetFileInformationByHandle(file, FileBasicInfo, &b, sizeof(b)))
		return -5;

	if (!UnmapViewOfFile(mapped_file))
		return -6;

	if (!CloseHandle(hMapObject))
		return -7;

	if (!CloseHandle(file))
		return -8;
		
	return 0;
}
