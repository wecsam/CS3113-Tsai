#pragma once
class Entity {
public:
	float GetLeftBoxBound() const;
	float GetRightBoxBound() const;
	float GetTopBoxBound() const;
	float GetBottomBoxBound() const;
	bool IsCollidingWith(const Entity&) const;
protected:
	void SetLeftBoxBound(float);
	void SetRightBoxBound(float);
	void SetTopBoxBound(float);
	void SetBottomBoxBound(float);
	void MoveX(float);
	void MoveY(float);
private:
	// Each entity is made of two triangles that form a rectangle together.
	// Each triangle has three vertices.
	// Each vertex has two floats.
	// That makes 12 floats total in the vertex array.
	static enum VERTEX_INDICES {
		T1_TOP_LEFT_X,
		T1_TOP_LEFT_Y,
		T1_BOTTOM_LEFT_X,
		T1_BOTTOM_LEFT_Y,
		T1_TOP_RIGHT_X,
		T1_TOP_RIGHT_Y,
		T2_TOP_RIGHT_X,
		T2_TOP_RIGHT_Y,
		T2_BOTTOM_LEFT_X,
		T2_BOTTOM_LEFT_Y,
		T2_BOTTOM_RIGHT_X,
		T2_BOTTOM_RIGHT_Y,
		NUM_VERTICES
	};
	float vertices[VERTEX_INDICES::NUM_VERTICES] = { 0.0f };
};
