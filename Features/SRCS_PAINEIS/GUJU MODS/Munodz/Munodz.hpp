#pragma once

//
// Memory Tools Internal by foxcheatsid@gmail.com
// Created on Tue Apr 4 13:13:13 2123
//

/*
* Type
*/
enum Type {
    TYPE_DWORD,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_WORD,
    TYPE_BYTE,
    TYPE_QWORD,
};

/*
* Maps, Semua hasil pencarian ada di sini 
*/
struct Maps_t {
    uintptr_t start, end;
	Type type;
    struct Maps_t *next;
};

#define SIZE sizeof(struct Maps_t)

/*
* List Memori Region 
*/
enum RegionType {
    ALL,
    JAVA_HEAP,
    C_HEAP,
    C_ALLOC,
    C_DATA,
    C_BSS,
    PPSSPP,
    ANONYMOUS,
    JAVA,
    STACK,
    ASHMEM,
    VIDEO,
    OTHER,
    BAD,
    CODE_APP,
    CODE_SYS
};

namespace kFox 
{
    /*
    * Mendapatkan PackageName
    */
	const char *GetPackageName();
	
    /*
    * Mendapatkan semua hasil pencarian memori
    */
	extern Maps_t* GetResult();
	
    /*
    * Menghapus semua hasil pencarian memori
    */
	extern void ClearResult();
	
    /*
    * Set maximal hasil pencarian
    */
	extern void SetMaxResult(int max_result);
	
    /*
    * Set Region
    */
	extern int SetSearchRange(RegionType type);
	
    /*
    * Mencari value berdasarkan Region dan Type
    */
    extern void MemorySearch(char* value, Type type);
	
    /*
    * Mencari value offset berdasarkan Region dan Type
    */
	extern void MemoryOffset(char *value, long int offset, Type type);
	
    /*
    * Write value berdasarkan offset
    */
	extern void MemoryWrite(char *value, long int offset, Type type);
	
    /*
    * Write value
    */
	extern void WriteValues(uintptr_t address, void *value, ssize_t size);
	
    /*
    * Read value
    */
	extern long ReadValues(uintptr_t address, void *buffer, ssize_t size);
	
	inline void ToggleValues2(char* onVal1, char* onVal2,
                               char* offVal1, char* offVal2,
                               bool state, Type type)
    {
        if (state) {
            // Bật: đổi onVal1 -> offVal1, onVal2 -> offVal2
            kFox::SetSearchRange(RegionType::ALL);
            kFox::MemorySearch(onVal1, type);
            kFox::MemoryWrite(offVal1, 0, type);
            kFox::ClearResult();

            kFox::SetSearchRange(RegionType::ALL);
            kFox::MemorySearch(onVal2, type);
            kFox::MemoryWrite(offVal2, 0, type);
            kFox::ClearResult();
        } else {
            // Tắt: đổi offVal1 -> onVal1, offVal2 -> onVal2
            kFox::SetSearchRange(RegionType::ALL);
            kFox::MemorySearch(offVal1, type);
            kFox::MemoryWrite(onVal1, 0, type);
            kFox::ClearResult();

            kFox::SetSearchRange(RegionType::ALL);
            kFox::MemorySearch(offVal2, type);
            kFox::MemoryWrite(onVal2, 0, type);
            kFox::ClearResult();
        }
    }
}

    

    

