#pragma once

#include <universal/assertive.h>
#include <stdarg.h>

template <int BIT_COUNT>
struct bitarray // sizeof=0x10
{                                                                             // ...
    bitarray()
    {
        for (int j = 0; j < ((BIT_COUNT + 31) / 32); ++j)
            this->array[j] = 0;
    }
    bitarray(unsigned int first, ...)
    {
        unsigned int v2; // kr00_4
        int i; // [esp+0h] [ebp-Ch]
#ifdef KISAK_NX
        va_list argList;
#else
        char *argList; // [esp+8h] [ebp-4h]
#endif
        va_list va; // [esp+1Ch] [ebp+10h] BYREF

        va_start(va, first);
        for (i = 0; i < ((BIT_COUNT + 31) / 32); ++i)
            this->array[i] = 0;
        setBit(first);
        va_copy(argList, va);
        while (1)
        {
            v2 = va_arg(argList, unsigned int);
            if (v2 == -1)
                break;
            setBit(v2);
        }
    }

    void setBit(unsigned int pos)
    {
            iassert(pos < BIT_COUNT);
            array[pos / 32] |= 0x80000000 >> (pos & 0x1F);
    }

    void resetBit(unsigned int pos)
    {
            iassert(pos < BIT_COUNT);

            array[pos / 32] &= ~(0x80000000 >> (pos & 0x1F));
    }

    bool testBit(unsigned int pos) const
    {
            iassert(pos < BIT_COUNT);
            return (array[pos / 32] & (0x80000000 >> (pos & 0x1F))) != 0;
    }

    int &operator[](int i)
    {
            iassert(i >= 0 && (i * 32) < BIT_COUNT);
            return array[i];
    }

    const int &operator [](int i) const
    {
            iassert(i >= 0 && (i * 32) < BIT_COUNT);
            return array[i];
    }

    bool areAllBitsEqual(const bitarray<BIT_COUNT> *otherBitSet, const bitarray<BIT_COUNT> *ignoreMaskBitSet) const
    {
        for (int i = 0; i < (BIT_COUNT + 31) / 32; i++)
        {
            if ((this->array[i] & ~ignoreMaskBitSet->array[i]) != (otherBitSet->array[i] & ~ignoreMaskBitSet->array[i]))
                return false;
        }
        return true;
    }

    //private:
    int array[(BIT_COUNT + 31) / 32]; // round up to nearest 32 if needed
    //static_assert((BIT_COUNT % 32) == 0, "BIT_COUNT is not mul of 32!");
};

//static_assert(sizeof(bitarray<51>) == sizeof(int) * 2);
//static_assert(sizeof(bitarray<54>) == sizeof(int) * 2);
//static_assert(sizeof(bitarray<160>) == sizeof(int) * 5);

//#define BIT_COUNT 0x80 // bitarray size for variables such as `ignorePartBits`
