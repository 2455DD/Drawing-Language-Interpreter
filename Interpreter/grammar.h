#pragma once
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include"scanner.h"

using namespace std;
using namespace  Scanner;
//语法分析器类中的类型定义
typedef double (*func_ptr) (double);  //定义参数为double类型的指针声明，服务于函数类别的记号

//语法树的节点结构体
typedef struct tree_node {
	enum Token_Type code;   //记号
	union {
		struct { tree_node *left_tree, *right_tree; }node_op; //节点记号为二元运算符，左右均有子树
		struct { tree_node* child_tree; func_ptr math_func_ptr; }node_func; //节点记号一侧子树为函数
		double node_const;   //节点为常数，无子树，绑定右值
		double* node_parameter;  //节点为函数，有参数，绑定左值
	}tree_content; //组合体，节点种类为其中之一
}*tree_node_ptr;  //指向结构体的指针类型


class grammar
{
protected:
	double  parameter;	// 参数T的存储空间
	Token	token;		// 记号
	tree_node_ptr 	start_ptr,	// 绘图起点表达式的语法树
					end_ptr, 	// 绘图终点表达式的语法树
					step_ptr,	// 步长表达式的语法树
					x_ptr,		// 点的横坐标表达式的语法树
					y_ptr,		// 点的横坐标表达式的语法树
					angle_ptr;	// 旋转角度表达式的语法树
	Lexer 	Lexer;	// 词法分析器对象

//辅助函数
	void fetch_token();						// 获取记号
	void match_token(enum Token_Type the_token);			// 匹配记号
	void syntax_error(int case_of);				// 指出语法错误(调用error_msg)
	
	void print_syntax_tree(tree_node* root, int indent);		// 打印语法树，int参数调整输出位置，对齐作用
	tree_node_ptr make_tree_node(enum Token_Type opcode, ...);	// 构造语法树
	virtual void error_msg(const char *descrip,const char *istring);	// 根据错误，进行输出

/*
非二义性文法，以此构造递归算法
program -> { statement semico }
statement -> originstatement | scalestatment | rotstatement | forstatement
originstatement -> origin is l_bracket expression comma r_bracket
scalestatement -> scale is l_bracket comma expression r_bracket
rotstatement -> rot is expression
forstatement -> for t from expression to expression step expression draw l_bracket expression comma r_bracket
*/
//主要产生式的递归子程序
	void program();
	void statement();
	virtual void for_statement();
	virtual void origin_statement();
	virtual void rot_statement();
	virtual void scale_statement();

/*
全部表达式的EBNF产生式，依据此进行下降递归构造语法树
expression -> term { ( plus | minus ) term }
term -> factor { ( mul | div ) factor }
factor -> ( mul | minus ) factor | component
component -> atom [ power component ]
atom -> const_id | T | func l_bracket expression r_bracket
                     | l_bracket expression r_bracket
*/
	tree_node_ptr expression();  
	tree_node_ptr term();        
	tree_node_ptr factor();      
	tree_node_ptr component();   
	tree_node_ptr atom();        

public:
	grammar() 				// 对象声明时需置初值
	{
		parameter = 0;
		start_ptr = NULL;
		end_ptr = NULL;
		step_ptr = NULL;
		x_ptr = NULL;
		y_ptr = NULL;
	}
	~grammar() {}		//析构
	void grammar_port(char* file_name);		// 语法分析器接口

private:
/*
	// 下述函数用于语法分析器的跟踪调试
	void enter(const char* x); //进入下一层递归
	void back(const char* x);  //返回上一层递归
	void call_match(const char* x); //匹配记号
	void tree_trace(tree_node_ptr x); //查看表达式的语法树，先序打印
*/
};


