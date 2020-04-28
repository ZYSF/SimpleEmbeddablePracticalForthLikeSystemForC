/* Zak's really really ridiculously small FORTH-style system.
 * PUBLIC DOMAIN BY EDICT OF THE AUTHOR.
 * No copyright, no warranty, only code.
 * Use the MIT or BSD licenses if you prefer. I don't require (or forbid) attribution.
 * Peace, Love and Buddhism for all.
 *
 * -Zak.
 */

#ifndef FORTH_H
#define FORTH_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// TODO: Replace calls to atoi etc. with portable code
#include <stdlib.h>
#include <stdio.h>

#ifdef FORTH_16BIT
typedef int16_t forth_word_t;
#else
#ifdef FORTH_64BIT
typedef int64_t forth_word_t;
#else
typedef int32_t forth_word_t;
#endif
#endif

typedef union forth forth_t;
typedef struct forth_header forth_header_t;
typedef struct forth_data forth_data_t;
typedef bool (*forth_callback_t)(forth_t* forth, void* udata, int sysnum);

struct forth_data {
	forth_word_t words[0];
};

struct forth_header {
	forth_word_t fmagic;
	forth_word_t fversion;
	forth_word_t fsize;
	forth_word_t hsize;
	forth_word_t resvd;
	forth_word_t pc;
	forth_word_t rsp;
	forth_word_t dsp;
	forth_word_t asp;
	forth_word_t indexstart;
	forth_word_t indexnext;
	forth_word_t indexend;
	forth_word_t codestart;
	forth_word_t codenext;
	forth_word_t codeend;
	forth_word_t heapstart;
	forth_word_t heapnext;
	forth_word_t heapend;
	forth_word_t rsstart;
	forth_word_t rsend;
	forth_word_t dsstart;
	forth_word_t dsend;
	forth_word_t asstart;
	forth_word_t asend;
};

union forth {
	forth_header_t header;
	forth_data_t data;
};

#define FORTH_INLINE static inline

FORTH_INLINE forth_word_t forth_pushdata(forth_t* forth, forth_word_t value) {
	if (forth->header.dsp >= forth->header.dsstart && forth->header.dsp < forth->header.dsend) {
		forth->data.words[forth->header.dsp++] = value;
		return 0;
	}
	return -1;
}

FORTH_INLINE forth_word_t forth_popdata(forth_t* forth) {
	forth->header.dsp--;
	if (forth->header.dsp >= forth->header.dsstart && forth->header.dsp < forth->header.dsend) {
		return forth->data.words[forth->header.dsp];
	}
	return -1;
}

FORTH_INLINE forth_word_t forth_pushreturn(forth_t* forth, forth_word_t value) {
	if (forth->header.rsp >= forth->header.rsstart && forth->header.rsp < forth->header.rsend) {
		forth->data.words[forth->header.rsp++] = value;
		return 0;
	}
	return -1;
}

FORTH_INLINE forth_word_t forth_popreturn(forth_t* forth) {
	forth->header.rsp--;
	if (forth->header.rsp >= forth->header.rsstart && forth->header.rsp < forth->header.rsend) {
		return forth->data.words[forth->header.rsp];
	}
	return -1;
}

FORTH_INLINE forth_word_t forth_pushasm(forth_t* forth, forth_word_t value) {
	if (forth->header.asp >= forth->header.asstart && forth->header.asp < forth->header.asend) {
		forth->data.words[forth->header.asp++] = value;
		return 0;
	}
	return -1;
}

FORTH_INLINE forth_word_t forth_popasm(forth_t* forth) {
	forth->header.asp--;
	if (forth->header.asp >= forth->header.asstart && forth->header.asp < forth->header.asend) {
		return forth->data.words[forth->header.asp];
	}
	return -1;
}

