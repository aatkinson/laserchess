// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

#ifndef GAME_HPP
#define GAME_HPP

#include <QGLWidget>
#include <QMatrix4x4>
#include <QtGlobal>
#include <vector>
#include <string>
#include <sstream>

#include "BVH.hpp"
#include "Piece.hpp"
#include "Sound.hpp"

class Game {
public:
	Game(SoundFX* sfx, QLabel* label);
	~Game();
	void reset();
	void initPiecesGrid();

	bool rotateSelectedPiece(int dir, bool animate=true);
	bool moveSelectedPiece(int dx, int dz, bool animate=true);

	void moveHighlight(int dx, int dz);
	void highlightPiece(int x, int z);
	void resetHighlight();
	bool selectHighlightedPiece();
	void deselectPiece();

	void switchTurn();

	void fireLaser(int player);
	void printLaserPath(int player);
	QVector<QVector3D> getWorldCoordsLaserPath(int player);

	std::vector<Piece*>* boardTiles;
	std::vector<GamePiece*>* gamePieces;
	QVector3D highlightCentreWorld;
	int currentPlayer;
	std::vector<std::pair<int,int>> laserPath1;
	std::vector<std::pair<int,int>> laserPath2;
	bool gameover;
	int loser;
	int winner;
	BVHTree* bvhtree;
	Piece* highlightedPiece;
	
private:
	void fireLaserBase(int player, std::vector<std::pair<int,int>>& path );
	void printLaserPathBase(int player, std::vector<std::pair<int,int>>& path );
    QLabel* status_label;
	GamePiece * piecesGrid[10][8];
	Piece * tilesGrid[10][8];
	int highlightx;
	int highlightz;
	int selectedx;
	int selectedz;
	GamePiece* selectedPiece;
	SoundFX* soundfx;
};

#endif