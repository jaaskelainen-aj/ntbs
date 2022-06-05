#pragma once

#include <cstring>

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
#endif    
    virtual ~ntbs() {
        if (type == ALLOC)
            delete[] data.store;
    }
    void realloc(size_t req_bytes);

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
    bool operator==(const ntbs& target) {
        return std::strcmp(data.store, target.data.store) ? false : true;
    }
    ntbs operator+(const ntbs& right);

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
