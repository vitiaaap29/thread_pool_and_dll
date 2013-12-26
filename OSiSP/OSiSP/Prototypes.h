#include <tchar.h>
#include "Pool.h"
#include "sizeFileInfo.h"
typedef void(*PFunction) (void*);

void dispatchCopyByPool(TCHAR* nameSource, TCHAR* destinationName, PFunction function, Pool* pool);
void dispatchSizeByPool(TCHAR* name, float* sum, PFunction function, Pool* pool);
void copyFile(void* context);
TCHAR* getShortName(TCHAR* fullName);
void sizeFile(void* context);