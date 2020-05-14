#include "parser.h"
#include "compiler.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <math.h>
#include <algorithm>

using namespace std;

vector<locationEntry> locationTable;

void Parser::syntax_error()
{
    cout << "SYNTAX ERROR !!!\n";
    exit(1);
}

int location(string variableName)
{
    for (int i = 0; i < locationTable.size(); i++)
    {
        if (locationTable[i].variableName == variableName)
        {
            return locationTable[i].memoryLocation;
        }
    }

    locationEntry *newLocationEntry = new locationEntry();
    newLocationEntry->memoryLocation = next_available;
    newLocationEntry->variableName = variableName;
    locationTable.push_back(*newLocationEntry);
    try
    {
        mem[newLocationEntry->memoryLocation] = stoi(variableName);
    }
    catch (exception &e)
    {
        mem[newLocationEntry->memoryLocation] = 0;
    }

    next_available++;

    return location(variableName);
}

InstructionNode *Parser::parse_program()
{
    // program → varsection body inputs

    parse_var_section();
    InstructionNode *instructionList = parse_body();
    parse_inputs();

    Token token = lexer.GetToken();
    if (token.token_type != END_OF_FILE)
    {
        syntax_error();
    }

    // Check the location table.
    // cout << "Memory:" << endl;
    // for (int i = 0; i < locationTable.size(); i++)
    // {
    //     cout << locationTable[i].memoryLocation << ": " << locationTable[i].variableName << " = " << mem[locationTable[i].memoryLocation] << endl;
    // }

    // cout << endl;

    // // Check the inputs table.
    // for (int i = 0; i < inputs.size(); i++)
    // {
    //     cout << inputs[i] << " ";
    // }
    // cout << endl
    //      << endl;

    return instructionList;
}

void Parser::parse_var_section()
{
    // var_section → idlist SEMICOLON

    parse_id_list();

    Token token = lexer.GetToken();
    // cout << "parse_var_section(): ";
    // token.Print();

    if (token.token_type != SEMICOLON)
    {
        syntax_error();
    }
}

void Parser::parse_id_list()
{
    // idlist → ID COMMA idlist
    // idList → ID

    Token token = lexer.GetToken();
    // cout << "parse_id_list(): ";
    // token.Print();

    if (token.token_type != ID)
    {
        syntax_error();
    }

    locationEntry *newLocationEntry = new locationEntry();
    newLocationEntry->memoryLocation = next_available;
    newLocationEntry->variableName = token.lexeme;
    locationTable.push_back(*newLocationEntry);
    mem[newLocationEntry->memoryLocation] = 0;
    next_available++;

    token = lexer.GetToken();
    if (token.token_type == COMMA)
    {
        parse_id_list();
    }
    else if (token.token_type == SEMICOLON)
    {
        lexer.UngetToken(token);
    }
    else
    {
        syntax_error();
    }
}

InstructionNode *Parser::parse_body()
{

    Token token = lexer.GetToken();
    // cout << "parse_body1(): ";
    // token.Print();

    if (token.token_type != LBRACE)
    {
        syntax_error();
    }

    InstructionNode *body = parse_stmt_list();

    token = lexer.GetToken();
    // cout << "parse_body2(): ";
    // token.Print();
    if (token.token_type != RBRACE)
    {
        syntax_error();
    }

    return body;
}

InstructionNode *Parser::parse_stmt_list()
{
    InstructionNode *statementList = parse_stmt();

    Token token = lexer.GetToken();

    if (token.token_type == RBRACE)
    {
        lexer.UngetToken(token);
    }
    else
    {
        lexer.UngetToken(token);

        InstructionNode *trav = statementList;
        while (trav->next != NULL)
        {
            trav = trav->next;
        }
        trav->next = parse_stmt_list();
    }

    return statementList;
}

