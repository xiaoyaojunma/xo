#include "pch.h"
#include "fmt.h"
#include "../Other/StackAllocators.h"
#include <stdarg.h>

static inline void fmt_settype(char argbuf[128], intp pos, const char* width, char type)
{
	if (width != NULL)
	{
		// set the type and the width specifier
		switch (argbuf[pos - 1])
		{
		case 'l':
		case 'h':
		case 'w':
			pos--;
			break;
		}

		for (; *width; width++, pos++)
			argbuf[pos] = *width;

		argbuf[pos++] = type;
		argbuf[pos++] = 0;
	}
	else
	{
		// only set the type, not the width specifier
		argbuf[pos++] = type;
		argbuf[pos++] = 0;
	}
}

static inline int fmt_output_with_snprintf(char* outbuf, char fmt_type, char argbuf[128], intp argbufsize, intp outputSize, const fmtarg* arg)
{
#define				SETTYPE1(type)			fmt_settype( argbuf, argbufsize, NULL, type )
#define				SETTYPE2(width, type)	fmt_settype( argbuf, argbufsize, width, type )

#ifdef _WIN32
	const char* i64Prefix = "I64";
	const char* wcharPrefix = "";
	const char wcharType = 'S';
#else
	const char* i64Prefix = "ll";
	const char* wcharPrefix = "l";
	const char wcharType = 's';
#endif

	bool tokenint = false;
	bool tokenreal = false;

	switch (fmt_type)
	{
	case 'd':
	case 'i':
	case 'o':
	case 'u':
	case 'x':
	case 'X':
		tokenint = true;
	}

	switch (fmt_type)
	{
	case 'e':
	case 'E':
	case 'f':
	case 'g':
	case 'G':
	case 'a':
	case 'A':
		tokenreal = true;
	}

	switch (arg->Type)
	{
	case fmtarg::TCStr:
		SETTYPE2("", 's');
		return fmt_snprintf(outbuf, outputSize, argbuf, arg->CStr);
	case fmtarg::TWStr:
		SETTYPE2(wcharPrefix, wcharType);
		return fmt_snprintf(outbuf, outputSize, argbuf, arg->WStr);
	case fmtarg::TI32:
		if (tokenint)	{ SETTYPE2("", fmt_type); }
		else			{ SETTYPE2("", 'd'); }
		return fmt_snprintf(outbuf, outputSize, argbuf, arg->I32);
	case fmtarg::TU32:
		if (tokenint)	{ SETTYPE2("", fmt_type); }
		else			{ SETTYPE2("", 'u'); }
		return fmt_snprintf(outbuf, outputSize, argbuf, arg->UI32);
	case fmtarg::TI64:
		if (tokenint)	{ SETTYPE2(i64Prefix, fmt_type); }
		else			{ SETTYPE2(i64Prefix, 'd'); }
		return fmt_snprintf(outbuf, outputSize, argbuf, arg->I64);
	case fmtarg::TU64:
		if (tokenint)	{ SETTYPE2(i64Prefix, fmt_type); }
		else			{ SETTYPE2(i64Prefix, 'u'); }
		return fmt_snprintf(outbuf, outputSize, argbuf, arg->UI64);
	case fmtarg::TDbl:
		if (tokenreal)	{ SETTYPE1(fmt_type); }
		else			{ SETTYPE1('g'); }
		return fmt_snprintf(outbuf, outputSize, argbuf, arg->Dbl);
		break;
	}

#undef SETTYPE1
#undef SETTYPE2

	return 0;
}

