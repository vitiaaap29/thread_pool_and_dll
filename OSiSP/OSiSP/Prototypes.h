#include <tchar.h>
#include "Pool.h"
typedef void(*PFunction) (void*);

void dispatchCopyByPool(TCHAR* nameSource, TCHAR* destinationName, PFunction function, Pool* pool);
void copyFile(void* context);
TCHAR* getShortName(TCHAR* fullName);
TCHAR* normalizeSlash(TCHAR* source);