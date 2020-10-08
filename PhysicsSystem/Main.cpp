#include <iostream>
#include <cmath>

#define SHIPPING 0

// Coordinate system: + X is right, + Y is up.
struct Point2D
{
	Point2D()
	{
		x = 0;
		y = 0;
	}

	Point2D(float x_in, float y_in) : x(x_in), y(y_in){}

	Point2D(const Point2D& other) : Point2D(other.x, other.y) {}

	bool operator==(const Point2D& other) const
	{
		return (x == other.x) && (y == other.y);
	}

	void operator=(const Point2D& other)
	{
		x = other.x;
		y = other.y;
	}

	Point2D operator-(const Point2D other) const
	{
		return Point2D(x - other.x, y - other.y);
	}

	float x;
	float y;
};

struct Vector2D : Point2D
{
public:
	using Point2D::Point2D;

	Vector2D() : Point2D() {}
	Vector2D(const Point2D& point_in) : Point2D(point_in) {}

	float dot_product(const Vector2D& other)
	{
		return x * other.x + y * other.y;
	}

};

namespace Math
{
	Vector2D get_vector_from_A_to_B(const Point2D& A, const Point2D& B)
	{
		return Vector2D(B - A);
	}

	float deg_to_radians(float degrees)
	{
		static const float PI = 3.14159265359f;
		return fmod(degrees, 360.0f) / 180.0f * PI;
	}

	bool nearly_equal(float a, float b)
	{
		return abs(a - b) < 1e-5;
	}
};

// Abstract class for other shapes to inherit from:
class Shape abstract
{
public:

	Shape() {}
	Shape(Point2D center_in) : center(center_in) {};

	virtual bool contains_point(const Point2D& point_in) const = 0;

protected:
	Point2D center;
};

class Rectangle : public Shape
{
public:

	//Rectangle() {}

	Rectangle(Point2D bottom_left_in, Point2D top_right_in)
	{
		Vector2D vec_bl_tr = Math::get_vector_from_A_to_B(bottom_left_in, top_right_in);

		center = Point2D(bottom_left_in.x + vec_bl_tr.x / 2.0f, bottom_left_in.y + vec_bl_tr.y / 2.0f);

		half_extents_major = abs(vec_bl_tr.x / 2.0f);
		half_extents_minor = abs(vec_bl_tr.y / 2.0f);
	}

	Rectangle(Point2D center_in, float half_extents_x_major_in, float half_extents_y_minor) : Shape(center_in), half_extents_major(abs(half_extents_x_major_in)), half_extents_minor(abs(half_extents_y_minor)) 
	{
		axis_major = Vector2D(1.0f, 0.0f);
		axis_minor = Vector2D(0.0f, 1.0f);
	}

	// Rotation constructor
	Rectangle(Point2D center_in, float half_extents_major_in, float half_extents_minor_in, float rotation_in) : Rectangle(center_in, half_extents_major_in, half_extents_minor_in)
	{
		float rot_radians = Math::deg_to_radians(rotation_in);
		axis_major = Vector2D(cos(rot_radians), sin(rot_radians));
		axis_minor = Vector2D(-sin(rot_radians), cos(rot_radians));
	}

	// I believe a default copy constructor is created if not already defined that copies members of the passed in object,
	// but this is explicit.
	Rectangle(const Rectangle& other)
	{
		center = other.center;
		half_extents_major = other.half_extents_major;
		half_extents_minor = other.half_extents_minor;
	}

	// Ideally would like these operators to be defined in the abstract base class so all
	// derived shapes can inherit from this, but types need to match.
	bool operator==(const Rectangle& other) const
	{
		return (center == other.center) 
			&& (half_extents_major == other.half_extents_major) 
			&& (half_extents_minor == other.half_extents_minor)
			&& (axis_major == other.axis_major)
			&& (axis_minor == other.axis_minor);
	}

	void operator=(const Rectangle& other)
	{
		center = other.center;
		half_extents_major = other.half_extents_major;
		half_extents_minor = other.half_extents_minor;
	}

