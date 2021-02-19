#pragma once
#define PROLOG_EPILOG(x) __asm \
{                              \
    __asm pushfd               \
    __asm pushad               \
    __asm call x               \
    __asm popad                \
    __asm popfd                \
    __asm ret                  \
}