Sleek
=====

### A compiled language for the LEEK architecture

Instead of writting a C compiler for this architecture,
I decided to create my own langauge that fixes some of the things that I really don't like about C.

Variable Definitions
--------------------

The syntax for variable definitions is quite different to that of C like languages. The syntax is
```
    [variable name] : [type] = [initial value];
```
And as an example, the following declares a `uint` variable named `foo` and assigns it the value `42`
```
    foo : uint = 42;
```
Either the type, or the initial value may be ommited (but not both).
If the initial value is ommited, then the variable is declared but not initialised.
```
    foo : uint;
```
If the type is ommited, the type is infered, similar to a C++11 `auto`. In the case of an integer, `int` is chosen over `uint`.
```
    foo := 42; // int
```

Functions
---------

The syntax for a function in sleek is

```
    [return type] ( [argument list] ) { [function  body] }
```
If the function does not return a value, the return type is `void`. However this may be omited for brevity
This produces an annonymous function, you can assign it a name in the same way as a variable.
For example, the main function of a sleek program is
```
    main := int(args : string[]) {
        // Program body goes here
    }
```
Although defining just a type is the typical way to define parameters, you can also use a general variable definition to, for example, define default values.

Functions do have types, but they cannot be assigned at runtime. The type of a function is
```
    [return type]([argument types])
```
Which is important to know if you want to make function references.

One can also define functions that run at compile time by replacing parentheses with angle brackets.
Unlike regular functions, they may return the types `type`, `func` and other function types.
The `func` type is used when the argument and/or return types are to be determined at compile time.
This is useful for generic functions as in
```
    inc := func<T : type> {
        return (arg : &T) {
            arg += 1;
        }
    }

    foo := 5;
    inc<uint>(foo); // foo == 6
```

A useful pattern is to create an anonymous compile time function and immediatly call it to create a compile time block
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
```
    ref : &int = 21; // Error: cannot assign values to null
```
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

