#include <stdbool.h>

// ----------------------------------
// ENUMS
// ----------------------------------
typedef enum {
    NODE_SELECT_STATEMENT,
    NODE_INSERT_STATEMENT,
    NODE_UPDATE_STATEMENT,
    NODE_DELETE_STATEMENT,
    NODE_EXIT_STATEMENT,

    NODE_SELECT_LIST,
    NODE_TABLE_LIST,
    NODE_WHERE_CLAUSE,
    NODE_ORDER_BY_CLAUSE,
    NODE_ORDER_LIST,
    NODE_ORDER_ITEM,

    NODE_EXPRESSION,
    NODE_SIMPLE_EXPR,
    NODE_QUALIFIED_IDENTIFIER,
    NODE_LITERAL,
    NODE_FUNCTION_CALL,
    NODE_ARG_LIST,
    NODE_PAREN_EXPR,
    NODE_NULL_EXPR,
    NODE_ARITHMETIC_EXPR,

    NODE_COLUMN_LIST,
    NODE_VALUE_LIST,
    NODE_SET_LIST,
    NODE_SET_ITEM
} NodeType;

typedef enum {
    COMP_EQ,
    COMP_NEQ,
    COMP_LT,
    COMP_LTE,
    COMP_GT,
    COMP_GTE
} ComparisonOperator;

typedef enum { LOGIC_AND, LOGIC_OR } LogicalOperator;

typedef enum { ARITH_ADD, ARITH_SUB, ARITH_MUL, ARITH_DIV } ArithmeticOperator;

typedef enum { SORT_ASC, SORT_DESC } SortOrder;

// ----------------------------------
// Base
// ----------------------------------
typedef struct ASTNode {
    NodeType type;
} ASTNode;

// ----------------------------------
// Condition / Expression
// ----------------------------------

// Comparison portion of: simple_expr comparison_op simple_expr
typedef struct ExpressionNode {
    ASTNode base;
    struct SimpleExprNode *left;
    struct SimpleExprNode *right;
    ComparisonOperator op;
} ExpressionNode;

// Logical chaining: expression { logical_op expression }
typedef struct ConditionNode {
    ASTNode base;
    int exprCount;
    ExpressionNode **expressions;
    LogicalOperator *logicalOps;
} ConditionNode;

// ----------------------------------
// Simple Expressions
// ----------------------------------
typedef struct SimpleExprNode {
    ASTNode base;
    // Using a variant node — parser fills mutual exclusivity
    ASTNode *expr;
} SimpleExprNode;

typedef struct QualifiedIdentifierNode {
    ASTNode base;
    char *tableName; // NULL if not qualified
    char *columnName;
} QualifiedIdentifierNode;

typedef struct LiteralNode {
    ASTNode base;
    char *value; // numeric or string literal text
} LiteralNode;

typedef struct FunctionCallNode {
    ASTNode base;
    char *functionName;
    struct ArgListNode *args; // NULL if no args
} FunctionCallNode;

typedef struct ArgListNode {
    ASTNode base;
    int argCount;
    SimpleExprNode **args;
} ArgListNode;

typedef struct ParenExprNode {
    ASTNode base;
    SimpleExprNode *inner;
} ParenExprNode;

typedef struct ArithmeticExprNode {
    ASTNode base;
    SimpleExprNode *left;
    SimpleExprNode *right;
    ArithmeticOperator op;
} ArithmeticExprNode;

typedef struct NullExprNode {
    ASTNode base;
} NullExprNode;

// ----------------------------------
// SELECT Section
// ----------------------------------
typedef struct SelectListNode {
    ASTNode base;
    int count;
    SimpleExprNode **items;
    bool isStar; // supports SELECT *
} SelectListNode;

typedef struct TableListNode {
    ASTNode base;
    int count;
    char **tableNames;
} TableListNode;

typedef struct WhereClauseNode {
    ASTNode base;
    ConditionNode *condition;
} WhereClauseNode;

typedef struct OrderItemNode {
    ASTNode base;
    QualifiedIdentifierNode *identifier;
    SortOrder order;
    bool hasSortOrder;
} OrderItemNode;

typedef struct OrderListNode {
    ASTNode base;
    int count;
    OrderItemNode **items;
} OrderListNode;

typedef struct OrderByClauseNode {
    ASTNode base;
    OrderListNode *list;
} OrderByClauseNode;

// ----------------------------------
// STATEMENTS
// ----------------------------------
typedef struct SelectStatementNode {
    ASTNode base;
    SelectListNode *selectList;
    TableListNode *tableList;
    WhereClauseNode *whereClause;     // NULL if absent
    OrderByClauseNode *orderByClause; // NULL if absent
} SelectStatementNode;

// ----------------------------------
// INSERT
// ----------------------------------
typedef struct ColumnListNode {
    ASTNode base;
    int count;
    QualifiedIdentifierNode **columns;
} ColumnListNode;

typedef struct ValueListNode {
    ASTNode base;
    int count;
    SimpleExprNode **values;
} ValueListNode;

typedef struct InsertStatementNode {
    ASTNode base;
    char *tableName;
    ColumnListNode *columnList;
    ValueListNode *valueList;
} InsertStatementNode;

// ----------------------------------
// UPDATE
// ----------------------------------
typedef struct SetItemNode {
    ASTNode base;
    QualifiedIdentifierNode *column;
    SimpleExprNode *value;
} SetItemNode;

typedef struct SetListNode {
    ASTNode base;
    int count;
    SetItemNode **items;
} SetListNode;

typedef struct UpdateStatementNode {
    ASTNode base;
    char *tableName;
    SetListNode *setList;
    WhereClauseNode *whereClause; // NULL if absent
} UpdateStatementNode;

// ----------------------------------
// DELETE
// ----------------------------------
typedef struct DeleteStatementNode {
    ASTNode base;
    char *tableName;
    WhereClauseNode *whereClause; // NULL if absent
} DeleteStatementNode;

// ----------------------------------
// EXIT
// ----------------------------------
typedef struct ExitStatementNode {
    ASTNode base;
} ExitStatementNode;