FORTH_INLINE forth_word_t forth_clear(forth_t* forth, forth_word_t size, forth_word_t indexsize, forth_word_t codesize) {
	if (forth == NULL || size < 1024 + (indexsize * 2) + codesize) {
		return -1;
	}
	forth_word_t i;
	for (i = 0; i < size; i++) {
		/*forth->data.words*/((forth_word_t*) (void*) forth)[i] = 0;
	}
	forth->header.fmagic = 0x54175E1F;
	forth->header.fversion = 1;
	forth->header.fsize = size;
	forth->header.hsize = sizeof(forth_header_t) / sizeof(forth_word_t);
	forth->header.resvd = 0;
	forth->header.pc = -1;

	forth->header.indexstart = forth->header.hsize;
	forth->header.indexnext = forth->header.indexstart;
	forth->header.indexend = forth->header.indexstart + (indexsize * 2);
	
	forth->header.codestart = forth->header.indexend;
	forth->header.codenext = forth->header.codestart;
	forth->header.codeend = forth->header.codestart + codesize;

	forth->header.heapstart = forth->header.codeend;
	forth->header.heapnext = forth->header.heapstart;
	forth->header.heapend = forth->header.fsize;

	forth->header.dsstart = forth->header.heapstart;
	forth->header.dsend = forth->header.heapend;
	forth->header.rsstart = forth->header.heapstart;
	forth->header.rsend = forth->header.heapend;
	forth->header.asstart = forth->header.heapstart;
	forth->header.asend = forth->header.heapend;

	forth->header.rsp = forth->header.heapnext;
	forth->header.heapnext += 100;
	forth->header.dsp = forth->header.heapnext;
	forth->header.heapnext += 100;
	forth->header.asp = forth->header.heapnext;
	forth->header.heapnext += 100;

	return 0;
}

FORTH_INLINE forth_word_t forth_peek(forth_t* forth, forth_word_t addr) {
	if (addr < 0 || addr >= forth->header.fsize) {
		return -1;
	}
	return forth->data.words[addr];
}

FORTH_INLINE forth_word_t forth_poke(forth_t* forth, forth_word_t addr, forth_word_t val) {
	if (addr < 0 || addr >= forth->header.fsize) {
		return -1;
	}
	forth->data.words[addr] = val;
	return 0;
}

FORTH_INLINE forth_word_t forth_peekstrl(forth_t* forth, forth_word_t startaddr, forth_word_t lenout, char* strout) {
	forth_word_t h = forth_peek(forth, startaddr);
	//fprintf(stderr, "Peeking string with code %x\n", h);
	if ((h & 0xF) != 4 || (h >> 4) + 1 > lenout) {
		return -1;
	}
	forth_word_t i;
	for (i = 0; i <= (h >> 4); i++) {
		strout[i] = (i < (h >> 4) ? forth_peek(forth, startaddr + 1 + i) : 0);
		//fprintf(stderr, "Got %d\n", (i < (h >> 4) ? forth_peek(forth, startaddr + 1 + i) : 0));
	}
	return (h >> 4);
}

FORTH_INLINE forth_word_t forth_pokestrl(forth_t* forth, forth_word_t startaddr, forth_word_t len, const char* str) {
	forth_word_t res = forth_poke(forth, startaddr, (len << 4) | 4);
	forth_word_t i;
	for (i = 0; res == 0 && i < len; i++) {
		res = forth_poke(forth, startaddr + 1 + i, str[i]);
	}
	if (res == 0) {
		return startaddr + len + 1;
	} else {
		return 0;
	}
}

FORTH_INLINE forth_word_t forth_strlen(forth_t* forth, const char* str) {
	forth_word_t len = 0;
	while (str != NULL && str[len] != 0) {
		len++;
	}
	return len;
}

FORTH_INLINE forth_word_t forth_pokestr(forth_t* forth, forth_word_t startaddr, const char* str) {
	return forth_pokestrl(forth, startaddr, forth_strlen(forth, str), str);
}

FORTH_INLINE forth_word_t forth_allocstrl(forth_t* forth, forth_word_t len, const char* str) {
	if (forth->header.heapnext + len + 1 > forth->header.heapend) {
		return 0;
	}
	forth_word_t tmp = forth_pokestrl(forth, forth->header.heapnext, len, str);
	if (tmp > 0) {
		forth_word_t oldnext = forth->header.heapnext;
		forth->header.heapnext = tmp;
		//fprintf(stderr, "Allocated '%s' at %d with size %d\n", str, oldnext, tmp - oldnext);
		return oldnext;
	} else {
		return 0;
	}
}

