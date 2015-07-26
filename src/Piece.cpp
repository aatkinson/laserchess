// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

#include "Piece.hpp"

QVector3D Piece::boardToWorldCoordinates(int bx, int bz){
	return QVector3D(((float)bx)-5, 0.5, ((float)bz)-4); // what about y???
}

Piece::Piece(std::string type, int bx, int bz)
	: type(type)
	, origx(bx)
	, origz(bz)
	, boardx(bx)
	, boardz(bz)
	, isVisible(true)
	, isHighlighted(false)
	, isSelected(false)
	, isAnimated(false)
	, thetaStep(0.0)
	, nAnimationSteps(0)
	, animationStepCount(0)
{
	origCentreWorld = Piece::boardToWorldCoordinates(boardx, boardz) + QVector3D(0.5, 0, 0.5);
}

void Piece::reset(){
	isVisible = true;
	isHighlighted = false;
	isSelected = false;
	isAnimated = false;
	translationStep = QVector3D(0,0,0);
	thetaStep = 0.0;
	nAnimationSteps = 0;
	animationStepCount = 0;
	srcWorldTransform.setToIdentity();
	destWorldTransform.setToIdentity();
	modelTransformMatrix = modelBaseMatrix;
	worldTransformMatrix = worldBaseMatrix;
	boardx = origx;
	boardz = origz;
}

QVector3D Piece::getCentreWorld(){
	return worldTransformMatrix * origCentreWorld;
}

void Piece::setStartingPositions(){
	modelBaseMatrix = modelTransformMatrix;
	worldBaseMatrix = worldTransformMatrix;
}

void Piece::modelRotateXZ(float theta){
	QMatrix4x4 transToOrigin;
	transToOrigin.translate(-0.5, 0, -0.5);
	QMatrix4x4 transBack;
	transBack.translate(0.5, 0, 0.5);
	QMatrix4x4 rotY;
	rotY.rotate(theta, 0, 1, 0);

	modelTransformMatrix = transBack * rotY * transToOrigin * modelTransformMatrix;
}

void Piece::worldTranslate(float x, float y, float z){
	worldTransformMatrix.translate(x,y,z);
}

void Piece::moveUnitXZ(int dx, int dz){
	boardx += dx;
	boardz += dz;
	worldTranslate((float)dx, 0.0, (float)dz);
}

QMatrix4x4 Piece::getMWTransform(){
	return worldTransformMatrix * modelTransformMatrix;
}

void Piece::setViewerData(GLenum dm, std::string tex, std::string perlinTex, std::string otherTex, int start, int n){
	drawMode = dm;
	texture = tex;
	perlinTexture = perlinTex;
	otherTexture = otherTex;
	vboStart = start;
	vboN = n;
}

void Piece::saveInitialPosition(){}

void Piece::startAnimation(QVector3D translation, float rotation, int nSteps){
	(void) translation;
	(void) rotation;
	(void) nSteps;
}
void Piece::advanceAnimationStep(){
	
}

// GAME PIECE

//theta,pair,vect,elem
const int GamePiece::reflectedPairs[4][2][2][2]={

	{ { {-1,0}, {0,-1}}, //0
	  { {0,1}, {1,0}} },

	{ { {0,1}, {-1,0}}, //90
	  { {1,0}, {0,-1}} },

	{ { {1,0}, {0,1}}, //180
	  { {0,-1}, {-1,0}} },

	{ { {0,-1}, {1,0}}, //270
	  { {-1,0}, {0,1}} }
};

GamePiece::GamePiece(std::string type, int bx, int bz, float rot, int player)
: Piece(type, bx, bz)
, rotation(0)
, player(player)
, origRotation(rot)
{
}

void GamePiece::reset(){
	rotation = origRotation;
	Piece::reset();
}

void GamePiece::modelRotateXZ(float theta){
	rotation += theta;
	rotation = fmod(rotation, 360.0);
	//printInfo();
	Piece::modelRotateXZ(theta);
}

