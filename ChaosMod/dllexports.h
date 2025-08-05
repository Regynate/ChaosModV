#include "Effects/EffectCategory.h"

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int GetActiveEffectIDs(const char** outIDs, int maxCount);
__declspec(dllexport) EffectCategory GetEffectCategory(const char* effectID);

#ifdef __cplusplus
}
#endif
