//LEX
#include "scanner.h"
using namespace Scanner;
//从输入源程序中读入一个字符并返回它
char Lexer::GetChar()
{
	int next_char = getc(file);   
	if(EOF == next_char)
		return -1;
	else
		return toupper(next_char);//统一返回大写 
}

//把当前读取的字符回退到输入文件流中，下次再读这个字符 
void Lexer::BackChar(char c)
{
	if(c == EOF)
		return;
	else   
		ungetc(c,file);
}

//将该字符加入到当前输入的字符串中
void Lexer::add_cur_str(char c)
{
	int length = strlen(cur_str);
	if(length + 1 >= sizeof(cur_str)) 
		return;
	cur_str[length] = c;//字符添加在末尾 
	cur_str[length+1] = '\0';//补齐终结符 
}


//判断该字符串是符号表中的哪一个，若不在则返回ERRTOKEN类型
struct Token Lexer::JudgeKeyToken(const char * c_str)
{	
	int count;
	struct Token err_token;
	int n = sizeof(TokenTab) / sizeof(TokenTab[0]);
	for(count=0; count<n; count++)
	{ 
		if(strcmp(TokenTab[count].lexeme, c_str)==0) 
			return TokenTab[count];
	}
	memset(&err_token, 0, sizeof(err_token));
	err_token.type = ERRTOKEN;
	err_token.lexeme = c_str;
	return err_token;
}

//识别并返回一个记号
struct Token Lexer::GetToken()
{	
	Token token;//定义用于返回的记号类型 
	int cur_char;//当前输入的字符 
	//清空缓冲区的值 
	memset(&token, 0, sizeof(Token));
	memset(cur_str, 0, MAX);
    //返回一个记号，需要得到该记号的类型、字符串、值和指针（其中函数指针系统自动分配空间）
	//因此只需得到类型、字符串和值这三个信息即可
	//其中数字类型需要额外知晓值是多少，其他类型只需知晓类型和他是什么字符串即可 
	
    //将缓冲区cur_str的地址传给token.lexeme，以便后面字符的填入 
	token.lexeme = cur_str;
	//跳过空白字符 
	for(;;)	
	{	
	    cur_char = GetChar() ;
	    if(cur_char == '\n') line++; 
		if(!isspace(cur_char)) break;
	}
	//文件结束了 
	if(cur_char == EOF)
	{	
	    token.type = NONTOKEN;
		return token;
	}
	
	//模拟DFA的作用
	//先将读入的第一个非空字符存入当前字符串中 
	add_cur_str(cur_char);
	
	//如果第一个字符是数字，则该记号一定是常量，若不是，则是错误记号 
	if(isdigit(cur_char))	
	{	
	    //不断读入字符，若是数字则代表该记号未读完，继续读；若不是数字则代表该记号已读完或者该常量的整数部分已读完，退出该循环 
	    for(;;)
		{	
		    cur_char = GetChar();
			if(isdigit(cur_char))	
				add_cur_str(cur_char);
			else			
				break;
		}
		//若是小数 
		if(cur_char == '.')
		{	
		    add_cur_str(cur_char);//将小数点放入当前字符串中 
			for(;;)
			{	
				cur_char = GetChar();
				if(isdigit(cur_char))	
					add_cur_str(cur_char);
				else			
					break;
			}
			
		} 
		BackChar(cur_char);//将不是该记号的字符回退到输入文件流中，下一次再读
		token.type = CONST_ID;
		token.value = atof(cur_str);
		return token;
	}
	
	//如果第一个字符是字母，则该记号一定是符号表中的内容，若不是，则是错误记号 
	else if(isalpha(cur_char))		
	{	
	    //不断读入字符，若是字母则代表该记号未读完，继续读；若不是字母则代表该记号已读完，退出该循环 
	    for(;;)
		{	
		    cur_char = GetChar();
			if(isalpha(cur_char))	
				add_cur_str(cur_char);
			else			
				break;
		}
		BackChar(cur_char);//将不是该记号的字符回退到输入文件流中，下一次再读 
		token = JudgeKeyToken(cur_str);//通过符号表判断该记号的类型 
		return token;
	}	
	
	//如果第一个字符不是字母和数字，则代表该记号一定是符号（分隔符和运算符），若不是，则是错误记号 
	else	
	{	
	    if(cur_char == '*')//若是*，则可能是乘法也可能是乘方 
	    {
	    	cur_char = GetChar();
			if(cur_char == '*')//乘方 
			{   
			    token.type = POWER;
				add_cur_str(cur_char);
			}
			else//乘法 
			{	
			    BackChar(cur_char);//将不是该记号的字符回退到输入文件流中，下一次再读	
			    token.type = MUL; 
			}
		}
		else if(cur_char == '/')//若是/，则可能是除法也可能是注释 
	    {
	    	cur_char = GetChar();
			if(cur_char =='/')//注释 
			{	
			    while(cur_char != '\n' && cur_char != EOF)//一直读到该注释的结尾 
			    {
			    	cur_char = GetChar();
				}
				BackChar(cur_char);//将不是该记号的字符回退到输入文件流中，下一次再读
				return GetToken();
			}
			else//除法 
			{	
			    BackChar(cur_char);//将不是该记号的字符回退到输入文件流中，下一次再读
				token.type = DIV;
			}
		}
		else if(cur_char == '-')//若是-，则可能是减法也可能是注释
	    {
	    	cur_char = GetChar();
			if(cur_char == '-')//注释 
			{	
			    while(cur_char != '\n' && cur_char != EOF)//一直读到该注释的结尾 
			    {
			    	cur_char = GetChar();
				}
				BackChar(cur_char);//将不是该记号的字符回退到输入文件流中，下一次再读
				return GetToken();
			}
			else//减法 
			{	
			    BackChar(cur_char);//将不是该记号的字符回退到输入文件流中，下一次再读
				token.type = MINUS;
			}
		}
	    else if(cur_char == ';')
	    	token.type = SEMICO;
	    else if(cur_char == '(')
	    	token.type = L_BRACKET;
	    else if(cur_char == ')')
	    	token.type = R_BRACKET;	
	    else if(cur_char == ',')
	    	token.type = COMMA;	
	    else if(cur_char == '+')
	    	token.type = PLUS;	
	    else 
	        token.type = ERRTOKEN; 
	        
	    return token;
	} 
} 

//初始化词法分析器，成功时返回非0，失败返回0 
int Lexer::InitScanner(const char *FileName)
{
	fopen_s(&file,FileName, "r");
	if(file != NULL)  
		return 1;
	else                  
		return 0;
}

//关闭词法分析器
void Lexer::CloseScanner()
{
	if(file != NULL)  
		fclose(file);
}

