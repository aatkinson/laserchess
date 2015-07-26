// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

#include "BVH.hpp"

// TREE
BVHTree::BVHTree()
: root(NULL)
{
	allocedNodes = new std::vector<BVHNode*>();
	leafNodes =  new std::vector<BVHNode*>();
	lastPath = new std::vector<BVHNode*>();
	root = new BVHNode(0,0,10,8);
	allocedNodes->push_back(root);
	root->constructDescendants(1, allocedNodes, leafNodes);

	//std::cerr << "n nodes " << allocedNodes->size() << std::endl;
	//std::cerr << "n leaves " << leafNodes->size() << std::endl;
}

BVHTree::~BVHTree(){
	// Because I am lazy..
	for (int i=0; i<allocedNodes->size(); i++){
		delete (*allocedNodes)[i];
	}
	delete allocedNodes;
	delete leafNodes;
}

void BVHTree::insertPiece(Piece* p){
	root->insert(p);
}

void BVHTree::removePiece(Piece* p){
	root->remove(p, leafNodes);
}

Piece* BVHTree::getIntersectedPiece(QVector3D ray){
	lastPath->clear();
	return NULL;

}

void BVHTree::clearPieces(){
	for (int i=0; i<leafNodes->size(); i++){
		(*leafNodes)[i]->pieceContained = NULL;
	}
}

void BVHTree::fillPieces(std::vector<GamePiece*>* pieces){
	for (int i=0; i<pieces->size(); i++){
		Piece* p = (*pieces)[i];
		insertPiece(p);
	}
}

std::vector<BVHNode*>* BVHTree::getPathToPiece(Piece* p){
	lastPath->clear();
	lastPath->push_back(root);
	root->findPiece(p, lastPath);
	return lastPath;
}

// NODE
BVHNode::BVHNode(int x0, int z0, int x1, int z1)
: pieceContained(NULL)
, isLeaf(false)
, isStrip(false)
,b_x0(x0)
,b_z0(z0)
,b_x1(x1)
,b_z1(z1)
{
	children[0] = NULL;
	children[1] = NULL;
	children[2] = NULL;
	children[3] = NULL;
	children[4] = NULL;

	w_x0 = ((float)x0) - 5.0;
	w_y0 = 0.0;
	w_z0 = ((float)z0) - 4.0;

	w_x1 = ((float)x1) - 5.0;
	w_y1 = 1.0;
	w_z1 = ((float)z1) - 4.0;
}

void BVHNode::constructDescendants(int level, std::vector<BVHNode*>* alloced, std::vector<BVHNode*>* leaves){

	BVHNode* c0;
	BVHNode* c1;
	BVHNode* c2;
	BVHNode* c3;
	BVHNode* c4;

	// handle strip case separately
	if (!isStrip){
		int x0 = b_x0;
		int z0 = b_z0;
		int xd = (b_x1 - b_x0)/2;
		int zd = (b_z1 - b_z0)/2;

		int x1 = x0 + xd;
		int z1 = z0 + zd;

		c0 = new BVHNode(x0, z0, x0+xd, z0+zd);
		alloced->push_back(c0);
		children[0] = c0;

		c1 = new BVHNode(x1, z0, x1+xd, z0+zd);
		alloced->push_back(c1);
		children[1] = c1;

		c2 = new BVHNode(x0, z1, x0+xd, z1+zd);
		alloced->push_back(c2);
		children[2] = c2;

		c3 = new BVHNode(x1, z1, x1+xd, z1+zd);
		alloced->push_back(c3); 
		children[3] = c3;

		if (level != 3){
			c0->constructDescendants(level+1, alloced, leaves);
			c1->constructDescendants(level+1, alloced, leaves);
			c2->constructDescendants(level+1, alloced, leaves);
			c3->constructDescendants(level+1, alloced, leaves);
		} 

		//Tricky/strip case
		if (level == 2){
			c4 = new BVHNode(x1+xd, b_z0, x1+xd+1, b_z1);
			c4->isStrip = true;
			children[4] = c4;
			alloced->push_back(c4);
			c4->constructDescendants(level+1, alloced, leaves);
		}
	}
	else {
		//level should == 3
		if (level !=3 ){
			std::cerr << "LEVEL NOT 3 and STRIP?!?!\n";
		}

		c0 = new BVHNode(b_x0, b_z0, b_x0+1, b_z0+1);
		alloced->push_back(c0);
		children[0] = c0;

		c1 = new BVHNode(b_x0, b_z0+1, b_x0+1, b_z0+2);
		alloced->push_back(c1);
		children[1] = c1;

		c2 = new BVHNode(b_x0, b_z0+2, b_x0+1, b_z0+3);
		alloced->push_back(c2);
		children[2] = c2;

		c3 = new BVHNode(b_x0, b_z0+3, b_x0+1, b_z0+4);
		alloced->push_back(c3);
		children[3] = c3; 

	}

	if (level == 3){
		c0->isLeaf = true;
		leaves->push_back(c0);
		c1->isLeaf = true;
		leaves->push_back(c1);
		c2->isLeaf = true;
		leaves->push_back(c2);
		c3->isLeaf = true;
		leaves->push_back(c3);
	}
}

