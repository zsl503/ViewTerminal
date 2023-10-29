#pragma once
#include <string>
#include <Windows.h>
#include <functional>
#include <vector>
namespace View
{
	class ComponentBase;

	enum class Color
	{
		BLACK, BLUE, GREEN, LIGHTGREEN, RED, PURPLE, YELLOW, WHITE,
		GREY, LIGHTBLUE, PALEGREEN, PALELIGHTGREEN, LIGHTRED,
		LIGHTPURPLE, LIGHTYELLOW, BRIGHTWHITE,
	};

	enum class MouseEventType
	{
		RCLICK,
		RDOUBLECLICK,
		LCLICK,
		LDOUBLECLICK,
		HOVER
	};

	class Point
	{
	public:
		enum class Relation
		{
			SAME, UP, DOWN, LEFT, RIGHT, LEFTUP, LEFTDOWN, RIGHTUP, RIGHTDOWN
		};

		Point(int x = 0, int y = 0) :x(x), y(y) {}
		Point(const COORD& point) :x(point.X), y(point.Y) {}
		Point(const Point& point) :x(point.x), y(point.y) {}
		Relation getNewPointRelation(Point newPoint);
		Point getAbsPoint(int reX, int reY);
		Point getAbsPoint(Point rePoint);
		COORD getCOORD(int reX = 0, int  reY = 0);
		unsigned short getAbsX(int reX = 0);
		unsigned short getAbsY(int reX = 0);
		unsigned short x, y;
	};

	typedef struct MouseEvent
	{
		Point point;
		MouseEventType type;
	}MouseEvent;

	typedef struct KeyEvent
	{
		Point point;
		MouseEventType type;
	}KeyEvent;

	class ComponentBase
	{
	public:
		void draw();	// �ڿ���̨����ͼ��
		virtual ~ComponentBase();	//	����
		virtual void update();	// ����ͼ��
		virtual ComponentBase* onMouseEvent(MouseEvent&);	// ����¼�
		virtual ComponentBase* onKeyEvent(KeyEvent&);	// �����¼�
		virtual void setColorStyle(Color, Color);	// ������ɫ
		virtual void setFocus();	//	����Ϊ����
		virtual void unsetFocus();	// ȡ������
		bool isPointIn(Point);	// �ж���������������
		bool isPointIn(COORD);	// �ж���������������
		Point getPoint();	// ��ȡ���Ͻ�����
		Point getRDPoint();	// ��ȡ���½�����
		size_t getWidth();	//��ȡ���
		size_t getHeight();	//��ȡ�߶�
		virtual void setPoint(Point);	// ���û�׼����
		virtual void setSize(size_t, size_t);	// ���ô�С
		virtual void setShow(bool);	// �����Ƿ���ҳ������ʾ
		bool getShow();	// ��ȡҳ����ʾ����
	protected:
		void setChange();	// ����ҳ�����״̬�����ú������������Ӧ��һ�θ���
		virtual void beginDraw() = 0;	// ����ǰִ��
		virtual void drawBackground();	// ���Ʊ���
		virtual void drawBorder();	// ���Ʊ߿�
		virtual void clearDraw();	//���ҳ�����
		ComponentBase(HANDLE outputHandle, Point point, size_t width = 0, size_t height = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: point(point), width(width), height(height), outputHandle(outputHandle),
			defaultForeColor(defaultForeColor), defaultBackColor(defaultBackColor),
			nowForeColor(defaultForeColor), nowBackColor(defaultBackColor),
			foreColor(defaultForeColor), backColor(defaultBackColor) {}
		size_t width, height;	// ��ȡ��߶�
		Point point;	// ��׼����㣬Ϊ���Ϸ�����
		HANDLE outputHandle;	// ���������������̨���
		Color nowForeColor;	// ��ǰ��ʾ���ı�ɫ
		Color nowBackColor;	//��ǰ��ʾ�ı���ɫ
		Color backColor;	// ��ǰ���õı���ɫ
		Color foreColor;	// ��ǰ���õ��ı�ɫ
		Color defaultForeColor;	// Ĭ���ı�ɫ
		Color defaultBackColor;	// Ĭ�ϱ���ɫ
		std::function<int(MouseEvent&, ComponentBase*)> mouseEventFun;	// ����¼��ص�
		std::function<int(KeyEvent&, ComponentBase*)> keyEventFun;	// �����¼��ص�
		bool isChange = false;	// �Ƿ���Ӧ����
		bool focus;	// �Ƿ�Ϊ����
	private:
		bool isDrawed = false;	// ��ǰ����Ƿ�������Ļ�ϻ���
		bool isShow = true;	// �Ƿ���ʾ���
	};

