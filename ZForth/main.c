/* A basic driver for the FORTH system.
 * Includes some code from StackExchange for making getline work on Windows, but it's all public domain.
 *
 * -Zak.
 */
//#define FORTH_16BIT
#include "forth.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* The original code is public domain -- Will Hartung 4/9/09 */
/* Modifications, public domain as well, by Antti Haapala, 11/10/17
 ^^ This applies to the getline function I've included here. -Zak Fenton, 2020
   - Switched to getc on 5/23/19 */

   // if typedef doesn't exist (msvc, blah)
typedef intptr_t ssize_t;

ssize_t getline(char** lineptr, size_t* n, FILE* stream) {
    size_t pos;
    int c;

    if (lineptr == NULL || stream == NULL || n == NULL) {
        errno = EINVAL;
        return -1;
    }

    c = getc(stream);
    if (c == EOF) {
        return -1;
    }

    if (*lineptr == NULL) {
        *lineptr = malloc(128);
        if (*lineptr == NULL) {
            return -1;
        }
        *n = 128;
    }

    pos = 0;
    while (c != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n + (*n >> 2);
            if (new_size < 128) {
                new_size = 128;
            }
            char* new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == NULL) {
                return -1;
            }
            *n = new_size;
            *lineptr = new_ptr;
        }

        ((unsigned char*)(*lineptr))[pos++] = c;
        if (c == '\r' || c == '\n') {
            break;
        }
        c = getc(stream);
    }

    (*lineptr)[pos] = '\0';
    return pos;
}

forth_word_t simplecallback(forth_t* forth, void* udata, int sysnum) {
    fprintf(stderr, "CALLBACK\n");
    switch (sysnum) {
    case 10:
        fprintf(stdout, "LOGNUM %d\n", forth_popdata(forth));
        return 0;
    case 11: {
        char strbuf[100];
        forth_peekstrl(forth, forth_popdata(forth), 100, strbuf);
        fprintf(stdout, "LOGSTR \"%s\"\n", strbuf);
        return 0;
    }
    case 20: {
        char strbuf[100];
        forth_word_t n = forth_peekstrl(forth, forth_popdata(forth), 100, strbuf);
        forth_setlookupinstrl(forth, strbuf, n, forth_encode(forth, FORTH_OP_CALLADDR, forth_popdata(forth)));
        return 0;
    }
    default:
        fprintf(stderr, "ERROR: Callback called with sysnum %d\n", sysnum);
        return -1;
    }
    
}

bool assemble(forth_t* forth, const char* src, forth_word_t len) {
    forth_word_t i = 0;
    forth_word_t result = 0;
    while ((result = forth_assemble(forth, src, i, len)) > 0) {
        //fprintf(stderr, "Token is %d len is %d\n", forth_tokentype(forth, src, i, len), forth_tokenlength(forth, src, i, len));
        //fprintf(stderr, "Assembled %d characters from character %d\n", result, i);
        //fprintf(stderr, "Next asm address is %d\n", forth->header.codenext);
        i += result;
    }
    if (result != 0) {
        fprintf(stderr, "Error at character %d\n", i);
        return false;
    } else {
        return true;
    }
}

