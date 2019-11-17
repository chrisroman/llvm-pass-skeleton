#pragma once

void escape(void *p) {
    asm volatile("" : : "g"(p) : "memory");
}
void *nullp = nullptr;
