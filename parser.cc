#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <string>
#include "lexer.h"
#include "compiler.h"
#include <iostream>
#include "parser.h"
using namespace std;

LexicalAnalyzer l;

// void expect(TokenType type) {
//     Token t = l.GetToken();
//     if ()
//     cout << "Syntax Error" << endl;
//     exit(0);
// }

Token peek() {
    Token t = l.GetToken();
    l.UngetToken(t);
    return t;
}

InstructionNode* parse_program() {
    new InstructionNode;
    parse_var_section();
    InstructionNode* firstInstr = parse_body();
    parse_inputs();
    return firstInstr;
}

void parse_var_section() {
    parse_id_list();
    l.GetToken();   // get semicolon
}

void parse_id_list() {
    Token t = l.GetToken();   // get ID
    location[t.lexeme] = next_available;
    mem[next_available] = 0;
    next_available++;

    Token t1 = peek();   // check for comma
    if (t1.token_type == COMMA) {
        l.GetToken();   // eat the comma
        parse_id_list();
    }
}

InstructionNode* parse_body() {
    l.GetToken();   // get LBRACE
    InstructionNode* in = parse_stmt_list();
    l.GetToken();    // get RBRACE
    return in;
}

InstructionNode* parse_stmt_list() {
    InstructionNode* instList1 = new InstructionNode;
    InstructionNode* instList2 = new InstructionNode;

    instList1 = parse_stmt();
    auto t = peek().token_type;
    if (t == ID || t == WHILE || t == IF || t == SWITCH || t == FOR || t == OUTPUT || t == INPUT) {
        instList2 = parse_stmt_list();
        // account for statements with bodies (if, while, for)
        // go to end of body, then append is2
        InstructionNode* dummy = instList1;
        while (dummy->next != nullptr) {
            dummy = dummy->next;
        }
        dummy->next = instList2;
    }
    return instList1;
}

InstructionNode* parse_stmt() {
    InstructionNode* in = new InstructionNode;
    auto t = peek().token_type;
    switch (t)
    {
    case ID:
        in = parse_assign_stmt();
        in->type = ASSIGN;
        break;
    case WHILE:
        in = parse_while_stmt();
        in->type = CJMP;
        break;
    case IF:
        in = parse_if_stmt();
        in->type = CJMP;
        break;
    case SWITCH:
        in = parse_switch_stmt();
        in->type = CJMP;
        break;
    case FOR:
        in = parse_for_stmt();
        in->type = ASSIGN;
        break;
    case OUTPUT:
        in = parse_output_stmt();
        in->type = OUT;
        break;
    case INPUT:
        in = parse_input_stmt();
        in->type = IN;
        break;
    default:
        break;
    }
    return in;
}

InstructionNode* parse_assign_stmt() {
    InstructionNode* inst = new InstructionNode;
    inst->type = ASSIGN;
    Token t = l.GetToken(); // get ID
    inst->assign_inst.left_hand_side_index = location[t.lexeme];
    l.GetToken();   // get EQUAL
    Token operand1 = parse_primary();    // get first operand
    if (operand1.token_type == ID) {
        inst->assign_inst.operand1_index = location[operand1.lexeme];
    }
    else if (operand1.token_type == NUM) {
        inst->assign_inst.operand1_index = next_available;
        mem[next_available] = stoi(operand1.lexeme);
        next_available++;
    }
    inst->assign_inst.op = OPERATOR_NONE;

    auto type = peek().token_type;
    if (type == PLUS || type == MINUS || type == MULT || type == DIV) {
        // if next token is an operator, that means it was an expression
        switch(parse_op()) {
            case PLUS:
                inst->assign_inst.op = OPERATOR_PLUS;
                break;
            case MINUS:
                inst->assign_inst.op = OPERATOR_MINUS;
                break;
            case MULT:
                inst->assign_inst.op = OPERATOR_MULT;
                break;
            case DIV:
                inst->assign_inst.op = OPERATOR_DIV;
                break;
        }
        Token operand2 = parse_primary();
        if (operand2.token_type == ID) {
            inst->assign_inst.operand2_index = location[operand2.lexeme];
        }
        else if (operand2.token_type == NUM) {
            inst->assign_inst.operand2_index = next_available;
            mem[next_available] = stoi(operand2.lexeme);
            next_available++;
        }
    }
    l.GetToken();   // get SEMICOLON
    return inst;
}

void parse_expr() {
    // unused
}

Token parse_primary() {
    Token t = l.GetToken(); // consume ID or NUM
    return t;
}