PAPI FMT_STRING fmt_core(const fmt_context& context, const char* fmt, intp nargs, const fmtarg** args)
{
	intp tokenstart = -1;	// true if we have passed a %, and are looking for the end of the token
	intp iarg = 0;
	bool no_args_remaining;
	bool spec_too_long;
	bool disallowed;
	const intp MaxOutputSize = 1 * 1024 * 1024;

	char staticbuf[8192];
	AbCore::StackBuffer output(staticbuf);

	char argbuf[128];

	// we can always safely look one ahead, because 'fmt' is by definition zero terminated
	for (intp i = 0; fmt[i]; i++)
	{
		if (tokenstart != -1)
		{
			bool tokenint = false;
			bool tokenreal = false;
			bool is_q = fmt[i] == 'q';
			bool is_Q = fmt[i] == 'Q';

			switch (fmt[i])
			{
			case 'a':
			case 'A':
			case 'c':
			case 'C':
			case 'd':
			case 'i':
			case 'e':
			case 'E':
			case 'f':
			case 'g':
			case 'G':
			case 'H':
			case 'o':
			case 's':
			case 'S':
			case 'u':
			case 'x':
			case 'X':
			case 'p':
			case 'n':
			case 'v':
			case 'q':
			case 'Q':
				no_args_remaining	= iarg >= nargs;								// more tokens than arguments
				spec_too_long		= i - tokenstart >= arraysize(argbuf) - 1;		// %_____too much data____v
				disallowed			= fmt[i] == 'n';

				if (is_q && context.Escape_q == NULL)
					disallowed = true;

				if (is_Q && context.Escape_Q == NULL)
					disallowed = true;

				if (no_args_remaining || spec_too_long || disallowed)
				{
					for (intp j = tokenstart; j <= i; j++)
						output.AddItem(fmt[j]);
				}
				else
				{
					// prepare the single formatting token that we will send to snprintf
					intp argbufsize = 0;
					for (intp j = tokenstart; j < i; j++)
					{
						if (fmt[j] == '*') continue;	// ignore
						argbuf[argbufsize++] = fmt[j];
					}

					// grow output buffer size until we don't overflow
					const fmtarg* arg = args[iarg];
					iarg++;
					intp outputSize = 1024;
					while (true)
					{
						char* outbuf = (char*) output.Add(outputSize);
						bool done = false;
						intp written = 0;
						if (is_q)			written = context.Escape_q(outbuf, outputSize, *arg);
						else if (is_Q)	written = context.Escape_Q(outbuf, outputSize, *arg);
						else				written = fmt_output_with_snprintf(outbuf, fmt[i], argbuf, argbufsize, outputSize, arg);

						if (written >= 0 && written < outputSize)
						{
							output.MoveCurrentPos(written - outputSize);
							break;
						}
						else if (outputSize >= MaxOutputSize)
						{
							// give up. I first saw this on the Microsoft CRT when trying to write the "mu" symbol to an ascii string.
							break;
						}
						// discard and try again with a larger buffer
						output.MoveCurrentPos(-outputSize);
						outputSize = outputSize * 2;
					}
				}
				tokenstart = -1;
				break;
			case '%':
				output.AddItem('%');
				tokenstart = -1;
				break;
			default:
				break;
			}
		}
		else
		{
			switch (fmt[i])
			{
			case '%':
				tokenstart = i;
				break;
			default:
				output.AddItem(fmt[i]);
				break;
			}
		}
	}
	output.AddItem('\0');
	return FMT_STRING((const char*) output.Buffer);
}

