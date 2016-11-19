#ifndef	VIOS_STRING_H
#define	VIOS_STRING_H

PUBLIC void*	memcpy(void *dst, void *src, int size);
PUBLIC void*	memset(void* des, char ch, int size);
PUBLIC char*	strcpy(char* dst, char* src);
PUBLIC int	strlen(char* str);

PUBLIC int	memcmp(const void * s1, const void *s2, int n);
PUBLIC int	strcmp(const char * s1, const char *s2);
PUBLIC char*	strcat(char * s1, const char *s2);

#endif