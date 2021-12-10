// Interpreter.cpp : Interpreter.h实现。
//

#include "Interpreter.h"
#include "grammar.h"
#include "framework.h"
#include <cmath>
#include <codecvt>
#include <comdef.h>


#ifndef MCOLORLEGAL
#define MCOLORLEGAL

#define RGBIsLegal(color) (color>=0&&color<=255)
#endif

void Interpreter::refreshMatrix(){
	// 缩放矩阵
	double scale[3][3] = {
		{scaleFactorX,0.0,0.0},
		{0.0,scaleFactorY,0.0},
		{0.0,0.0,1.0}
	};
	// 旋转矩阵
	double rotate[3][3] = {
		{cos(rotationAngle),-sin(rotationAngle),0.0},
		{sin(rotationAngle),cos(rotationAngle),0.0},
		{0.0,0.0,1.0}
	};

	// 平移矩阵
	double pan[3][3] = {
		{1.0,0.0,0.0},
		{0.0,1.0,0.0},
		{origin.x,origin.y,1.0}
	};
	double tmp[3][3] = {
		{1.0,0.0,0.0},
		{0.0,1.0,0.0},
		{0.0,0.0,1.0}
	};
	MultiplyMatrixByMatrix(tmp,scale);
	MultiplyMatrixByMatrix(tmp,rotate);
	MultiplyMatrixByMatrix(tmp,pan);
	memcpy(&transformMatrix, &tmp, sizeof(tmp));
}

/**
 * \brief 计算表达式的值, 方法是DFS递归先序遍历子树
 * \param root 调用时为子树的根节点
 * \return 表达式的值
 */
double Interpreter::CalcExprValue(tree_node_ptr root)
{
	double rightTreeValue;
	if(root==nullptr)	//nullptr==空树
	{
		return 0.0;
	}
	switch(root->code)
	{
	case PLUS:
		return CalcExprValue(root->tree_content.node_op.left_tree) +
			CalcExprValue(root->tree_content.node_op.right_tree);
	case MINUS: 
		return CalcExprValue(root->tree_content.node_op.left_tree) -
			CalcExprValue(root->tree_content.node_op.right_tree);
	case MUL: 
		return CalcExprValue(root->tree_content.node_op.left_tree) *
			CalcExprValue(root->tree_content.node_op.right_tree);
	case DIV:
		rightTreeValue = CalcExprValue(root->tree_content.node_op.right_tree);
		if (rightTreeValue != 0)
		{
			return CalcExprValue(root->tree_content.node_op.left_tree) /
				rightTreeValue;
		}else
		{
			ErrMessage(L"Math error: No n div 0");
			return 0.0;
		}

	case POWER: 
		return pow(CalcExprValue(root->tree_content.node_op.left_tree), 
			CalcExprValue(root->tree_content.node_op.right_tree));
	case FUNC:
		return (*root->tree_content.node_func.math_func_ptr)	// Function Pointer
				(CalcExprValue(root->tree_content.node_func.child_tree));
	case T:
		return *(root->tree_content.node_parameter);
	case CONST_ID: 
		return root->tree_content.node_const;
	default: 
		return 0.0;
	}
}

Interpreter::~Interpreter()
{
	DeleteObject(penBorder);
	DeleteObject(brush);
}


/**
 * \brief 删除给定子树，释放空间，等待下次解析，方法是DFS递归先序遍历子树
 * \param root 调用时为子树的根节点
 */
void Interpreter::DeleteTree(tree_node_ptr root)
{
	if (root == nullptr)	//空树不用处理
	{
		return;
	}
	switch (root->code)
	{
	// 二元运算处理
	case PLUS: 
	case MINUS: 
	case MUL:
	case DIV:
	case POWER:
		DeleteTree(root->tree_content.node_op.left_tree);
		DeleteTree(root->tree_content.node_op.right_tree);
		break;
	// 函数类处理
	case FUNC: 
		DeleteTree(root->tree_content.node_func.child_tree);
		break;
	default:;
	}
	delete(root);
}


