#ifndef FILE_H_
#define FILE_H_

#include <stdio.h>
#include <stdlib.h>

const char *read_file(const char *path);
long file_size(FILE *fp);

#endif // FILE_H_
