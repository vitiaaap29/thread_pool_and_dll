#include <tchar.h>
typedef void(*PFunction) (void*);

void dispatchCopyByPool(TCHAR* nameSource, TCHAR* destinationName, PFunction function);
void copyFile(void* context);
TCHAR* getShortName(TCHAR* fullName);