bool BVHNode::isPieceWithinVolume(Piece* p){
	float x0 = (float)b_x0;
	float z0 = (float)b_z0;
	float x1 = (float)b_x1;
	float z1 = (float)b_z1;

	float px = ((float)p->boardx)+0.5;
	float pz = ((float)p->boardz)+0.5;
	return ((x0<=px) && (px<=x1) && (z0<=pz) && (pz<=z1));
}

void BVHNode::insert(Piece* p){
	if (isPieceWithinVolume(p)){

		if (isLeaf){
			// what if a piece is already there??
			if (pieceContained != NULL){
				std::cerr << "ACK! a piece was already at this leaf in the BVH tree!\n";
				std::cerr << "Piece type " << pieceContained->type << std::endl;
				std::cerr << "is strip? " << isStrip << std::endl;
				std::cerr << "vol bounds x0 " << b_x0 << " z0 " << b_z0 << " x1 " << b_x1 << " z1 " << b_z1 << std::endl; 
			}
			pieceContained = p;
			//std::cerr << "successfully added piece to BVH tree\n";
		}
		else {
			children[0]->insert(p);
			children[1]->insert(p);
			children[2]->insert(p);
			children[3]->insert(p);
			if (children[4] != NULL){
				children[4]->insert(p);
			}
		}
	}
}

void BVHNode::remove(Piece* p, std::vector<BVHNode*>* leaves){
	for (int i=0; i<leaves->size(); i++){

		Piece* curPiece = ((*leaves)[i])->pieceContained;
		if (curPiece == p){
			//std::cerr << "successfully removed piece from BVH tree\n";
			((*leaves)[i])->pieceContained = NULL;
			return;
		}
	}
}

void BVHNode::findPiece(Piece* p, std::vector<BVHNode*>* path){

	if (isPieceWithinVolume(p) && isLeaf) return;

	if (children[0]->isPieceWithinVolume(p)){
		path->push_back(children[0]);
		children[0]->findPiece(p, path);
	}

	if (children[1]->isPieceWithinVolume(p)){
		path->push_back(children[1]);
		children[1]->findPiece(p, path);
	}

	if (children[2]->isPieceWithinVolume(p)){
		path->push_back(children[2]);
		children[2]->findPiece(p, path);
	}

	if (children[3]->isPieceWithinVolume(p)){
		path->push_back(children[3]);
		children[3]->findPiece(p, path);
	}

	if (children[4] != NULL){
		if (children[4]->isPieceWithinVolume(p)){
			path->push_back(children[4]);
			children[4]->findPiece(p, path);
		}
	}
}
