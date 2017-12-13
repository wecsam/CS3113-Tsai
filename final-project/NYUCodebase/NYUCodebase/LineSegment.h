#pragma once
struct LineSegment {
	float X1, Y1, X2, Y2;
	LineSegment();
	LineSegment(float X1, float Y1, float X2, float Y2);
	float Slope() const;
	float YIntercept() const;
	// Returns true if the two line segments intersect
	bool IntersectionWith(const LineSegment&, float& IntersectionX, float& IntersectionY);
};
