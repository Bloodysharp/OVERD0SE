#include <intrin.h>
#include <d3d11.h>

#include "memory.h"
#include "crt.h"
#include "pe64.h"

#include "../core/spoofcall/invoker.h"

bool MEM::Setup()
{
	bool bSuccess = true;

	const void* hSDL3 = spoof_call<void*>(_fake_addr, &GetModuleBaseHandle, SDL3_DLL);
	const void* hDbgHelp = spoof_call<void*>(_fake_addr, &GetModuleBaseHandle, DBGHELP_DLL);
	const void* hTier0 = GetModuleBaseHandle(TIER0_DLL);

	if (hSDL3 == nullptr || hDbgHelp == nullptr)
		return false;

	fnUnDecorateSymbolName = reinterpret_cast<decltype(fnUnDecorateSymbolName)>(GetExportAddress(hDbgHelp, CXOR("UnDecorateSymbolName")));
	bSuccess &= (fnUnDecorateSymbolName != nullptr);

	fnCreateMaterial = reinterpret_cast<decltype(fnCreateMaterial)>(FindPattern(MATERIAL_SYSTEM2_DLL, CXOR("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05")));
	bSuccess &= (fnCreateMaterial != nullptr);

	printf(CXOR("[Memory] Loaded fnCreateMaterial\n"));

	fnUtlBufferPutString = reinterpret_cast<decltype(fnUtlBufferPutString)>(GetExportAddress(hTier0, CXOR("?PutString@CUtlBuffer@@QEAAXPEBD@Z")));
	bSuccess &= (fnUtlBufferPutString != nullptr);

	fnUtlBufferInit = reinterpret_cast<decltype(fnUtlBufferInit)>(GetExportAddress(hTier0, CXOR("??0CUtlBuffer@@QEAA@HHH@Z")));
	bSuccess &= (fnUtlBufferInit != nullptr);

	fnUtlBufferEnsureCapacity = reinterpret_cast<decltype(fnUtlBufferEnsureCapacity)>(GetExportAddress(hTier0, CXOR("?EnsureCapacity@CUtlBuffer@@QEAAXH@Z")));
	bSuccess &= (fnUtlBufferEnsureCapacity != nullptr);

	//SetMeshGroupMask = reinterpret_cast<decltype(SetMeshGroupMask)>(FindPattern(CLIENT_DLL, CXOR("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71")));
	//bSuccess &= (SetMeshGroupMask != nullptr);

	//fnGetInventoryManager = reinterpret_cast<decltype(fnGetInventoryManager)>(FindPattern(CLIENT_DLL, CXOR("48 8d 05 ?? ?? ?? ?? c3 cc cc cc cc cc cc cc cc 8b 91 ?? ?? ?? ?? b8")));
	//bSuccess &= (fnGetInventoryManager != nullptr);
	//fnGetClientSystem = reinterpret_cast<decltype(fnGetClientSystem)>(FindPattern(CLIENT_DLL, CS_XOR("48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B FA 48 8B F1")));
	//bSuccess &= (fnGetClientSystem != nullptr);
	return bSuccess;
}

void* CS_CDECL operator new(const std::size_t nSize)
{
	return MEM::HeapAlloc(nSize);
}

void* CS_CDECL operator new[](const std::size_t nSize)
{
	return MEM::HeapAlloc(nSize);
}

void CS_CDECL operator delete(void* pMemory) noexcept
{
	MEM::HeapFree(pMemory);
}

void CS_CDECL operator delete[](void* pMemory) noexcept
{
	MEM::HeapFree(pMemory);
}

void* MEM::HeapAlloc(const std::size_t nSize)
{
	const HANDLE hHeap = ::GetProcessHeap();
	return ::HeapAlloc(hHeap, 0UL, nSize);
}

void MEM::HeapFree(void* pMemory)
{
	if (pMemory != nullptr)
	{
		const HANDLE hHeap = ::GetProcessHeap();
		::HeapFree(hHeap, 0UL, pMemory);
	}
}

void* MEM::HeapRealloc(void* pMemory, const std::size_t nNewSize)
{
	if (pMemory == nullptr)
		return HeapAlloc(nNewSize);

	if (nNewSize == 0UL)
	{
		HeapFree(pMemory);
		return nullptr;
	}

	const HANDLE hHeap = ::GetProcessHeap();
	return ::HeapReAlloc(hHeap, 0UL, pMemory, nNewSize);
}