InstructionNode *Parser::parse_stmt()
{
    Token token = lexer.GetToken();
    lexer.UngetToken(token);

    if (token.token_type == ID)
    {
        return parse_assign_stmt();
    }
    else if (token.token_type == WHILE)
    {
        return parse_while_stmt();
    }
    else if (token.token_type == IF)
    {
        return parse_if_stmt();
    }
    else if (token.token_type == SWITCH)
    {
        return parse_switch_stmt();
    }
    else if (token.token_type == FOR)
    {
        return parse_for_stmt();
    }
    else if (token.token_type == OUTPUT)
    {
        return parse_output_stmt();
    }
    else if (token.token_type == INPUT)
    {
        return parse_input_stmt();
    }
    else
    {
        syntax_error();
    }
}

InstructionNode *Parser::parse_assign_stmt()
{
    // assign_stmt → ID EQUAL primary SEMICOLON
    // assign_stmt → ID EQUAL expr SEMICOLON

    InstructionNode *currentInstructionNode = new InstructionNode;
    currentInstructionNode->type = ASSIGN;
    currentInstructionNode->next = NULL;

    Token token = lexer.GetToken();
    if (token.token_type != ID)
    {
        syntax_error();
    }

    currentInstructionNode->assign_inst.left_hand_side_index = location(token.lexeme);

    token = lexer.GetToken();
    if (token.token_type != EQUAL)
    {
        syntax_error();
    }

    token = lexer.GetToken();
    // cout << "parse_assign_stmt(): ";
    // token.Print();

    Token op = lexer.GetToken();
    lexer.UngetToken(op);
    lexer.UngetToken(token);

    // cout << "parse_assign_stmt() operator: ";
    // op.Print();

    if (op.token_type == PLUS || op.token_type == MINUS || op.token_type == MULT || op.token_type == DIV)
    {
        parse_expr(currentInstructionNode);
    }
    else
    {
        currentInstructionNode->assign_inst.operand1_index = parse_primary();
        currentInstructionNode->assign_inst.op = OPERATOR_NONE;
    }

    token = lexer.GetToken();
    if (token.token_type != SEMICOLON)
    {
        syntax_error();
    }

    return currentInstructionNode;
}

void Parser::parse_expr(InstructionNode *currentInstructionNode)
{
    currentInstructionNode->assign_inst.operand1_index = parse_primary();
    currentInstructionNode->assign_inst.op = parse_op();
    currentInstructionNode->assign_inst.operand2_index = parse_primary();
}

int Parser::parse_primary()
{
    Token token = lexer.GetToken();

    if (token.token_type == ID)
    {
        return location(token.lexeme);
    }
    else if (token.token_type == NUM)
    {
        return location(token.lexeme);
    }
    else
    {
        syntax_error();
    }

    return -1;
}

ArithmeticOperatorType Parser::parse_op()
{
    Token token = lexer.GetToken();

    if (token.token_type == PLUS)
    {
        return OPERATOR_PLUS;
    }
    else if (token.token_type == MINUS)
    {
        return OPERATOR_MINUS;
    }
    else if (token.token_type == MULT)
    {
        return OPERATOR_MULT;
    }
    else if (token.token_type == DIV)
    {
        return OPERATOR_DIV;
    }
    else
    {
        syntax_error();
    }
}

InstructionNode *Parser::parse_output_stmt()
{
    InstructionNode *currentInstructionNode = new InstructionNode;
    currentInstructionNode->type = OUT;
    currentInstructionNode->next = NULL;

    Token token = lexer.GetToken();
    if (token.token_type != OUTPUT)
    {
        syntax_error();
    }

    token = lexer.GetToken();
    if (token.token_type != ID)
    {
        syntax_error();
    }

    currentInstructionNode->output_inst.var_index = location(token.lexeme);

    token = lexer.GetToken();
    if (token.token_type != SEMICOLON)
    {
        syntax_error();
    }

    return currentInstructionNode;
}

InstructionNode *Parser::parse_input_stmt()
{
    InstructionNode *currentInstructionNode = new InstructionNode;
    currentInstructionNode->type = IN;
    currentInstructionNode->next = NULL;

    Token token = lexer.GetToken();
    if (token.token_type != INPUT)
    {
        syntax_error();
    }

    token = lexer.GetToken();
    if (token.token_type != ID)
    {
        syntax_error();
    }

    currentInstructionNode->input_inst.var_index = location(token.lexeme);

    token = lexer.GetToken();
    if (token.token_type != SEMICOLON)
    {
        syntax_error();
    }

    return currentInstructionNode;
}

