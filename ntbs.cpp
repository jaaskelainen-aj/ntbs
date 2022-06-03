#include <cstdarg>
#include <cstdio>
#include <stdexcept>
#include <iostream>

#include "ntbs.hpp"

ntbs::ntbs(size_t _max)
    : max(_max)
{
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
