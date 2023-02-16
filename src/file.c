#include "include/file.h"

// Error: NULL.
const char *read_file(const char *path) {
	char *file_contents = NULL;
	FILE *fp;

	fp = fopen(path, "r");
	if (!fp) return NULL;

	long path_file_size = file_size(fp);

	file_contents = (char *)calloc((path_file_size + 1L), sizeof(char));
	// Is this necessary? May as well do it. ------^
	if (!file_contents) {
		fclose(fp);
		return NULL;
	}

	// Ensure cursor is at the beginning of the file.
	fseek(fp, 0, SEEK_SET);
	for (long i = 0L; i < path_file_size; ++i) {
		int c = getc(fp);
		if (c == EOF) {
			file_contents[i] = '\0';
			break;
		}

		file_contents[i] = (char)c;
	}

	fclose(fp);
	return (const char *)file_contents;
}

// Leaves file cursor at EOF.
long file_size(FILE *fp) {
	long total_size = 0L;
	fseek(fp, 0, SEEK_END);
	total_size = ftell(fp);
	return total_size;
}
