/*
  This file is part of Musca.

  Musca is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as publishead by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Musca is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Musca.  If not, see <http://www.gnu.org/licenses/>.

  Sean Pringle
  sean dot pringle at gmail dot com
  https://launchpad.net/musca
*/

#include "tools.h"
#include "tools_proto.h"

void notice(const char *file, ucell line, const char *func, const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	if (DEBUG)
	{
		if (file) fprintf(stderr, "%s ", file);
		if (line) fprintf(stderr, "%d ", line);
		if (func) fprintf(stderr, "%s ", func);
	}
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}
void bailout(const char *file, ucell line, const char *func, const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	if (DEBUG)
	{
		if (file) fprintf(stderr, "%s ", file);
		if (line) fprintf(stderr, "%d ", line);
		if (func) fprintf(stderr, "%s ", func);
	}
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	throw(1);
}

#define assert(f,...) if (!(f)) bailout(__FILE__,__LINE__,__func__,__VA_ARGS__)
#define wtf(...) bailout(__FILE__,__LINE__,__func__,__VA_ARGS__)
#define note(...) notice(__FILE__,__LINE__,__func__,__VA_ARGS__)

void* allocate(ucell size)
{
	void *p = malloc(size);
	assert(p, "malloc failed for %d bytes", size);
	return p;
}
void* reallocate(void *p, ucell size)
{
	p = realloc(p, size);
	assert(p, "realloc failed for %d bytes", size);
	return p;
}
// few extra string utils
ucell strskip(char **str, str_cb_chr cb)
{
	char *p = *str, *s = p;
	while (*p && cb(*p)) p++;
	*str = p;
	return p - s;
}
ucell strscan(char **str, str_cb_chr cb)
{
	char *p = *str, *s = p;
	while (*p && !cb(*p)) p++;
	*str = p;
	return p - s;
}
ucell strskipthese(char **str, const char *these)
{
	char *p = *str, *s = p;
	while (*p && strchr(these, *p)) p++;
	*str = p;
	return p - s;
}
ucell strscanthese(char **str, const char *these)
{
	char *p = *str, *s = p;
	while (*p && !strchr(these, *p)) p++;
	*str = p;
	return p - s;
}
ucell strltrim(byte *addr)
{
	ucell len = strlen(addr);
	if (!len) return len;
	byte *start = addr;
	strskip(&start, isspace);
	ucell rlen = len - (start - addr);
	memmove(addr, start, rlen);
	addr[rlen] = '\0';
	return rlen;
}
ucell strrtrim(byte *addr)
{
	ucell len = strlen(addr);
	if (!len) return len;
	byte *end = addr + len;
	while (end > addr && isspace(*(end-1))) end--;
	*end = '\0';
	return end - addr;
}
ucell strtrim(byte *addr)
{
	ucell len = strlen(addr);
	len = strltrim(addr);
	len = strrtrim(addr);
	return len;
}
// simple string handler
void str_create(autostr *s)
{
	s->pad = allocate(NOTE);
	s->pad[0] = '\0'; s->len = 0; s->lim = NOTE;
}
void str_require(autostr *s, ucell len)
{
	if (s->len + len + 1 > s->lim)
	{
		s->lim += CHUNKS(NOTE, len + 1);
		s->pad = reallocate(s->pad, s->lim);
	}
}
void str_append(autostr *s, char *p, ucell len)
{
	str_require(s, len);
	memmove(s->pad + s->len, p, len);
	s->len += len; s->pad[s->len] = '\0';
}
void str_drop(autostr *s, ucell len)
{
	len = MIN(len, s->len);
	s->len -= len;
	s->pad[s->len] = '\0';
}
void str_delete(autostr *s)
{
	free(s->pad);
}
ubyte regmatch(regex_t *reg, char *pattern, char *subject, ucell slots, regmatch_t *subs, ucell flags)
{
	ubyte match = 0;
	regex_t *re = reg, re2;
	if (!re)
	{
		re = &re2;
		regcomp(re, pattern, flags);
	}
	match = regexec(re, subject, slots, subs, 0) == 0 ? 1: 0;
	regfree(re);
	return match;
}
// extract a matchead substring from a posix regex regmatch_t
char* regsubstr(char *subject, regmatch_t *subs, int slot)
{
	if (subs[slot].rm_so == -1) return NULL;
	int len = subs[slot].rm_eo - subs[slot].rm_so;
	char *pad = allocate(len+1);
	strncpy(pad, subject + subs[slot].rm_so, len);
	pad[len] = '\0';
	return pad;
}
// simple file io
void blurt(const char *name, void *data)
{
	ucell dlen = strlen(data);
	FILE *f = fopen(name, "w+");
	assert(f, "file open failed: %s", name);
	assert(fwrite(data, sizeof(char), strlen(data), f) == dlen, "file write failed");
	fclose(f);
}
ucell suck(FILE *src, byte *pad, ucell len, byte stop)
{
	ucell ptr = 0;
	for (;;)
	{
		byte c = fgetc(src);
		if (c == EOF)
			break;

		pad[ptr++] = c;

		if (len == ptr || c == stop)
			break;
	}
	return ptr;
}
byte* slurp(const char *name, ucell *len)
{
	ucell blocks = 0;
	FILE *f = fopen(name, "r");
	assert(f, "file open failed: %s", name);
	byte *pad = (byte*)allocate(BLOCK);
	ucell read = 0;
	for (;;)
	{
		read = suck(f, pad+(BLOCK*blocks), BLOCK, EOF);
		if (read < BLOCK) break;
		pad = reallocate(pad, BLOCK * (++blocks+1));
	}
	pad[BLOCK * blocks + read] = '\0';
	if (len) *len = BLOCK * blocks + read;
	fclose(f);
	return pad;
}
void stack_push(stack *s, void *v)
{
	if (s->depth == s->limit)
	{
		s->limit += STACK;
		s->items = reallocate(s->items, s->limit * sizeof(void*));
	}
	s->items[s->depth++] = v;
}
void* stack_pop(stack *s)
{
	assert(s->depth > 0, "stack underflow");
	return s->items[--(s->depth)];
}
void* stack_top(stack *s)
{
	assert(s->depth > 0, "stack underflow");
	return s->items[s->depth-1];
}
void* stack_shift(stack *s)
{
	void *item;
	assert(s->depth > 0, "stack underflow");
	item = s->items[0]; s->depth--;
	memmove(&(s->items[0]), &(s->items[1]), s->depth * sizeof(void*));
	return item;
}
void stack_shove(stack *s, void *item)
{
	if (s->depth == s->limit)
	{
		s->limit += STACK;
		s->items = reallocate(s->items, s->limit * sizeof(void*));
	}
	memmove(&(s->items[1]), &(s->items[0]), s->depth * sizeof(void*));
	s->items[0] = item; s->depth++;
}
void stack_del(stack *s, ucell index)
{
	assert(index < s->depth, "invalid stack item");
	memmove(&(s->items[index]), &(s->items[index+1]), s->depth - index);
	s->depth--;
}
stack *stack_create()
{
	stack *s = (stack*)allocate(sizeof(stack));
	s->items = allocate(sizeof(void*)*STACK);
	s->limit = STACK;
	s->depth = 0;
	return s;
}
void stack_destroy(stack *s)
{
	free(s);
}

