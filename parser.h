#ifndef _PARSER_H
#define _PARSER_H
#include "lexer.h"
#include <string>
#include <map>

std::map<std::string, int> location;
InstructionNode* parse_program();
void parse_var_section();
InstructionNode* parse_body();
void parse_inputs();
void parse_id_list();
InstructionNode* parse_stmt_list();
InstructionNode* parse_stmt();
InstructionNode* parse_assign_stmt();
void parse_expr();
Token parse_primary();
TokenType parse_op();
InstructionNode* parse_output_stmt();
InstructionNode* parse_input_stmt();
InstructionNode* parse_while_stmt();
InstructionNode* parse_if_stmt();
InstructionNode* parse_for_stmt();
InstructionNode* parse_switch_stmt();
void parse_condition(InstructionNode* inst);
TokenType parse_relop();
InstructionNode* parse_for_stmt();
InstructionNode* parse_case(std::string var, InstructionNode* end);
InstructionNode* parse_case_list(std::string var, InstructionNode* end);
InstructionNode* parse_default_case();
void parse_inputs();
void parse_num_list();

#endif /* _PARSER_H_ */
