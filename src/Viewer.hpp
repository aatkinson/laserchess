// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

#ifndef VIEWER_HPP
#define VIEWER_HPP

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <QGLWidget>
#include <QGLShaderProgram>
#include <QMatrix4x4>
#include <QtGlobal>
#include <QPixmap>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#else 
#include <QGLBuffer>
#endif

#include "BVH.hpp"
#include "Game.hpp"
#include "Piece.hpp"


class Viewer : public QGLWidget {
    
    Q_OBJECT

public:
    Viewer(const QGLFormat& format, Game* m_game, QWidget *parent = 0);
    virtual ~Viewer();
    
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    static const float COLOUR_INTENSITIES[][3];

    enum ColourName {
        RED,
        GREEN,
        BLUE,
        YELLOW,
        MAGENTA,
        CYAN,
        ORANGE,
        DARKGREEN,
        GREY,
        BLACK
    };

    enum PieceID {
        SKYBOX,
        TILE, // slab
        BLOCKER, // cube
        TWOWAY, // 2 slabs
        PAWN, //tetrahedron
        KING, //sphere
        LASERGUN //cylinder
    };

    enum ViewerMode {
        REGULAR_MODE,
        HIGHLIGHT_MODE,
        SELECTED_MODE,
        SHOW_LASER_MODE,
        SHOW_BVH_MODE
    };

    // If you want to render a new frame, call do not call paintGL(),
    // instead, call update() to ensure that the view gets a paint 
    // event.

    virtual void setRenderColour(int c);
    virtual void reset();
    virtual void focusCamera(QVector3D p);
    virtual void resetCameraFocus();
    virtual void flipCameraPlayer(int player);
    virtual void updateLaserData(int player);
    virtual void drawLaser(int player);
    virtual void updateBVHData();

    // mode and state stuff
    void togglePerlinDisplay();
    void toggleOtherTextureDisplay();
    void toggleBVHDisplay();
    bool showPerlinTextures;
    bool showOtherTextures;
    bool showBoundingVolumes;
    ViewerMode mode;

protected:

    // Events we implement

    // Called when GL is first initialized
    virtual void initializeGL();
    // Called when our window needs to be redrawn
    virtual void paintGL();
    // Called when the window is resized (formerly on_configure_event)
    virtual void resizeGL(int width, int height);
    // Called when a mouse button is pressed
    virtual void mousePressEvent ( QMouseEvent * event );
    // Called when a mouse button is released
    virtual void mouseReleaseEvent ( QMouseEvent * event );
    // Called when the mouse moves
    virtual void mouseMoveEvent ( QMouseEvent * event );

private slots:
    void advanceAnimatedPieces();

private:

    QMatrix4x4 getCameraMatrix();
    QMatrix4x4 getCameraMatrixNoTranslation();
    QMatrix4x4 getViewMatrix(); 
    void translateWorld(float x, float y, float z);
    void rotateWorld(float angle, float x, float y, float z);
    void scaleWorld(float x, float y, float z);
    //void drawCube (QMatrix4x4& modelTransform);

    void updateCameraPosition();
    void moveCameraLatitude(float angle);
    void moveCameraLongitude(float angle);
    void moveCameraZoom(float dist);
    void moveCameraRoll(float angle);

    void setPhongParameters(QColor colours[], float coefs[], float shininess);
    
    void setFaceColour(float red, float green, float blue);

    // Returns the texture unit/index in vector
    unsigned int loadTexture(std::string filename, std::string texname);
    void generateAndLoadTexture(std::string name);
    void setTexture(std::string name);

    // Perlin
    float perlinGradients[251][251][2];
    int PERLIN_GRID_DIM = 251;
    void setPerlinGradients();
    void resetPerlinGradients();
    float getRandom();
    float linearInterpolate(float a, float b, float w);
    float dotGridGradient(float x, float y, int gridx, int gridy, float gradx, float grady);
    void generateAndLoadPerlinTexture(std::string name, int nOctaves, float persistence, QVector3D baseColour);

    // BVH
    void setBoundingVolumeData(Piece* p);
    QVector<QVector3D> getDataForVolume(BVHNode* n);
    void drawBoundingVolumes(int nVols=-1);
    Piece* lastHighlightedPiece;

    // Primitives
    void initVBOData();
    void initBaseCubeData();
    void initBaseTetrahedronData();
    void initBaseSphereData();
    void initBaseCylinderData();

    void initSkyboxCubeData();
    void initTileData();
    void initBlockerData();
    void initTwoWayData();
    void drawSkybox();
    void initBoard();
    void drawBoard();
    void drawPiece(Piece* p);
    std::vector<Piece*>* boardTiles;
    std::vector<GamePiece*>* gamePieces;

    QVector<QVector3D> CUBE_VERTICES;
    QVector<QVector3D> CUBE_NORMALS;
    QVector<QVector2D> CUBE_TEX_COORDS;

    QVector<QVector3D> allVertexData;
    QVector<QVector3D> allNormalData;
    QVector<QVector2D> allTexCoordData;
    QVector<QVector3D> currentLaserPath;
    std::vector<unsigned int> vboDataStart;
    std::vector<unsigned int> vboDataNElements;
    int firstOpenVBOIndex;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    QOpenGLBuffer mVBO_Vertices;
    QOpenGLBuffer mVBO_Normals;
    QOpenGLBuffer mVBO_2D_Texture_Coords;
    QOpenGLBuffer mVBO_Laser;
    QOpenGLVertexArrayObject mVertexArrayObject;
#else 
    QGLBuffer mVertexBufferObject;
#endif
    QMatrix4x4 mPerspMatrix;
    QMatrix4x4 mTransformMatrix;

    QMatrix4x4 mCameraTransformMatrix;
    QVector3D cameraPosition;
    QVector3D cameraLookAt;
    QVector3D cameraEyeVector;
    QVector3D cameraUpVector;
    QVector3D cameraLatVector;
    QVector3D cameraRadVector; //NOT EYE VECTOR!!
    float cameraTheta;
    float cameraPhi;
    float cameraRoll;

    QVector3D mLightPosition;

    std::map<std::string, GLuint> textures;

    float mLastRotation[4] = {0,0,0,0};
    QTimer* mTimer;
    QTimer* animationTimer;
    QGLShaderProgram mProgram;
    bool mouseButtonDown[3] = {false, false, false};
    int lastMouseX;
    int secondLastMouseX;
    float cumulativeScale;
    Piece* animatedPiece1;
    Piece* animatedPiece2;
    Game* m_game;
    BVHTree* bvhtree;
    std::vector<BVHNode*>* bvhPath;
    QVector<QVector3D> bvhPathData;
};

#endif
