#pragma once

inline void escape(void *p) {
    asm volatile("" : : "g"(p) : "memory");
}
void *nullp = nullptr;