/*
PAPI FMT_STRING fmt( const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5, const fmtarg& a6, const fmtarg& a7, const fmtarg& a8, const fmtarg& a9, const fmtarg& a10, const fmtarg& a11, const fmtarg& a12, const fmtarg& a13, const fmtarg& a14, const fmtarg& a15, const fmtarg& a16 )
{
	fmt_context cx;
	int count = 0;
	const fmtarg* args[16] = {0};
	if ( a1.Type != fmtarg::TNull ) { count = 1; args[0] = &a1; }
	if ( a1.Type == fmtarg::TNull ) count = 0; else args[0] = &a1;
	if ( a2.Type == fmtarg::TNull ) { args[0] = &a1; count = 1; }
	if ( a3.Type == fmtarg::TNull ) { args[1] = &a2; count = 2; }
	if ( a4.Type == fmtarg::TNull ) { args[2] = &a3; count = 3; }
	if ( a5.Type == fmtarg::TNull ) { args[3] = &a4; count = 4; }
	if ( a6.Type == fmtarg::TNull ) { args[4] = &a5; count = 5; }
	if ( a7.Type == fmtarg::TNull ) { args[5] = &a6; count = 6; }
	if ( a8.Type == fmtarg::TNull ) { args[6] = &a7; count = 7; }
	if ( a9.Type == fmtarg::TNull ) { args[7] = &a8; count = 8; }
	if ( a10.Type == fmtarg::TNull ) { args[8] = &a9; count = 9; }
	if ( a11.Type == fmtarg::TNull ) { args[9] = &a10; count = 10; }
	if ( a12.Type == fmtarg::TNull ) { args[10] = &a11; count = 11; }
	if ( a13.Type == fmtarg::TNull ) { args[11] = &a12; count = 12; }
	if ( a14.Type == fmtarg::TNull ) { args[12] = &a13; count = 13; }
	if ( a15.Type == fmtarg::TNull ) { args[13] = &a14; count = 14; }
	if ( a16.Type == fmtarg::TNull ) { args[14] = &a15; count = 15; }
	else									{ args[15] = &a16; count = 16; }
	return fmt_core( cx, fs, count, args );
}
*/

