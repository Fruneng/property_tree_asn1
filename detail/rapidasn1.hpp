// ----------------------------------------------------------------------------
// Copyright (C) 2015-2016 zunceng@gmail.com
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see www.boost.org
// ----------------------------------------------------------------------------
#ifndef BOOST_PROPERTY_TREE_RAPIDASN1_HPP_INCLUDED
#define BOOST_PROPERTY_TREE_RAPIDASN1_HPP_INCLUDED

//! \file rapidasn1.hpp This file contains rapidasn1 parser. 
#include <boost/lexical_cast.hpp>
#include <boost/assert.hpp>
#include <cstdlib>      // For std::size_t
#include <new>          // For placement new

///////////////////////////////////////////////////////////////////////////
// BOOST_PROPERTY_TREE_RAPIDASN1_PARSE_ERROR
    
#include <exception>    // For std::exception

#define BOOST_PROPERTY_TREE_RAPIDASN1_PARSE_ERROR(what, where) throw parse_error(what, where)

namespace boost { namespace property_tree { namespace detail {namespace rapidasn1
{

    //! Parse error exception. 
    //! This exception is thrown by the parser when an error occurs. 
    //! Use what() function to get human-readable error message. 
    //! Use where() function to get a pointer to position within source text where error was detected.
    //! <br><br>
    //! If throwing exceptions by the parser is undesirable, 
    //! it can be disabled by defining RAPIDASN1_NO_EXCEPTIONS macro before rapidasn1.hpp is included.
    //! This will cause the parser to call rapidasn1::parse_error_handler() function instead of throwing an exception.
    //! This function must be defined by the user.
    //! <br><br>
    //! This class derives from <code>std::exception</code> class.
    class parse_error: public std::exception
    {
    
    public:
    
        //! Constructs parse error
        parse_error(const char *wa, size_t we)
            : m_what(wa)
            , m_where(we)
        {
        }
        
        //! Destructor parse error
        virtual ~parse_error() throw()
        {
            
        }
        
        //! Gets human readable description of error.
        //! \return Pointer to null terminated description of the error.
        virtual const char *what() const throw()
        {
            return m_what;
        }

        //! Gets position of data where error happened.
        //! \return Pointer to location within the parsed string where error occured.
        size_t where() const throw()
        {
            return m_where;
        }

    private:  

        const char *m_what;
        size_t m_where;

    };
}}}}

///////////////////////////////////////////////////////////////////////////
// Pool sizes

#ifndef BOOST_PROPERTY_TREE_RAPIDASN1_STATIC_POOL_SIZE
    // Size of static memory block of memory_pool.
    // Define BOOST_PROPERTY_TREE_RAPIDASN1_STATIC_POOL_SIZE before including rapidxml.hpp if you want to override the default value.
    // No dynamic memory allocations are performed by memory_pool until static memory is exhausted.
    #define BOOST_PROPERTY_TREE_RAPIDASN1_STATIC_POOL_SIZE (64 * 1024)
#endif

#ifndef BOOST_PROPERTY_TREE_RAPIDASN1_DYNAMIC_POOL_SIZE
    // Size of dynamic memory block of memory_pool.
    // Define BOOST_PROPERTY_TREE_RAPIDASN1_DYNAMIC_POOL_SIZE before including rapidxml.hpp if you want to override the default value.
    // After the static block is exhausted, dynamic blocks with approximately this size are allocated by memory_pool.
    #define BOOST_PROPERTY_TREE_RAPIDASN1_DYNAMIC_POOL_SIZE (64 * 1024)
#endif

#ifndef BOOST_PROPERTY_TREE_RAPIDASN1_ALIGNMENT
    // Memory allocation alignment.
    // Define BOOST_PROPERTY_TREE_RAPIDASN1_ALIGNMENT before including rapidxml.hpp if you want to override the default value, which is the size of pointer.
    // All memory allocations for nodes, attributes and strings will be aligned to this value.
    // This must be a power of 2 and at least 1, otherwise memory_pool will not work.
    #define BOOST_PROPERTY_TREE_RAPIDASN1_ALIGNMENT sizeof(void *)
#endif

namespace boost { namespace property_tree { namespace detail {namespace rapidasn1
{
    // Forward declarations
    template<class Byte> class asn1_node;
    
    //! Enumeration listing all node types produced by the parser.
    //! Use asn1_node::type() function to query node type.
    enum node_type
    {
        node_nongroup = 0x00,      //!< a non-group node.
        node_group    = 0x01,      //!< a group node. 
        node_integer,              //!< a interger data node.
        node_string,               //!< a string data node.
    };
    
