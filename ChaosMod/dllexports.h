#include "Effects/EffectCategory.h"

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int GetActiveEffectIDs(char **outIDs, int maxLen, int maxCount);
__declspec(dllexport) int GetEffectCategory(const char *effectID);

#ifdef __cplusplus
}
#endif
