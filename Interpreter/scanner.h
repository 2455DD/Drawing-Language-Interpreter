//LEX
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#define MAX 1000
using namespace std;

namespace Scanner
{
	//函数绘图语言中记号的分类与表示 
	enum Token_Type					          //记号的类别 
	{	
		ORIGIN, SCALE, ROT, IS,               //保留字 
		TO,	STEP, DRAW,	FOR, FROM,			  //保留字
		T,                                    //参数
		SEMICO, L_BRACKET, R_BRACKET, COMMA,  //分隔符
		PLUS, MINUS, MUL, DIV, POWER,	      //运算符
		FUNC,						          //函数（调用） 
		CONST_ID,					          //常数
		NONTOKEN,					          //空记号（源程序结束） 
		ERRTOKEN					          //出错记号（非法输入） 
	};

	static const char *TOKEN_TYPE[] =  
	{	
		"ORIGIN", "SCALE", "ROT", "IS",                                                      //保留字 
		"TO", "STEP", "DRAW", "FOR", "FROM",                                                 //保留字
		"T",                                                                                 //参数
		"SEMICO", "L_BRACKET", "R_BRACKET", "COMMA",		                                 //分隔符
		"PLUS", "MINUS", "MUL", "DIV", "POWER",			                                     //运算符
		"FUNC",						                                                         //函数（调用）
		"CONST_ID",					                                                         //常数
		"NONTOKEN",					                                                         //空记号（源程序结束）
		"ERRTOKEN"                                                                           //出错记号（非法输入）
	};

	//记号的数据结构
	struct Token								
	{	
		enum Token_Type  	type;			  //类别
		const char		* lexeme;				  //原始输入的字符串，亦可为数组 
		double		value;					  //若记号是常数则存常数的值 
		double		(* FuncPtr)(double);	  //若记号是函数则存函数地址 
	}; 

	//区分记号的符号表
	static struct Token TokenTab[] =		
	{	
		{CONST_ID,	"PI",		3.1415926,	NULL},
		{CONST_ID,	"E",		2.71828,	NULL},
		{T,		    "T",		0.0,		NULL},
		{FUNC,		"SIN",		0.0,		sin},
		{FUNC,		"COS",		0.0,		cos},
		{FUNC,		"TAN",		0.0,		tan},
		{FUNC,		"LN",		0.0,		log},
		{FUNC,		"EXP",		0.0,		exp},
		{FUNC,		"SQRT",		0.0,		sqrt},
		{ORIGIN,	"ORIGIN",	0.0,		NULL},
		{SCALE,		"SCALE",	0.0,		NULL},
		{ROT,		"ROT",		0.0,		NULL},
		{IS,		"IS",		0.0,		NULL},
		{FOR,		"FOR",		0.0,		NULL},
		{FROM,		"FROM",		0.0,		NULL},
		{TO,		"TO",		0.0,		NULL},
		{STEP,		"STEP",		0.0,		NULL},
		{DRAW,		"DRAW",		0.0,		NULL}
	};

	//词法分析器的类构造 
	class Lexer 
	{		
	private:
		FILE *file;						                //定义输入的文件 
		char cur_str[MAX];			                    //当前输入的字符串 
	
	public:
		int line;                                       //行号定义（用于追踪报错的行） 
		Lexer()	{line = 1;}                              //构造函数 
		~Lexer() {}                                     //析构函数 
		int InitScanner(const char *FileName);	        //初始化词法分析器，成功时返回非0，失败返回0 
		struct Token GetToken();			            //识别并返回一个记号 
		void CloseScanner();			                //关闭词法分析器
		
		char GetChar();				                    //从输入源程序中读入一个字符并返回它 
		void BackChar(char c);			                //把当前读取的字符回退到输入文件流中，下次再读这个字符 
		struct Token JudgeKeyToken(const char * c_str);	//判断该字符串是符号表中的哪一个，若不在则返回ERRTOKEN类型 
		
		void add_cur_str(char c);		                //将该字符加入到当前输入的字符串中 
	};
	
}
