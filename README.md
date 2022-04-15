# ntbs - library
A small C++ string library for Null Terminated Byte Strings.

Library is intended to be used in applications where `std::string` is not feasible. This is often case with embeded systems and applications that make extensive use of C-library. It basically acts as a smart pointer for `const char*` and `char*` types. Class stores only three things: maximum size reserved for the string, type of allocation and the pointer to null terminated byte string.

Major differences with std::string :
- As with std::string, on assignment more memory is reserved if needed but class only reserves required amount i.e. no exponential growth like in std::string. If memory has been allocated, it will be automatically released once object goes out of scope.
- Library has a macro `NTBS(var, size)` that reserves desired size char-array from stack and creates ntbs-object for it. This allows programmer to choose wether to use local stack or heap instead of forcing the use of heap.
- ntbs-class does not have any support for standard library streams. Streams don't offer clear benefit over buffered streams and carry same memory disadvantage as std::strings. 

Actual ntbs string can be accessed with get()-function. If returns pointer to allocated null terminated memory. This memory can be manipulated just as regular char-buffers can be manipulated. *Please note that assigning new value to ntbs-object may require a reservation of more memory and thus invalidates previous pointer received by get()*.

# Samples
Basic declaration and copy

	  ntbs foo;				// Initialize empty zero-length object
	  ntbs bar(64);		// Reserve 64 byte buffer from heap
    ntbs cee("Init to const string");
    //
	  foo = "Hello world";	// Allocate memory for new value
	  bar = foo;				// Copy value to existing buffer

Use stack memory and sprint

    NTBS(txt, 64);
    txt.sprint("Current unix time = %ld", time());
    fputs(txt.get());
    
