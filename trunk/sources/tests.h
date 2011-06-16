#ifndef _TESTS_H_
#define _TESTS_H_
bool validCiosContents(struct stCiosMap *stCios,u16 intCiosRevision,const char *strModWiiTestFolder,const char *strPrebuildCiosFolder);
void showTmdDetails(const tmd *pTmd);
void showTmdsDiffs(const tmd *pTmd,const tmd *pPrebuildTmd);
void showCiosMaps(struct stCiosGroup *stCiosMaps,unsigned int intCiosCount);
#endif
