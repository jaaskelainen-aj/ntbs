#pragma once

#include <cstring>
#include <cstdint>
#include <functional>

#define NTBS(V,L)\
    char V ## _ntbs[L];\
    ntbs V(V ## _ntbs, sizeof(V ## _ntbs), ntbs::NONE)

class ntbs
{
public:
    enum TYPE { NONE, ALLOC, CONST };
    enum TRIM { BOTH, LEFT, RIGHT };

    ntbs(size_t _max = 0);
    ntbs(const char* src, TYPE tt = CONST);
    ntbs(char* src, size_t srclen, TYPE type);
    ntbs(const ntbs& orig);
#ifdef _LIBCPP_STRING
    ntbs(const std::string&);
    void operator=(const std::string&);
#endif    
    virtual ~ntbs() {
        if (type == ALLOC)
            delete[] data.store;
    }
    void realloc(size_t req_bytes);
    void clear() {
        if (max && type != CONST)
            *data.store = 0;
    }

    void operator=(char* src) { 
        *this = (const char*) src; 
    }
    void operator=(const char*);
    void operator=(const ntbs&);
    void operator+=(const char*);
    void operator+=(char *source) {
        operator+=((const char*) source);
    }
    void operator+=(const ntbs& source) {
        operator+=((const char*) source.data.store);
    }
    void operator+=(char);
    ntbs operator+(const ntbs& right);

    bool operator==(const ntbs& target) const {
        return std::strcmp(data.store, target.data.store) ? false : true;
    }
    int compare(const ntbs& target) const {
        return std::strcmp(data.store, target.data.store);
    }
    size_t find(int ch) const {
        if (!max)
            return SIZE_MAX;
        const char* ndx = std::strchr(data.store, ch);
        return ndx ? data.store - ndx : SIZE_MAX;
    }
    size_t find(const char* target) const {
        if (!max || target == data.store)
            return SIZE_MAX;
        const char* ndx = std::strstr(data.store, target);
        return ndx ? data.store - ndx : SIZE_MAX;
    }
    size_t find(const ntbs& target) const {
        return find(target.data.store);
    }

    int sprint(const char* fmt, ...);
    int addprint(const char* fmt, ...);

    char* get() const { 
        return max ? data.store : (char*)data.bytes; 
    }
    void terminate(size_t bw) { 
        if (bw < max) data.store[bw] = 0;
    }
    bool is_allocated() const { 
        return type == ALLOC ? true : false;
    }
    size_t size() const { 
        return max > 0 ? max-1 : 0; 
    }
    size_t len() const { 
        return std::strlen(data.store); 
    }

    void trim(TRIM tt = ntbs::BOTH);
    uint64_t hash_fnv(uint64_t salt=0) const;

#ifdef NTBS_DEBUG
    void dump(std::ostream&);
#endif

protected:
    union {
        char   bytes[sizeof(char*)];
        char*  store;
    } data;
    
    size_t  max;
    TYPE    type;
};

namespace std {
    template<> struct hash<ntbs> {
        size_t operator() (const ntbs& target) const {
            return (size_t) target.hash_fnv();
        }
    };
}