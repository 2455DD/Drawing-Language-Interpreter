#define _CRT_SECURE_NO_WARNINGS

#include "grammar.h"
using namespace Scanner;
// 构造语法树，仅一个节点
tree_node_ptr grammar::make_tree_node(enum Token_Type opcode, ...) {
	tree_node_ptr t_ptr = new (struct tree_node);
	t_ptr->code = opcode;   //接收记号类别
	va_list arg_ptr;        //解决变参问题
	va_start(arg_ptr, opcode); //指向opcode之后的那个参数的地址
	switch (opcode) {        //根据记号的类别构造不同的节点
		case CONST_ID:	//常数节点
			t_ptr->tree_content.node_const = (double)va_arg(arg_ptr, double); //返回double类型的下一个可变参数地址
			break;
		case T:        //参数节点
			t_ptr->tree_content.node_parameter = &parameter;
			break;
		case FUNC:    //函数调用节点
			t_ptr->tree_content.node_func.math_func_ptr = (func_ptr)va_arg(arg_ptr, func_ptr);  //函数名
			t_ptr->tree_content.node_func.child_tree = tree_node_ptr va_arg(arg_ptr, tree_node_ptr); //子树，包含具体函数运算值的子树
			break;
		default :    //二元运算符
			t_ptr->tree_content.node_op.left_tree = tree_node_ptr va_arg(arg_ptr, tree_node_ptr);  //左子树
			t_ptr->tree_content.node_op.right_tree = tree_node_ptr va_arg(arg_ptr, tree_node_ptr); //右子树
			break;
	}
	va_end(arg_ptr);  //清空，释放空间
	return t_ptr;
}

// 获取记号
void grammar::fetch_token() {
	token = Lexer.GetToken();   //借由词法分析器中方法获取记号
	if (token.type == ERRTOKEN) syntax_error(1);  //错误处理，错误类型1：未声明的记号
}

// 匹配记号
void grammar::match_token(enum Token_Type the_token) {
	if (token.type != the_token) syntax_error(2); //错误处理，错误类型2：记号顺序不符合语法
	fetch_token();      //获取下一个记号
}

//错误处理，调用方法，方便调整
void grammar::syntax_error(int case_of) {
	switch (case_of) {
		case 1: error_msg(token.lexeme, "Illegal Token");
			break;
		case 2: error_msg(token.lexeme, "Out of order of grammar,token is not as expected");
	}
}

//打印错误信息，输出单独列为函数方便调整测试
void grammar::error_msg(const char* descrip, const char* istring) {
	if (descrip!=nullptr&&istring!=nullptr)
	{
		cout << "记号" << descrip << istring << endl;
	}
	Lexer.CloseScanner();  //关闭词法分析器
}

/*
递归下降的实现，共11个
EBNF产生式如下：
program -> { statement semico }
statement -> originstatement | scalestatment | rotstatement | forstatement
originstatement -> origin is l_bracket expression comma r_bracket
scalestatement -> scale is l_bracket comma expression r_bracket
rotstatement -> rot is expression
forstatement -> for t from expression to expression step expression draw l_bracket expression comma r_bracket
expression -> term { ( plus | minus ) term }
term -> factor { ( mul | div ) factor }
factor -> ( mul | minus ) factor | component
component -> atom [ power component ]
atom -> const_id | T | func l_bracket expression r_bracket
					 | l_bracket expression r_bracket
*/

//语法格式
void grammar::program() {
	//enter("program"); //调试，查看语法树构建过程用
	while (token.type != NONTOKEN) {
		statement();           //开始读取非空语句
		match_token(SEMICO);  //匹配分号结束语句
	}
	//back("program");  //对应于enter
}

void grammar::statement() {
	//enter("statement");
	switch (token.type) {  // 依次匹配句首关键字
		case ORIGIN : origin_statement();	break;
		case SCALE  : scale_statement();	break;
		case ROT    : rot_statement();	    break;
		case FOR    : for_statement();    	break;
		default     : syntax_error(2);
	}
	//back("statement");
}

void grammar::origin_statement() {
	//enter("origin_statement");
	//call_match为测试语句，观察匹配进度
		//call_match("ORIGIN");		
	match_token(ORIGIN);
		//call_match("IS");
	match_token(IS);
		//call_match("(");
	match_token(L_BRACKET);
	x_ptr = expression();  // 构造横坐标表达式语法树
		//call_match(",");		    
	match_token(COMMA);
	y_ptr = expression();  // 构造纵坐标表达式语法树
		//call_match(")");		    
	match_token(R_BRACKET);
	//back("origin_statement");
}

void grammar::scale_statement() {
	//enter("scale_statement");
		//call_match("SCALE");		
	match_token(SCALE);
		//call_match("IS");
	match_token(IS);
		//call_match("(");
	match_token(L_BRACKET);
	x_ptr = expression();   // 构造横坐标表达式语法树
		//call_match(",");
	match_token(COMMA);
	y_ptr = expression();  // 构造纵坐标表达式语法树
		//call_match(")");
	match_token(R_BRACKET);
	//back("scale_statement");
}

void grammar::rot_statement() {
	//enter("rot_statement");
		//call_match("ROT");
	match_token(ROT);
		//call_match("IS");
	match_token(IS);
	angle_ptr = expression();     //构造参数旋转角度表达式语法树
	//back("rot_statement");
}