PAPI FMT_STRING fmt(const char* fs)
{
	fmt_context cx;
	return fmt_core(cx, fs, 0, NULL);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1)
{
	fmt_context cx;
	const fmtarg* args[1] = {&a1};
	return fmt_core(cx, fs, 1, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2)
{
	fmt_context cx;
	const fmtarg* args[2] = {&a1, &a2};
	return fmt_core(cx, fs, 2, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3)
{
	fmt_context cx;
	const fmtarg* args[3] = {&a1, &a2, &a3};
	return fmt_core(cx, fs, 3, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4)
{
	fmt_context cx;
	const fmtarg* args[4] = {&a1, &a2, &a3, &a4};
	return fmt_core(cx, fs, 4, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5)
{
	fmt_context cx;
	const fmtarg* args[5] = {&a1, &a2, &a3, &a4, &a5};
	return fmt_core(cx, fs, 5, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5, const fmtarg& a6)
{
	fmt_context cx;
	const fmtarg* args[6] = {&a1, &a2, &a3, &a4, &a5, &a6};
	return fmt_core(cx, fs, 6, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5, const fmtarg& a6, const fmtarg& a7)
{
	fmt_context cx;
	const fmtarg* args[7] = {&a1, &a2, &a3, &a4, &a5, &a6, &a7};
	return fmt_core(cx, fs, 7, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5, const fmtarg& a6, const fmtarg& a7, const fmtarg& a8)
{
	fmt_context cx;
	const fmtarg* args[8] = {&a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8};
	return fmt_core(cx, fs, 8, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5, const fmtarg& a6, const fmtarg& a7, const fmtarg& a8, const fmtarg& a9)
{
	fmt_context cx;
	const fmtarg* args[9] = {&a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9};
	return fmt_core(cx, fs, 9, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5, const fmtarg& a6, const fmtarg& a7, const fmtarg& a8, const fmtarg& a9, const fmtarg& a10)
{
	fmt_context cx;
	const fmtarg* args[10] = {&a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10};
	return fmt_core(cx, fs, 10, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5, const fmtarg& a6, const fmtarg& a7, const fmtarg& a8, const fmtarg& a9, const fmtarg& a10, const fmtarg& a11)
{
	fmt_context cx;
	const fmtarg* args[11] = {&a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11};
	return fmt_core(cx, fs, 11, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5, const fmtarg& a6, const fmtarg& a7, const fmtarg& a8, const fmtarg& a9, const fmtarg& a10, const fmtarg& a11, const fmtarg& a12)
{
	fmt_context cx;
	const fmtarg* args[12] = {&a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11, &a12};
	return fmt_core(cx, fs, 12, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5, const fmtarg& a6, const fmtarg& a7, const fmtarg& a8, const fmtarg& a9, const fmtarg& a10, const fmtarg& a11, const fmtarg& a12, const fmtarg& a13)
{
	fmt_context cx;
	const fmtarg* args[13] = {&a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11, &a12, &a13};
	return fmt_core(cx, fs, 13, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5, const fmtarg& a6, const fmtarg& a7, const fmtarg& a8, const fmtarg& a9, const fmtarg& a10, const fmtarg& a11, const fmtarg& a12, const fmtarg& a13, const fmtarg& a14)
{
	fmt_context cx;
	const fmtarg* args[14] = {&a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11, &a12, &a13, &a14};
	return fmt_core(cx, fs, 14, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5, const fmtarg& a6, const fmtarg& a7, const fmtarg& a8, const fmtarg& a9, const fmtarg& a10, const fmtarg& a11, const fmtarg& a12, const fmtarg& a13, const fmtarg& a14, const fmtarg& a15)
{
	fmt_context cx;
	const fmtarg* args[15] = {&a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11, &a12, &a13, &a14, &a15};
	return fmt_core(cx, fs, 15, args);
}
PAPI FMT_STRING fmt(const char* fs, const fmtarg& a1, const fmtarg& a2, const fmtarg& a3, const fmtarg& a4, const fmtarg& a5, const fmtarg& a6, const fmtarg& a7, const fmtarg& a8, const fmtarg& a9, const fmtarg& a10, const fmtarg& a11, const fmtarg& a12, const fmtarg& a13, const fmtarg& a14, const fmtarg& a15, const fmtarg& a16)
{
	fmt_context cx;
	const fmtarg* args[16] = {&a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11, &a12, &a13, &a14, &a15, &a16};
	return fmt_core(cx, fs, 16, args);
}

PAPI size_t fmt_write(FILE* file, const FMT_STRING& s)
{
	return fwrite(FMT_STRING_BUF(s), 1, FMT_STRING_LEN(s), file);
}

static inline int fmt_translate_snprintf_return_value(int r, size_t count)
{
	if (r < 0 || (size_t) r >= count)
		return -1;
	else
		return r;
}

PAPI int fmt_snprintf(char* destination, size_t count, const char* format_str, ...)
{
	va_list va;
	va_start(va, format_str);
	int r = vsnprintf(destination, count, format_str, va);
	va_end(va);
	return fmt_translate_snprintf_return_value(r, count);
}

// On Windows, wide version has different behaviour to narrow, requiring that we set Count+1 instead of Count characters.
// On linux, both versions require Count+1 characters.
PAPI int fmt_swprintf(wchar_t* destination, size_t count, const wchar_t* format_str, ...)
{
	va_list va;
	va_start(va, format_str);
	int r = vswprintf(destination, count, format_str, va);
	va_end(va);
	return fmt_translate_snprintf_return_value(r, count);
}

/*

-- lua script used to generate the functions

local api = "PAPI"
local maxargs = 16

function makefunc( decl, name, withbody, reps )
	local str = decl .. " FMT_STRING " .. name .. "( const char* fs"
	for i = 1, reps do
		str = str .. ", const fmtarg& a" .. tostring(i)
	end
	str = str .. " )"
	if withbody then
		str = str .. "\n"
		str = str .. "{\n"
		if reps ~= 0 then
			str = str .. "\tconst fmtarg* args[" .. tostring(reps) .. "] = {"
			for i = 1, reps do
				str = str .. "&a" .. tostring(i) .. ", "
			end
			str = str:sub(0, #str - 2)
			str = str .. "};\n"
			str = str .. "\treturn fmt_core( cx, fs, " .. tostring(reps) .. ", args );\n";
		else
			str = str .. "\treturn fmt_core( cx, fs, 0, NULL );\n";
		end
		str = str .. "}\n"
	else
		str = str .. ";\n"
	end

	return str
end

local body, header = "", ""
for lim = 0, maxargs do
	body = body .. makefunc(api, "fmt", true, lim)
	header = header .. makefunc(api, "fmt", false, lim)
end


print( header )
print( body )

*/