TokenType parse_op() {
    Token t = l.GetToken(); // consume PLUS, MINUS, MULT, DIV
    return t.token_type;
}

InstructionNode* parse_output_stmt() {
    l.GetToken();   // get OUTPUT
    InstructionNode* inst = new InstructionNode;
    inst->type = OUT;
    Token outputVar = l.GetToken();   // get ID
    inst->output_inst.var_index = location[outputVar.lexeme];
    l.GetToken();   // get SEMICOLON
    return inst;
}

InstructionNode* parse_input_stmt() {
    l.GetToken();   // get INPUT
    InstructionNode* inst = new InstructionNode;
    inst->type = IN;
    Token inputVar = l.GetToken();
    inst->input_inst.var_index = location[inputVar.lexeme];
    l.GetToken();   // get SEMICOLON
    return inst;
}

InstructionNode* parse_if_stmt() {
    l.GetToken();   // get IF token
    InstructionNode* inst = new InstructionNode;
    inst->type = CJMP;
    parse_condition(inst);
    inst->next = parse_body();

    InstructionNode* noopInst = new InstructionNode;
    noopInst->type = NOOP;
    InstructionNode* dummy = inst;
    // get to end of body of if stmt
    while (dummy->next != nullptr) {
        dummy = dummy->next;
    }
    dummy->next = noopInst;
    inst->cjmp_inst.target = noopInst;

    return inst;
}

InstructionNode* parse_while_stmt() {
    l.GetToken();   // get WHILE token
    InstructionNode* whileInst = new InstructionNode;
    whileInst->type = CJMP;
    parse_condition(whileInst);
    whileInst->next = parse_body();

    InstructionNode* jumpInstr = new InstructionNode;
    jumpInstr->type = JMP;
    jumpInstr->jmp_inst.target = whileInst;
    jumpInstr->next = nullptr;
    // go to end of body
    InstructionNode* dummy = whileInst;
    while (dummy->next != nullptr) dummy = dummy->next;
    dummy->next = jumpInstr;

    // set noop node, attach it to end of body after jmp instruction
    InstructionNode* noopInstr = new InstructionNode;
    noopInstr->type = NOOP;
    noopInstr->next = nullptr;
    jumpInstr->next = noopInstr;
    // set conditional jump of while stmt to noop node
    whileInst->cjmp_inst.target = noopInstr;
    return whileInst;
}

void parse_condition(InstructionNode* inst) {

    Token op1 = parse_primary();    // get first ID or NUM in condition
    // cout << "operand 1: " << op1.lexeme << endl;
    // cout << op1.token_type << endl;
    if (op1.token_type == ID) {
        inst->cjmp_inst.operand1_index = location[op1.lexeme];
    }
    if (op1.token_type == NUM) {
        inst->cjmp_inst.operand1_index = next_available;
        cout << op1.lexeme << endl;
        mem[next_available] = stoi(op1.lexeme);
        next_available++;
    }

    TokenType cond = parse_relop();
    switch (cond) {
        case GREATER:
            inst->cjmp_inst.condition_op = CONDITION_GREATER;
            break;
        case LESS:
            inst->cjmp_inst.condition_op = CONDITION_LESS;
            break;
        case NOTEQUAL:
            inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
            break;
    }

    Token op2 = parse_primary();
    // cout << "operand 2: " << op2.lexeme << endl;
    // cout << op2.token_type << endl;
    if (op2.token_type == ID) {
        inst->cjmp_inst.operand2_index = location[op2.lexeme];
    }
    if (op2.token_type == NUM) {
        inst->cjmp_inst.operand2_index = next_available;
        mem[next_available] = stoi(op2.lexeme);
        next_available++;
    }
}

TokenType parse_relop() {
    Token t = l.GetToken(); // get greater, less or notequal
    return t.token_type;
}

InstructionNode* parse_for_stmt() {
    l.GetToken();   // get FOR
    l.GetToken();   // get LPAREN

    InstructionNode* assign1 = new InstructionNode;
    InstructionNode* whilestmt = new InstructionNode;
    InstructionNode* assign2 = new InstructionNode;
	InstructionNode* noopstmt = new InstructionNode;
	InstructionNode* jumpstmt = new InstructionNode;
    assign1->type = ASSIGN;
    assign1->next = nullptr;
    assign1 = parse_assign_stmt();

    whilestmt->type = CJMP;
    whilestmt->next = nullptr;
    parse_condition(whilestmt);
    l.GetToken();   // get SEMICOLON

    assign2->type = ASSIGN;
	assign2->next = nullptr;
    assign2 = parse_assign_stmt();
    l.GetToken();   // get RPAREN

	assign1->next = whilestmt;
    whilestmt->next = parse_body();
	InstructionNode* dummy = whilestmt;
	while (dummy->next != nullptr) {
		dummy = dummy->next;
	}
	dummy->next = assign2;
	noopstmt->type = NOOP;
	noopstmt->next = nullptr;
	jumpstmt->type = JMP;
	jumpstmt->jmp_inst.target = whilestmt;
	assign2->next = jumpstmt;
	jumpstmt->next = noopstmt;
	whilestmt->cjmp_inst.target = noopstmt;

	return assign1;
}