void grammar::for_statement() {
	//enter("for_statement");
		//call_match("FOR");			
	match_token(FOR);
		//call_match("T");			
	match_token(T);
		//call_match("FROM");			
	match_token(FROM);
	start_ptr = expression();	// 构造参数起始表达式语法树
		//call_match("TO");			
	match_token(TO);
	end_ptr = expression();		// 构造参数终结表达式语法树
		//call_match("STEP");			
	match_token(STEP);
	step_ptr = expression();	// 构造参数步长表达式语法树
		//call_match("DRAW");			
	match_token(DRAW);
		//call_match("(");			
	match_token(L_BRACKET);
	x_ptr = expression();		// 构造横坐标表达式语法树
		//call_match(",");			
	match_token(COMMA);
	y_ptr = expression(); 		// 构造纵坐标表达式语法树
		//call_match(")");			
	match_token(R_BRACKET);
	//back("for_statement");
}

//涉及到变量的语法树构造
tree_node_ptr grammar::expression() {
	tree_node_ptr left, right;			// 左右子树节点的指针
	Token_Type token_tmp;				// 当前记号

	//enter("expression");
	left = term();					// 分析左操作数且得到其语法树
	while (token.type == PLUS || token.type == MINUS)
	{
		token_tmp = token.type;
		match_token(token_tmp);
		right = term();				// 分析右操作数且得到其语法树
		left = make_tree_node(token_tmp, left, right);
		// 构造运算的语法树，结果为左子树
	}
	//tree_trace(left);				// 打印表达式的语法树
	//back("expression");
	return left;					// 返回最终表达式的语法树
}

tree_node_ptr grammar::term() {
	tree_node_ptr left, right;
	Token_Type token_tmp;

	left = factor();
	while (token.type == MUL || token.type == DIV)
	{
		token_tmp = token.type;
		match_token(token_tmp);
		right = factor();
		left = make_tree_node(token_tmp, left, right);
	}
	return left;
}

tree_node_ptr grammar::factor() {
	tree_node_ptr left, right;

	if (token.type == PLUS) 			// 匹配一元加运算
	{
		match_token(PLUS);
		right = factor();		//  表达式退化为仅有右操作数的表达式
	}
	else if (token.type == MINUS)		// 匹配一元减运算
	{
		match_token(MINUS); 		// 表达式转化为二元减运算的表达式
		right = factor();
		left = new tree_node;
		left->code = CONST_ID;
		left->tree_content.node_const = 0.0;
		right = make_tree_node(MINUS, left, right);
	}
	else right = component();		// 匹配非终结符component
	return right;
}

tree_node_ptr grammar::component() {
	tree_node_ptr left, right;

	left = atom();
	if (token.type == POWER)
	{
		match_token(POWER);
		right = component();	// 递归调用component以实现POWER的右结合性质
		left = make_tree_node(POWER, left, right);
	}
	return left;
}

tree_node_ptr grammar::atom() {
	struct Token t = token;
	tree_node_ptr t_ptr = new (struct tree_node);
	tree_node_ptr tmp = new (struct tree_node);

	switch (token.type)
	{
	case CONST_ID:
		match_token(CONST_ID);
		t_ptr = make_tree_node(CONST_ID, t.value);
		break;
	case T:
		match_token(T);
		t_ptr = make_tree_node(T);
		break;
	case FUNC:
		match_token(FUNC);
		match_token(L_BRACKET);
		tmp = expression();
		t_ptr = make_tree_node(FUNC, t.FuncPtr, tmp);
		match_token(R_BRACKET);
		break;
	case L_BRACKET:
		match_token(L_BRACKET);
		t_ptr = expression();
		match_token(R_BRACKET);
		break;
	default:
		syntax_error(2);
	}
	return t_ptr;
}

//先序遍历打印表达式的语法树
void grammar::print_syntax_tree(tree_node* root, int indent) {
	int	temp;

	for (temp = 1; temp <= indent; temp++)
		cout << "	";	// 缩进
	switch (root->code)					// 打印根节点
	{
	case PLUS:      cout << "+" << endl;	break;
	case MINUS:     cout << "-" << endl;	break;
	case MUL:       cout << "*" << endl;	break;
	case DIV:       cout << "/" << endl;	break;
	case POWER:     cout << "**" << endl;	break;
	case FUNC:      cout << root->tree_content.node_func.math_func_ptr << endl;	break;
	case CONST_ID:  cout << root->tree_content.node_const << endl;			break;
	case T:         cout << "T" << endl;					break;
	default:        cout << "非法的树节点!" << endl;			exit(0);
	}
	if (root->code == CONST_ID || root->code == T)
		return;		// 叶子节点返回
	if (root->code == FUNC)						// 递归打印一个孩子的节点
		print_syntax_tree(root->tree_content.node_func.child_tree, indent + 1);
	else									// 递归打印两个孩子的节点
	{
		print_syntax_tree(root->tree_content.node_op.left_tree, indent + 1);
		print_syntax_tree(root->tree_content.node_op.right_tree, indent + 1);
	}
}

//语法分析器接口
void grammar::grammar_port(char* file_name) {
	//enter("grammar");
	if (!(Lexer.InitScanner(file_name)))		// 初始化词法分析器
	{
		cout << "打开文件错误 !" << endl;
		return;
	}
	fetch_token();					// 获取第一个记号
	program();					// 递归下降分析
	Lexer.CloseScanner();			// 关闭词法分析器
	//back("grammer");
	return;
}
/*
//用于跟踪调试的工具函数

//进入下一层递归
void grammar::enter(const char* x) {
	//cout << "调用\t" << x << endl;
}

//返回上一层递归
void grammar::back(const char* x) {
	//cout << "调用\t" << x << "\t结束，返回" << endl;
}

//匹配记号
void grammar::call_match(const char* x) {
	//cout << "记号\t" << x << "\t成功匹配" << endl;
}

//查看表达式的语法树，先序打印
void grammar::tree_trace(tree_node_ptr x) {
	print_syntax_tree(x, 1);
}
*/