#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define P5 5
#define P2 2

typedef uint8_t u8;

typedef struct PGM {
	char name[256];
	int type;
	int width, height;
	int max;
	u8 **data;
} PGM;

PGM *readPGM(char *name);
int writePGM(PGM *pgm, char *name);
void freePGM(PGM *pgm);

PGM *average(PGM *pgm, int width, int height);

PGM *median(PGM *pgm, int width, int height);
u8 find_median(PGM *pgm, int width, int height, int pgm_i, int pgm_j);

int main() {
	PGM *p1, *p2;
	char str[256];
	int choice, width, height;
	
	p1 = NULL;
	p2 = NULL;
	
	do {
		printf("0 - Exit\n1 - Average Filter\n2 - Median Filter\nChoice: ");
		scanf("%d", &choice);
		if (choice == 1 || choice == 2) {
			printf("PGM file name: ");
			scanf("%s", str);
			p1 = readPGM(str);
			if (p1 != NULL) {
				printf("Filter width: ");
				scanf("%d", &width);
				printf("Filter height: ");
				scanf("%d", &height);
				p2 = choice == 1 ? average(p1, width, height) : median(p1, width, height);
				printf("Result PGM file name: ");
				scanf("%s", str);
				writePGM(p2, str);
				freePGM(p1);
				freePGM(p2);
			}
		}
	} while (choice != 0);
	
	return 0;
}

PGM *readPGM(char *name) {
	PGM *pgm;
	FILE *f;
	u8 temp;
	int i, j;

	pgm = (PGM *) malloc(sizeof(PGM));
	f = fopen(name, "rb");

	if (f == NULL) {
		fprintf(stderr, "Error opening file \"%s\" in read mode.\n", name);
		return NULL;
	} else if (pgm == NULL) {
		fprintf(stderr, "Error allocating space for PGM instance.\n");
		return NULL;
	}

	strcpy(pgm->name, name);

	/* 'P' */
	fgetc(f);

	/* PGM type */
	fscanf(f, "%d", &pgm->type);

	if (pgm->type != P2 && pgm->type != P5) {
		fprintf(stderr, "Error opening PGM file. Expected type: P2 or P5, found: P%d\n", pgm->type);
		fclose(f);
		free(pgm);
		return NULL;
	}

	/* Whitespace */
	while (fgetc(f) != '\n');

	temp = fgetc(f);
	/* Skip comments */
	if (temp == '#') {
		do {
			temp = fgetc(f);
		} while (temp != '\n');
	} else {
		fseek(f, -1, SEEK_CUR);
	}

	/* Width */
	fscanf(f, "%d", &pgm->width);

	/* Height */
	fscanf(f, "%d", &pgm->height);

	/* Max */
	fscanf(f, "%d", &pgm->max);

	pgm->data = (u8 **) malloc(pgm->height * sizeof(u8 *));
	if (pgm->data == NULL) {
		fprintf(stderr, "Error allocating space for PGM data.\n");
		fclose(f);
		free(pgm);
		return NULL;
	}
	for (i = 0; i < pgm->height; i++) {
		pgm->data[i] = (u8 *) malloc(pgm->width * sizeof(u8));
		if (pgm->data[i] == NULL) {
			fprintf(stderr, "Error allocating space for PGM data.\n");
			fclose(f);
			free(pgm->data);
			free(pgm);
			return NULL;
		}
		for (j = 0; j < pgm->width; j++) {
			if (pgm->type == P2) fscanf(f, "%hhu", &pgm->data[i][j]);
			else pgm->data[i][j] = fgetc(f);
		}
	}

	fclose(f);
	return pgm;
}

int writePGM(PGM *pgm, char *name) {
	FILE *f;
	int i, j, k;

	f = fopen(name, "wb");
	if (f == NULL) {
		fprintf(stderr, "Error opening file \"%s\" in write mode.\n", pgm->name);
		return 1;
	}

	/* Headers */
	fprintf(f, "P%d\n%d %d\n%d\n", pgm->type, pgm->width, pgm->height, pgm->max);

	k = 0;
	for (i = 0; i < pgm->height; i++) {
		if (pgm->type == P2) {
			for (j = 0; j < pgm->width; j++) {
				fprintf(f, "%hhu ", pgm->data[i][j]);
				k++;
				if (k % 17 == 0) {
					fprintf(f, "\n");
				}
			}
		} else {
			fwrite(pgm->data[i], sizeof(u8), pgm->width, f);
		}
	}

	fclose(f);
	return 0;
}

void freePGM(PGM *pgm) {
	int i;
	for (i = 0; i < pgm->height; i++) {
		free(pgm->data[i]);
	}
	free(pgm->data);
	free(pgm);
}

PGM *average(PGM *pgm, int width, int height) {
	PGM *filtered;
	int i, j, k, l;
	double result;
	double avg;
	
	avg = 1.0 / (width * height);

	/* Initialize the filtered PGM */
	filtered = (PGM *) malloc(sizeof(PGM));
	memcpy(filtered, pgm, sizeof(PGM));
	filtered->data = (u8 **) malloc(filtered->height * sizeof(u8 *));
	for (i = 0; i < filtered->height; i++) {
		filtered->data[i] = (u8 *) malloc(filtered->width * sizeof(u8));
		memcpy(filtered->data[i], pgm->data[i], filtered->width * sizeof(u8));
	}

	/* Apply convolution and save the results */
	for (i = height / 2; i < pgm->height - height / 2; i++) {
		for (j = width / 2; j < pgm->width - width / 2; j++) {
			result = 0;
			for (k = 0; k < height; k++) {
				for (l = 0; l < width; l++) {
					result += avg * pgm->data[i + k - height / 2][j + l - width / 2];
				}
			}
			filtered->data[i][j] = (u8) result;
		}
	}

	return filtered;
}

PGM *median(PGM *pgm, int width, int height) {
	PGM *filtered;
	int i, j, k, l;

	/* Initialize the filtered PGM */
	filtered = (PGM *) malloc(sizeof(PGM));
	memcpy(filtered, pgm, sizeof(PGM));
	filtered->data = (u8 **) malloc(filtered->height * sizeof(u8 *));
	for (i = 0; i < filtered->height; i++) {
		filtered->data[i] = (u8 *) malloc(filtered->width * sizeof(u8));
		memcpy(filtered->data[i], pgm->data[i], filtered->width * sizeof(u8));
	}

	/* Set the new values as the median of the numbers around every pixel */
	for (i = height / 2; i < pgm->height - height / 2; i++) {
		for (j = width / 2; j < pgm->width - width / 2; j++) {
			filtered->data[i][j] = find_median(pgm, width, height, i, j);
		}
	}
	
	return filtered;
}

u8 find_median(PGM *pgm, int width, int height, int pgm_i, int pgm_j) {
	int i, j, n;
	u8 *sorted, min, temp;

	/* Fill the array with pixels */
	n = width * height;
	sorted = (u8 *) malloc(n * sizeof(u8));
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			sorted[i * width + j] = pgm->data[pgm_i + i - height / 2][pgm_j + j - width / 2];
		}
	}
	
	/* Apply selection sort on the array */
	for (i = 0; i < n; i++) {
		min = i;
		for (j = i + 1; j < n; j++) {
			if (sorted[j] < sorted[min]) {
				min = j;
			}
		}
		temp = sorted[i];
		sorted[i] = sorted[min];
		sorted[min] = temp;
	}
	
	/* Return the median */
	temp = n % 2 == 0 ? (sorted[n / 2] + sorted[n / 2 + 1]) / 2 : sorted[n / 2];
	free(sorted);
	return temp;
}