void Interpreter::MultiplyMatrixByMatrix(double(&a)[3][3], const double(&b)[3][3]) {
	double tmp[3][3];
	for (size_t i = 0; i < 3; ++i)
	{
		for (size_t j = 0; j < 3; ++j)
		{
			tmp[i][j] = 0;
			for (size_t k = 0; k < 3; ++k)
			{
				tmp[i][j] += a[i][k] * b[k][j];
			}
		}
	}
	memcpy(&a, &tmp, sizeof(tmp));
}

void Interpreter::MultiplyMatrixByVector(double(&a)[3], const double(&b)[3][3]) {
	double tmp[3];
	for (size_t i = 0; i < 3; ++i)
	{
		tmp[i] = 0;
		for (size_t j = 0; j < 3; ++j)
		{
			tmp[i] += a[j] * b[j][i];
		}
	}
	memcpy(&a, &tmp, sizeof(tmp));
}

void Interpreter::SetScaleFactor(double x, double y)
{
	this->scaleFactorX = x;
	this->scaleFactorY = y;
	this->refreshMatrix();
}

void Interpreter::SetRotationAngle(double angle)
{
	this->rotationAngle = angle;
	this->refreshMatrix();
}

void Interpreter::DrawPoint(Position pos)
{
	double homocord[3] = {pos.x,pos.y,1.0};
	MultiplyMatrixByVector(homocord, this->transformMatrix);
	// 齐次->笛卡尔
	homocord[0] /= homocord[2];
	homocord[1] /= homocord[2];
	preparedPoints.push_back({ homocord[0],homocord[1] });
}

void Interpreter::SetPointSize(int size)
{
	PointSize = size;
}

void Interpreter::SetColor(int r, int g, int b)
{
	if (RGBIsLegal(r)&&RGBIsLegal(g)&&RGBIsLegal(b))
	{
		Color = FormatColor(r, g, b);
	}else
	{
		ErrMessage(L"Syntax Error, RGB value should be from 0, 255");
	}
	
}

void Interpreter::SetColor(COLORREF colorref)
{
	Color = colorref;
}


void Interpreter::SetOrigin(double x, double y)
{
	origin = { x,y };
	refreshMatrix();
}

int Interpreter::GetPointSize() const
{
	return PointSize;
}

void Interpreter::Clear(HWND hwnd)
{
	preparedPoints.clear();
	InvalidateRect(hwnd, nullptr, TRUE);
}

void Interpreter::Refresh(HWND hwnd)
{
	InvalidateRect(hwnd, nullptr, TRUE);
	UpdateWindow(hwnd);
}

LRESULT Interpreter::DrawErrMessage(HWND hwnd)
{
	PAINTSTRUCT ps;
	RECT rect;
	HDC hdc = BeginPaint(hwnd, &ps);
	GetClientRect(hwnd, &rect);
	DrawText(
		hdc,
		errMessage.front(),
		-1,
		&rect,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER
	);
	errMessage.clear();
	EndPaint(hwnd, &ps);
	return 0;
}

void Interpreter::ErrMessage(const wchar_t* str)
{
	errMessage.push_back(const_cast<wchar_t*>(str));
}

void Interpreter::Interprete(HWND hwnd,WCHAR* filePath)
{
	if (wcslen(filePath) == 0)
	{
		ErrMessage(L"Empty filePath");
		return;
	}
	_bstr_t sfilePath(filePath);
	char* charFilePath=sfilePath;

	grammar_port(charFilePath);
}

void Interpreter::example(HWND hwnd)
{
	SetOrigin(100, 300);
	SetRotationAngle(0);
	SetScaleFactor(1, 1);
	SetColor(255, 0, 0);
	for(double i=0;i<200;i+=1)
	{
		DrawPoint({i,0});
	}
	//ShowResult(hwnd);
	SetColor(240, 248, 255);
	for (double i = 0; i < 150; i += 1)
	{
		DrawPoint({ 0,-i });
	}
	//ShowResult(hwnd);
	SetColor(115, 103, 60);
	for (double i = 0; i < 120; i += 1)
	{
		DrawPoint({ i,-i });
	}
	//ShowResult(hwnd);
}

void Interpreter::ShowResult(HWND hWnd)
{
	InvalidateRect(hWnd, nullptr , FALSE);
	UpdateWindow(hWnd);
}


