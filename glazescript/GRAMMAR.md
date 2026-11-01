### =================================== ###

### TOP-LEVEL PROGRAM & DECLARATIONS    ###

### =================================== ###

program = { top_level_decl } ;

top_level_decl = struct_decl
| component_decl
| enum_decl
| system_decl
| function_decl ;

### Struct & Component Declarations ###

struct_decl = "struct" , IDENTIFIER , "{" , { field_decl } , "}" ;
component_decl = "component" , IDENTIFIER , "{" , { field_decl } , "}" ;
field_decl = type , IDENTIFIER , [ "=" , expression ] , [ ";" ] ;

### Enum Declaration ###

enum_decl = "enum" , IDENTIFIER , "{" , [ enum_member , { "," , enum_member } , [ "," ] ] , "}" ;
enum_member = IDENTIFIER , [ "=" , INT_LITERAL ] ;

### System Declaration ###

### Semantic Rule: Must contain at least one on_update_decl or on_event_decl ###

system_decl = "system" , IDENTIFIER , "{" , { system_member } , "}" ;
system_member = on_update_decl
| on_event_decl
| function_decl ;

on_update_decl = "on_update" , "(" , "float" , IDENTIFIER , ")" , block ;
on_event_decl = "on_event" , "(" , "event" , IDENTIFIER , ")" , block ;

### Standalone & System Helper Functions ###

function_decl = "fn" , return_type , IDENTIFIER , "(" , [ param_list ] , ")" , block ;
param_list = param , { "," , param } ;
param = type , IDENTIFIER ;

### =================================== ###

### TYPES                               ###

### =================================== ###

return_type = "void" | type ;

type = primitive_type
| math_type
| builtin_ecs_type
| list_type
| user_type ;

### Passed by value, except 'string' which is passed by reference in C++ ###

primitive_type = "uint8"  | "int8"
| "uint16" | "int16"
| "uint32" | "int32"
| "uint64" | "int64"
| "float"  | "double"
| "char"   | "string"
| "bool" ;

### Passed by reference in C++ ###

math_type = "vec2" | "vec3" | "vec4"
| "mat2" | "mat3" | "mat4"
| "quat" ;

builtin_ecs_type = "entity" | "event" ;

### Generic list container: maps to std::vector<T> ###

list_type = "list" , "<" , type , ">" ;

### Struct, Component, or Enum identifier: passed by reference in C++ ###

user_type = IDENTIFIER ;

### =================================== ###

### STATEMENTS & CONTROL FLOW           ###

### =================================== ###

block = "{" , { statement } , "}" ;

statement = var_decl_stmt
| if_stmt
| switch_stmt
| for_stmt
| while_stmt
| do_while_stmt
| return_stmt
| break_stmt
| continue_stmt
| expr_stmt
| block ;

### Variable Declarations: 'let' (immutable) & 'var' (mutable) ###

var_decl_stmt = (let_decl | var_decl ) , [ ";" ] ;
let_decl = "let" , type , IDENTIFIER , "=" , expression ;
var_decl = "var" , type , IDENTIFIER , [ "=" , expression ] ;

### Conditionals ###

if_stmt = "if" , "(" , expression , ")" , block ,
{ "else" , "if" , "(" , expression , ")" , block } ,
[ "else" , block ] ;

switch_stmt = "switch" , "(" , expression , ")" , "{" ,
{ switch_case } ,
[ switch_default ] ,
"}" ;
switch_case = "case" , expression , "->" , (block | statement ) ;
switch_default = "default" , "->" , (block | statement ) ;

### Loops ###

for_stmt = for_c_stmt
| for_range_stmt
| for_ecs_stmt ;

### Standard C-Style Loop ###

for_c_stmt = "for" , "(" , [ for_init ] , ";" , [ expression ] , ";" , [ expr_Or_assign ] , ")" , block ;
for_init = let_decl | var_decl | expr_Or_assign ;

### List Range Loop ###