    enum class_type
    {
        class_a = 0x00,
        class_b = 0x01,
        class_c = 0x10,
        class_d = 0x11,
    };

    //! \cond internal
    namespace internal
    {

        // Struct that contains lookup tables for the parser
        // It must be a template to allow correct linking (because it has static data members, which are defined in a header file).
        // template<int Dummy>
        // struct lookup_tables
        // {
            // static const unsigned char lookup_whitespace[256];              // Whitespace table
            // static const unsigned char lookup_node_name[256];               // Node name table
            // static const unsigned char lookup_text[256];                    // Text table
            // static const unsigned char lookup_text_pure_no_ws[256];         // Text table
            // static const unsigned char lookup_text_pure_with_ws[256];       // Text table
            // static const unsigned char lookup_attribute_name[256];          // Attribute name table
            // static const unsigned char lookup_attribute_data_1[256];        // Attribute data table with single quote
            // static const unsigned char lookup_attribute_data_1_pure[256];   // Attribute data table with single quote
            // static const unsigned char lookup_attribute_data_2[256];        // Attribute data table with double quotes
            // static const unsigned char lookup_attribute_data_2_pure[256];   // Attribute data table with double quotes
            // static const unsigned char lookup_digits[256];                  // Digits
            // static const unsigned char lookup_upcase[256];                  // To uppercase conversion table for ASCII characters
        // };        

        
    }
    //! \endcond

    ///////////////////////////////////////////////////////////////////////
    // Memory pool
    
    //! This class is used by the parser to create new nodes and attributes, without overheads of dynamic memory allocation.
    //! In most cases, you will not need to use this class directly. 
    //! However, if you need to create nodes manually or modify names/values of nodes, 
    //! you are encouraged to use memory_pool of relevant asn1_document to allocate the memory. 
    //! Not only is this faster than allocating them by using <code>new</code> operator, 
    //! but also their lifetime will be tied to the lifetime of document, 
    //! possibly simplyfing memory management. 
    //! <br><br>
    //! Call allocate_node() or allocate_attribute() functions to obtain new nodes or attributes from the pool. 
    //! You can also call allocate_string() function to allocate strings.
    //! Such strings can then be used as names or values of nodes without worrying about their lifetime.
    //! Note that there is no <code>free()</code> function -- all allocations are freed at once when clear() function is called, 
    //! or when the pool is destroyed.
    //! <br><br>
    //! It is also possible to create a standalone memory_pool, and use it 
    //! to allocate nodes, whose lifetime will not be tied to any document.
    //! <br><br>
    //! Pool maintains <code>BOOST_PROPERTY_TREE_RAPIDASN1_STATIC_POOL_SIZE</code> bytes of statically allocated memory. 
    //! Until static memory is exhausted, no dynamic memory allocations are done.
    //! When static memory is exhausted, pool allocates additional blocks of memory of size <code>BOOST_PROPERTY_TREE_RAPIDASN1_DYNAMIC_POOL_SIZE</code> each,
    //! by using global <code>new[]</code> and <code>delete[]</code> operators. 
    //! This behaviour can be changed by setting custom allocation routines. 
    //! Use set_allocator() function to set them.
    //! <br><br>
    //! Allocations for nodes, attributes and strings are aligned at <code>BOOST_PROPERTY_TREE_RAPIDASN1_ALIGNMENT</code> bytes.
    //! This value defaults to the size of pointer on target architecture.
    //! <br><br>
    //! To obtain absolutely top performance from the parser,
    //! it is important that all nodes are allocated from a single, contiguous block of memory.
    //! Otherwise, cache misses when jumping between two (or more) disjoint blocks of memory can slow down parsing quite considerably.
    //! If required, you can tweak <code>BOOST_PROPERTY_TREE_RAPIDASN1_STATIC_POOL_SIZE</code>, <code>BOOST_PROPERTY_TREE_RAPIDASN1_DYNAMIC_POOL_SIZE</code> and <code>BOOST_PROPERTY_TREE_RAPIDASN1_ALIGNMENT</code> 
    //! to obtain best wasted memory to performance compromise.
    //! To do it, define their values before rapidasn1.hpp file is included.
    //! \param Ch Character type of created nodes. 
    template<class Byte = unsigned char>
    class memory_pool
    {
        
    public:

        //! \cond internal
        // Prefixed names to work around weird MSVC lookup bug.
        typedef void *(boost_ptree_raw_alloc_func)(std::size_t);       // Type of user-defined function used to allocate memory
        typedef void (boost_ptree_raw_free_func)(void *);              // Type of user-defined function used to free memory
        //! \endcond
        