std::pair<int,int> GamePiece::getReflectedDirection(std::pair<int,int> inpair, bool& hit){

	QVector3D in(inpair.first, 0, inpair.second);

	if (type == "KING" || type == "BLOCKER") {
		hit = true;
		return std::make_pair(0,0);
	}

	QVector<QVector3D> ins;
	QVector<QVector3D> outs;

	int i = ((int)rotation)/90;

	//Laser gun
	int in1x = reflectedPairs[i][0][0][0];
	int in1z = reflectedPairs[i][0][0][1];
	int out1x = reflectedPairs[i][0][1][0];
	int out1z = reflectedPairs[i][0][1][1];

	if (type == "LASERGUN") {
		hit = false;
		return std::make_pair(out1x, out1z);
	}

	if (type == "PAWN" || type == "TWOWAY"){

		int in2x = reflectedPairs[i][1][0][0];
		int in2z = reflectedPairs[i][1][0][1];
		int out2x = reflectedPairs[i][1][1][0];
		int out2z = reflectedPairs[i][1][1][1];

		ins << QVector3D(in1x, 0, in1z) << QVector3D(in2x, 0, in2z);
		outs << QVector3D(out1x, 0, out1z) << QVector3D(out2x, 0, out2z);

		if (type == "TWOWAY"){

			int j = (((int)rotation+180) % 360) / 90;

			int in3x = reflectedPairs[j][0][0][0];
			int in3z = reflectedPairs[j][0][0][1];
			int out3x = reflectedPairs[j][0][1][0];
			int out3z = reflectedPairs[j][0][1][1];

			int in4x = reflectedPairs[j][1][0][0];
			int in4z = reflectedPairs[j][1][0][1];
			int out4x = reflectedPairs[j][1][1][0];
			int out4z = reflectedPairs[j][1][1][1];

			ins << QVector3D(in3x, 0, in3z) << QVector3D(in4x, 0, in4z);
			outs << QVector3D(out3x, 0, out3z) << QVector3D(out4x, 0, out4z);

		}
	}

	for (int k=0; k<ins.size(); k++){

		if (ins[k] == in){

			//std::cerr << "Testing ray " << ins[k][0] << " " << ins[k][2] << std::endl;

			hit = false;
			QVector3D out = outs[k];
			return std::make_pair((int)out[0], (int)out[2]);
		}
	}

	hit = true;
	return std::make_pair(0,0);
}

void GamePiece::printInfo(){
	std::cerr << "Type: " << type << ", bx: " << boardx << ", bz: " << boardz << ", rot: " << rotation << std::endl;
	std::cerr << "Reflected Beam Pairs:\n";

	if (type == "KING" || type == "BLOCKER") return;

	//Laser gun
	int i = ((int)rotation)/90;
	int in1x = reflectedPairs[i][0][0][0];
	int in1z = reflectedPairs[i][0][0][1];
	int out1x = reflectedPairs[i][0][1][0];
	int out1z = reflectedPairs[i][0][1][1];

	std::cerr << "-Pair1 - in: (" << in1x << "," << in1z << "), out: ("  << out1x << "," << out1z << ")\n";

	if (type == "PAWN" || type == "TWOWAY"){

		int in2x = reflectedPairs[i][1][0][0];
		int in2z = reflectedPairs[i][1][0][1];
		int out2x = reflectedPairs[i][1][1][0];
		int out2z = reflectedPairs[i][1][1][1];

		std::cerr << "-Pair2 - in: (" << in2x << "," << in2z << "), out: ("  << out2x << "," <<  out2z << ")\n";

		if (type == "TWOWAY"){

			int j = (((int)rotation+180) % 360) / 90;
			int in3x = reflectedPairs[j][0][0][0];
			int in3z = reflectedPairs[j][0][0][1];
			int out3x = reflectedPairs[j][0][1][0];
			int out3z = reflectedPairs[j][0][1][1];
			int in4x = reflectedPairs[j][1][0][0];
			int in4z = reflectedPairs[j][1][0][1];
			int out4x = reflectedPairs[j][1][1][0];
			int out4z = reflectedPairs[j][1][1][1];

			std::cerr << "-Pair3 - in: (" << in3x << "," << in3z << "), out: ("  << out3x << "," << out3z << ")\n";
			std::cerr << "-Pair4 - in: (" << in4x << "," << in4z << "), out: ("  << out4x << "," << out4z << ")\n";
		}

	}
	std::cerr << std::endl;
}

// Animation stuff
void GamePiece::saveInitialPosition(){
	srcWorldTransform = worldTransformMatrix;
	srcModelTransform = modelTransformMatrix;
	srcRotation = rotation;
}

void GamePiece::startAnimation(QVector3D trans, float rot, int nSteps){
	destWorldTransform = worldTransformMatrix;
	destModelTransform = modelTransformMatrix;
	destRotation = rotation;
	worldTransformMatrix = srcWorldTransform;
	modelTransformMatrix = srcModelTransform;
	rotation = srcRotation;
	thetaStep = rot/nSteps;
	translationStep = trans/nSteps;
	isAnimated = true;
	animationStepCount = 0;
	nAnimationSteps = nSteps;
}

void GamePiece::advanceAnimationStep(){
	if (isAnimated && (animationStepCount < nAnimationSteps)){
		modelRotateXZ(thetaStep);
		worldTranslate(translationStep[0], translationStep[1], translationStep[2]);
		animationStepCount += 1;
	} else {
		//std::cerr << "done animating\n";
		isAnimated = false;
		thetaStep = 0.0;
		translationStep = QVector3D(0,0,0);
		animationStepCount = 0;
		nAnimationSteps = 0;
		worldTransformMatrix = destWorldTransform;
		modelTransformMatrix = destModelTransform;
		rotation = destRotation;
	}
}
