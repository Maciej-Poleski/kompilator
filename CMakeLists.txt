cmake_minimum_required(VERSION 3.5)
project(kompilator)

find_package(BISON REQUIRED 3)
find_package(FLEX REQUIRED 2.6)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader mc amdgpuinfo amdgpucodegen amdgpudesc amdgpuasmparser x86info x86codegen x86desc x86asmparser bpfinfo bpfcodegen bpfdesc nvptxinfo nvptxcodegen nvptxdesc)

set(CMAKE_CXX_STANDARD 14)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-Wall -Wextra)

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(SOURCE_FILES
        main.cpp
        src/Parser.cxx
        src/AST/Declarations.cxx
        src/AST/RecordDefinition.cxx
        src/AST/NamedType.cxx
        src/AST/AnnotatedType.cxx
        src/AST/PtrType.cxx
        src/AST/Ast.cxx
        src/AST/FunctionDefinition.cxx
        src/AST/Module.cxx
        src/AST/Type.cxx
        src/AST/FunctionDefinitionDeclarative.cxx
        src/AST/ReferenceTerm.cxx
        src/AST/Term.cxx
        src/AST/DefinitionTerm.cxx
        src/AST/FunctionTerm.cxx
        src/AST/FunctionDefinitionImperative.cxx
        src/AST/FunctionBodyDeclarative.cxx
        src/AST/FunctionBody.cxx
        src/AST/BooleanLiteralExpression.cxx
        src/AST/LiteralExpression.cxx
        src/AST/Expression.cxx
        src/AST/DecimalLiteralExpression.cxx
        src/AST/FunctionBodyImperative.cxx
        src/AST/ExpressionStatement.cxx
        src/AST/Statement.cxx
        src/AST/BreakStatement.cxx
        src/AST/ContinueStatement.cxx
        src/AST/IfStatement.cxx
        src/AST/ForStatement.cxx
        src/AST/FunctionBodyExtern.cxx
        src/AST/utils.cxx
        src/AST/NameReferenceExpression.cxx
        src/AST/TermDefinitionExpression.cxx
        src/AST/AtomicTermDefinitionExpression.cxx
        src/AST/FunctionTermExpression.cxx
        src/AST/OperatorExpression.cxx
        src/AST/UnaryOperatorExpression.cxx
        src/AST/BinaryOperatorExpression.cxx
        src/AST/UnifyExpression.cxx
        src/AST/AssignExpression.cxx
        src/AST/ExplicitCallExpression.cxx
        src/AST/QuoteExpression.cxx
        src/AST/OrExpression.cxx
        src/AST/AndExpression.cxx
        src/AST/EqualExpression.cxx
        src/AST/NotEqualExpression.cxx
        src/AST/LessExpression.cxx
        src/AST/GreaterExpression.cxx
        src/AST/LessOrEqualExpression.cxx
        src/AST/GreaterOrEqualExpression.cxx
        src/AST/AddExpression.cxx
        src/AST/SubtractExpression.cxx
        src/AST/MultiplyExpression.cxx
        src/AST/DivideExpression.cxx
        src/AST/ModuloExpression.cxx
        src/AST/NotExpression.cxx
        src/AST/NegateExpression.cxx
        src/AST/DereferenceExpression.cxx
        src/AST/AddressOfExpression.cxx
        src/AST/MemberExpression.cxx
        src/ASTVisitors/IAstVisitor.cxx
        src/ASTVisitors/AstVisitorBase.cxx
        src/Compiler.cxx
        src/ASTVisitors/BasicCorrectness.cxx
        src/ASTVisitors/ErrorCollector.cxx
        src/ASTVisitors/NameResolution.cxx
        src/ASTVisitors/QuoteCallResolution.cxx
        src/ASTVisitors/RecordDefinitionsCollector.cxx
        src/ASTVisitors/WithResult.cxx
        src/ASTVisitors/TypeChecking.cxx
        src/AST/BuiltinType.cxx
        src/AST/IntType.cxx
        src/AST/BoolType.cxx
        src/AST/VarType.cxx
        src/AST/DynamicType.cxx
        src/AST/FunctionType.cxx
        src/ASTVisitors/Codegen.cxx
        src/ASTVisitors/NameMangler.cxx)

FLEX_TARGET(lexer
        src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cxx
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lexer.hxx)

BISON_TARGET(parser
        src/parser.yxx ${CMAKE_CURRENT_BINARY_DIR}/parser.cxx
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.hxx
        VERBOSE COMPILE_FLAGS "--report=itemset")

ADD_FLEX_BISON_DEPENDENCY(lexer parser)

add_executable(
        kompilator
        ${SOURCE_FILES}
        ${FLEX_lexer_OUTPUTS}
        ${BISON_parser_OUTPUTS})

target_link_libraries(
        kompilator
        ${FLEX_LIBRARIES}
        ${llvm_libs})

add_library(
        runtime
        runtime/runtime.cxx)