        //! Constructs empty pool with default allocator functions.
        memory_pool()
            : m_alloc_func(0)
            , m_free_func(0)
        {
            init();
        }

        //! Destroys pool and frees all the memory. 
        //! This causes memory occupied by nodes allocated by the pool to be freed.
        //! Nodes allocated from the pool are no longer valid.
        ~memory_pool()
        {
            clear();
        }

        //! Allocates a new node from the pool, and optionally assigns name and value to it. 
        //! If the allocation request cannot be accomodated, this function will throw <code>std::bad_alloc</code>.
        //! If exceptions are disabled by defining RAPIDXML_NO_EXCEPTIONS, this function
        //! will call rapidasn1::parse_error_handler() function.
        //! \param type Type of node to create.
        //! \param name Name to assign to the node, or 0 to assign no name.
        //! \param value Value to assign to the node, or 0 to assign no value.
        //! \param value_size Size of value to assign.
        //! \return Pointer to allocated node. This pointer will never be NULL.
        asn1_node<Byte> *allocate_node(node_type type, 
                                    const size_t tag = 0, 
                                    const Byte *value = 0, 
                                    std::size_t value_size = 0)
        {
            void *memory = allocate_aligned(sizeof(asn1_node<Byte>));
            asn1_node<Byte> *node = new(memory) asn1_node<Byte>(type);
            if (tag)
            {
                node->tag(tag);
            }
            if (value)
            {
                if (value_size > 0)
                    node->value(value, value_size);
                else
                    node->value(value);
            }
            return node;
        }
        
        //! Clones an asn1_node and its hierarchy of child nodes and attributes.
        //! Nodes and attributes are allocated from this memory pool.
        //! Names and values are not cloned, they are shared between the clone and the source.
        //! Result node can be optionally specified as a second parameter, 
        //! in which case its contents will be replaced with cloned source node.
        //! This is useful when you want to clone entire document.
        //! \param source Node to clone.
        //! \param result Node to put results in, or 0 to automatically allocate result node
        //! \return Pointer to cloned node. This pointer will never be NULL.
        asn1_node<Byte> *clone_node(const asn1_node<Byte> *source, asn1_node<Byte> *result = 0)
        {
            // Prepare result node
            if (result)
            {
                result->remove_all_nodes();
                result->type(source->type());
                result->tag(source->tag());
            }
            else
                result = allocate_node(source->type());

            // Clone name and value
            result->tag(source->tag());
            result->value(source->value(), source->value_size());

            // Clone child nodes
            for (asn1_node<Byte> *child = source->first_node(); child; child = child->next_sibling())
                result->append_node(clone_node(child));

            return result;
        }

        //! Clears the pool. 
        //! This causes memory occupied by nodes allocated by the pool to be freed.
        //! Any nodes or strings allocated from the pool will no longer be valid.
        void clear()
        {
            while (m_begin != m_static_memory)
            {
                char *previous_begin = reinterpret_cast<header *>(align(m_begin))->previous_begin;
                if (m_free_func)
                    m_free_func(m_begin);
                else
                    delete[] m_begin;
                m_begin = previous_begin;
            }
            init();
        }

        //! Sets or resets the user-defined memory allocation functions for the pool.
        //! This can only be called when no memory is allocated from the pool yet, otherwise results are undefined.
        //! Allocation function must not return invalid pointer on failure. It should either throw,
        //! stop the program, or use <code>longjmp()</code> function to pass control to other place of program. 
        //! If it returns invalid pointer, results are undefined.
        //! <br><br>
        //! User defined allocation functions must have the following forms:
        //! <br><code>
        //! <br>void *allocate(std::size_t size);
        //! <br>void free(void *pointer);
        //! </code><br>
        //! \param af Allocation function, or 0 to restore default function
        //! \param ff Free function, or 0 to restore default function
        void set_allocator(boost_ptree_raw_alloc_func *af, boost_ptree_raw_free_func *ff)
        {
            BOOST_ASSERT(m_begin == m_static_memory && m_ptr == align(m_begin));    // Verify that no memory is allocated yet
            m_alloc_func = af;
            m_free_func = ff;
        }

    private:

        struct header
        {
            char *previous_begin;
        };

        void init()
        {
            m_begin = m_static_memory;
            m_ptr = align(m_begin);
            m_end = m_static_memory + sizeof(m_static_memory);
        }
        
