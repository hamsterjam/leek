Sleek
=====

### A compiled language for the LEEK architecture

Instead of writting a C compiler for this architecture,
I decided to create my own langauge that fixes some of the things that I really don't like about C.

Variable Definitions
--------------------

The syntax for variable definitions is quite different to that of C like languages. The syntax is

`[variable name] : [type] = [initial value];`

And as an example, the following declares a `uint` variable named `foo` and assigns it the value `42`

`foo : uint = 42;`

Either the type, or the initial value may be ommited (but not both).
If the initial value is ommited, then the variable is declared but not initialised.

`foo : uint;`

If the type is ommited, the type is infered, similar to a C++11 `auto`. In the case of an integer, `int` is chosen over `uint`.

`foo := 42; // int`


Functions
---------

Functions can be defined with the syntax

`[return type] ( [arg 1], [arg 2], ... [arg n] )`

Where `arg n` is a variable definition.
Functions will typically assigned a name using the function overload operator `::`, although, they may be defined as a variable.
For example, the signature for the entry point of a sleek program is
```
main :: int(args : string[]) {
    return 0;
}
```
The arguments are typically defined as above with just a type. However you may also include default values as in the following convoluted example
```
incOrAdd :: int(lhs : int = 1, rhs : int = 1) {
    return lhs + rhs;
}

incOrAdd(2, 3); // 5
incOrAdd(2);    // 3
incOrAdd(, 3);  // 4
```
As the name of the operator implies, functions declared with the function overload operator may be overloaded (not true of functions declared as variables).
The sole exception to this rule is the function `main` which must be declared with the above signature.
Functions declared as variables have type that is impliclty `const`

`[return type]([argtype 1], [argtype 2], ..., [argtype n])`

For instance, the type of the above `incOrAdd` function is `int(int, int)`.
As these types are implicitly `const`, they may not be assigned at run time.
If you need to assign functions at run time, consider using function references.
The use of function variables is in their use with compile time functions.

A function may be declared to run at compile time by replacing the parentheses `()` with angle brackets `<>`.
The return type and arguments of these functions are implicitly `const` and they may not reference any variables out of their scope that are not `const`.
However, the upside is they may (must) return a `const` value including functions and types.
This can be used to define generics
```
add :: func<T : type> {
    return T(lhs : T, rhs : T) {
        return lhs + rhs;
    }
}
```
The type `func` is a special type that can be considered a parent type of all function types.
This is nessecary as the signature of generic functions is... generic.
Note that this lets you do full on functional programming (at compile time).
```
swapArgs :: func<retT : type, lhsT : type, rhsT : type> {
    return func<f : retT(lhsT, rhsT)> {
        return retT(lhs : rhsT, rhs : lhsT) {
            return f(rhs, lhs);
        }
    }
}

pow :: int(lhs : int, rhs : int) {
    ret := 1;
    for (i := 0; i < rhs; ++i) ret *= lhs;
    return ret;
}

swapArgs<int, int, int><pow>(2, 3); // 9
```
Another useful pattern is to declare and immediatly call an anonymous compile time function to create blocks that will run at compile time.
```
<>{
    // Code here runs at compile time
}<>
```

References
----------

Sleek does not use pointers, instead, explicitly declared, reasignable references are used.
The `&` operator should always be read as "reference to".
When applied to a type it signifies a reference type. For example `&int` is a reference to an `int`.
When applied to a variable, it produces a reference to that variable.
The semantics for assigning a variable of reference type is a little complicated.
If the variable is assigned a value of the type it is referencing, it will modify the value of the variable.
If the variable is assigned a value of the appropriate reference type, it will assign the reference to the variable.
```
foo : int = 1;
bar : int = 2;
ref := &foo; // ref has type &int and value &foo
ref = 21;    // foo now has value 21
ref = &bar;  // ref now refers to bar
ref = 22;    // bar now has value 22
```
Notably, the default value of a reference variable is a special null reference that cannot be assigned a value so the following is an error.

`ref : &int = 21; // Error: cannot assign values to null`

However, you may *initalise* a reference variable without using the `&` operator. This is useful for passing values by reference to a function
```
inc := (val : &int) {
    val += 1;
}

foo : int = 2;
inc(foo);   // foo now holds the value 3
inc(2);     // Error
```
Higher order references can also be assigned recursively, for example
```
var1   : int;
var2   : int;
ref1   := &var1;
ref2   := &var2;
refRef : &&int;

refRef = &ref1; // Assigned type &&int so refRef now references ref1

refRef = ref2;  // Assigned a referenced type, so the value referenced is assigned the value ref2
// ref1 = ref2; // This is equivalent.

refRef = 20;    // Assigned a referenced type, so the value referenced is assigned the value 20
// ref1 = 20;   // These are equivalent
// var2 = 20;
```
Function references also work as one would expect.
```
inc := int(val : int) {
    return int + 1;
}

funcRef := &inc;

two := funcRef(1);
```
Special care should be taken with the types of function references.
For example, the type `&int(int)` is a function that takes a single `int` as an argument and returns an `&int`.
In contrast, the type `&(int(int))` is a reference to a function that takes a single `int` as an argument and returns an `int`.

Classes
-------

Data structures in sleek are known as classes.
Although they are called classes, by default they behave more like a C struct.
However, you may define constructors, destructors and classes may have members which are functions.
To define a class, use the `class` keyword to define a new type.
You can assign this type to a variable in the usual manner.
```
Point := class {
    x : int;
    y : int;
}

vec : Point;
vec.x = 5;
vec.y = 2;
```
To define a constructor, overload the `this` keyword with a function inside the class body.
To define a destructor, do the same with `~this`.
Class constructors and destructors may return values, but this is usually discarded.
To construct an object, use the new keyword. Note that unlike in C++, this *staticaly* allocates an object.

There is a `this` keyword which refers to the object itself. More specifically,
if a function member is called, the `this` keyword is a reference to the object that called the function as you would expect.
If a constructor is called, the `this` keyword is a reference to the object to be created.
If a destructor is called, the `this keyword is a reference to the object to be destroyed.

Note that by default class members are public. If you want to declare a member private you may prepend the `private` keyword onto the definition.
A rather pointless example follows
```
Point := class {
    this :: (x : int, y : int) {
        this.x = x;
        this.y = y;
    }

    sqrLength :: int() {
        return x*x + y*y;
    }

    private x : int;
    private y : int;
}

vec := new Point(3, 4);
vec.sqrLength(); // 25
vec.x = 5;       // Error, x is private
```

Operators can be overloaded in sleek. The names of the operator functions are `op` with the operator appended to it.
For example, to make a functor we can overload the `()` operator.
```
Adder := class {
    this :: (val : int) {
        this.val = val;
    }

    op() :: int(lhs : int) {
        return lhs + val;
    }

    private val : int;
}

add5 := new Adder(5);

add5(3); // 8
```
Generic clases can be created by simply returning a `type` from a compile time function.
A simple vector class follows as an example
```
Vector := type<T : type> {
    return class {
        this :: (x : T, y : T) {
            this.x = x;
            this.y = y;
        }

        op+= :: (rhs : &T) {
            this.x := this.x + rhs.x;
            this.y := this.y + rhs.y;
        }
            
        private x : T;
        private y : T;
    }
}

p := new Vector<int>(1, 2);
q := new Vector<int>(3, 4);

p += q; // p contains a Vector<int> with x = 4, y = 6
```
