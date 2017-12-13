#include "LineSegment.h"
#include <cmath>
LineSegment::LineSegment() {}
LineSegment::LineSegment(float X1, float Y1, float X2, float Y2)
	: X1(X1), Y1(Y1), X2(X2), Y2(Y2) {}
float LineSegment::Slope() const {
	if (X1 == X2) {
		return NAN;
	}
	return (Y2 - Y1) / (X2 - X1);
}
float LineSegment::YIntercept() const {
	return Y1 - Slope() * X1;
}
bool LineSegment::IntersectionWith(const LineSegment& other, float& IntersectionX, float& IntersectionY) {
	// Make sure that both line segments are not points.
	if (X1 == X2 && Y1 == Y2) {
		return false;
	}
	if (other.X1 == other.X2 && other.Y1 == other.Y2) {
		return false;
	}
	// For each line segment, find the minimum X value.
	// Then, find the maximum of those two minumums.
	float intervalLeft = fmax(fmin(X1, X2), fmin(other.X1, other.X2));
	// For each line segment, find the maximum X value.
	// Then, find the minumum of those two maximums.
	float intervalRight = fmin(fmax(X1, X2), fmax(other.X1, other.X2));
	// The intersection must occur between intervalLeft and intervalRight,
	// or it does not exist. Also, intervalLeft must be to the left of
	// intervalRight, or the interval is not valid.
	if (intervalLeft > intervalRight) {
		return false;
	}
	// Find the slope of each segment.
	float thisSlope = Slope(), otherSlope = other.Slope();
	// Check whether the line segments are parallel.
	if (thisSlope == otherSlope) {
		return false;
	}
	// Find the Y intercept of each segment.
	float thisIntercept = YIntercept(), otherIntercept = other.YIntercept();
	// Check for vertical line segments.
	if (isnan(thisSlope)) {
		// This line segment is vertical.
		IntersectionX = X1;
		IntersectionY = otherSlope * X1 + otherIntercept;
		if (IntersectionY < fmin(Y1, Y2) || IntersectionY > fmax(Y1, Y2)) {
			return false;
		}
	}
	else if (isnan(otherSlope)) {
		// The other line segment is vertical.
		IntersectionX = other.X1;
		IntersectionY = thisSlope * other.X1 + thisIntercept;
		if (IntersectionY < fmin(other.Y1, other.Y2) || IntersectionY > fmax(other.Y1, other.Y2)) {
			return false;
		}
	}
	else {
		// Find the X value of the intersection point.
		IntersectionX = (otherIntercept - thisIntercept) / (thisSlope - otherSlope);
		// The X value of the intersection point must be within the interval.
		if (IntersectionX < intervalLeft || IntersectionX > intervalRight) {
			return false;
		}
		// The line segments intersect.
		// Just calculate the Y value of the intersection.
		IntersectionY = thisSlope * IntersectionX + thisIntercept;
	}
	return true;
}
