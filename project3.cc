#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#include "parser.h"

struct InstructionNode *parse_generate_intermediate_representation()
{
    Parser parser;
    InstructionNode *instructionList = parser.parse_program();

    // InstructionNode *trav = instructionList;

    // while (trav != NULL)
    // {
    //     if (trav->type == ASSIGN)
    //     {
    //         cout << trav->assign_inst.left_hand_side_index << " = " << trav->assign_inst.operand1_index << " ";
    //         if (trav->assign_inst.op != OPERATOR_NONE)
    //         {
    //             cout << trav->assign_inst.op << " ";
    //             cout << trav->assign_inst.operand2_index;
    //         }
    //         cout << ";" << endl;
    //     }
    //     else if (trav->type == IN)
    //     {
    //         cout << "input " << trav->input_inst.var_index << endl;
    //     }
    //     else if (trav->type == OUT)
    //     {
    //         cout << "output " << trav->output_inst.var_index << endl;
    //     }
    //     else if (trav->type == CJMP)
    //     {
    //         cout << "conditional jump" << trav->cjmp_inst.operand1_index << " " << trav->cjmp_inst.condition_op << " " << trav->cjmp_inst.operand2_index << endl;
    //     }
    //     else if (trav->type == NOOP)
    //     {
    //         cout << "noop" << endl;
    //     }

    //     trav = trav->next;
    // }

    return instructionList;
}