InstructionNode *Parser::parse_while_stmt()
{
    InstructionNode *currentInstructionNode = new InstructionNode;
    currentInstructionNode->type = CJMP;
    currentInstructionNode->next = NULL;

    Token token = lexer.GetToken();
    if (token.token_type != WHILE)
    {
        syntax_error();
    }

    parse_condition(currentInstructionNode);

    currentInstructionNode->next = parse_body();

    InstructionNode *jumpInstruction = new InstructionNode;
    jumpInstruction->type = JMP;
    jumpInstruction->jmp_inst.target = currentInstructionNode;
    jumpInstruction->next = NULL;

    InstructionNode *trav = currentInstructionNode->next;
    while (trav->next != NULL)
    {
        trav = trav->next;
    }
    trav->next = jumpInstruction;

    InstructionNode *noopNode = new InstructionNode;
    noopNode->type = NOOP;
    noopNode->next = nullptr;

    jumpInstruction->next = noopNode;
    currentInstructionNode->cjmp_inst.target = noopNode;

    return currentInstructionNode;
}

InstructionNode *Parser::parse_if_stmt()
{
    InstructionNode *currentInstructionNode = new InstructionNode;
    currentInstructionNode->type = CJMP;
    currentInstructionNode->next = NULL;

    Token token = lexer.GetToken();
    if (token.token_type != IF)
    {
        syntax_error();
    }

    parse_condition(currentInstructionNode);

    currentInstructionNode->next = parse_body();

    InstructionNode *noopNode = new InstructionNode;
    noopNode->type = NOOP;
    noopNode->next = nullptr;

    InstructionNode *trav = currentInstructionNode->next;
    while (trav->next != NULL)
    {
        trav = trav->next;
    }
    trav->next = noopNode;
    currentInstructionNode->cjmp_inst.target = noopNode;

    return currentInstructionNode;
}

void Parser::parse_condition(InstructionNode *currentInstructionNode)
{
    currentInstructionNode->cjmp_inst.operand1_index = parse_primary();
    currentInstructionNode->cjmp_inst.condition_op = parse_relop();
    currentInstructionNode->cjmp_inst.operand2_index = parse_primary();
}

ConditionalOperatorType Parser::parse_relop()
{
    Token token = lexer.GetToken();
    if (token.token_type == GREATER)
    {
        return CONDITION_GREATER;
    }
    else if (token.token_type == LESS)
    {
        return CONDITION_LESS;
    }
    else if (token.token_type == NOTEQUAL)
    {
        return CONDITION_NOTEQUAL;
    }
    else
    {
        syntax_error();
    }
}

InstructionNode *Parser::parse_switch_stmt()
{
    Token token = lexer.GetToken();
    if (token.token_type != SWITCH)
    {
        syntax_error();
    }

    token = lexer.GetToken();
    if (token.token_type != ID)
    {
        syntax_error();
    }

    string id = token.lexeme;

    token = lexer.GetToken();
    if (token.token_type != LBRACE)
    {
        syntax_error();
    }

    InstructionNode *caseList = parse_case_list(id); // Returns case list with null body pointers

    token = lexer.GetToken();

    // Create common noop node
    InstructionNode *noopNode = new InstructionNode;
    noopNode->type = NOOP;
    noopNode->next = nullptr;

    // Iterate through case list, and change case pointers to noopNode.
    InstructionNode *trav = caseList;
    while (trav->next != NULL)
    {
        InstructionNode *caseTrav = trav->cjmp_inst.target;
        while (caseTrav->next != NULL)
        {
            caseTrav = caseTrav->next;
        }
        caseTrav->next = noopNode;

        trav = trav->next;
    }
    // Last case
    trav->next = noopNode;
    InstructionNode *caseTrav = trav->cjmp_inst.target;
    while (caseTrav->next != NULL)
    {
        caseTrav = caseTrav->next;
    }
    caseTrav->next = noopNode;

    if (token.token_type == DEFAULT)
    {
        lexer.UngetToken(token);

        InstructionNode *defaultCase = parse_default_case();

        InstructionNode *newTrav = defaultCase;
        while (newTrav->next != NULL)
        {
            newTrav = newTrav->next;
        }
        newTrav->next = noopNode;
        trav->next = defaultCase;

        token = lexer.GetToken();
        if (token.token_type != RBRACE)
        {
            syntax_error();
        }
    }
    else if (token.token_type != RBRACE)
    {
        syntax_error();
    }

    return caseList;
}