        char *align(char *ptr)
        {
            std::size_t alignment = ((BOOST_PROPERTY_TREE_RAPIDASN1_ALIGNMENT - (std::size_t(ptr) & (BOOST_PROPERTY_TREE_RAPIDASN1_ALIGNMENT - 1))) & (BOOST_PROPERTY_TREE_RAPIDASN1_ALIGNMENT - 1));
            return ptr + alignment;
        }
        
        char *allocate_raw(std::size_t size)
        {
            // Allocate
            void *memory;   
            if (m_alloc_func)   // Allocate memory using either user-specified allocation function or global operator new[]
            {
                memory = m_alloc_func(size);
                BOOST_ASSERT(memory); // Allocator is not allowed to return 0, on failure it must either throw, stop the program or use longjmp
            }
            else
            {
                memory = new char[size];
            }
            return static_cast<char *>(memory);
        }
        
        void *allocate_aligned(std::size_t size)
        {
            // Calculate aligned pointer
            char *result = align(m_ptr);

            // If not enough memory left in current pool, allocate a new pool
            if (result + size > m_end)
            {
                // Calculate required pool size (may be bigger than BOOST_PROPERTY_TREE_RAPIDASN1_DYNAMIC_POOL_SIZE)
                std::size_t pool_size = BOOST_PROPERTY_TREE_RAPIDASN1_DYNAMIC_POOL_SIZE;
                if (pool_size < size)
                    pool_size = size;
                
                // Allocate
                std::size_t alloc_size = sizeof(header) + (2 * BOOST_PROPERTY_TREE_RAPIDASN1_ALIGNMENT - 2) + pool_size;     // 2 alignments required in worst case: one for header, one for actual allocation
                char *raw_memory = allocate_raw(alloc_size);
                    
                // Setup new pool in allocated memory
                char *pool = align(raw_memory);
                header *new_header = reinterpret_cast<header *>(pool);
                new_header->previous_begin = m_begin;
                m_begin = raw_memory;
                m_ptr = pool + sizeof(header);
                m_end = raw_memory + alloc_size;

                // Calculate aligned pointer again using new pool
                result = align(m_ptr);
            }

            // Update pool and return aligned pointer
            m_ptr = result + size;
            return result;
        }

        char *m_begin;                                      // Start of raw memory making up current pool
        char *m_ptr;                                        // First free byte in current pool
        char *m_end;                                        // One past last available byte in current pool
        char m_static_memory[BOOST_PROPERTY_TREE_RAPIDASN1_STATIC_POOL_SIZE];    // Static raw memory
        boost_ptree_raw_alloc_func *m_alloc_func;           // Allocator function, or 0 if default is to be used
        boost_ptree_raw_free_func *m_free_func;             // Free function, or 0 if default is to be used
    };
    
    
    ///////////////////////////////////////////////////////////////////////////
    // ASN1 base

    //! Base class for xml_node and xml_attribute implementing common functions: 
    //! tag(), value(), value_size() and parent().
    //! \param Ch Character type to use
    template<class Byte = unsigned char>
    class asn1_base
    {
    public:
        
        ///////////////////////////////////////////////////////////////////////////
        // Construction & destruction
    
        // Construct a base with empty name, value and parent
        asn1_base()
            : m_tag(0)
            , m_value(0)
            , m_parent(0)
        {
        }

        ///////////////////////////////////////////////////////////////////////////
        // Node data access
    
        //! Gets tag of the node. 
        //! Interpretation of tag depends on type of node.
        //! <br><br>
        //! \return tag of node, or zero if node has no tag.
        std::size_t tag() const
        {
            return m_tag;
        }

        //! Gets value of node. 
        //! Interpretation of value depends on type of node.
        //! Note that value will not be zero-terminated if rapidxml::parse_no_string_terminators option was selected during parse.
        //! <br><br>
        //! Use value_size() function to determine length of the value.
        //! \return Value of node, or empty string if node has no value.
        Byte *value() const
        {
            return m_value ? m_value : nullstr();
        }

        //! Gets size of node value, not including terminator character.
        //! This function works correctly irrespective of whether value is or is not zero terminated.
        //! \return Size of node value, in characters.
        std::size_t value_size() const
        {
            return m_value_size;
        }

        ///////////////////////////////////////////////////////////////////////////
        // Node modification
    
        //! Sets tag of node to a unsigned integer.
        //! See \ref ownership_of_strings.
        //! <br><br>
        void tag(std::size_t tag)
        {
            m_tag = tag;
        }
        
        //! Sets size of node to a unsigned integer.
        //! See \ref ownership_of_strings.
        //! <br><br>
        void value_size(size_t size)
        {
            m_value_size = size;
        }