	class Text : public ComponentBase
	{
	public:
		Text(HANDLE outputHandle, std::wstring text, size_t abX, size_t abY, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: ComponentBase(outputHandle, Point(abX, abY), width, height, defaultForeColor, defaultBackColor),
			text(text), endPoint(Point(abX, abY)) {}

		Text(HANDLE outputHandle, std::wstring text, Point point, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Text(outputHandle, text, point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}

		Text(HANDLE outputHandle, int text, Point point, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Text(outputHandle, std::to_wstring(text), point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}
		virtual std::wstring setText(std::wstring);
		std::wstring setText(int);
		std::wstring getText();
	protected:
		void beginDraw();
		Point endPoint;
	private:
		Point drawLine(std::wstring, Point, bool&);
		std::wstring text;
	};

	class Button : public ComponentBase
	{
	public:
		Button(HANDLE outputHandle, std::wstring text, size_t abX, size_t abY, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: ComponentBase(outputHandle, Point(abX, abY), width, height, defaultForeColor, defaultBackColor),
			text(outputHandle, text, 0, 0) {
			hoverBackColor = defaultBackColor;
		}

		Button(HANDLE outputHandle, std::wstring text, Point point, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Button(outputHandle, text, point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}
		void setColorStyle(Color, Color);
		void setHoverStyle(Color, Color);
		void setClickStyle(Color, Color);
		ComponentBase* onMouseEvent(MouseEvent&);
		std::wstring setText(std::wstring);
		std::wstring setText(int);
		std::wstring getText();
		std::function<int(MouseEvent&, ComponentBase*)> onClickFun;
	protected:
		void beginDraw();
		Text text;
		size_t textAbX = 1;
		size_t textAbY = 1;
		size_t textSubHeight = 2;
		size_t textSubWidth = 2;
	private:
		Color hoverBackColor;
		Color hoverForeColor = Color::LIGHTYELLOW;
		Color clickBackColor = Color::WHITE;
		Color clickForeColor = Color::LIGHTRED;
		bool isHover = false;
		bool isOnclick = false;
	};

	class List : public ComponentBase
	{
	public:
		~List();
		List(HANDLE outputHandle, std::vector<std::wstring> items, size_t abX, size_t abY, size_t width = 10, size_t itemHeight = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK);
		List(HANDLE outputHandle, std::vector<std::wstring> items, Point point, size_t width = 10, size_t itemHeight = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK);
	protected:
		void beginDraw();
	private:
		std::vector<Button*> buttons;
	};

	class Line : public ComponentBase
	{
	public:
		Line(HANDLE outputHandle, size_t abX, size_t abY, bool isHorizontal, size_t length
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: ComponentBase(outputHandle, Point(abX, abY), isHorizontal ? length : 1, isHorizontal ? 1 : length, defaultForeColor, defaultBackColor),
			isHorizontal(isHorizontal), length(length) {}

		Line(HANDLE outputHandle, Point point, bool isHorizontal, size_t length = 10
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Line(outputHandle, point.x, point.y, isHorizontal, length, defaultForeColor, defaultBackColor) {}
	protected:
		void beginDraw();
	private:
		bool isHorizontal;
		size_t length;
	};

	class InputText :public Text
	{
	public:
		InputText(HANDLE outputHandle, std::wstring text, size_t abX, size_t abY, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Text(outputHandle, text, abX, abY, width, height, defaultForeColor, defaultBackColor) {}

		InputText(HANDLE outputHandle, std::wstring text, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: InputText(outputHandle, text, point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}

		ComponentBase* onMouseEvent(MouseEvent&);
		std::function<int(InputText*)> textChanged;
	private:
		std::function<int(MouseEvent&, ComponentBase*)> focusFun;
	};

	class LayoutBase : public ComponentBase
	{
	public:
		LayoutBase(HANDLE outputHandle, size_t abX, size_t abY, size_t width, size_t height
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: ComponentBase(outputHandle, Point(abX, abY), width, height, defaultForeColor, defaultBackColor) {}

		LayoutBase(HANDLE outputHandle, Point point, size_t width, size_t height
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: LayoutBase(outputHandle, point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}
		virtual void registerComponents(std::vector<ComponentBase*>&, std::vector<ComponentBase*>&, std::vector<ComponentBase*>&);
		virtual bool addComponent(ComponentBase*);
		virtual bool removeComponent(ComponentBase*);
		virtual void clear();
		void setShow(bool);
		~LayoutBase();
		void update();
		ComponentBase* onMouseEvent(MouseEvent&);
		ComponentBase* onKeyEvent(KeyEvent&);
		void setPoint(Point);

	protected:
		void beginDraw();
		std::vector<ComponentBase*> allComponents;
		std::vector<ComponentBase*> mouseComponents;
		std::vector<ComponentBase*> keyComponents;
	};

	class Span :public LayoutBase
	{
	public:
		Span(HANDLE outputHandle, size_t abX, size_t abY, size_t width, size_t height, size_t space = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: LayoutBase(outputHandle, Point(abX, abY), width, height, defaultForeColor, defaultBackColor),
			space(space) {}
		Span(HANDLE outputHandle, Point point, size_t width, size_t height, size_t space = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Span(outputHandle, point.x, point.y, width, height, space, defaultForeColor, defaultBackColor) {}
		void setPadding(unsigned short top = 0, unsigned short right = 0, unsigned short bottom = 0, unsigned short left = 0);
		void registerComponents(std::vector<ComponentBase*>&, std::vector<ComponentBase*>&, std::vector<ComponentBase*>&);
		bool addComponent(ComponentBase*);
		bool removeComponent(ComponentBase*);
		void setSpace(size_t space);
		void clear();
	protected:
		unsigned short padding[4] = { 0,0,0,0 };
		size_t currentWidth = 0;
		size_t space;
	};

	class Div : public LayoutBase
	{
	public:
		Div(HANDLE outputHandle, size_t abX, size_t abY, size_t width, size_t height, size_t space = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: LayoutBase(outputHandle, Point(abX, abY), width, height, defaultForeColor, defaultBackColor),
			space(space) {}

		Div(HANDLE outputHandle, Point point, size_t width, size_t height, size_t space = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Div(outputHandle, point.x, point.y, width, height, space, defaultForeColor, defaultBackColor) {}
		void registerComponents(std::vector<ComponentBase*>&, std::vector<ComponentBase*>&, std::vector<ComponentBase*>&);
		bool addComponent(ComponentBase*);
		bool removeComponent(ComponentBase*);
		void setSpace(size_t space);
		void setPadding(unsigned short top = 0, unsigned short right = 0, unsigned short bottom = 0, unsigned short left = 0);
		void clear();

	protected:
		unsigned short padding[4] = { 0,0,0,0 };
		size_t currentHeight = 0;
		size_t space;
	};
	std::vector<std::wstring> split(const std::wstring& in, const std::wstring& delim);
	std::string UnicodeToANSI(const std::wstring& wstr);
}