InstructionNode *Parser::parse_for_stmt()
{
    Token token = lexer.GetToken();
    if (token.token_type != FOR)
    {
        syntax_error();
    }

    token = lexer.GetToken();
    if (token.token_type != LPAREN)
    {
        syntax_error();
    }

    InstructionNode *assignmentNode = parse_assign_stmt();
    InstructionNode *currentInstructionNode = new InstructionNode;
    currentInstructionNode->type = CJMP;
    assignmentNode->next = currentInstructionNode;
    parse_condition(currentInstructionNode);

    token = lexer.GetToken();
    if (token.token_type != SEMICOLON)
    {
        syntax_error();
    }

    InstructionNode *assignmentStatement = parse_assign_stmt();

    token = lexer.GetToken();
    if (token.token_type != RPAREN)
    {
        syntax_error();
    }

    currentInstructionNode->next = parse_body();

    InstructionNode *trav = currentInstructionNode->next;
    while (trav->next != NULL)
    {
        trav = trav->next;
    }

    InstructionNode *jumpNode = new InstructionNode;
    jumpNode->type = JMP;
    jumpNode->jmp_inst.target = currentInstructionNode;

    trav->next = assignmentStatement;
    trav->next->next = jumpNode;

    InstructionNode *noopNode = new InstructionNode;
    noopNode->type = NOOP;
    noopNode->next = nullptr;
    jumpNode->next = noopNode;
    currentInstructionNode->cjmp_inst.target = noopNode;

    return assignmentNode;
}

InstructionNode *Parser::parse_case_list(string id)
{
    InstructionNode *currentInstructionNode = new InstructionNode;
    currentInstructionNode->type = CJMP;
    currentInstructionNode->next = NULL;
    currentInstructionNode->cjmp_inst.operand1_index = location(id);
    currentInstructionNode->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

    parse_case(currentInstructionNode); // Sets up body and operand2

    Token token = lexer.GetToken();
    lexer.UngetToken(token);

    if (token.token_type == CASE)
    {
        currentInstructionNode->next = parse_case_list(id);
    }

    return currentInstructionNode;
}

void Parser::parse_case(InstructionNode *currentInstructionNode)
{
    Token token = lexer.GetToken();
    if (token.token_type != CASE)
    {
        syntax_error();
    }

    token = lexer.GetToken();
    if (token.token_type != NUM)
    {
        syntax_error();
    }

    currentInstructionNode->cjmp_inst.operand2_index = location(token.lexeme);

    token = lexer.GetToken();
    if (token.token_type != COLON)
    {
        syntax_error();
    }

    currentInstructionNode->cjmp_inst.target = parse_body();
}

InstructionNode *Parser::parse_default_case()
{
    Token token = lexer.GetToken();
    if (token.token_type != DEFAULT)
    {
        syntax_error();
    }

    token = lexer.GetToken();
    if (token.token_type != COLON)
    {
        syntax_error();
    }

    return parse_body();
}

void Parser::parse_inputs()
{
    parse_num_list();
}

void Parser::parse_num_list()
{
    Token token = lexer.GetToken();
    if (token.token_type != NUM)
    {
        syntax_error();
    }

    int input = stoi(token.lexeme);
    inputs.push_back(input);

    token = lexer.GetToken();
    lexer.UngetToken(token);

    if (token.token_type == NUM)
    {
        parse_num_list();
    }
}