	bool contains_point(const Point2D& point_in) const override
	{
		// get a vector from this shape's center to the point.
		Vector2D vec_center_pt = Math::get_vector_from_A_to_B(center, point_in);
		float magnitude_major = std::abs(vec_center_pt.dot_product(axis_major));
		float magnitude_minor = std::abs(vec_center_pt.dot_product(axis_minor));

		if (magnitude_major <= half_extents_major && magnitude_minor <= half_extents_minor)
		{
			return true;
		}
		// This happens if the point is at the border, some floating point errors arise
		if (Math::nearly_equal(magnitude_major, half_extents_major) && Math::nearly_equal(magnitude_minor, half_extents_minor))
		{
			return true;
		}
		// The point is at the border on one of the axis:
		if (Math::nearly_equal(magnitude_major, half_extents_major) && magnitude_minor <= half_extents_minor)
		{
			return true;
		}
		if (magnitude_major <= half_extents_major && Math::nearly_equal(magnitude_minor, half_extents_minor))
		{
			return true;
		}
		return false;
	}

#if !SHIPPING
	static void run_unit_tests()
	{
		// Test #1: for a point inside the rectangle:
		Rectangle TestRect(Point2D(3, 4), 2.0f, 1.0f);
		Point2D InsidePoint(2, 3.5);

		bool b_contains_point = TestRect.contains_point(InsidePoint);
		bool b_tests_passed = true;

		if (!b_contains_point)
		{
			std::cout << "Unit test failure: Point should be inside the rectangle.\n";
			b_tests_passed = false;
		}

		// Test #2: test for a point at the border.
		// Also test copy constructor
		Rectangle TestRect2(TestRect);
		Point2D BorderPoint(1, 5);
		b_contains_point = TestRect2.contains_point(BorderPoint);

		if (!b_contains_point)
		{
			std::cout << "Unit test failure: Point should be inside the rectangle, at the border.\n";
			b_tests_passed = false;
		}

		// Test #3: test for a point outside.
		Point2D OutsidePoint(6, 5);
		Rectangle TestRect3(Point2D(6, 5), 100, 100);

		// Also test assignment operator.
		TestRect3 = TestRect;
		b_contains_point = TestRect.contains_point(OutsidePoint);

		if (b_contains_point)
		{
			std::cout << "Unit test failure: Point should be outside the rectangle.\n";
			b_tests_passed = false;
		}

		// Test #4: constructor with passing in min and max points.
		Rectangle RectMinMax(Point2D(1, 3), Point2D(5, 5));
		b_contains_point = RectMinMax.contains_point(InsidePoint);
		if (!b_contains_point)
		{
			std::cout << "Unit test failure: Point should be inside the rectangle.\n";
			b_tests_passed = false;
		}

		// Test #5: constructor with rotating a rectangle.
		Rectangle TestRotatedRect(Point2D(3, 4), 2, 1, 90);
		InsidePoint = Point2D(4, 6);
		b_contains_point = TestRotatedRect.contains_point(InsidePoint);

		if (!b_contains_point)
		{
			std::cout << "Unit test failure: Point should be inside rotated rectangle.\n";
			b_tests_passed = false;
		}

		if (b_tests_passed)
		{
			std::cout << "Congratulations, tests passed.\n";
		}
	}
#endif

private:
	float half_extents_major = 0.0f;
	float half_extents_minor = 0.0f;

	Vector2D axis_major;	// normalized vector in the major axis direction
	Vector2D axis_minor;	// normalized vector in the minor axis direction
};

/*
Example circle class:
class Circle : public Shape
{
	// Add constructor for center and radius
	Circle(Point2D center, float radius)

	define == and = operators
	override contains_point(point)
	{
		point is inside the circle if the euclidian distance is <= radius
	}

	// include a static function for running unit tests
}
*/

int main()
{
#if !SHIPPING
	Rectangle::run_unit_tests();
#endif
	return 0;
}