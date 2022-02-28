interface Vec2 
{
    constructor(double x, double y);

	attribute double x;
	attribute double y;

	void test();
	double getX();
	double getY();
	void setX(double x);
	void setY(double y);
};

enum Color
{
	"RED",
	"GREEN",
	"BLUE"
};