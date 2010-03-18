
typedef struct _dir_contents
{
   int size;
   char list[256][256];
}
dir_contents;

dir_contents* getDirList (char* path);
dir_contents* getDirListFull (char* path);
int verifyContents (dir_contents* dc, int n);
int populateDir (char* path, int n);
void cleanupDir (char* path, int n);
int CnsUserInfoCompare (const void * elem1, const void * elem2);
int CnsGroupInfoCompare (const void * elem1, const void * elem2);

int report (char* testdesc, char* message, char* serrnostr, int result);
