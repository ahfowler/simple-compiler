#ifndef __PARSER_H__
#define __PARSER_H__

/* parser.h: This file contains all struct defintions pertaining to rule list. */

#include <string>
#include "lexer.h"
#include "compiler.h"

using namespace std;

struct locationEntry
{
    string variableName;
    int memoryLocation;
};

class Parser
{
public:
    InstructionNode *parse_program();
    void parse_var_section();
    void parse_id_list();
    InstructionNode *parse_body();
    InstructionNode *parse_stmt_list();
    InstructionNode *parse_stmt();
    InstructionNode *parse_assign_stmt();
    void parse_expr(InstructionNode *currentInstructionNode);
    int parse_primary();
    ArithmeticOperatorType parse_op();
    InstructionNode *parse_output_stmt();
    InstructionNode *parse_input_stmt();
    InstructionNode *parse_while_stmt();
    InstructionNode *parse_if_stmt();
    void parse_condition(InstructionNode *currentInstructionNode);
    ConditionalOperatorType parse_relop();
    InstructionNode *parse_switch_stmt();
    InstructionNode *parse_for_stmt();
    InstructionNode *parse_case_list(string id);
    void parse_case(InstructionNode *currentInstructionNode);
    InstructionNode *parse_default_case();
    void parse_inputs();
    void parse_num_list();

private:
    LexicalAnalyzer lexer;
    void syntax_error();
    Token expect(TokenType expected_type);
    Token peek();
};

#endif