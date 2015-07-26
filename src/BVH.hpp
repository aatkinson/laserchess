// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

#ifndef BVH_HPP
#define BVH_HPP

#include <vector>
#include <QMatrix4x4>
#include <QtGlobal>

#include "Piece.hpp"

class BVHNode {

public:
	BVHNode(int a,int b,int c,int d);
	void constructDescendants(int level, std::vector<BVHNode*>* alloced, std::vector<BVHNode*>* leaves);
	void insert(Piece* p);
	void remove(Piece* p, std::vector<BVHNode*>* leaves);
	bool isPieceWithinVolume(Piece* p);
	void findPiece(Piece* p, std::vector<BVHNode*>* path);
	BVHNode* children[5];
	Piece* pieceContained;
	bool isLeaf;
	bool isStrip;

	int b_x0;
	int b_z0;
	int b_x1;
	int b_z1;

	float w_x0;
	float w_x1;
	float w_z0;
	float w_z1;
	float w_y0;
	float w_y1;
};

class BVHTree {
public:
	BVHTree();
	~BVHTree();
	void insertPiece(Piece* p);
	void removePiece(Piece* p);
	Piece* getIntersectedPiece(QVector3D ray);
	void clearPieces();
	void fillPieces(std::vector<GamePiece*>* pieces);
	std::vector<BVHNode*>* getPathToPiece(Piece* p);

private:
	BVHNode* root;
	std::vector<BVHNode*>* allocedNodes;
	std::vector<BVHNode*>* leafNodes;
	std::vector<BVHNode*>* lastPath;
};

#endif