FORTH_INLINE forth_word_t forth_allocstr(forth_t* forth, const char* str) {
	return forth_allocstrl(forth, forth_strlen(forth, str), str);
}

FORTH_INLINE forth_word_t forth_lookuptableaddrl(forth_t* forth, const char* name, forth_word_t len) {
	if (len >= 100) {
		return 0;
	}
	char tmpstr[100];
	forth_word_t tmpl;
	forth_word_t i;

	for (i = forth->header.indexstart; i < forth->header.indexnext; i += 2) {
		tmpl = forth_peekstrl(forth, forth_peek(forth, i), 100, tmpstr);
		//fprintf(stderr, "Looking at '%s'", tmpstr);
		if (tmpl == len) {
			int j;
			bool iseq = true;
			for (j = 0; j < len; j++) {
				if (tmpstr[j] != name[j]) {
					iseq = false;
				}
			}
			if (iseq) {
				return i;
			}
		}
	}

	if (forth->header.indexnext + 2 > forth->header.indexend) {
		return 0;
	}

	if (forth_poke(forth, forth->header.indexnext, forth_allocstrl(forth, len, name))) {
		return 0;
	}
	if (forth_poke(forth, forth->header.indexnext + 1, 0)) { // TODO: Better code to signal "not set yet" ? Maybe a function that's called in this case?
		return 0;
	}
	forth_word_t result = forth->header.indexnext;
	forth->header.indexnext += 2;
	return result;
}

FORTH_INLINE forth_word_t forth_lookuptableaddr(forth_t* forth, const char* name) {
	return forth_lookuptableaddrl(forth, name, forth_strlen(forth, name));
}

FORTH_INLINE forth_word_t forth_lookupinstrl(forth_t* forth, const char* name, forth_word_t len) {
	forth_word_t tableaddr = forth_lookuptableaddrl(forth, name, len);
	if (tableaddr == NULL) {
		return 0;
	}
	return forth_peek(forth, tableaddr + 1);
}

FORTH_INLINE forth_word_t forth_lookupinstr(forth_t* forth, const char* name) {
	return forth_lookupinstrl(forth, name, forth_strlen(forth, name));
}

FORTH_INLINE forth_word_t forth_setlookupinstrl(forth_t* forth, const char* name, forth_word_t len, forth_word_t instr) {
	return forth_poke(forth, forth_lookuptableaddrl(forth, name, len) + 1, instr);
}

FORTH_INLINE forth_word_t forth_setlookupinstr(forth_t* forth, const char* name, forth_word_t instr) {
	return forth_setlookupinstrl(forth, name, forth_strlen(forth, name), instr);
}

FORTH_INLINE forth_word_t forth_tokentype(forth_t* forth, const char* source, forth_word_t i, forth_word_t totallen) {
	if (i >= totallen || source[i] == 0) {
		return -1;
	} else if (source[i] == ' ' || source[i] == '\n' || source[i] == '\t' || source[i] == '\r') {
		return 0;
	} else if (source[i] >= '0' && source[i] <= '9') {
		return 1;
	} else if (source[i] == '\"') {
		return 2;
	} else if ((source[i] >= 'a' && source[i] <= 'z') || (source[i] >= 'A' && source[i] <= 'Z') || (source[i] == '_')) {
		return 3;
	} else if (source[i] == '+' || source[i] == '-' || source[i] == '*' || source[i] == '/' || source[i] == '%' || source[i] == '=' || source[i] == '&' || source[i] == '|' || source[i] == '?' || source[i] == '!' || source[i] == ';') {
		return 4;
	} else if (source[i] == '[' || source[i] == ']') {
		return 5;
	} else {
		return -2;
	}
}

