// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

#ifndef PIECE_HPP
#define PIECE_HPP

#include <QGLWidget>
#include <QMatrix4x4>
#include <QtGlobal>
#include <QLabel>
#include <string>
#include <iostream>
#include <cmath>
#include <utility>

class Piece {
public:
	Piece(std::string type, int bx, int bz);
	virtual void reset();
	virtual void setStartingPositions();

	virtual void modelRotateXZ(float theta);
	virtual void worldTranslate(float x, float y, float z);
	virtual void moveUnitXZ(int dx, int dy);

	virtual void setViewerData(GLenum drawMode, std::string texture, std::string perlinTexture, std::string otherTexture, int vboStart, int vboN);
	virtual QMatrix4x4 getMWTransform();

	static QVector3D boardToWorldCoordinates(int x, int z);
	virtual QVector3D getCentreWorld();

	// Functions for animation
	virtual void saveInitialPosition();
	virtual void startAnimation(QVector3D translation, float rotation, int nSteps);
	virtual void advanceAnimationStep();

	GLenum drawMode;
	std::string texture;
	std::string perlinTexture;
	std::string otherTexture;
	int vboStart;
	int vboN;
	int boardx;
	int boardz;
	QVector3D origCentreWorld;
	int origx;
	int origz;
	std::string type;
	bool isVisible;
	bool isHighlighted;
	bool isSelected;
	bool isAnimated;
protected:
	QMatrix4x4 modelBaseMatrix;
	QMatrix4x4 modelTransformMatrix;
	QMatrix4x4 worldBaseMatrix;
	QMatrix4x4 worldTransformMatrix;

	QMatrix4x4 srcWorldTransform;
	QMatrix4x4 destWorldTransform;
	QMatrix4x4 srcModelTransform;
	QMatrix4x4 destModelTransform;
	float srcRotation;
	float destRotation;
	QVector3D translationStep;
	float thetaStep;
	int nAnimationSteps;
	int animationStepCount;
};


class GamePiece : public Piece {
public:
	GamePiece(std::string type, int bx, int bz, float rotation, int player);
	virtual void reset();
	virtual void modelRotateXZ(float theta);
	float rotation;
	float origRotation;
	int player;
	std::pair<int,int> getReflectedDirection(std::pair<int,int> in, bool& hit);
	void printInfo();
	std::vector< std::pair<int,int> > pathLaser1;
	std::vector< std::pair<int,int> > pathLaser2;

	virtual void saveInitialPosition();
	virtual void startAnimation(QVector3D translation, float rotation, int nSteps);
	virtual void advanceAnimationStep();

private:
	static const int reflectedPairs[4][2][2][2];
};


#endif