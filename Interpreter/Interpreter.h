// Interpreter.h : 定义Interpreter类的文件
//
#pragma once

#include <vector>

#include "framework.h"
#include "grammar.h"

using namespace Scanner;
typedef struct Position {
	double x;
	double y;
}Position;

class Interpreter: public grammar {
	friend class grammar;
protected:
	Position origin{0,0};
	double scaleFactorX = 1.0;
	double scaleFactorY = 1.0;
	double rotationAngle = 0.0;
	
	double transformMatrix[3][3]= {
		{1.0, 0.0, 0.0},
		{ 0.0,1.0,0.0 },
		{ 0.0,0.0,1.0 } }; // Three in ONE!, 等于旋转矩阵*缩放矩阵*平移矩阵
	HPEN penBorder;
	HBRUSH brush;
	void refreshMatrix();
	double CalcExprValue(tree_node_ptr root);
	~Interpreter();
	void DeleteTree(tree_node_ptr root);
private:

	int PointSize = 3;
	COLORREF Color=RGB(0,0,0);
	constexpr COLORREF FormatColor(int r,int g,int b) { return RGB(r, g, b); }
	std::vector<Position> preparedPoints;
	std::vector<wchar_t*> errMessage;
	
	void MultiplyMatrixByMatrix(double(&a)[3][3], const double(&b)[3][3]);
	void MultiplyMatrixByVector(double(&a)[3], const double(&b)[3][3]);
	void DrawPoint(Position pos);
	void drawLoop(double start_val,double end_val,double step_val,tree_node_ptr	x_ptr,tree_node_ptr	y_ptr);

	
	// 无用部分,为继承后protected方法运行提供必要环境
		// 下述函数用于语法分析器的跟踪调试
	void enter(const char* x);//进入下一层递归
	void back(const char* x);  //返回上一层递归
	void call_match(const char* x); //匹配记号
	void tree_trace(tree_node_ptr x); //查看表达式的语法树，先序打印
	LRESULT DrawErrMessage(HWND hwnd);
	LRESULT DrawAllPoint(HWND hWnd);
public:
	void SetPointSize(int size);
	void SetScaleFactor(double x, double y);
	void SetRotationAngle(double angle);
	void SetColor(int r, int g, int b);
	void SetColor(COLORREF colorref);
	void SetOrigin(double x,double y);

	int GetPointSize() const;

	void Clear(HWND hwnd);
	void Refresh(HWND hwnd);

	void ErrMessage(const wchar_t* str);
	
	void Interprete(HWND hwnd,WCHAR* filePath);
	void example(HWND hwnd);
	static void ShowResult(HWND hWnd);
	LRESULT Draw(HWND hWnd);
// Overridden
protected:
	void error_msg(const char* descrip, const char* iString) override;
	void for_statement() override;
	void origin_statement() override;
	void rot_statement() override;
	void scale_statement() override;
	void color_statement() override;
	void size_statement() override;
};


