/* ConsoleApplication1.cpp: определяет точку входа для консольного приложения.*/

/*
**Полезные ссылки:
**http://en.cppreference.com/w/c/io/fread
**http://en.cppreference.com/w/c/io/fwrite
**http://en.cppreference.com/w/c/io/fopen
**http://en.cppreference.com/w/c/io/feof
**http://en.cppreference.com/w/c/io/ferror
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_FAILURE 1
#define BUFF_SIZE 131072 /* 2^17 = 131072 */
#define POS_MOD 1
#define POS_INPUTFILE 2
#define POS_OUTPUTFILE 3
#define MOD_ENCODE "encode"
#define MOD_DECODE "decode"
#define A64 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

/* Генерация алфавита декодировки */
void Symb(char RevA64[256]) {
	int i;
	for (i = 0; i < 256; i++)
	{
		RevA64[i] = 0;
	}
	for (i = 'A'; i <= 'Z'; i++)
	{
		RevA64[i] = i - 'A';
	}
	for (i = 'a'; i <= 'z'; i++)
	{
		RevA64[i] = i - 'a' + 26;
	}
	for (i = '0'; i <= '9'; i++)
	{
		RevA64[i] = i - '0' + 52;
	}
	RevA64['+'] = 62;
	RevA64['/'] = 63;
}

/* Кодировщик */
void Encode64(FILE* InputFile, FILE* OutputFile)
{
	char InputChars[3 * BUFF_SIZE];
	char OutputChars[4 * BUFF_SIZE];
	size_t i, j = 0;
	do {
		i = 0;
		j = 0;
		size_t ReadCounter = fread(InputChars, sizeof InputChars[0], 3 * BUFF_SIZE, InputFile);
		while (ReadCounter > 2)
		{
			OutputChars[i + 0] = A64[(InputChars[j + 0] & 252) >> 2];
			OutputChars[i + 1] = A64[((InputChars[j + 0] & 3) << 4) | ((InputChars[j + 1] & 240) >> 4)];
			OutputChars[i + 2] = A64[((InputChars[j + 1] & 15) << 2) | ((InputChars[j + 2] & 192) >> 6)];
			OutputChars[i + 3] = A64[InputChars[j + 2] & 63];
			ReadCounter -= 3;
			i += 4;
			j += 3;
		}
		if (ReadCounter == 1) {
			OutputChars[i + 0] = A64[(InputChars[j + 0] & 252) >> 2];
			OutputChars[i + 1] = A64[((InputChars[j + 0] & 3) << 4) | ((InputChars[j + 1] & 240) >> 4)];
			OutputChars[i + 2] = '=';
			OutputChars[i + 3] = '=';
			i += 4;
		}
		if (ReadCounter == 2) {
			OutputChars[i + 0] = A64[(InputChars[j + 0] & 252) >> 2];
			OutputChars[i + 1] = A64[((InputChars[j + 0] & 3) << 4) | ((InputChars[j + 1] & 240) >> 4)];
			OutputChars[i + 2] = A64[((InputChars[j + 1] & 15) << 2) | ((InputChars[j + 2] & 192) >> 6)];
			OutputChars[i + 3] = '=';
			i += 4;
		}
		fwrite(OutputChars, sizeof OutputChars[0], i, OutputFile);
	} while (!feof(InputFile));
}

/* Декодировщик */
void Decode64(FILE* InputFile, FILE* OutputFile) {
	char InputChars[4 * BUFF_SIZE];
	char OutputChars[3 * BUFF_SIZE];
	int count; /* Кол-во знаков = */
	char RevA64[256]; Symb(RevA64); /* Обратный алфавит к A64 */
	size_t i, j;
	do {
		size_t ReadCounter = fread(InputChars, sizeof InputChars[0], 4 * BUFF_SIZE, InputFile);
		i = 0;
		j = 0;

		/* Подсчет кол-ва знаков = */
		count = 0;
		if ((const char)InputChars[ReadCounter - 1] == (const char)'=')
			count++;
		if ((const char)InputChars[ReadCounter - 2] == (const char)'=')
			count++;

		while (ReadCounter > 0) {
			InputChars[j + 0] = (unsigned char)RevA64[InputChars[j + 0]];
			InputChars[j + 1] = (unsigned char)RevA64[InputChars[j + 1]];
			InputChars[j + 2] = (unsigned char)RevA64[InputChars[j + 2]];
			InputChars[j + 3] = (unsigned char)RevA64[InputChars[j + 3]];
			OutputChars[i + 0] = (InputChars[j + 0] << 2) | ((InputChars[j + 1] & 48) >> 4);
			OutputChars[i + 1] = ((InputChars[j + 1] & 15) << 4) | ((InputChars[j + 2] & 60) >> 2);
			OutputChars[i + 2] = ((InputChars[j + 2] & 3) << 6) | (InputChars[j + 3]);
			i += 3;
			j += 4;
			ReadCounter -= 4;
		}
		fwrite(OutputChars, sizeof OutputChars[0], i - count, OutputFile);
	} while (!feof(InputFile));
}

int main(int argc, char* argv[]) /* Вид вызова: base64.exe MOD_ENCODE|MOD_DECODE InputFileName OutputFileName */
{
	/* Проверка на верность мода */
	if (strcmp(argv[POS_MOD], MOD_ENCODE) & strcmp(argv[POS_MOD], MOD_DECODE)) {
		printf("Wrong mod for Base64 \n");
		return EXIT_FAILURE;
	}
	/* Открытие файлов */
	char* InputFileName = argv[POS_INPUTFILE];
	FILE* InputFile;
	fopen_s(&InputFile, InputFileName, "rb");
	if (!InputFile) {
		perror("Input file opening failed");
		return EXIT_FAILURE;
	}
	char* OutputFileName = argv[POS_OUTPUTFILE];
	FILE* OutputFile;
	fopen_s(&OutputFile, OutputFileName, "wb");
	if (!OutputFile) {
		perror("Output file opening failed");
		return EXIT_FAILURE;
	}
	/* Вызов кодировщика или декодировщика */
	if (!strcmp(argv[POS_MOD], MOD_ENCODE))
		Encode64(InputFile, OutputFile);
	if (!strcmp(argv[POS_MOD], MOD_DECODE))
		Decode64(InputFile, OutputFile);

	/* Закрытие файлов */
	fclose(InputFile);
	fclose(OutputFile);
	return(0);
}