        //! Sets value of node to a non zero-terminated string.
        //! See \ref ownership_of_strings.
        //! <br><br>
        void value(const Byte *val, std::size_t size)
        {
            m_value = const_cast<Byte *>(val);
            m_value_size = size;
        }
        
        void value(const Byte *val)
        {
            m_value = const_cast<Byte *>(val);
        }

        ///////////////////////////////////////////////////////////////////////////
        // Related nodes access
    
        //! Gets node parent.
        //! \return Pointer to parent node, or 0 if there is no parent.
        asn1_node<Byte> *parent() const
        {
            return m_parent;
        }

    protected:

        // Return empty string
        static Byte *nullstr()
        {
            static Byte zero = Byte('\0');
            return &zero;
        }

        std::size_t m_tag;                 // Name of node, or 0 if no name
        Byte *m_value;                        // Value of node, or 0 if no value
        std::size_t m_value_size;           // Length of node value, or undefined if no value
        asn1_node<Byte> *m_parent;            // Pointer to parent node, or 0 if none

    };

    ///////////////////////////////////////////////////////////////////////////
    // ASN1 node

    //! Class representing a node of XML document. 
    //! Each node may have associated name and value strings, which are available through name() and value() functions. 
    //! Interpretation of name and value depends on type of the node.
    //! Type of node can be determined by using type() function.
    //! <br><br>
    //! Note that after parse, both name and value of node, if any, will point interior of source text used for parsing. 
    //! Thus, this text must persist in the memory for the lifetime of node.
    //! \param Ch Character type to use.
    template<class Byte = unsigned char>
    class asn1_node : public asn1_base<Byte>
    {
    public:
        ///////////////////////////////////////////////////////////////////////////
        // Construction & destruction
    
        //! Constructs an empty node with the specified type. 
        //! Consider using memory_pool of appropriate document to allocate nodes manually.
        //! \param t Type of node to construct.
        asn1_node(node_type t)
            : m_node_type(t)
            , m_node_class(class_b)
            , m_first_node(0)
        {
        }

        ///////////////////////////////////////////////////////////////////////////
        // Node data access
    
        //! Gets type of node.
        //! \return Type of node.
        node_type type() const
        {
            return m_node_type;
        }

        //! Gets class of node.
        //! \return Class of node
        class_type node_class() const
        {
            return m_node_class;
        }
        
        ///////////////////////////////////////////////////////////////////////////
        // Related nodes access
    
        //! Gets first child node, optionally matching node tag.
        //! \param tag The id of the asn1 node
        //! \return Pointer to found child, or 0 if not found.
        asn1_node<Byte> *first_node(std::size_t tag = 0) const
        {
            if (tag)
            {
                for (asn1_node<Byte> *child = m_first_node; child; child = child->next_sibling())
                    if (child->tag() == tag)
                        return child;
                return 0;
            }
            else
                return m_first_node;
        }

        //! Gets last child node, optionally matching node name. 
        //! Behaviour is undefined if node has no children.
        //! Use first_node() to test if node has children.
        //! \param n Name of child to find, or 0 to return last child regardless of its name; this string doesn't have to be zero-terminated if nsize is non-zero
        //! \param nsize Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found child, or 0 if not found.
        asn1_node<Byte> *last_node(std::size_t tag = 0) const
        {
            BOOST_ASSERT(m_first_node);  // Cannot query for last child if node has no children
            if (tag)
            {
                for (asn1_node<Byte> *child = m_last_node; child; child = child->previous_sibling())
                    if (child->tag() == tag)
                        return child;
                return 0;
            }
            else
                return m_last_node;
        }

        //! Gets previous sibling node, optionally matching node tag. 
        //! Behaviour is undefined if node has no parent.
        //! Use parent() to test if node has a parent.
        //! \param n tag of sibling to find, or 0 to return previous sibling regardless of its tag;
        //! \return Pointer to found sibling, or 0 if not found.
        asn1_node<Byte> *previous_sibling(std::size_t tag = 0) const
        {
            BOOST_ASSERT(this->m_parent);     // Cannot query for siblings if node has no parent
            if (tag)
            {
                for (asn1_node<Byte> *sibling = m_prev_sibling; sibling; sibling = sibling->m_prev_sibling)
                    if (sibling->tag() == tag)
                        return sibling;
                return 0;
            }
            else
                return m_prev_sibling;
        }