InstructionNode* parse_switch_stmt() {
	l.GetToken();   // get SWITCH

    InstructionNode* switchInstr = new InstructionNode;
    switchInstr->type = CJMP;
    switchInstr->next = nullptr;

    InstructionNode* endLabel = new InstructionNode;
    endLabel->type = NOOP;
    endLabel->next = nullptr;
    Token var = l.GetToken();   // get ID
    l.GetToken();   // get LBRACE

    switchInstr = parse_case_list(var.lexeme, endLabel);
    // go to end of case list
    InstructionNode* dummy = switchInstr;
    while (dummy->next != nullptr) dummy = dummy->next;

    TokenType t = peek().token_type;
    if (t == DEFAULT) {
        InstructionNode* defCase = parse_default_case();
        InstructionNode* defaultDummy = defCase;
        while (defaultDummy->next != nullptr) {
            defaultDummy = defaultDummy->next;
        }
        dummy->next = defCase;
		defaultDummy->next = endLabel;
    } else {
        dummy->next = endLabel;
    }

    l.GetToken();   // get RBRACE

    // to do: set end of case list to noop label node
    // or if there is default case, attach that and then do noop label
    return switchInstr;
}

InstructionNode* parse_case_list(string var, InstructionNode* end) {
    InstructionNode* case1 = new InstructionNode;
    InstructionNode* caseList = new InstructionNode;

    case1 = parse_case(var, end);
    case1->next = nullptr;
    if (peek().token_type == CASE) {
        caseList = parse_case_list(var, end);
		InstructionNode* dummy = case1;
		while (dummy->next != nullptr) dummy = dummy->next;
        dummy->next = caseList;
    }
    return case1;
}

InstructionNode* parse_case(string var, InstructionNode* end) {
    InstructionNode* caseStmt = new InstructionNode;
	InstructionNode* noop = new InstructionNode;
	InstructionNode* jumpInstr = new InstructionNode;
    l.GetToken();   // get CASE

    caseStmt->type = CJMP;
	caseStmt->next = nullptr;
    caseStmt->cjmp_inst.operand1_index = location[var];
    caseStmt->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    Token equalto = l.GetToken();   // get NUM

    // set the case value
    caseStmt->cjmp_inst.operand2_index = next_available;
    mem[next_available] = stoi(equalto.lexeme);
    next_available++;
    l.GetToken();   // get COLON
	InstructionNode* body = parse_body();

    // set the target to the body, since the relational operator is !=
    // we want to go to the body if condition is true, aka if "notequal" evaluates to false
    caseStmt->cjmp_inst.target = body;
    jumpInstr->type = JMP;
    jumpInstr->next = nullptr;
	// if body is executed, set jump to end
	jumpInstr->jmp_inst.target = end;
    InstructionNode* dummy = body;
    // attach jump instr to end of body
    while (dummy->next != nullptr) {
        dummy = dummy->next;
    }
    dummy->next = jumpInstr;
	// noop->type = NOOP;
	// noop->next = nullptr;
	// caseStmt->next = noop;
	// cout << "case parsed, var: " << var << " case: " << mem[caseStmt->cjmp_inst.operand2_index] << endl;
    return caseStmt;
}

InstructionNode* parse_default_case() {
    InstructionNode* defCase = new InstructionNode;
	defCase->next = nullptr;
    l.GetToken();   // get default
    l.GetToken();   // get colon
    defCase = parse_body();
	// cout << "parsed default case" << endl;
    return defCase;
}

void parse_inputs() {
    parse_num_list();
}

void parse_num_list() {
    Token num = l.GetToken();   // get NUM
    inputs.push_back(stoi(num.lexeme));
    if (peek().token_type == NUM) {
        parse_num_list();
    }
}

InstructionNode* parse_generate_intermediate_representation() {
    InstructionNode* program = parse_program();
    return program;
}