FORTH_INLINE forth_word_t forth_tokenlength(forth_t* forth, const char* source, forth_word_t i, forth_word_t totallen) {
	if (i >= totallen || source[i] == 0) {
		return 0;
	}
	forth_word_t result = 0;
	switch (forth_tokentype(forth, source, i, totallen)) {
	case 0:
	case 4:
	case 5:
		return 1;
	case 1:
		while (i + result < totallen && source[i + result] >= '0' && source[i + result] <= '9') {
			result++;
		}
		break;
	case 2:
		result = 1;
		while (i + result < totallen && source[i + result] != '\"') {
			result++;
		}
		if (source[i + result] == '\"') {
			result++;
		} else {
			return 0;
		}
		break;
	case 3:
		while (i + result < totallen && (source[i + result] >= 'a' && source[i + result] <= 'z') || (source[i + result] >= 'A' && source[i + result] <= 'Z') || (source[i + result] >= '0' && source[i + result] <= '9') || (source[i + result] == '_') || (source[i + result] == '.')) {
			result++;
		}
		break;
	default:
		return 0;
	}

	return result;
}

FORTH_INLINE forth_word_t forth_tokenmemorysize(forth_t* forth, const char* source, forth_word_t i, forth_word_t totallen) {
	switch (forth_tokentype(forth, source, i, totallen)) {
	case 0:
		return 0;
	case 1:
	case 3:
	case 4:
	case 5:
		return 1;
	case 2:
		return forth_tokenlength(forth, source, i, totallen) - 1;
	default:
		return 0;
	}
}

FORTH_INLINE forth_word_t forth_assemble(forth_t* forth, const char* source, forth_word_t i, forth_word_t totallen) {
	forth_word_t len = forth_tokenlength(forth, source, i, totallen);
	forth_word_t tmp = 0;
	char tmpchar[20];
	switch (forth_tokentype(forth, source, i, totallen)) {
	case 0: // Whitespace
		break;
	case 1: // Number
		for (tmp = 0; tmp <= len; tmp++) {
			if (tmp < len) {
				tmpchar[tmp] = source[i + tmp];
			} else {
				tmpchar[tmp] = 0;
			}
		}
		if (forth_poke(forth, forth->header.codenext, atoi((const char*)tmpchar) << 4)) {
			return 0;
		}
		forth->header.codenext++;
		break;
	case 2: // String
		tmp = forth_pokestrl(forth, forth->header.codenext, len - 2, source + i + 1);
		if (tmp == 0) {
			return 0;
		} else {
			forth->header.codenext = tmp;
		}
		break;
	case 3: // Name
		tmp = forth_lookuptableaddrl(forth, source + i, len);
		if (tmp > 0) {
			if (forth_poke(forth, forth->header.codenext, ((tmp + 1) << 4) | 7)) {
				//fprintf(stderr, "Failed to store within index\n");
				return 0;
			}
			//fprintf(stderr, "Put a %x at %d\n", ((tmp + 1) << 4) | 7, forth->header.codenext);
			forth->header.codenext++;
		} else {
			//fprintf(stderr, "Failed to index\n");
			return 0;
		}
		break;
	case 4: // Simple op, with special handling of ! and ;
		if (forth_poke(forth, forth->header.codenext, (source[i] == '!' ? (9) : (source[i] == ';' ? (6) : (((int)source[i]) << 4) | 5)))) {
			return 0;
		} else {
			forth->header.codenext++;
		}
		break;
	case 5: // '[' ... ']'
		if (source[i] == '[') {
			forth_pushasm(forth, forth->header.codenext);
			forth_poke(forth, forth->header.codenext, 8);
			forth->header.codenext++;
		} else { // ']'
			forth_poke(forth, forth->header.codenext, 6); // Add a return statement
			forth->header.codenext++;
			forth_word_t startaddr = forth_popasm(forth);
			forth_poke(forth, startaddr, (forth->header.codenext << 4) | 8); // Patch end address
		}
		break;
	default:
		return 0;
	}
	return len;
}

#define FORTH_OP_PUSHINT	0
#define FORTH_OP_CALLADDR	1
#define FORTH_OP_CALLSYS	2
#define FORTH_OP_PUSHSTR	4
#define FORTH_OP_SIMPLE		5
#define FORTH_OP_CONTROL	6
#define FORTH_OP_CALLINDEX	7

FORTH_INLINE forth_word_t forth_encode(forth_t* forth, forth_word_t opcode, forth_word_t arg) {
	return (arg << 4) | opcode;
}