// TODO:绘制For_loop内的点，我看看怎么处理
void Interpreter::drawLoop(double start_val, double end_val, double step_val,
	tree_node_ptr	x_ptr, tree_node_ptr	y_ptr)
{
	for (parameter = start_val; parameter <= end_val; parameter += step_val)
	{
		DrawPoint({ CalcExprValue(x_ptr), CalcExprValue(y_ptr)});
	}
}

// 绘制缓存区内所有的点
LRESULT Interpreter::DrawAllPoint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);

	penBorder = CreatePen(PS_SOLID,0,Color);
	brush = CreateSolidBrush(Color);
	SelectObject(hDC, brush);
	SelectObject(hDC, penBorder);
	for (auto& point : preparedPoints)
	{
		if (PointSize>0)
		{
			Ellipse(
				hDC,
				static_cast<int>(round(point.x - static_cast<double>(PointSize))),
				static_cast<int>(round(point.y - static_cast<double>(PointSize))),
				static_cast<int>(round(point.x + static_cast<double>(PointSize) + 1.0)),
				static_cast<int>(round(point.y + static_cast<double>(PointSize) + 1.0))
			);
		}else
		{
			SetPixel(hDC, point.x, point.y, Color);
		}

	}
	EndPaint(hWnd,&ps);
	return 0;
}

// 如果没有错误信息就绘制缓存区里的图，否则绘制错误信息
LRESULT Interpreter::Draw(HWND hWnd)
{
	if(!errMessage.empty())
	{
		preparedPoints.clear();
		return DrawErrMessage(hWnd);
	}else
	{
		return DrawAllPoint(hWnd);
	}
}

void Interpreter::error_msg(const char* descrip, const char* iString)
{
	grammar::error_msg(descrip, iString);
	//setup converter
	typedef codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	char* msg=new char[256];
	sprintf_s(msg,strlen(descrip)+strlen(iString)+10,"Err: %s %s!",descrip,iString);

	wstring wmsg=converter.from_bytes(msg);
	ErrMessage(wmsg.c_str());
	MessageBox(NULL, wmsg.c_str(), L"Error!", MB_OK);
	Lexer.CloseScanner();
}

void Interpreter::for_statement()
{
	// 调用语法分析器解析内容
	grammar::for_statement();

	drawLoop(CalcExprValue(start_ptr),CalcExprValue(end_ptr),CalcExprValue(step_ptr),
		x_ptr,y_ptr);

	// 结束，销毁所有已解析子树
	DeleteTree(start_ptr);
	DeleteTree(end_ptr);
	DeleteTree(step_ptr);
	DeleteTree(x_ptr);
	DeleteTree(y_ptr);
}

void Interpreter::origin_statement()
{
	grammar::origin_statement();

	SetOrigin(CalcExprValue(x_ptr),CalcExprValue(y_ptr));

	DeleteTree(x_ptr);
	DeleteTree(y_ptr);
}

void Interpreter::rot_statement()
{
	grammar::rot_statement();

	SetRotationAngle(CalcExprValue(angle_ptr));

	DeleteTree(angle_ptr);
}

void Interpreter::scale_statement()
{
	grammar::scale_statement();

	SetScaleFactor(CalcExprValue(x_ptr),CalcExprValue(y_ptr));
	
	DeleteTree(x_ptr);
	DeleteTree(y_ptr);
}

void Interpreter::color_statement()
{
	grammar::color_statement();

	SetColor(round(CalcExprValue(rgb_ptr[0])), round(CalcExprValue(rgb_ptr[1])), round(CalcExprValue(rgb_ptr[2])));
	for (auto& tree_node : rgb_ptr)
	{
		DeleteTree(tree_node);
	}
}

void Interpreter::size_statement()
{
	grammar::size_statement();
	SetPointSize(CalcExprValue(size_ptr));
	DeleteTree(size_ptr);
}

// Grammar 遗留部分，重载防止影响
//
void Interpreter::enter(const char* x)
{
}

void Interpreter::back(const char* x)
{
}

void Interpreter::call_match(const char* x)
{
}

void Interpreter::tree_trace(tree_node_ptr x)
{
}