        //! Gets next sibling node, optionally matching node name. 
        //! Behaviour is undefined if node has no parent.
        //! Use parent() to test if node has a parent.
        //! \param n Name of sibling to find, or 0 to return next sibling regardless of its name; this string doesn't have to be zero-terminated if nsize is non-zero
        //! \return Pointer to found sibling, or 0 if not found.
        asn1_node<Byte> *next_sibling(std::size_t tag = 0) const
        {
            BOOST_ASSERT(this->m_parent);     // Cannot query for siblings if node has no parent
            if (tag)
            {
                for (asn1_node<Byte> *sibling = m_next_sibling; sibling; sibling = sibling->m_next_sibling)
                    if (sibling->tag() == tag)
                        return sibling;
                return 0;
            }
            else
                return m_next_sibling;
        }

        ///////////////////////////////////////////////////////////////////////////
        // Node modification
    
        //! Sets type of node.
        //! \param t Type of node to set.
        void type(node_type t)
        {
            m_node_type = t;
        }

        //! sets class of node.
        //! \return Class of node
        void node_class(class_type c)
        {
            m_node_class = c;
        }
        
        ///////////////////////////////////////////////////////////////////////////
        // Node manipulation

        //! Prepends a new child node.
        //! The prepended child becomes the first child, and all existing children are moved one position back.
        //! \param child Node to prepend.
        void prepend_node(asn1_node<Byte> *child)
        {
            BOOST_ASSERT(child && !child->parent());
            if (first_node())
            {
                child->m_next_sibling = m_first_node;
                m_first_node->m_prev_sibling = child;
            }
            else
            {
                child->m_next_sibling = 0;
                m_last_node = child;
            }
            m_first_node = child;
            child->m_parent = this;
            child->m_prev_sibling = 0;
        }

        //! Appends a new child node. 
        //! The appended child becomes the last child.
        //! \param child Node to append.
        void append_node(asn1_node<Byte> *child)
        {
            BOOST_ASSERT(child && !child->parent());
            if (first_node())
            {
                child->m_prev_sibling = m_last_node;
                m_last_node->m_next_sibling = child;
            }
            else
            {
                child->m_prev_sibling = 0;
                m_first_node = child;
            }
            m_last_node = child;
            child->m_parent = this;
            child->m_next_sibling = 0;
        }

        //! Inserts a new child node at specified place inside the node. 
        //! All children after and including the specified node are moved one position back.
        //! \param where Place where to insert the child, or 0 to insert at the back.
        //! \param child Node to insert.
        void insert_node(asn1_node<Byte> *where, asn1_node<Byte> *child)
        {
            BOOST_ASSERT(!where || where->parent() == this);
            BOOST_ASSERT(child && !child->parent());
            if (where == m_first_node)
                prepend_node(child);
            else if (where == 0)
                append_node(child);
            else
            {
                child->m_prev_sibling = where->m_prev_sibling;
                child->m_next_sibling = where;
                where->m_prev_sibling->m_next_sibling = child;
                where->m_prev_sibling = child;
                child->m_parent = this;
            }
        }

        //! Removes first child node. 
        //! If node has no children, behaviour is undefined.
        //! Use first_node() to test if node has children.
        void remove_first_node()
        {
            BOOST_ASSERT(first_node());
            asn1_node<Byte> *child = m_first_node;
            m_first_node = child->m_next_sibling;
            if (child->m_next_sibling)
                child->m_next_sibling->m_prev_sibling = 0;
            else
                m_last_node = 0;
            child->m_parent = 0;
        }

        //! Removes last child of the node. 
        //! If node has no children, behaviour is undefined.
        //! Use first_node() to test if node has children.
        void remove_last_node()
        {
            BOOST_ASSERT(first_node());
            asn1_node<Byte> *child = m_last_node;
            if (child->m_prev_sibling)
            {
                m_last_node = child->m_prev_sibling;
                child->m_prev_sibling->m_next_sibling = 0;
            }
            else
                m_first_node = 0;
            child->m_parent = 0;
        }

        //! Removes specified child from the node
        // \param where Pointer to child to be removed.
        void remove_node(asn1_node<Byte> *where)
        {
            BOOST_ASSERT(where && where->parent() == this);
            BOOST_ASSERT(first_node());
            if (where == m_first_node)
                remove_first_node();
            else if (where == m_last_node)
                remove_last_node();
            else
            {
                where->m_prev_sibling->m_next_sibling = where->m_next_sibling;
                where->m_next_sibling->m_prev_sibling = where->m_prev_sibling;
                where->m_parent = 0;
            }
        }

        //! Removes all child nodes (but not attributes).
        void remove_all_nodes()
        {
            for (asn1_node<Byte> *node = first_node(); node; node = node->m_next_sibling)
                node->m_parent = 0;
            m_first_node = 0;
        }
        