for_range_stmt = "for" , "(" , type , IDENTIFIER , ":" , expression , ")" , block ;

### ECS View Query Loop (Valid only inside on_update and on_event) ###

for_ecs_stmt = "for" , "(" , [ "entity" , IDENTIFIER , "," ] ,
component_binding , { "," , component_binding } , ")" ,
[ "exclude" , IDENTIFIER , { "," , IDENTIFIER } ] ,
block ;
component_binding = IDENTIFIER , IDENTIFIER ; ### ComponentType varName ###

while_stmt = "while" , "(" , expression , ")" , block ;
do_while_stmt = "do" , block , "while" , "(" , expression , ")" , [ ";" ] ;

### Jump Statements ###

return_stmt = "return" , [ expression ] , [ ";" ] ;
break_stmt = "break" , [ ";" ] ;
continue_stmt = "continue" , [ ";" ] ;

### Expression / Assignment Statement ###

expr_stmt = expr_Or_assign , [ ";" ] ;
expr_Or_assign = expression , [ assign_op , expression ] ;
assign_op = "=" | "+=" | "-=" | "*=" | "/=" | "%=" ;

### ===================================== ###

### EXPRESSIONS & PRECEDENCE HIERARCHY    ###

### ===================================== ###

expression = logical_or_expr ;

logical_or_expr = logical_and_expr , { "||" , logical_and_expr } ;
logical_and_expr = equality_expr , { "&&" , equality_expr } ;
equality_expr = relational_expr , { ( "==" | "!=" ) , relational_expr } ;
relational_expr = additive_expr , { ( "<" | ">" | "<=" | ">=" ) , additive_expr } ;
additive_expr = multiplicative_expr , { ( "+" | "-" ) , multiplicative_expr } ;
multiplicative_expr= unary_expr , { ( "*" | "/" | "%" ) , unary_expr } ;

unary_expr = ( "!" | "-" | "++" | "--" ) , unary_expr
| postfix_expr ;

postfix_expr = primary_expr , { postfix_op } , [ "++" | "--" ] ;

postfix_op = "." , IDENTIFIER ### Member access: e.id, list.size, t.position ###
| "." , intrinsic_call ### Built-in entity / list methods ###
| "(" , [ arg_list ] , ")"               ### Standard function or constructor call ###
| "[" , expression , "]" ; ### Optional indexing ###

### Built-in Intrinsics for 'entity' and 'list<T>' ###

### Note: For entity.get/add/has/remove, the first arg in arg_list is a Component IDENTIFIER ###

intrinsic_call = ( "get" | "add" | "has" | "remove" | "remove_at" | "clear" ) ,
"(" , [ arg_list ] , ")" ;

arg_list = expression , { "," , expression } ;

primary_expr = LITERAL
| math_type , "(" , [ arg_list ] , ")"   ### e.g., vec3 (0.0, 1.0, 0.0) ###
| IDENTIFIER ### Variable, Function, Type, or Enum name ###
| IDENTIFIER , "::" , IDENTIFIER ### Enum value or Static System Call ###
| "(" , expression , ")" ;

### =================================== ###

### LEXICAL TOKENS                      ###

### =================================== ###

LITERAL = FLOAT_LITERAL
| INT_LITERAL
| BOOL_LITERAL
| CHAR_LITERAL
| STRING_LITERAL ;

BOOL_LITERAL = "true" | "false" ;
IDENTIFIER = LETTER , { LETTER | DIGIT | "_" } ;
INT_LITERAL = DIGIT , { DIGIT } ;
FLOAT_LITERAL = DIGIT , { DIGIT } , "." , DIGIT , { DIGIT } , [ "f" | "F" ] ;
CHAR_LITERAL = "'" , CHARACTER , "'" ;
STRING_LITERAL = '"' , { CHARACTER } , '"' ;

LETTER = "a".."z" | "A".."Z" | "_" ;
DIGIT = "0".."9" ;