void* MEM::GetModuleBaseHandle(const wchar_t* wszModuleName)
{
	const _PEB* pPEB = reinterpret_cast<_PEB*>(__readgsqword(0x60));

	if (wszModuleName == nullptr)
		return pPEB->ImageBaseAddress;

	void* pModuleBase = nullptr;
	for (LIST_ENTRY* pListEntry = pPEB->Ldr->InMemoryOrderModuleList.Flink; pListEntry != &pPEB->Ldr->InMemoryOrderModuleList; pListEntry = pListEntry->Flink)
	{
		const _LDR_DATA_TABLE_ENTRY* pEntry = CONTAINING_RECORD(pListEntry, _LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

		if (pEntry->FullDllName.Buffer != nullptr && CRT::StringCompare(wszModuleName, pEntry->BaseDllName.Buffer) == 0)
		{
			pModuleBase = pEntry->DllBase;
			break;
		}
	}

	if (pModuleBase == nullptr)
		printf(CXOR("module base not found: %s \n"), wszModuleName);

	return pModuleBase;
}

const wchar_t* MEM::GetModuleBaseFileName(const void* hModuleBase)
{
	const _PEB* pPEB = reinterpret_cast<_PEB*>(__readgsqword(0x60));

	if (hModuleBase == nullptr)
		hModuleBase = pPEB->ImageBaseAddress;

	::EnterCriticalSection(pPEB->LoaderLock);

	const wchar_t* wszModuleName = nullptr;
	for (LIST_ENTRY* pListEntry = pPEB->Ldr->InMemoryOrderModuleList.Flink; pListEntry != &pPEB->Ldr->InMemoryOrderModuleList; pListEntry = pListEntry->Flink)
	{
		const _LDR_DATA_TABLE_ENTRY* pEntry = CONTAINING_RECORD(pListEntry, _LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

		if (pEntry->DllBase == hModuleBase)
		{
			wszModuleName = pEntry->BaseDllName.Buffer;
			break;
		}
	}

	::LeaveCriticalSection(pPEB->LoaderLock);

	return wszModuleName;
}

void* MEM::GetExportAddress(const void* hModuleBase, const char* szProcedureName)
{
	const auto pBaseAddress = static_cast<const std::uint8_t*>(hModuleBase);

	const auto pIDH = static_cast<const IMAGE_DOS_HEADER*>(hModuleBase);
	if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
		return nullptr;

	const auto pINH = reinterpret_cast<const IMAGE_NT_HEADERS64*>(pBaseAddress + pIDH->e_lfanew);
	if (pINH->Signature != IMAGE_NT_SIGNATURE)
		return nullptr;

	const IMAGE_OPTIONAL_HEADER64* pIOH = &pINH->OptionalHeader;
	const std::uintptr_t nExportDirectorySize = pIOH->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	const std::uintptr_t uExportDirectoryAddress = pIOH->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

	if (nExportDirectorySize == 0U || uExportDirectoryAddress == 0U)
	{
		printf(CXOR("module has no exports: %s\n"), GetModuleBaseFileName(hModuleBase));
		return nullptr;
	}

	const auto pIED = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(pBaseAddress + uExportDirectoryAddress);
	const auto pNamesRVA = reinterpret_cast<const std::uint32_t*>(pBaseAddress + pIED->AddressOfNames);
	const auto pNameOrdinalsRVA = reinterpret_cast<const std::uint16_t*>(pBaseAddress + pIED->AddressOfNameOrdinals);
	const auto pFunctionsRVA = reinterpret_cast<const std::uint32_t*>(pBaseAddress + pIED->AddressOfFunctions);

	// Perform binary search to find the export by name
	std::size_t nRight = pIED->NumberOfNames, nLeft = 0U;
	while (nRight != nLeft)
	{
		// Avoid INT_MAX/2 overflow
		const std::size_t uMiddle = nLeft + ((nRight - nLeft) >> 1U);
		const int iResult = CRT::StringCompare(szProcedureName, reinterpret_cast<const char*>(pBaseAddress + pNamesRVA[uMiddle]));

		if (iResult == 0)
		{
			const std::uint32_t uFunctionRVA = pFunctionsRVA[pNameOrdinalsRVA[uMiddle]];

			printf(CXOR("export found: %s\n"), szProcedureName);

			// Check if it's a forwarded export
			if (uFunctionRVA >= uExportDirectoryAddress && uFunctionRVA - uExportDirectoryAddress < nExportDirectorySize)
			{
				// Forwarded exports are not supported
				break;
			}

			return const_cast<std::uint8_t*>(pBaseAddress) + uFunctionRVA;
		}

		if (iResult > 0)
			nLeft = uMiddle + 1;
		else
			nRight = uMiddle;
	}

	printf(CXOR("export not found: %s\n"), szProcedureName);

	// Export not found
	return nullptr;
}

bool MEM::GetSectionInfo(const void* hModuleBase, const char* szSectionName, std::uint8_t** ppSectionStart, std::size_t* pnSectionSize)
{
	const auto pBaseAddress = static_cast<const std::uint8_t*>(hModuleBase);

	const auto pIDH = static_cast<const IMAGE_DOS_HEADER*>(hModuleBase);
	if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	const auto pINH = reinterpret_cast<const IMAGE_NT_HEADERS*>(pBaseAddress + pIDH->e_lfanew);
	if (pINH->Signature != IMAGE_NT_SIGNATURE)
		return false;

	const IMAGE_SECTION_HEADER* pISH = IMAGE_FIRST_SECTION(pINH);

	// go through all code sections
	for (WORD i = 0U; i < pINH->FileHeader.NumberOfSections; i++, pISH++)
	{
		// @test: use case insensitive comparison instead?
		if (CRT::StringCompareN(szSectionName, reinterpret_cast<const char*>(pISH->Name), IMAGE_SIZEOF_SHORT_NAME) == 0)
		{
			if (ppSectionStart != nullptr)
				*ppSectionStart = const_cast<std::uint8_t*>(pBaseAddress) + pISH->VirtualAddress;

			if (pnSectionSize != nullptr)
				*pnSectionSize = pISH->SizeOfRawData;

			return true;
		}
	}

	printf(CXOR("code section not found: %s"), szSectionName);
	return false;
}

UTILPtr MEM::FindPatterns(const wchar_t* wszModuleName, const char* szPattern)
{
	// convert pattern string to byte array
	const std::size_t nApproximateBufferSize = (CRT::StringLength(szPattern) >> 1U) + 1U;
	std::uint8_t* arrByteBuffer = static_cast<std::uint8_t*>(MEM_STACKALLOC(nApproximateBufferSize));
	char* szMaskBuffer = static_cast<char*>(MEM_STACKALLOC(nApproximateBufferSize));
	PatternToBytes(szPattern, arrByteBuffer, szMaskBuffer);

	// @test: use search with straight in-place conversion? do not think it will be faster, cuz of bunch of new checks that gonna be performed for each iteration
	return FindPattern(wszModuleName, reinterpret_cast<const char*>(arrByteBuffer), szMaskBuffer);
}

std::uint8_t* MEM::FindPattern(const wchar_t* wszModuleName, const char* szPattern)
{
	// convert pattern string to byte array
	const std::size_t nApproximateBufferSize = (CRT::StringLength(szPattern) >> 1U) + 1U;
	std::uint8_t* arrByteBuffer = static_cast<std::uint8_t*>(MEM_STACKALLOC(nApproximateBufferSize));
	char* szMaskBuffer = static_cast<char*>(MEM_STACKALLOC(nApproximateBufferSize));
	PatternToBytes(szPattern, arrByteBuffer, szMaskBuffer);

	// @test: use search with straight in-place conversion? do not think it will be faster, cuz of bunch of new checks that gonna be performed for each iteration
	return FindPattern(wszModuleName, reinterpret_cast<const char*>(arrByteBuffer), szMaskBuffer);
}

std::uint8_t* MEM::FindPattern(const wchar_t* wszModuleName, const char* szBytePattern, const char* szByteMask)
{
	const void* hModuleBase = spoof_call<void*>(_fake_addr, &GetModuleBaseHandle, wszModuleName);

	if (hModuleBase == nullptr)
	{
		printf(CXOR("failed to get module handle for: %s\n"), wszModuleName);
		return nullptr;
	}

	const auto pBaseAddress = static_cast<const std::uint8_t*>(hModuleBase);

	const auto pIDH = static_cast<const IMAGE_DOS_HEADER*>(hModuleBase);
	if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
	{
		printf(CXOR("failed to get module size, image is invalid\n"));
		return nullptr;
	}

	const auto pINH = reinterpret_cast<const IMAGE_NT_HEADERS*>(pBaseAddress + pIDH->e_lfanew);
	if (pINH->Signature != IMAGE_NT_SIGNATURE)
	{
		printf(CXOR("failed to get module size, image is invalid\n"));
		return nullptr;
	}

	const std::uint8_t* arrByteBuffer = reinterpret_cast<const std::uint8_t*>(szBytePattern);
	const std::size_t nByteCount = CRT::StringLength(szByteMask);

	std::uint8_t* pFoundAddress = nullptr;

	// perform little overhead to keep all patterns unique
#ifdef CS_PARANOID_PATTERN_UNIQUENESS 
	const std::vector<std::uint8_t*> vecFoundOccurrences = FindPatternAllOccurrencesEx(pBaseAddress, pINH->OptionalHeader.SizeOfImage, arrByteBuffer, nByteCount, szByteMask);

	// notify user about non-unique pattern
	if (!vecFoundOccurrences.empty())
	{
		// notify user about non-unique pattern
		if (vecFoundOccurrences.size() > 1U)
		{
			char* szPattern = static_cast<char*>(MEM_STACKALLOC((nByteCount << 1U) + nByteCount));
			[[maybe_unused]] const std::size_t nConvertedPatternLength = BytesToPattern(arrByteBuffer, nByteCount, szPattern);

			printf(CXOR("found more than one occurrence with %s pattern, consider updating it!\n"), szPattern);

			MEM_STACKFREE(szPattern);
		}

		// return first found occurrence
		pFoundAddress = vecFoundOccurrences[0];
	}
#else
	// @todo: we also can go through code sections and skip noexec pages, but will it really improve performance? / or at least for all occurrences search
	// https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_section_header
#if 0
	IMAGE_SECTION_HEADER* pCurrentSection = IMAGE_FIRST_SECTION(pINH);
	for (WORD i = 0U; i != pINH->FileHeader.NumberOfSections; i++)
	{
		// check does page have executable code
		if (pCurrentSection->Characteristics & IMAGE_SCN_CNT_CODE || pCurrentSection->Characteristics & IMAGE_SCN_MEM_EXECUTE)
		{
			pFoundAddress = FindPatternEx(pBaseAddress + pCurrentSection->VirtualAddress, pCurrentSection->SizeOfRawData, arrByteBuffer, nByteCount, szByteMask);

			if (pFoundAddress != nullptr)
				break;
		}

		++pCurrentSection;
	}
#else
	pFoundAddress = FindPatternEx(pBaseAddress, pINH->OptionalHeader.SizeOfImage, arrByteBuffer, nByteCount, szByteMask);
#endif
#endif

	if (pFoundAddress == nullptr)
	{
		char* szPattern = static_cast<char*>(MEM_STACKALLOC((nByteCount << 1U) + nByteCount));
		[[maybe_unused]] const std::size_t nConvertedPatternLength = BytesToPattern(arrByteBuffer, nByteCount, szPattern);

		printf(CXOR("pattern not found: %s\n"), szPattern);

		MEM_STACKFREE(szPattern);
	}

	return pFoundAddress;
}

// @todo: msvc poorly optimizes this, it looks even better w/o optimization at all
std::uint8_t* MEM::FindPatternEx(const std::uint8_t* pRegionStart, const std::size_t nRegionSize, const std::uint8_t* arrByteBuffer, const std::size_t nByteCount, const char* szByteMask)
{
	std::uint8_t* pCurrentAddress = const_cast<std::uint8_t*>(pRegionStart);
	const std::uint8_t* pRegionEnd = pRegionStart + nRegionSize - nByteCount;
	const bool bIsMaskUsed = (szByteMask != nullptr);

	while (pCurrentAddress < pRegionEnd)
	{
		// check the first byte before entering the loop, otherwise if there two consecutive bytes of first byte in the buffer, we may skip both and fail the search
		if ((bIsMaskUsed && *szByteMask == '?') || *pCurrentAddress == *arrByteBuffer)
		{
			if (nByteCount == 1)
				return pCurrentAddress;

			// compare the least byte sequence and continue on wildcard or skip forward on first mismatched byte
			std::size_t nComparedBytes = 0U;
			while ((bIsMaskUsed && szByteMask[nComparedBytes + 1U] == '?') || pCurrentAddress[nComparedBytes + 1U] == arrByteBuffer[nComparedBytes + 1U])
			{
				// check does byte sequence match
				if (++nComparedBytes == nByteCount - 1U)
					return pCurrentAddress;
			}

			// skip non suitable bytes
			pCurrentAddress += nComparedBytes;
		}

		++pCurrentAddress;
	}

	return nullptr;
}

std::vector<std::uint8_t*> MEM::FindPatternAllOccurrencesEx(const std::uint8_t* pRegionStart, const std::size_t nRegionSize, const std::uint8_t* arrByteBuffer, const std::size_t nByteCount, const char* szByteMask)
{
	const std::uint8_t* pRegionEnd = pRegionStart + nRegionSize - nByteCount;
	const bool bIsMaskUsed = (szByteMask != nullptr);

	// container for addresses of the all found occurrences
	std::vector<std::uint8_t*> vecOccurrences = {};

	for (std::uint8_t* pCurrentByte = const_cast<std::uint8_t*>(pRegionStart); pCurrentByte < pRegionEnd; ++pCurrentByte)
	{
		// do a first byte check before entering the loop, otherwise if there two consecutive bytes of first byte in the buffer, we may skip both and fail the search
		if ((!bIsMaskUsed || *szByteMask != '?') && *pCurrentByte != *arrByteBuffer)
			continue;

		// check for bytes sequence match
		bool bSequenceMatch = true;
		for (std::size_t i = 1U; i < nByteCount; i++)
		{
			// compare sequence and continue on wildcard or skip forward on first mismatched byte
			if ((!bIsMaskUsed || szByteMask[i] != '?') && pCurrentByte[i] != arrByteBuffer[i])
			{
				// skip non suitable bytes
				pCurrentByte += i - 1U;

				bSequenceMatch = false;
				break;
			}
		}

		// check did we found address
		if (bSequenceMatch)
			vecOccurrences.push_back(pCurrentByte);
	}

	return vecOccurrences;
}

std::size_t MEM::PatternToBytes(const char* szPattern, std::uint8_t* pOutByteBuffer, char* szOutMaskBuffer)
{
	std::uint8_t* pCurrentByte = pOutByteBuffer;

	while (*szPattern != '\0')
	{
		// check is a wildcard
		if (*szPattern == '?')
		{
			++szPattern;
#ifdef CS_PARANOID
			CS_ASSERT(*szPattern == '\0' || *szPattern == ' ' || *szPattern == '?'); // we're expect that next character either terminating null, whitespace or part of double wildcard (note that it's required if your pattern written without whitespaces)
#endif

			// ignore that
			* pCurrentByte++ = 0U;
			*szOutMaskBuffer++ = '?';
		}
		// check is not space
		else if (*szPattern != ' ')
		{
			// convert two consistent numbers in a row to byte value
			std::uint8_t uByte = static_cast<std::uint8_t>(CRT::CharToHexInt(*szPattern) << 4);

			++szPattern;
#ifdef CS_PARANOID
			CS_ASSERT(*szPattern != '\0' && *szPattern != '?' && *szPattern != ' '); // we're expect that byte always represented by two numbers in a row
#endif

			uByte |= static_cast<std::uint8_t>(CRT::CharToHexInt(*szPattern));

			*pCurrentByte++ = uByte;
			*szOutMaskBuffer++ = 'x';
		}

		++szPattern;
	}

	// zero terminate both buffers
	*pCurrentByte = 0U;
	*szOutMaskBuffer = '\0';

	return pCurrentByte - pOutByteBuffer;
}

std::size_t MEM::BytesToPattern(const std::uint8_t* pByteBuffer, const std::size_t nByteCount, char* szOutBuffer)
{
	char* szCurrentPattern = szOutBuffer;

	for (std::size_t i = 0U; i < nByteCount; i++)
	{
		// manually convert byte to chars
		const char* szHexByte = &CRT::_TWO_DIGITS_HEX_LUT[pByteBuffer[i] * 2U];
		*szCurrentPattern++ = szHexByte[0];
		*szCurrentPattern++ = szHexByte[1];
		*szCurrentPattern++ = ' ';
	}
	*--szCurrentPattern = '\0';

	return szCurrentPattern - szOutBuffer;
}