
#include <stdio.h>
#include <string.h>     /* for strncmp() */
#include <ctype.h>      /* for isalnum() */
#if defined(MSDOS) || defined(WIN32) || defined(OS2)
# include <io.h>        /* for setmode() */
# include <fcntl.h>     /* for O_BINARY e.t.c. */
#endif
#include <stdlib.h>

#if defined(MSDOS) || defined(WIN32) || defined(OS2)
# define BIN_READ(yes)  ((yes) ? "rb" : "rt")
# define BIN_WRITE(yes) ((yes) ? "wb" : "wt")
# define BIN_CREAT(yes) ((yes) ? (O_CREAT|O_BINARY) : O_CREAT)
# define BIN_ASSIGN(fp, yes) _setmode(_fileno(fp), (yes) ? O_BINARY : O_TEXT)
// CMake is changing the path separator before invoking this application.
# define PATH_SEP '/'
//# define PATH_SEP '\\'
#else
# define BIN_READ(dummy)  "r"
# define BIN_WRITE(dummy) "w"
# define BIN_CREAT(dummy) O_CREAT
# define BIN_ASSIGN(fp, dummy) fp
# define PATH_SEP '/'
#endif

char hexxa[] = "0123456789abcdef0123456789ABCDEF", *hexx = hexxa;

int main(int argc, char* argv[])
{
	FILE *fpi = 0, *fpo = 0;
	int c, e, p = 0, cols = 64, len;
	long length = -1;
	char *pname, *pp;//, *fn;
	unsigned int revert = 1;

	if (argc > 3)
		return 1;

	pname = argv[0];
	for (pp = pname; *pp; )
		if (*pp++ == PATH_SEP)
			pname = pp;
	
	if (argc == 1 || (argv[1][0] == '-' && !argv[1][1]))
		BIN_ASSIGN(fpi = stdin, !revert);
	else if ((fpi = fopen(argv[1], BIN_READ(!revert))) == 0)
	{
		fprintf(stderr,"%s: ", pname);
		perror(argv[1]);
		return 2;
	}
	
	if (argc < 3 || (argv[2][0] == '-' && !argv[2][1]))
		BIN_ASSIGN(fpo = stdout, revert);
	else if ((fpo = fopen(argv[2], BIN_WRITE(revert))) == NULL)
	{
		fprintf(stderr, "%s: ", pname);
		perror(argv[2]);
		return 3;
	}

	if (fpi != stdin)
	{
		len = strlen(argv[1]);
		while(len > 0 && argv[1][len-1] != PATH_SEP)
		{
			len--;
		}
		fprintf(fpo, "unsigned char %s", isdigit(argv[1][len]) ? "__" : "");
		for (e = len; (c = argv[1][e]) != 0; e++)
			putc(isalnum(c) ? c : '_', fpo);
		fputs("[] = {\n", fpo);
	}

	p = 0;
	while ((length < 0 || p < length) && (c = getc(fpi)) != EOF)
	{
		fprintf(fpo, (hexx == hexxa) ? "%s0x%02x" : "%s0X%02X",
			(p % cols) ? ", " : ",\n  "+2*!p,  c);
		p++;
	}

	if (p)
	{
		fputs("\n};\n"+3*(fpi == stdin), fpo);
	}

	if (fpi != stdin)
	{
		fprintf(fpo, "unsigned int %s", isdigit(argv[1][len]) ? "__" : "");
		for (e = len; (c = argv[1][e]) != 0; e++)
			putc(isalnum(c) ? c : '_', fpo);
		fprintf(fpo, "_len = %d;\n", p);
	}

	fclose(fpi);
	fclose(fpo);

	return 0;
}