        template<int Flags>
        void print(size_t blank = 0)
        {
            char tmp[1024];
            if (this->type() == node_group)
            {
                snprintf(tmp, sizeof(tmp), "%stag:[%lu] len:[%lu] group:", std::string(blank, ' ').c_str(), this->tag(), this->value_size());
                std::cout << tmp;
            }
            else
            {
                if (this->value_size() > 256)
                {
                    snprintf(tmp, sizeof(tmp), "%stag:[%lu] len:[%lu] value:", std::string(blank, ' ').c_str(), this->tag(), this->value_size());
                }
                else
                {
                    snprintf(tmp, sizeof(tmp), "%stag:[%lu] len:[%lu] value:", std::string(blank, ' ').c_str(), this->tag(), this->value_size());
                }
                std::cout << tmp;
                for (size_t i=0; i<(this->value_size()>16?16:this->value_size()); i++)
                {
                    snprintf(tmp, sizeof(tmp), "%02X", (unsigned int)(*(this->value() + i)));
                    std::cout << tmp;
                }   
            }
            std::cout << std::endl;
            

            if (this->type() == node_group)
            {
                for (asn1_node<Byte> *node = first_node(); node; node = node->m_next_sibling)
                        node->print<Flags>(blank+2);
            }
        }
        
    private:

        ///////////////////////////////////////////////////////////////////////////
        // Restrictions

        // No copying
        asn1_node(const asn1_node &);
        void operator =(const asn1_node &);
    
        ///////////////////////////////////////////////////////////////////////////
        // Data members
    
        // Note that some of the pointers below have UNDEFINED values if certain other pointers are 0.
        // This is required for maximum performance, as it allows the parser to omit initialization of 
        // unneded/redundant values.
        //
        // The rules are as follows:
        // 1. first_node and first_attribute contain valid pointers, or 0 if node has no children/attributes respectively
        // 2. last_node and last_attribute are valid only if node has at least one child/attribute respectively, otherwise they contain garbage
        // 3. prev_sibling and next_sibling are valid only if node has a parent, otherwise they contain garbage

        node_type m_node_type;                   // Type of node; always valid
        class_type m_node_class;                 // Class of node; always 01
        asn1_node<Byte> *m_first_node;             // Pointer to first child node, or 0 if none; always valid
        asn1_node<Byte> *m_last_node;              // Pointer to last child node, or 0 if none; this value is only valid if m_first_node is non-zero
        asn1_node<Byte> *m_prev_sibling;           // Pointer to previous sibling of node, or 0 if none; this value is only valid if m_parent is non-zero
        asn1_node<Byte> *m_next_sibling;           // Pointer to next sibling of node, or 0 if none; this value is only valid if m_parent is non-zero
    };

    ///////////////////////////////////////////////////////////////////////////
    // XML document
    
    //! This class represents root of the DOM hierarchy. 
    //! It is also an asn1_node and a memory_pool through public inheritance.
    //! Use parse() function to build a DOM tree from a zero-terminated XML text string.
    //! parse() function allocates memory for nodes and attributes by using functions of asn1_document, 
    //! which are inherited from memory_pool.
    //! To access root node of the document, use the document itself, as if it was an asn1_node.
    //! \param Byte Date type to use.
    template<class Byte = unsigned char>
    class asn1_tree: public asn1_node<Byte>, public memory_pool<Byte>
    {
    
    public:

        //! Constructs empty asn1 tree
        asn1_tree()
            : asn1_node<Byte>(node_group)
        {
        }

        template<int Flags>
        void parse(const Byte *text, size_t size)
        {
            BOOST_ASSERT(text);
            // Remove current contents
            this->remove_all_nodes();
            this->value(text, size);
            
            // Parse children
            while (1)
            {
                if (size==0)
                    break;
                    
                std::size_t child_size = 0;
                asn1_node<Byte> *child_node = this->allocate_node(node_nongroup);
                if ( (child_size = this->parse_node<Flags>(text, size, child_node)) )
                {
                    this->append_node(child_node);
                }
                else
                {
                    BOOST_PROPERTY_TREE_RAPIDASN1_PARSE_ERROR("prase error: parse()", text-this->value());
                }
                text += child_size;
                size -= child_size;
            }

        }

        //! Clears the document by deleting all nodes and clearing the memory pool.
        //! All nodes owned by document pool are destroyed.
        void clear()
        {
            this->remove_all_nodes();
            memory_pool<Byte>::clear();
        }
        
//    private:

        ///////////////////////////////////////////////////////////////////////
        // Internal parsing functions
        
