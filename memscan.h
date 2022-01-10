#pragma once

#include <Windows.h>
#include <vector>

namespace mem
{
	typedef struct _module_ctx
	{
		DWORD_PTR start;
		DWORD_PTR end;

	} module_ctx;

	typedef struct _memscan
	{

	public:

		LPVOID base;

		__forceinline _memscan* add(INT value)
		{
			if (base != NULL)
				base = (LPVOID)((DWORD_PTR)base + value);
			return this;
		}

		__forceinline _memscan* rip()
		{
			if (base != NULL)
			{
				DWORD offset = *reinterpret_cast<DWORD*>(base);
				base = (LPVOID)((DWORD_PTR)base + offset + sizeof(DWORD));
			}
			return this;
		}

		__forceinline LPVOID get_addr() { return base; }
		__forceinline LPVOID get_call()
		{
			if (base != NULL)
			{
				INT offset = *reinterpret_cast<INT*>((DWORD_PTR)base + 1);
				base = (LPVOID)((DWORD_PTR)base + 5 + offset);
			}
			return base;
		}

	} memscan;

	static __forceinline DWORD get_pattern_size(LPCSTR pattern, PDWORD markers)
	{
		DWORD length = 0;
		DWORD _markers = 0;

		for (DWORD i = 0; i < strlen(pattern); i++)
		{
			if ((BYTE)pattern[i] == 32) continue; //space
			else if ((BYTE)pattern[i] == 63) //question mark (?)
			{
				_markers++;
				continue;
			}
			length++;
		}

		if ((length % 2) != 0) return 0;

		*markers = _markers;
		return (length / 2);
	}

	static inline BOOL mem_compare(PBYTE pattern, PBYTE mem, DWORD searchmask, DWORD symbols)
	{
		if (pattern == NULL || mem == NULL) return FALSE;

		BOOL is_in_search = FALSE;
		DWORD counter = 0;

		for (DWORD i = 0; i < symbols; i++)
		{
			is_in_search = searchmask & (1 << i);
			if (is_in_search)
			{
				if (pattern[counter] != mem[i]) return FALSE;
				else
				{
					counter++;
					continue;
				}
			}
		}
		return TRUE;
	}

	static PBYTE alloc_strhex2array(LPCSTR pattern, PDWORD symbols, PDWORD searchmask)
	{
		if (pattern == NULL || symbols == NULL || searchmask == NULL) return NULL;

		PBYTE sig_bytes = NULL;
		DWORD markers = 0;
		DWORD sig_length = 0;

		DWORD _searchmask = 0;
		memset(&_searchmask, 0xff, sizeof(DWORD));

		if ((sig_length = get_pattern_size(pattern, &markers)) == 0) return NULL;
		if ((sig_bytes = (PBYTE)malloc(sig_length)) == NULL) return NULL;

		DWORD counter = 0, offset = 0;
		for (DWORD i = 0; i < strlen(pattern); i++)
		{
			if (counter == (sig_length + markers)) break;
			if ((BYTE)pattern[i] != 63 && (BYTE)pattern[i] != 32)
			{
				sscanf_s(&pattern[i], "%2hhx", &sig_bytes[counter]);
				i += 2;
				counter++;
				offset++;
			}

			if (pattern[i] == 63)
			{
				_searchmask &= ~(1 << offset);
				offset++;
			}
		}

		*searchmask = _searchmask;
		*symbols = sig_length + markers;
		return sig_bytes;
	}

	static memscan find(LPCSTR pattern, module_ctx* ctx)
	{
		PBYTE sig_bytes = NULL;
		DWORD symbols;
		DWORD searchmask;
		LPVOID ret = NULL;
		LPVOID curr_ptr = NULL;
		memscan scan = { NULL };

		if ((sig_bytes = alloc_strhex2array(pattern, &symbols, &searchmask)) == NULL) return scan;

		DWORD counter = 0;
		while (TRUE)
		{
			curr_ptr = (LPVOID)(ctx->start + (DWORD_PTR)counter);
			if ((DWORD_PTR)curr_ptr + symbols >= ctx->end) break;

			if (mem_compare(sig_bytes, (PBYTE)curr_ptr, searchmask, symbols))
			{
				ret = curr_ptr;
				break;
			}
			counter++;
		}

		free(sig_bytes);
		scan.base = ret;
		return scan;
	}
}