FORTH_INLINE forth_word_t forth_step(forth_t* forth, forth_callback_t callback, void* udata) {
	if (forth->header.pc < forth->header.codestart || forth->header.pc >= forth->header.codenext) {
		return -1;
	}
	forth_word_t instr = forth->data.words[forth->header.pc];
	forth_word_t result = 0;

	switch (instr & 0xF) {
	case 0: // Push integer value
		forth_pushdata(forth, instr >> 4);
		forth->header.pc++;
		break;
	case 1: // Call already-known function
		forth_pushreturn(forth, forth->header.pc);
		forth->header.pc = instr >> 4;
		break;
	case 2: // Call system function
		result = callback(forth, udata, instr >> 4);
		if (result == 0) {
			// A system function can return non-zero to pause the interpreter for later execution. In this case the same instruction will run again.
			// But normally, we return to the following instruction.
			forth->header.pc++;
		}
		break;
	case 4: // Push inline string
		//fprintf(stderr, "Pushing string %d\n", forth->header.pc);
		forth_pushdata(forth, forth->header.pc);
		forth->header.pc++;
		forth->header.pc += instr >> 4;
		//fprintf(stderr, "Done %d\n", forth->header.pc);
		break;
	case 5: { // Simple op
		forth_word_t rhs = forth_popdata(forth);
		forth_word_t lhs = forth_popdata(forth);
		forth_word_t res = 0;
		char c = (char)(instr >> 4);
		switch (c) {
		case '+':
			res = lhs + rhs;
			break;
		case '-':
			res = lhs - rhs;
			break;
		case '*':
			res = lhs * rhs;
			break;
		case '/':
			res = lhs / rhs;
			break;
		case '%':
			res = lhs % rhs;
			break;
		case 'R':
			res = lhs >> rhs;
			break;
		case 'L':
			res = lhs << rhs;
			break;
		case '=':
			res = (lhs == rhs) ? -1 : 0;
			break;
		case 'A':
			res = (lhs && rhs) ? -1 : 0;
			break;
		case 'O':
			res = (lhs || rhs) ? -1 : 0;
			break;
		case '&':
			res = lhs & rhs;
			break;
		case '|':
			res = lhs | rhs;
			break;
		case '?': // Quick conditional jump, if lhs != 0 then jump to rhs
			if (lhs != 0) {
				forth->header.pc = rhs - 1; // will be +1 again at end!
			}
			break;
		default:
			return -1;
		}
		forth_pushdata(forth, res);
		forth->header.pc++;
	} break;
	case 6: // Return op
		forth->header.pc = forth_popreturn(forth) + 1;
		if (forth_peek(forth, forth->header.pc - 1) == 9) { // Special handling of return-to-!-loop
			//fprintf(stderr, "Returning to loop\n");
			forth_word_t w = forth_popdata(forth); // Pop a boolean value from the stack
			if (w != 0) { // Repeat loop if value != 0
				//fprintf(stderr, "Repeating loop\n");
				forth->header.pc--;
			}
		} else {
			//fprintf(stderr, "Eventual return address is %d\n", forth->header.pc);
		}
		break;
	case 7: { // Call by index lookup (data is pointer to instruction in table)
		forth_word_t tmp = forth_peek(forth, instr >> 4);
		//fprintf(stderr, "Looking up %d (type %d)\n", tmp, tmp & 0xf);
		// Execute a single function or system call inline
		switch (tmp & 0xF) {
		case 1: // Call already-known function
			//fprintf(stderr, "Pushing return addr %d\n", forth->header.pc);
			forth_pushreturn(forth, forth->header.pc);
			forth->header.pc = tmp >> 4;
			break;
		case 2: // Call system function
			result = callback(forth, udata, tmp >> 4);
			forth->header.pc++;
			break;
		default:
			return 1;
		}
	} break;
	case 8: { // Push simple block address as data and jump over it.
		forth_pushdata(forth, forth->header.pc + 1);
		forth->header.pc = instr >> 4;
	} break;
	case 9: { // Quick loop, pop loop address from stack, call it, upon returning there is special handling to loop only if popped != 0
		forth_pushreturn(forth, forth->header.pc);
		forth->header.pc = forth_popdata(forth);
		forth_pushdata(forth, forth->header.pc); // Push it again for next iteration
	} break;
	default:
		return -1;
	}

	return 0;
}

/* From ifndef FORTH_H at top of file: */
#endif 