        template<int Flags>
        size_t parse_node(const Byte* text, size_t size, asn1_node<Byte> *node)
        {
            std::size_t pos = 0;
            // parse tag
            pos += parse_tag<Flags>(text+pos, size-pos, node);
            
            // parse len
            int is_varlen = 0;
            pos += parse_len<Flags>(text+pos, size-pos, node, is_varlen);
            
            // parse value
            if (node->type() == node_group)
            {
                // parse group
                const Byte* group_value = text + pos;
                size_t group_size = 0;
                while(1)
                {
                    // quit condition
                    if (is_varlen)
                    {
                        if (detect_end<Flags>(text + pos, size))
                        {
                            node->value(group_value, group_size);
                            return pos+2;
                        }
                    }
                    else
                    {
                        if (group_size == node->value_size())
                        {
                            node->value(group_value);
                            return pos;
                        }
                        else if (group_size > node->value_size())
                        {
                            BOOST_PROPERTY_TREE_RAPIDASN1_PARSE_ERROR("prase error: parse_node()", text-this->value() + pos);
                        }
                    }
                
                    // parse child node
                    std::size_t child_size = 0;
                    asn1_node<Byte> *child_node = this->allocate_node(node_nongroup);
                    if ( (child_size = parse_node<Flags>(text+pos, is_varlen?size:node->value_size(), child_node)) )
                    {
                        node->append_node(child_node);
                    }
                    else
                    {
                        BOOST_PROPERTY_TREE_RAPIDASN1_PARSE_ERROR("prase error: parse_node()", text-this->value() + pos);
                    }
                    pos += child_size;
                    group_size += child_size;
                }
            }
            else
            {
                // parse data
                if (is_varlen)
                    BOOST_PROPERTY_TREE_RAPIDASN1_PARSE_ERROR("prase error: parse_node()", text-this->value() + pos);
                
                node->value(text + pos);
                return pos + node->value_size();
            }
            
        }
        
        template<int Flags>
        size_t parse_tag(const Byte* text, size_t size, asn1_node<Byte> *node)
        {
            if (size)
            {
                node->node_class(static_cast<class_type>(((*text) & 0xC0) >> 6));
                node->type(static_cast<node_type>(((*text) & 0x20) >> 5));
                size_t tmp = (*text) & 0x1F;
                if (tmp > 30)
                {
                    tmp = 0;
                    size_t pos = 1;
                    while(pos<size)
                    {
                        Byte cur = *(text+pos);
                        Byte cur_val = cur & 0x7F;
                        tmp <<= 7;
                        tmp += cur_val;
                        if (!(cur&0x80))
                        {
                            node->tag(tmp);
                            return pos+1;
                        }
                        pos++;
                    }
                }
                else
                {
                    node->tag(tmp);
                    return 1;
                }
            }
            return 0;
        }
        
        template<int Flags>
        size_t parse_len(const Byte* text, size_t size, asn1_node<Byte> *node, int& is_varlen)
        {
            if (!size)
                BOOST_PROPERTY_TREE_RAPIDASN1_PARSE_ERROR("unexpect end: parse_len()", text-this->value());
            
            if ((*text) & 0x80)
            {
                std::size_t n = (*text) & 0x7F;
                if (n==0)
                {
                    is_varlen=1;
                    return 1;
                }
                else
                {
                    is_varlen = 0;
                }
                    
                if (n > 4)
                    BOOST_PROPERTY_TREE_RAPIDASN1_PARSE_ERROR("prase error: parse_len()", text-this->value());
                if (n+1 > size)
                    BOOST_PROPERTY_TREE_RAPIDASN1_PARSE_ERROR("prase error: parse_len()", text-this->value());
                std::size_t len = 0;
                for(size_t i=1; i<=n; i++)
                {
                    len <<= 8;
                    len |= (*(text+i));
                }
                node->value_size(len);
                return n+1;
            }
            else
            {
                std::size_t len = (*text) & 0x7F;
                is_varlen = 0;
                node->value_size(len);
                return 1;
            }
        }
        
        template<int Flags>
        bool detect_end(const Byte *text, size_t size)
        {
            if (size < 2)
            {
                BOOST_PROPERTY_TREE_RAPIDASN1_PARSE_ERROR("expected end: detect_end()", text-this->value());
            }
            else
            {
                if ((*text == 0) && (*(text+1) == 0))
                    return true;
                else
                    return false;
            }
        }
        
    };


}}}}

// Undefine internal macros
#undef BOOST_PROPERTY_TREE_RAPIDASN1_PARSE_ERROR

// On MSVC, restore warnings state
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#endif
