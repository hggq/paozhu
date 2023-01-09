#ifndef JSON_HH
#define JSON_HH

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <array>
#include <list>
#include <unordered_map>
#include <iomanip>
namespace json 
{
    /*
    create from https://github.com/tunnuz/json
    */
    /** Possible JSON type of a value (array, object, bool, ...). */
    enum ValueType
    {
        INT,        // JSON's int
        FLOAT,      // JSON's float 3.14 12e-10
        BOOL,       // JSON's boolean (true, false)
        STRING,     // JSON's string " ... " or (not really JSON) ' ... '
        OBJECT,     // JSON's object { ... }
        ARRAY,      // JSON's array [ ... ]
        NIL         // JSON's null
    };

    // Forward declaration
    class Value;

    /** A JSON object, i.e., a container whose keys are strings, this
    is roughly equivalent to a Python dictionary, a PHP's associative
    array, a Perl or a C++ map (depending on the implementation). */
    class Object
    {
    public:

        /** Constructor. */
        Object();
    
        /** Copy constructor. 
            @param o object to copy from
        */
        Object(const Object& o);
    
        /** Move constructor. */
        Object(Object&& o);
        Object(std::initializer_list<std::list<std::pair<std::string,json::Value>>::value_type> nsl);
        /** Assignment operator. 
            @param o object to copy from
        */
        Object& operator=(const Object& o);
    
        /** Move operator. 
            @param o object to copy from
        */
        Object& operator=(Object&& o);
    
        /** Destructor. */
        ~Object();

        /** Subscript operator, access an element by key.
            @param key key of the object to access
        */
        Value& operator[] (const std::string& key);

        /** Subscript operator, access an element by key.
            @param key key of the object to access
        */
        const Value& operator[] (const std::string& key) const;

        /** Retrieves the starting iterator (const).
            @remark mainly for printing
        */
        std::list<std::pair<std::string,json::Value>>::const_iterator begin() const;

        /** Retrieves the ending iterator (const).
            @remark mainly for printing
        */
        std::list<std::pair<std::string,json::Value>>::const_iterator end() const;
    
        /** Retrieves the starting iterator */
        std::list<std::pair<std::string,json::Value>>::iterator begin();

        /** Retrieves the ending iterator */
        std::list<std::pair<std::string,json::Value>>::iterator end();

         /** Retrieves the starting iterator */
        std::map<std::string,std::list<std::pair<std::string,Value>>::iterator>::iterator order_begin();

        /** Retrieves the ending iterator */
        std::map<std::string,std::list<std::pair<std::string,Value>>::iterator>::iterator order_end();

        bool isset(std::string &key) const;
        bool isset(std::string &&key) const;

        bool unset(std::string &key);
        bool unset(std::string &&key);
        /** Inserts a field in the object.
            @param v pair <key, value> to insert
            @return an iterator to the inserted object
        */
        void insert(const std::pair<std::string, Value>& v);
        void insert(std::string,Value);
        /** Size of the object. */
        std::size_t size() const;

         /** check value type */
        bool isInt() const { return  false; }
        bool isFloat() const { return  false; }
        bool isNum() const { return  false;  }
        bool isString() const { return  false; }
        bool isArray() const { return  false; }
        bool isObject() const { return  true;  }
        bool isNull() const { return  false; }
        bool isBool() const { return  false;  }    
        void clear();
    protected:

        /** Inner container. */
        std::list<std::pair<std::string,Value>> _object;
        std::map<std::string,std::list<std::pair<std::string,Value>>::iterator> _mp;//哈希表    
    };

    /** A JSON array, i.e., an indexed container of elements. It contains
    JSON values, that can have any of the types in ValueType. */
    class Array
    {
    public:

        /** Constructor. */
        Array();
    
        /** Destructor. */
        ~Array();
    
        /** Copy constructor. 
            @param o the object to copy from
        */
        Array(const Array& a);
        Array(std::initializer_list<Value> nsl);
        Array(std::initializer_list<std::map<std::string, Value>::value_type> nsl);
        /** Assignment operator. 
            @param a array to copy from
        */
        Array& operator=(const Array& a);
    
        /** Copy constructor. 
            @param o the object to copy from
        */
        Array(Array&& a);

        /** Assignment operator. 
            @param a array to copy from
        */
        Array& operator=(Array&& a);

        /** Subscript operator, access an element by index. 
            @param i index of the element to access
        */
        Value& operator[] (size_t i);
        
        /** Subscript operator, access an element by index. 
            @param i index of the element to access
        */
        const Value& operator[] (size_t i) const;

        /** Retrieves the starting iterator (const).
            @remark mainly for printing
        */
        std::vector<Value>::const_iterator begin() const;

        /** Retrieves the ending iterator (const).
            @remark mainly for printing
        */
        std::vector<Value>::const_iterator end() const;

        /** Retrieves the starting iterator. */
        std::vector<Value>::iterator begin();

        /** Retrieves the ending iterator */
        std::vector<Value>::iterator end();

        /** Inserts an element in the array.
            @param n (a pointer to) the value to add
        */
        void push_back(const Value& n);
    
        /** Size of the array. */
        size_t size() const;

         /** check value type */
        bool isInt() const { return  false; }
        bool isFloat() const { return  false; }
        bool isNum() const { return  false;  }
        bool isString() const { return  false; }
        bool isArray() const { return  true; }
        bool isObject() const { return  false;  }
        bool isNull() const { return  false; }
        bool isBool() const { return  false;  }