int main(int argc, char **argv) {
	forth_t* forth = malloc(1024 * 24 * sizeof(forth_word_t));
	if (forth_clear(forth, (1024 * 24), 1024, 4096) != 0) {
		fprintf(stderr, "Couldn't initialise.\n");
		return -1;
	}

    fprintf(stdout, "Zak's simple FORTH-like system. You're running in %d-bit mode.\n", (sizeof(forth_word_t) * 8));

	//fprintf(stderr, "Bootstrapping.\n");

	while (forth_step(forth, &simplecallback, NULL) == 0) {
		// System is running...
        // This is only in case future/user-modified implementations insert any startup code in the VM themselves. Otherwise will stop immediately.
	}

    //fprintf(stderr, "Registering system functions.\n");
    forth_setlookupinstr(forth, "sys2", forth_encode(forth, FORTH_OP_CALLSYS, 2));
    forth_setlookupinstr(forth, "sys3", forth_encode(forth, FORTH_OP_CALLSYS, 3));
    forth_setlookupinstr(forth, "sys1", forth_encode(forth, FORTH_OP_CALLSYS, 1));

    forth_setlookupinstr(forth, "sys.lognum", forth_encode(forth, FORTH_OP_CALLSYS, 10));
    forth_setlookupinstr(forth, "sys.logstr", forth_encode(forth, FORTH_OP_CALLSYS, 11));
    forth_setlookupinstr(forth, "sys.logstack", forth_encode(forth, FORTH_OP_CALLSYS, 12));

    forth_setlookupinstr(forth, "sys.reg", forth_encode(forth, FORTH_OP_CALLSYS, 20));

    //fprintf(stderr, "sys1=0x%x (type %d)", forth_lookupinstr(forth, "sys1"), forth_lookupinstr(forth, "sys1") & 0xf);
    //fprintf(stderr, "%d %d\n", forth_lookuptableaddr(forth, "sys1"), forth_lookuptableaddr(forth, "sys1"));
    
    /* This was the initial implementation, which shows the ahead-of-time compilation abilities. 
	fprintf(stderr, "Awaiting commands until \"EOF\".\n");

	char* lbuffer = malloc(1024);
    lbuffer[0] = 0;
	size_t lsize = 1024;
	while (strcmp(lbuffer, "EOF\n") != 0 && strcmp(lbuffer, "EOF") != 0 && strcmp(lbuffer, "EOF\r\n") != 0) {
        //fprintf(stdout, "Got \"%s\"\n", lbuffer);
		fprintf(stdout, " > ");
		size_t n = getline(&lbuffer, &lsize, stdin);
		if (n > 0 && strcmp(lbuffer, "EOF\n") != 0 && strcmp(lbuffer, "EOF") != 0 && strcmp(lbuffer, "EOF\r\n") != 0) {
			assemble(forth, (const char*) lbuffer, (forth_word_t) n);
		}
	}

    fprintf(stderr, "Running commands.\n");

    forth->header.pc = forth->header.codestart;
    //fprintf(stderr, "instr is %x\n", forth->data.words[forth->header.pc]);
    //fprintf(stderr, "... %d %d %d\n", forth->header.pc, forth->header.dsp, forth->header.rsp);

	while (forth_step(forth, &simplecallback, NULL) == 0) {
        //fprintf(stderr, "instr is %x\n", forth->data.words[forth->header.pc]);
        //fprintf(stderr, "... %d %d %d\n", forth->header.pc, forth->header.dsp, forth->header.rsp);
		// System is running...
	}
    */

    /* The new implementation shows the ability to make a simple read-eval-print loop. */
    bool vmstopped = false;
    while (!vmstopped) {
        fprintf(stdout, "[EXIT to quit] > "); // Display a simple prompt.

        // Allocate a small buffer, enough to hopefully fit any reasonable line of code.
        char* lbuffer = malloc(1024);
        lbuffer[0] = 0;
        size_t lsize = 1024;

        size_t n = getline(&lbuffer, &lsize, stdin);

        if (strcmp(lbuffer, "EXIT\n") == 0 || strcmp(lbuffer, "EXIT") == 0 || strcmp(lbuffer, "EXIT\r\n") == 0) {
            vmstopped = true;
        } else if (n > 0) {
            // This ensures the next execution will begin where the line is assembled to.
            forth->header.pc = forth->header.codenext;

            // Assemble the line (this function is just a simple loop using the built-in assembler, TODO: Better error handling)
            bool okay = assemble(forth, (const char*)lbuffer, (forth_word_t)n);

            // Begin execution.
            while (okay && (!vmstopped) && forth_step(forth, &simplecallback, NULL) == 0) {
                // System is running...
            }
        } else {
            // Just ignore empty lines.
        }

        free(lbuffer);
    }

	fprintf(stderr, "Done.\n");

	return 0;
}