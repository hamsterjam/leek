Sleek Specification
===================

This document is intended to be a technical specification of the sleek language.
If you want a more human readable guide to using sleek, please check the documentation.

Context Free Grammar
--------------------

The following is the context free grammar for sleek in Backus-Naur form.
```
<program> ::= <block>

<block>     ::= <statement> | <statement> <block>
<statement> ::= <expression> ";"
              | "defer" <ws> <expression>
              | <line-definition> ";"
              | <block-definition>
              | <control-statement>
              | "{" <block> "}"

<expression> ::= <identifier>
               | <number>
               | <class-exp>
               | <function-exp>
               | "(" <expression> ")"
               | <expression> "[" <expression "]"
               | <expression> "(" <arg-list> ")"
               | <expression> "<" <arg-list> ">"
               | <unary-op> <expression>
               | <expression> <binary-op> <expression>

<definition> ::= <line-definition> | <block-definition>

<line-definition> ::= <identifier> ":" <expression> "=" <expression>
                    | <identifier> ":" <expression>
                    | <identifier> ":=" <expression>

<block-definition> ::= <identifier> ":=" <class-exp>
                     | <identifier> ":=" <function-exp>
                     | <identifier> "::" <function-exp>

<class-exp>       ::= "{" <class-block> "}"
<class-block>     ::= <class-statment> | <class-statement> <class-block>
<class-statement> ::= <visibility> <line-definition> ";" | <visibility> <block-definition>

<visibility> ::= ""
               | "[" <visibility-list> "]"

<visibility-list> ::= ""
                    | <visibility-specifier>
                    | <visibility-specifier> <visibility-list>

<visibility-specifier> ::= "R" | "W"

<function-exp> ::= <identifier> "(" <param-list> ")" <block>

<control-statement> ::= "for" "(" <expression> ";" <expression> ";" <expression> ")" <control-body>
                      | "while" "(" <expression> ")" <control-body>
                      | "do" <control-body> "while" "(" <expression> ")" ";"
                      | "if" "(" <expression> ")" <control-body>
                      | "elif" "(" <expression> ")" <control-body>
                      | "else" <control-body>

<control-body> ::= <expression> ";" | "{" <block> "}" | <control-statement>

<arg-list>     ::= <expression>
                 | <expression> "," <arg-list>
                 | "," <arg-list>

<param-list>   ::= <definition>
                 | <definition> "," <param-list>

<unary-op>  ::= "&"  | "-"  | "!"  | "const"

<binary-op> ::= "+"  | "-"  | "*"  | "/"  | "%"
              | "&"  | "|"  | "^"  | "<<" | ">>"
              | "&&" | "||" | "==" | "!="
              | "<=" | ">=" | "<"  | ">"
              | "="  | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^="

<raw-identifier> ::= "[a-zA-Z][a-zA-Z0-9_\-]*"
<identifier> ::= <raw-identifier>
               | <raw-identifier>"."<identifier>
               | "op"<unary-op>
               | "op"<binary-op>
               | "op()"
               | "op[]"

<number> ::= "0x[0-9a-fA-F]+"
           | "0b[01]+"
           | "0o[0-7]+"
           | "[0-9]+"
```
