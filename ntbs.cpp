#include <cstdarg>
#include <cstdio>
#include <stdexcept>
#include <iostream>

#include "ntbs.hpp"


ntbs::ntbs(size_t _max)
    : max(_max)
{
#ifdef NTBS_DEBUG
    printf("ntbs::ntbs default - max %ld\n", max);
#endif
    if (max == 0) {
        data.store = 0;
        type = NONE;
    }
    else {
        max++;
        data.store = new char[max]();
        type = ALLOC;
    }
}

ntbs::ntbs(const char* source, TYPE tt)
    : type(tt)
{
    max = strlen(source) + 1;
    if(type == ALLOC) {
        data.store = new char[max];
        strcpy(data.store, source);
    } else {
        data.store = (char *) source;
    }
}

ntbs::ntbs(char* source, size_t srclen, ntbs::TYPE _type)
    : type(_type)
{
    max = srclen+1;
    if(type == ALLOC) {
        data.store = new char[max];
        strcpy(data.store, source);
        return;
    } else {
        type = NONE; // CONST type not allowed here
        *source = 0;
    }
    data.store = source;
}

ntbs::ntbs(const ntbs& orig)
{
#ifdef NTBS_DEBUG
    printf("ntbs::ntbs - copy constructor: %s\n", orig.data.store);
#endif
    max = orig.max;
    if (max) {
        type = ALLOC;
        data.store = new char[max];
        strcpy(data.store, orig.data.store);
    } else {
        type = NONE;
        data.store = 0;
    }
}

#ifdef _LIBCPP_STRING
ntbs::ntbs(const std::string& orig)
{
    max = orig.size()+1;
    type = ALLOC;
    data.store = new char[max];
    strcpy(data.store, orig.data());
}
void
ntbs::operator=(const std::string& orig)
{
    // Force reallocation for const type
    if (type == CONST)
        max = 0;
    realloc(orig.size());
    strcpy(data.store, orig.c_str());
}
#endif
// --------------------------------------------------------
void ntbs::realloc(size_t req_bytes)
{
    if (req_bytes < max)
        return;
    max = req_bytes + 1;
    char* news = new char[max]();
    if (data.store) {
        strcpy(news, data.store);
        if (type == ALLOC)
            delete[] data.store;
    }
    data.store = news;
    type = ALLOC;
}
// --------------------------------------------------------
void
ntbs::operator=(const char* source)
{
#ifdef NTBS_DEBUG
    printf("ntbs::operator=(const char*) %s\n", source);
#endif
    size_t sl = strlen(source);
    if (type == CONST)
        max = 0; // Force allocation
    if (sl > max) 
        realloc (sl);
    strcpy(data.store, source);
}
void 
ntbs::operator=(const ntbs& orig)
{
#ifdef NTBS_DEBUG
    printf("ntbs::operator=(const ntbs&) %s\n", orig.get());
#endif
    max = orig.max;
    type = orig.type;
    if (type == ALLOC) {
        data.store = new char[max];
        strcpy(data.store, orig.data.store);
    } else {
        data.store = orig.data.store;
    }
}
void
ntbs::operator+=(const char* source)
{
    size_t sl = strlen(source);
    size_t orig_len = strlen(data.store);
    if (orig_len + sl >= max)
        realloc(orig_len + sl);
    strcpy(data.store + orig_len, source);
}
void
ntbs::operator+=(char ch)
{
    size_t slen = strlen(data.store);
    if (slen + 2 >= max)
        realloc(slen + 5);
    data.store[slen] = ch;
    data.store[slen+1] = 0;
}

ntbs
ntbs::operator+(const ntbs& right)
{
    ntbs cc(max + right.max + 1);
    cc = *this;
    cc += right;
    return cc;
}
// --------------------------------------------------------
int
ntbs::sprint(const char* fmt, ...)
{
    va_list vl;
    bool first = true;
    if (type == CONST)
        max = 0; // Force allocation
    retry_p:
    va_start(vl, fmt);
    int bw = std::vsnprintf(data.store, max, fmt, vl);
    va_end(vl);
    if (bw < 0)
        throw std::runtime_error("ntbs::printf format error.");
    if ((size_t)bw >= max) {
        if (first) {
            first = false;
            realloc(bw);
            goto retry_p;
        } else
            throw std::runtime_error("ntbs::printf unable to alloc more memory.");
    }
    return bw;
}
int
ntbs::addprint(const char* fmt, ...)
{
    va_list vl;
    size_t orig_len = strlen(data.store);
    size_t _max = 0;
    char*  ptr = nullptr;

    retry_a:
    va_start(vl, fmt);
    int bw = std::vsnprintf(ptr, _max, fmt, vl);
    va_end(vl);
    if (bw < 0)
        throw std::runtime_error("ntbs::append format error.");
    if (!ptr) {
        realloc(orig_len + bw);
        ptr = data.store + orig_len;
        _max = max;
        goto retry_a;
    }
    return bw;
}

// NOTE: Trim function will not work properly if gcc optimization is -O2
void
ntbs::trim(TRIM tt)
{
    using namespace std;
    const char *wi, *ws = " \t\n\r";
    if (!data.bytes[0])
        return;
    size_t sl = strlen(data.store);
    if (tt == BOTH || tt == RIGHT) {
        char* end = data.store + sl - 1;
        do {
            for (wi=ws; *wi; wi++) {
                if (*end == *wi) break;   
            }
            if (*wi) {
                *end = 0;
                end--;
                sl--;
            }
        } while (*wi && end >= data.store);
        if (!data.store)
            return;
    }
    if (tt == BOTH || tt == LEFT) {
        char* beg = data.store;
        do {
            for (wi=ws; *wi; wi++) {
                if (*beg == *wi) break;
            }
            if (*wi) {
                beg++;
                sl--;
            }
        } while (*wi && *beg);
        if (*beg)
            memmove(data.store, beg, sl+1);
        else 
            *data.store = 0;
    }
}

// http://www.isthe.com/chongo/src/fnv/hash_64.c
uint64_t
ntbs::hash_fnv(uint64_t salt) const
{
    if (!max)
        return 0;
    uint64_t hash = salt;
    size_t slen = strlen(data.store);

    unsigned char* s = (unsigned char*) data.store;
    while (slen) {
        hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) + (hash << 8) + (hash << 40);
        hash ^= (uint64_t)*s++;
        slen--;
    }
    return hash;
}

// --------------------------------------------------------
#ifdef NTBS_DEBUG
void
ntbs::dump(std::ostream& os)
{
    os << "ntbs (" << (void*) data.store << ' ';
    if (data.store) os << strlen(data.store);
    else os << "--";
    os << "/" << max << "/";
    os << (type == ALLOC ? "Alloc" : (type == CONST ? "Const" : "None"));
    os << ") = '" << get() << "'\n";
}
#endif