        void clear();
    protected:

        /** Inner container. */
        std::vector<Value> _array;

    };

    /** A JSON value. Can have either type in ValueTypes. */
    class Value
    {
    public:
    
        /** Default constructor (type = NIL). */
        Value();
    
        /** Copy constructor. */
        Value(const Value& v);
    
        /** Constructor from int. */
        Value(const long long int i);
        Value(const unsigned int i);
        Value(const unsigned long long i);
        /** Constructor from int. */
        Value(const long int i);
    
        /** Constructor from int. */
        Value(const int i);
    
        /** Constructor from float. */
        Value(const long double f);
        
        /** Constructor from float. */
        Value(const double f);
        Value(const float f);
        /** Constructor from bool. */
        Value(const bool b);
    
        /** Constructor from pointer to char (C-string).  */
        Value(const char* s);

        /** Constructor from STD string  */
        Value(const std::string& s);
    
        /** Constructor from pointer to Object. */
        Value(const Object& o);
    
        /** Constructor from pointer to Array. */
        Value(const Array& a);
    
        /** Move constructor. */
        Value(Value&& v);
    
        /** Move constructor from STD string  */
        Value(std::string&& s);
    
        /** Move constructor from pointer to Object. */
        Value(Object&& o);
    
        /** Move constructor from pointer to Array. */
        Value(Array&& a);

        Value(std::initializer_list<std::string> nsl);
        
        /** Type query. */
        ValueType type() const
        {
            return type_t;
        }
    
        /** Subscript operator, access an element by key.
            @param key key of the object to access
        */
        Value& operator[] (const std::string& key);

        /** Subscript operator, access an element by key.
            @param key key of the object to access
        */
        const Value& operator[] (const std::string& key) const;
        
        /** Subscript operator, access an element by index. 
            @param i index of the element to access
        */
        Value& operator[] (size_t i);
    
        /** Subscript operator, access an element by index. 
            @param i index of the element to access
        */
        const Value& operator[] (size_t i) const;
    
        /** Assignment operator. */
        Value& operator=(const Value& v);
    
        /** Move operator. */
        Value& operator=(Value&& v);
        // Value& operator=(std::string&& v);
        // Value& operator=(long long && v);
        // Value& operator=(double && v);
        // Value& operator=(bool && v);
        /** Cast operator for float */
        explicit operator long double() const { return float_v; }
        explicit operator float() const { return float_v; }
        /** Cast operator for int */
        explicit operator long long int() const { return int_v; }
        explicit operator int() const { return int_v; }
        /** Cast operator for bool */
        explicit operator bool() const { return bool_v; }
    
        /** Cast operator for string */
        explicit operator std::string () const { return string_v; }
    
        /** Cast operator for Object */
        operator Object () const { return object_v; }
    
        /** Cast operator for Object */
        operator Array () const { return array_v; }

        /** Cast operator for Object */
         Object as_object() const { return object_v; }
    
        /** Cast operator for Object */
        Array as_array() const { return array_v; }
        
        /** Cast operator for float */
        double as_float() const {  return float_v; }
    
        /** Cast operator for int */
        long long int as_int() const { return int_v; }
    
        /** Cast operator for bool */
        bool as_bool() const { return bool_v; }
    
        /** Cast operator for string */
        std::string as_string() const { return string_v; }

        /** check value type */
        bool isInt() const { return  type_t == INT; }
        bool isFloat() const { return  type_t == FLOAT; }
        bool isNum() const { return  type_t == INT||type_t == FLOAT; }
        bool isString() const { return  type_t == STRING; }
        bool isArray() const { return  type_t == ARRAY; }
        bool isObject() const { return  type_t == OBJECT; }
        bool isNull() const { return  type_t == NIL; }
        bool isBool() const { return  type_t == BOOL; }

        Value& set_float()   {    type_t = FLOAT; return *this; }
        Value& set_int()   {    type_t = INT; return *this; }
        Value& set_string()   { type_t = STRING; return *this; }
        Value& set_array()   {  type_t = ARRAY; return *this; }
        Value& set_null()   {   type_t = NIL; return *this; }
        Value& set_bool()   {   type_t = BOOL; return *this; }
        void clear();
    protected:
    
        double         float_v;
        long long int       int_v;
        bool                bool_v;
        std::string         string_v;
    
        Object              object_v;
        Array               array_v;
    
        ValueType           type_t;
    };
     
    std::string JSON_UTF8_TO_ASCII(const std::string&);
    std::string JSON_STR(const std::string &,unsigned int &);
    std::string JSON_VALUE(const std::string &,unsigned int &);
    int JSON_OBJ(const std::string &,Object &,unsigned int);
    int JSON_ARRAY(const std::string &,Array &,unsigned int);

    std::string to_json(json::Value&);
    std::string to_json(json::Object&);
    std::string to_json(json::Array&);

    //Value decode(std::string&&);
    Value from_json(const std::string&);
   // Value decode(std::string&&,bool);
    Value from_json(const std::string&,bool);

    std::string load_json(const std::string&);
    bool save_json(const std::string&,const std::string&);

}

/** Output operator for Values */
std::ostream& operator<<(std::ostream&, const json::Value&);

/** Output operator for Objects */
std::ostream& operator<<(std::ostream&, const json::Object&);

/** Output operator for Arrays */
std::ostream& operator<<(std::ostream&, const json::Array&);

#endif