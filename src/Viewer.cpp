// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

// #include <GL/glew.h>
#include <QtWidgets>
#include <QtOpenGL>
#include <QVector3D>
#include "Viewer.hpp"
#include <iostream>
#include <math.h>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

const float Viewer::COLOUR_INTENSITIES[][3] = { {1.0f, 0.0f, 0.0f}, //red                                        
                                                {0.0f, 1.0f, 0.0f}, //green
                                                {0.0f, 0.0f, 1.0f}, //blue
                                                {1.0f, 1.0f, 0.0f}, //yellow
                                                {1.0f, 0.0f, 1.0f}, //magenta
                                                {0.0f, 1.0f, 1.0f}, //cyan
                                                {1.0f, 0.5f, 0.0f}, //orange
                                                {0.0f, 0.5f, 0.0f},  //dark green
                                                {0.5f, 0.5f, 0.5f}, //grey
                                                {0.0f, 0.0f, 0.0f} //black
                                                };

Viewer::Viewer(const QGLFormat& format, Game* game, QWidget *parent)
    : QGLWidget(format, parent)
    , lastMouseX(0)
    , secondLastMouseX(0)
    , cumulativeScale(1.0f)
    , cameraPhi(M_PI/2)
    , cameraTheta(0)
    , firstOpenVBOIndex(0)
    , m_game(game)
    , mode(Viewer::REGULAR_MODE)
    , showPerlinTextures(false)
    , showOtherTextures(false)
    , showBoundingVolumes(false)
    , lastHighlightedPiece(NULL)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    , mVBO_Vertices(QOpenGLBuffer::VertexBuffer)
    , mVBO_Normals(QOpenGLBuffer::VertexBuffer)
    , mVBO_2D_Texture_Coords(QOpenGLBuffer::VertexBuffer)
    , mVBO_Laser(QOpenGLBuffer::VertexBuffer)
    , mVertexArrayObject(this)
#else
    , mVertexBufferObject(QGLBuffer::VertexBuffer)
    , mVBO_Normals(QGLBuffer::VertexBuffer)
#endif
{
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(update()));
    mTimer->start(1000/30);

    animationTimer = new QTimer(this);
    connect(animationTimer, SIGNAL(timeout()), this, SLOT(advanceAnimatedPieces()));
    animationTimer->start(75);

    cameraPosition = QVector3D(0, 0, 10.0);
    cameraUpVector = QVector3D(0,1,0);
    cameraRadVector = QVector3D(0,0,-10.0);
    cameraLatVector = QVector3D(1,0,0);

    cameraLookAt = QVector3D(0,0,0);
    moveCameraLongitude(-20.0);

    mLightPosition = QVector3D(1.0, 10.0, -1.0);

    // Get the piece data from game
    gamePieces = m_game->gamePieces;
    boardTiles = m_game->boardTiles;

    // seed rand for Perlin
    srand (static_cast <unsigned> (time(0)));
    resetPerlinGradients();

    bvhtree = game->bvhtree;
}

Viewer::~Viewer() {
}

QSize Viewer::minimumSizeHint() const {
    return QSize(50, 50);
}

QSize Viewer::sizeHint() const {
    return QSize(600, 600);
}

void Viewer::initializeGL() {
    //std::cerr << "GL init in the viewer" << std::endl;
    QGLFormat glFormat = QGLWidget::format();
    if (!glFormat.sampleBuffers()) {
        std::cerr << "Could not enable sample buffers." << std::endl;
        return;
    }

    glClearColor(0.7, 0.7, 1.0, 0.0);

    if (!mProgram.addShaderFromSourceFile(QGLShader::Vertex, "shader.vert")) {
        std::cerr << "Cannot load vertex shader." << std::endl;
        return;
    }

    if (!mProgram.addShaderFromSourceFile(QGLShader::Fragment, "shader.frag")) {
        std::cerr << "Cannot load fragment shader." << std::endl;
        return;
    }

    if ( !mProgram.link() ) {
        std::cerr << "Cannot link shaders." << std::endl;
        return;
    }

    // Create the piece data and fill VBO
    initBaseCubeData();
    initVBOData();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    mVertexArrayObject.create();
    mVertexArrayObject.bind();

    mVBO_Vertices.create();
    mVBO_Vertices.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVBO_Vertices.bind();
    mVBO_Vertices.allocate(allVertexData.constData(), allVertexData.size() * 3 * sizeof(float));
    mVBO_Vertices.release();

    mVBO_Normals.create();
    mVBO_Normals.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVBO_Normals.bind();
    mVBO_Normals.allocate(allNormalData.constData(), allNormalData.size()*3*sizeof(float));
    mVBO_Normals.release();

    mVBO_2D_Texture_Coords.create();
    mVBO_2D_Texture_Coords.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVBO_2D_Texture_Coords.bind();
    mVBO_2D_Texture_Coords.allocate(allTexCoordData.constData(), allTexCoordData.size()*3*sizeof(float));
    mVBO_2D_Texture_Coords.release();

    mVBO_Laser.create();
    mVBO_Laser.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    mVBO_Laser.bind();
    //QVector<QVector3D> path;
    //path << QVector3D(-10.0,1.0,10.0) << QVector3D(10.0,1.0,10.0) << QVector3D(10.0,1.0,-10.0) << QVector3D(-10.0,1.0,-10.0);
    mVBO_Laser.allocate(10*10*3*sizeof(float));
    mVBO_Laser.release();

#else
    /*
     * if qt version is less than 5.1, use the following commented code
     * instead of QOpenGLVertexVufferObject. Also use QGLBuffer instead of
     * QOpenGLBuffer.
     */
    uint vao;

    typedef void (APIENTRY *_glGenVertexArrays) (GLsizei, GLuint*);
    typedef void (APIENTRY *_glBindVertexArray) (GLuint);

    _glGenVertexArrays glGenVertexArrays;
    _glBindVertexArray glBindVertexArray;

    glGenVertexArrays = (_glGenVertexArrays) QGLWidget::context()->getProcAddress("glGenVertexArrays");
    glBindVertexArray = (_glBindVertexArray) QGLWidget::context()->getProcAddress("glBindVertexArray");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    mVertexBufferObject.create();
    mVertexBufferObject.setUsagePattern(QGLBuffer::StaticDraw);
#endif

    if (!mVBO_Vertices.bind()) {
        std::cerr << "could not bind vertex buffer to the context." << std::endl;
        return;
    }

    mProgram.bind();

    mVBO_Vertices.bind();
    mProgram.enableAttributeArray("vert");
    mProgram.setAttributeBuffer("vert", GL_FLOAT, 0, 3);
    mVBO_Vertices.release();

    mVBO_Normals.bind();
    mProgram.enableAttributeArray("normal");
    mProgram.setAttributeBuffer("normal", GL_FLOAT, 0, 3);
    mVBO_Normals.release();

    mVBO_2D_Texture_Coords.bind();
    mProgram.enableAttributeArray("textureCoordinate");
    mProgram.setAttributeBuffer("textureCoordinate", GL_FLOAT, 0, 2);
    mVBO_2D_Texture_Coords.release();

    mVBO_Laser.bind();
    mProgram.enableAttributeArray("laser");
    mProgram.setAttributeBuffer("laser", GL_FLOAT, 0, 3);
    mVBO_Laser.release();

    // mPerspMatrixLocation = mProgram.uniformLocation("cameraMatrix");
    QColor cols[3] = {QColor(128, 128, 128), QColor(128, 128, 128), QColor(255, 255, 255)};
    float coefs[3] = {1.0,1.0,1.0};
    setPhongParameters(cols, coefs, 100.0);

    //// TEXTURE MAPPING
    /*
    http://stackoverflow.com/questions/8818125/opengl-shader-model-3-3-texturing-black-textures
    http://stackoverflow.com/questions/22182781/texture-mapping-with-opengl-and-qt-c
    http://www.cs.uregina.ca/Links/class-info/315/WWW/Lab5/index_gl3.html
    http://qt.apidoc.info/5.2.0/qtopengl/qtopengl-textures-glwidget-cpp.html
    */

    loadTexture("../textures/neongreen.png", "highlightgreen");
    loadTexture("../textures/neonyellow.jpg", "highlightyellow");
    loadTexture("../textures/diamondplate.jpg", "diamondplate");
    loadTexture("../textures/camo.png", "camo");
    loadTexture("../textures/lightwood.jpg", "lightwood");
    loadTexture("../textures/darkwood.jpg", "darkwood");
    loadTexture("../textures/darkred.jpg", "darkred");
    loadTexture("../textures/darkgrey.jpg", "darkgrey");

    // skybox
    loadTexture("../textures/skybox/back.jpg","skybox_back");
    loadTexture("../textures/skybox/front.jpg","skybox_front");
    loadTexture("../textures/skybox/top.jpg","skybox_top");
    loadTexture("../textures/skybox/bottom.jpg","skybox_bottom");
    loadTexture("../textures/skybox/left.jpg","skybox_left");
    loadTexture("../textures/skybox/right.jpg","skybox_right");

    //generateAndLoadTexture("test");
    generateAndLoadPerlinTexture("perlin-whiteblack", 5, 0.95, QVector3D(255, 255, 255));
    generateAndLoadPerlinTexture("perlin-red", 8, 0.50, QVector3D(255, 0, 0));
    generateAndLoadPerlinTexture("perlin-blue", 8, 0.5, QVector3D(0, 0, 255));
    generateAndLoadPerlinTexture("perlin-purple", 5, 0.75, QVector3D(128, 0, 128));
    mProgram.setUniformValue("texture", 0);

    // Init the game board and pieces
    initBoard();

    glEnable(GL_DEPTH_TEST); 
    glEnable(GL_TEXTURE_2D);

    float val = 1.0;
    mProgram.setUniformValue("USE_LIGHTING", val);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    mVertexArrayObject.bind();
#endif

}

void Viewer::initBaseCubeData (){
    // 1. Vertices
    CUBE_VERTICES
    // Face 1 Back
    << QVector3D(0.0f, 0.0f, 0.0f)
    << QVector3D(1.0f, 0.0f, 0.0f)
    << QVector3D(0.0f, 1.0f, 0.0f)

    << QVector3D(1.0f, 1.0f, 0.0f)
    << QVector3D(1.0f, 0.0f, 0.0f)
    << QVector3D(0.0f, 1.0f, 0.0f)

    // Face 2 Front
    << QVector3D(0.0f, 0.0f, 1.0f)
    << QVector3D(1.0f, 0.0f, 1.0f)
    << QVector3D(0.0f, 1.0f, 1.0f)

    << QVector3D(1.0f, 1.0f, 1.0f)
    << QVector3D(1.0f, 0.0f, 1.0f)
    << QVector3D(0.0f, 1.0f, 1.0f)

    // Face 3 Left
    << QVector3D(0.0f, 0.0f, 0.0f)
    << QVector3D(0.0f, 0.0f, 1.0f)
    << QVector3D(0.0f, 1.0f, 0.0f)

    << QVector3D(0.0f, 1.0f, 1.0f)
    << QVector3D(0.0f, 0.0f, 1.0f)
    << QVector3D(0.0f, 1.0f, 0.0f)

    // Face 4 Right
    << QVector3D(1.0f, 0.0f, 0.0f)
    << QVector3D(1.0f, 0.0f, 1.0f)
    << QVector3D(1.0f, 1.0f, 0.0f)

    << QVector3D(1.0f, 1.0f, 1.0f)
    << QVector3D(1.0f, 0.0f, 1.0f)
    << QVector3D(1.0f, 1.0f, 0.0f)

    // Face 5 Bottom
    << QVector3D(0.0f, 0.0f, 0.0f)
    << QVector3D(0.0f, 0.0f, 1.0f)
    << QVector3D(1.0f, 0.0f, 0.0f)

    << QVector3D(1.0f, 0.0f, 1.0f)
    << QVector3D(0.0f, 0.0f, 1.0f)
    << QVector3D(1.0f, 0.0f, 0.0f)

    // Face 6 Top
    << QVector3D(0.0f, 1.0f, 0.0f)
    << QVector3D(0.0f, 1.0f, 1.0f)
    << QVector3D(1.0f, 1.0f, 0.0f)

    << QVector3D(1.0f, 1.0f, 1.0f)
    << QVector3D(0.0f, 1.0f, 1.0f)
    << QVector3D(1.0f, 1.0f, 0.0f);

    // 2 Normals
    CUBE_NORMALS
    // Face 1 Back
    << QVector3D(0.0, 0.0, -1.0)
    << QVector3D(0.0, 0.0, -1.0)
    << QVector3D(0.0, 0.0, -1.0)
    << QVector3D(0.0, 0.0, -1.0)
    << QVector3D(0.0, 0.0, -1.0)
    << QVector3D(0.0, 0.0, -1.0)

    // Face 2 Front
    << QVector3D(0.0, 0.0, 1.0)
    << QVector3D(0.0, 0.0, 1.0)
    << QVector3D(0.0, 0.0, 1.0)
    << QVector3D(0.0, 0.0, 1.0)
    << QVector3D(0.0, 0.0, 1.0)
    << QVector3D(0.0, 0.0, 1.0)

    // Face 3 Left
    << QVector3D(-1.0, 0.0, 0.0)
    << QVector3D(-1.0, 0.0, 0.0)
    << QVector3D(-1.0, 0.0, 0.0)
    << QVector3D(-1.0, 0.0, 0.0)
    << QVector3D(-1.0, 0.0, 0.0)
    << QVector3D(-1.0, 0.0, 0.0)

    // Face 4 Right
    << QVector3D(1.0, 0.0, 0.0)
    << QVector3D(1.0, 0.0, 0.0)
    << QVector3D(1.0, 0.0, 0.0)
    << QVector3D(1.0, 0.0, 0.0)
    << QVector3D(1.0, 0.0, 0.0)
    << QVector3D(1.0, 0.0, 0.0)

    // Face 5 Bottom
    << QVector3D(0.0, -1.0, 0.0)
    << QVector3D(0.0, -1.0, 0.0)
    << QVector3D(0.0, -1.0, 0.0)
    << QVector3D(0.0, -1.0, 0.0)
    << QVector3D(0.0, -1.0, 0.0)
    << QVector3D(0.0, -1.0, 0.0)

    // Face 6 Top
    << QVector3D(0.0, 1.0, 0.0)
    << QVector3D(0.0, 1.0, 0.0)
    << QVector3D(0.0, 1.0, 0.0)
    << QVector3D(0.0, 1.0, 0.0)
    << QVector3D(0.0, 1.0, 0.0)
    << QVector3D(0.0, 1.0, 0.0);

    // 3 Tex Coords
    CUBE_TEX_COORDS
    // Face 1 Back
    << QVector2D(1.0, 0.0)
    << QVector2D(0.0, 0.0)
    << QVector2D(1.0, 1.0)
    << QVector2D(0.0, 1.0)
    << QVector2D(0.0, 0.0)
    << QVector2D(1.0, 1.0)
    // Face 2 Front
    << QVector2D(0.0, 0.0)
    << QVector2D(1.0, 0.0)
    << QVector2D(0.0, 1.0)
    << QVector2D(1.0, 1.0)
    << QVector2D(1.0, 0.0)
    << QVector2D(0.0, 1.0)
    // Face 3 Left
    << QVector2D(0.0, 0.0)
    << QVector2D(1.0, 0.0)
    << QVector2D(0.0, 1.0)
    << QVector2D(1.0, 1.0)
    << QVector2D(1.0, 0.0)
    << QVector2D(0.0, 1.0)
    // Face 4 Right
    << QVector2D(1.0, 0.0)
    << QVector2D(0.0, 0.0)
    << QVector2D(1.0, 1.0)
    << QVector2D(0.0, 1.0)
    << QVector2D(0.0, 0.0)
    << QVector2D(1.0, 1.0)
    // Face 5 Bottom
    << QVector2D(1.0, 1.0)
    << QVector2D(1.0, 0.0)
    << QVector2D(0.0, 1.0)
    << QVector2D(0.0, 0.0)
    << QVector2D(1.0, 0.0)
    << QVector2D(0.0, 1.0)
    // Face 6 Top
    << QVector2D(0.0, 1.0)
    << QVector2D(0.0, 0.0)
    << QVector2D(1.0, 1.0)
    << QVector2D(1.0, 0.0)
    << QVector2D(0.0, 0.0)
    << QVector2D(1.0, 1.0);
}

void Viewer::initVBOData(){
    initSkyboxCubeData();
    initTileData();
    initBlockerData();
    initTwoWayData();
    initBaseTetrahedronData();
    initBaseSphereData();
    initBaseCylinderData();
}

void Viewer::initSkyboxCubeData(){
    QMatrix4x4 vertTrans;

    vertTrans.translate(-10.0, -10.0, -10.0);
    vertTrans.scale(20.0, 20.0, 20.0);
    for (int i=0; i<CUBE_VERTICES.size(); i+=1){
        allVertexData << vertTrans*CUBE_VERTICES[i];
    }

    allNormalData += (CUBE_NORMALS); // this doesn't matter

    // map texture to opposite sides to get the right tex coords
    // since the skybox is inside out
    for (int i=0; i<6; i++){
        allTexCoordData << CUBE_TEX_COORDS[i+6];
    }
    for (int i=0; i<6; i++){
        allTexCoordData << CUBE_TEX_COORDS[i];
    }

    for (int i=0; i<6; i++){
        allTexCoordData << CUBE_TEX_COORDS[i+6+12];
    }
    for (int i=0; i<6; i++){
        allTexCoordData << CUBE_TEX_COORDS[i+12];
    }

    for (int i=0; i<6; i++){
        allTexCoordData << CUBE_TEX_COORDS[i+6+24];
    }
    for (int i=0; i<6; i++){
        allTexCoordData << CUBE_TEX_COORDS[i+24];
    }

    vboDataStart.push_back(firstOpenVBOIndex);
    vboDataNElements.push_back(CUBE_VERTICES.size());
    firstOpenVBOIndex += CUBE_VERTICES.size();
}

void Viewer::initTileData(){
    QMatrix4x4 vertTrans;
    vertTrans.scale(1.0, 0.33, 1.0);
    for (int i=0; i<CUBE_VERTICES.size(); i+=1){
        allVertexData << vertTrans*CUBE_VERTICES[i];
    }

    allNormalData += (CUBE_NORMALS);
    allTexCoordData += (CUBE_TEX_COORDS);

    vboDataStart.push_back(firstOpenVBOIndex);
    vboDataNElements.push_back(CUBE_VERTICES.size());
    firstOpenVBOIndex += CUBE_VERTICES.size();
}

void Viewer::initBlockerData(){
    QMatrix4x4 vertTrans;
    vertTrans.scale(0.8, 1.0, 0.8);
    vertTrans.translate(0.1, 0, 0.1);
    for (int i=0; i<CUBE_VERTICES.size(); i+=1){
        allVertexData << vertTrans*CUBE_VERTICES[i];
    }

    allNormalData += (CUBE_NORMALS);
    allTexCoordData += (CUBE_TEX_COORDS);

    vboDataStart.push_back(firstOpenVBOIndex);
    vboDataNElements.push_back(CUBE_VERTICES.size());
    firstOpenVBOIndex += CUBE_VERTICES.size();
}

void Viewer::initTwoWayData(){
    QMatrix4x4 vtrans1, vtrans2;

    // Base
    vtrans1.scale(0.8, 0.1, 0.8);
    vtrans1.translate(0.1, 0, 0.1);
    for (int i=0; i<CUBE_VERTICES.size(); i+=1){
        allVertexData << vtrans1*CUBE_VERTICES[i];
    }
    allNormalData += (CUBE_NORMALS);
    allTexCoordData += (CUBE_TEX_COORDS);

    // Angled piece
    QMatrix4x4 vtrans2Norm;
    vtrans2Norm.rotate(-45.0, 0, 1, 0);
    vtrans2.rotate(-45.0, 0, 1, 0);
    vtrans2.scale(1.0, 1.0, 0.1);
    vtrans2.translate(0.2, 0, -0.05);
    for (int i=0; i<CUBE_VERTICES.size(); i+=1){
        allVertexData << vtrans2*CUBE_VERTICES[i];
        allNormalData << vtrans2Norm*CUBE_NORMALS[i];
    }
    allTexCoordData += (CUBE_TEX_COORDS);

    vboDataStart.push_back(firstOpenVBOIndex);
    vboDataNElements.push_back(2*CUBE_VERTICES.size());
    firstOpenVBOIndex += 2*CUBE_VERTICES.size();
}

void Viewer::initBaseTetrahedronData(){

    // FIRST, get all vertices & coord data
    QMatrix4x4 vertTrans;
    vertTrans.scale(0.8, 0.9, 0.8);
    vertTrans.translate(0.1,0.1,0.1);

    allVertexData
    // Face 1 reflective side
    << vertTrans * QVector3D(0, 0, 0)
    << vertTrans * QVector3D(0.5, 1, 0.5)
    << vertTrans * QVector3D(1, 0, 1)

    // Face 2 left solid
    << vertTrans * QVector3D(0, 0, 0)
    << vertTrans * QVector3D(0.5, 1, 0.5)
    << vertTrans * QVector3D(0, 0, 1)

    // Face 3 front solid
    << vertTrans * QVector3D(0, 0, 1)
    << vertTrans * QVector3D(0.5, 1, 0.5)
    << vertTrans * QVector3D(1, 0, 1)

    // Face 4 bottom
    << vertTrans * QVector3D(1, 0, 1)
    << vertTrans * QVector3D(0, 0, 1)
    << vertTrans * QVector3D(0, 0, 0);

    QVector<QVector3D> tetraNorms;
    QVector3D edgeBackLeft = QVector3D(0, 0, 0) - vertTrans * QVector3D(0.5, 1, 0.5);
    QVector3D edgeFrontLeft = QVector3D(0, 0, 1) - vertTrans * QVector3D(0.5, 1, 0.5);
    QVector3D edgeFrontRight = QVector3D(1, 0, 1) - vertTrans * QVector3D(0.5, 1, 0.5);
    tetraNorms  << QVector3D(1, 0, -1) // 1
                << QVector3D::normal(edgeBackLeft, edgeFrontLeft) // 2
                << QVector3D::normal(edgeFrontLeft, edgeFrontRight) // 3
                << QVector3D(0,-1,0); //4 
    
    for (int i=0; i<4; i++){

        for (int j=0; j<3; j++){
            allNormalData << tetraNorms[i];
        }

        allTexCoordData << QVector2D(0,0) << QVector2D(0,1) << QVector2D(1,0);
    }

    // Base
    QMatrix4x4 baseTrans;
    baseTrans.scale(0.8, 0.1, 0.8);
    baseTrans.translate(0.1, 0, 0.1);
    for (int i=0; i<CUBE_VERTICES.size(); i+=1){
        allVertexData << baseTrans*CUBE_VERTICES[i];
    }
    allNormalData += (CUBE_NORMALS);
    allTexCoordData += (CUBE_TEX_COORDS);

    // NEXT set vbo data
    vboDataStart.push_back(firstOpenVBOIndex);
    vboDataNElements.push_back(12+CUBE_VERTICES.size());
    firstOpenVBOIndex += 12+CUBE_VERTICES.size();
}

void Viewer::initBaseSphereData(){
    // Approximate the sphere using spherical coords
    int nLatFaces = 25;
    int nLongFaces = 25;
    float sRad = 0.5;
    float thetaInc = 2*M_PI / nLongFaces;
    float phiInc = M_PI / nLatFaces;

    int nSpherePoints = nLatFaces*2*(nLongFaces+1);

    QMatrix4x4 vertTrans;
    vertTrans.scale(0.9, 0.9, 0.9);
    vertTrans.translate(sRad, sRad+0.1, sRad);
    // Need to split rect texture up to wrap around FIX THIS!!!!

    // Generate the points for the sphere
    for (int p=0; p < nLatFaces; p++){

        float phiT = p*phiInc;
        float phiB = (p+1)*phiInc;

        float yB = sRad * cos(phiB);
        float yT = sRad * cos(phiT);

        QVector<QVector2D> stripTexCoords;

        for (int t=0; t <= nLongFaces; t++){ //because of zigzag need to re-write first 2 vertices
            float thetaL = t * thetaInc;
            float thetaR = (t+1) * thetaInc;

            // T/L and B/R
            float xTL = sRad * sin(phiT) * sin(thetaL);
            float zTL = sRad * sin(phiT) * cos(thetaL);
            allVertexData << vertTrans * QVector3D(xTL, yT, zTL);
            allNormalData << QVector3D(xTL, yT, zTL);
            allTexCoordData << QVector2D(((float)t)/nLongFaces, ((float)p+1)/nLatFaces);

            float xBR = sRad * sin(phiB) * sin(thetaR);
            float zBR = sRad * sin(phiB) * cos(thetaR);
            allVertexData << vertTrans * QVector3D(xBR, yB, zBR);
            allNormalData << QVector3D(xBR, yB, zBR);
            allTexCoordData << QVector2D(((float)t+1)/nLongFaces, ((float)p)/nLatFaces);
        }

        /*
        std::cerr << "\nTEX COORDS for STRIP\n:";
        for (int i=0; i<stripTexCoords.size(); i++){
            std::cerr << " x " << stripTexCoords[i][0] << " y " << stripTexCoords[i][1] << std::endl;
        }
        allTexCoordData += stripTexCoords;
        */
    }

    // Base
    QMatrix4x4 baseTrans;
    baseTrans.scale(0.8, 0.1, 0.8);
    baseTrans.translate(0.1, 0, 0.1);
    for (int i=0; i<CUBE_VERTICES.size(); i+=1){
        allVertexData << baseTrans*CUBE_VERTICES[i];
    }
    allNormalData += (CUBE_NORMALS);
    allTexCoordData += (CUBE_TEX_COORDS);

    // NEXT set vbo data
    vboDataStart.push_back(firstOpenVBOIndex);
    vboDataNElements.push_back(nSpherePoints+CUBE_VERTICES.size());
    firstOpenVBOIndex += nSpherePoints+CUBE_VERTICES.size();
}

void Viewer::initBaseCylinderData(){

    int nCircumferenceFaces = 25;
    float radius = 0.5;
    float thetaInc = 2*M_PI / nCircumferenceFaces;
    QMatrix4x4 vertTrans;
    vertTrans.scale(0.9,0.9,0.8);
    vertTrans.translate(radius, radius+0.1, 0.1);

    float zFront = 1.0;
    float zBack = 0.0;

    // Pass 1: middle
    for (int t=0; t <= nCircumferenceFaces; t++){ //because of zigzag need to re-write first 2 vertices
        float theta1 = t * thetaInc;
        float theta2 = (t+1) * thetaInc;

        float xFront1 = radius * cos(theta1);
        float yFront1 = radius * sin(theta1);
        allVertexData << vertTrans * QVector3D(xFront1, yFront1, zFront);
        allNormalData << QVector3D(xFront1, yFront1, 0);

        float xBack2 = radius * cos(theta2);
        float yBack2 = radius * sin(theta2);
        allVertexData << vertTrans * QVector3D(xBack2, yBack2, zBack);
        allNormalData << QVector3D(xBack2, yBack2, 0);

        allTexCoordData << QVector2D(((float)t)/nCircumferenceFaces, zFront)
                        << QVector2D(((float)t+1)/nCircumferenceFaces, zBack);
    }

    // Pass 2: ends
    for (int p=0; p<2; p++){
        float zdep, norm, x1, x2, y1, y2;
        if (p==0) {
            zdep = 0;
            norm = -1;
        } else {
            zdep = 1;
            norm = 1;
        }
        for (int t=0; t <= nCircumferenceFaces; t++){
            float theta = t * thetaInc;

            float x1 = 0;
            float y1 = 0;
            allVertexData << vertTrans * QVector3D(x1, y1, zdep);

            float x2 = radius * cos(theta);
            float y2 = radius * sin(theta);
            allVertexData << vertTrans * QVector3D(x2, y2, zdep);

            allNormalData << QVector3D(0,0,norm) << QVector3D(0,0,norm);

            allTexCoordData << QVector2D(0.5, 0.5)
                            << QVector2D(( (x2/radius) +1)/2, ( (y2/radius) +1)/2);
        }
    }

    // Add base
    QMatrix4x4 baseTrans;
    baseTrans.scale(0.8, 0.1, 0.8);
    baseTrans.translate(0.1, 0, 0.1);

    // Get rid of artifacts caused by triangle strip
    allVertexData << baseTrans*QVector3D(0.5, 1, 0.5) << baseTrans*QVector3D(0.5, 1, 0.5);
    allNormalData << QVector3D(0,1,0) << QVector3D(0,1,0);
    allTexCoordData << QVector2D(0,0) << QVector2D(1,1);

    for (int i=0; i<CUBE_VERTICES.size(); i+=1){
        allVertexData << baseTrans*CUBE_VERTICES[i];
    }
    allNormalData += (CUBE_NORMALS);
    allTexCoordData += (CUBE_TEX_COORDS);

    // NEXT set vbo data
    vboDataStart.push_back(firstOpenVBOIndex);
    vboDataNElements.push_back(6*(nCircumferenceFaces+1) + 2 + CUBE_VERTICES.size());
    firstOpenVBOIndex += 6*(nCircumferenceFaces+1) + 2 + CUBE_VERTICES.size();
}

void Viewer::initBoard(){
    std::string tex, perlinTex, otherTex;
    int bx, bz, player, vboS, vboN;
    GLenum drawMode;
    perlinTex = "";

    // Board Tiles
    for (int i=0; i<boardTiles->size(); i++){
        Piece* t = (*boardTiles)[i];
        bx = t->boardx;
        bz = t->boardz;

        if ((bx+bz)%2 == 0){
            tex = "lightwood";
            perlinTex = "perlin-purple";
            otherTex = "lightwood";

        } else {
            tex = "darkwood";
            perlinTex = "perlin-blue";
            otherTex = "darkwood";
        }

        t->setViewerData(GL_TRIANGLES, tex, perlinTex, otherTex, vboDataStart[Viewer::TILE],
                                        vboDataNElements[Viewer::TILE]);
        t->worldTranslate((float)bx-5, -0.33, (float)bz-4);
        t->setStartingPositions();
    }

    // Board Pieces
    for (int i=0; i<gamePieces->size(); i++){
        GamePiece* p = (*gamePieces)[i];
        bx = p->boardx;
        bz = p->boardz;

        if (p->player == 1){
            tex = "darkgrey";
            perlinTex = "perlin-whiteblack";
            otherTex = "diamondplate";
        } else {
            tex = "darkred";
            perlinTex = "perlin-red";
            otherTex = "camo";
        }

        if (p->type == "BLOCKER"){
            drawMode = GL_TRIANGLES;
            vboS = vboDataStart[Viewer::BLOCKER];
            vboN = vboDataNElements[Viewer::BLOCKER];
        }
        else if (p->type == "TWOWAY"){
            drawMode = GL_TRIANGLES;
            vboS = vboDataStart[Viewer::TWOWAY];
            vboN = vboDataNElements[Viewer::TWOWAY];
        }
        else if (p->type == "PAWN"){
            drawMode = GL_TRIANGLES;
            vboS = vboDataStart[Viewer::PAWN];
            vboN = vboDataNElements[Viewer::PAWN];
        }
        else if (p->type == "KING"){
            drawMode = GL_TRIANGLE_STRIP;
            vboS = vboDataStart[Viewer::KING];
            vboN = vboDataNElements[Viewer::KING];
            //perlinTex = "";
        }
        else if (p->type == "LASERGUN"){
            drawMode = GL_TRIANGLE_STRIP;
            vboS = vboDataStart[Viewer::LASERGUN];
            vboN = vboDataNElements[Viewer::LASERGUN];
            //perlinTex = "";
        }
        p->modelRotateXZ(p->origRotation);
        p->worldTranslate((float)bx-5, 0.0, (float)bz-4);
        p->setViewerData(drawMode, tex, perlinTex, otherTex, vboS, vboN);
        p->setStartingPositions();
        //p->printInfo();
    }
}

void Viewer::drawSkybox(){
    glDisable(GL_DEPTH_TEST);
    float val = 0.0;
    mProgram.setUniformValue("USE_LIGHTING", val);

    mProgram.setUniformValue("mvpMatrix", getCameraMatrixNoTranslation());

    setTexture("skybox_back");
    glDrawArrays(GL_TRIANGLES, vboDataStart[Viewer::SKYBOX], 6);

    setTexture("skybox_front");
    glDrawArrays(GL_TRIANGLES, vboDataStart[Viewer::SKYBOX]+6, 6);

    setTexture("skybox_left");
    glDrawArrays(GL_TRIANGLES, vboDataStart[Viewer::SKYBOX]+12, 6);

    setTexture("skybox_right");
    glDrawArrays(GL_TRIANGLES, vboDataStart[Viewer::SKYBOX]+18, 6);

    setTexture("skybox_bottom");
    glDrawArrays(GL_TRIANGLES, vboDataStart[Viewer::SKYBOX]+24, 6);

    setTexture("skybox_top");
    glDrawArrays(GL_TRIANGLES, vboDataStart[Viewer::SKYBOX]+30, 6);

    val = 1.0;
    mProgram.setUniformValue("USE_LIGHTING", val);
    glEnable(GL_DEPTH_TEST);
}

void Viewer::drawBoard(){
    for (int i=0; i<boardTiles->size(); i++){
        drawPiece((*boardTiles)[i]);
    }
    for (int i=0; i<gamePieces->size(); i++){
        drawPiece((*gamePieces)[i]); /// MAY CAUSE TROUBLE..
    }
}

void Viewer::drawPiece(Piece* p){
    if (!p->isVisible) return;

    if (mode == Viewer::HIGHLIGHT_MODE && p->isHighlighted){
        setTexture("highlightyellow");
    }
    else if (mode == Viewer::SELECTED_MODE && p->isSelected){
        setTexture("highlightgreen");
    }
    else if (showPerlinTextures) {
        setTexture(p->perlinTexture);
    }
    else if (showOtherTextures){
        setTexture(p->otherTexture);
    }
    else {
        setTexture(p->texture);
    }

    if (p->isAnimated){
        if (animatedPiece1 == NULL){
            animatedPiece1 = p;
        } else {
            animatedPiece2 = p;
        }
    }

    QMatrix4x4 modelTransform = p->getMWTransform();
    QMatrix4x4 mvMatrix = getViewMatrix() * modelTransform;
    QMatrix3x3 normalMatrix = mvMatrix.normalMatrix();

    mProgram.setUniformValue("mvpMatrix", getCameraMatrix() * modelTransform);
    mProgram.setUniformValue("mvMatrix", mvMatrix);
    mProgram.setUniformValue("normalMatrix", normalMatrix);
    mProgram.setUniformValue("lightPosition", getViewMatrix()*mLightPosition);

    glDrawArrays(p->drawMode, p->vboStart, p->vboN);
}

// use laser buffer and laser stuff as model for BVH drawing
QVector<QVector3D> Viewer::getDataForVolume(BVHNode* n){
    QVector<QVector3D> data;

    float x0 = n->w_x0;
    float x1 = n->w_x1;
    float y0 = n->w_y0;
    float y1 = 2.0;
    float z0 = n->w_z0;
    float z1 = n->w_z1;

    data 
    << QVector3D(x0,y0,z0) << QVector3D(x1,y0,z0)
    << QVector3D(x0,y0,z1) << QVector3D(x1,y0,z1)
    << QVector3D(x0,y0,z0) << QVector3D(x0,y0,z1)
    << QVector3D(x1,y0,z0) << QVector3D(x1,y0,z1)

    << QVector3D(x0,y1,z0) << QVector3D(x1,y1,z0)
    << QVector3D(x0,y1,z1) << QVector3D(x1,y1,z1)
    << QVector3D(x0,y1,z0) << QVector3D(x0,y1,z1)
    << QVector3D(x1,y1,z0) << QVector3D(x1,y1,z1)

    << QVector3D(x0,y0,z0) << QVector3D(x0,y1,z0)
    << QVector3D(x1,y0,z0) << QVector3D(x1,y1,z0)
    << QVector3D(x0,y0,z1) << QVector3D(x0,y1,z1)
    << QVector3D(x1,y0,z1) << QVector3D(x1,y1,z1);

    return data;
}

void Viewer::updateBVHData(){
    Piece* p = m_game->highlightedPiece;
    if (p != NULL && p->type != "TILE"){
        setBoundingVolumeData(p);
    }
}

void Viewer::setBoundingVolumeData(Piece* p){
    bvhPath = bvhtree->getPathToPiece(p);
    BVHNode* n;

    //std::cerr << "\nBVH PATH\n";
    bvhPathData.clear();
    for (int i=0; i<bvhPath->size(); i++){
        n = (*bvhPath)[i];
        bvhPathData += getDataForVolume(n);
        //std::cerr << i << " vol bounds x0 " << n->b_x0 << " z0 " << n->b_z0 << " x1 " << n->b_x1 << " z1 " << n->b_z1 << std::endl; 
    }

    /*
    std::cerr << "BVH PATH WORLD\n";
    for (int i=0; i<bvhPathData.size(); i++){
        std::cerr << bvhPathData[i][0] << " " 
                    << bvhPathData[i][1] << " " 
                    << bvhPathData[i][2] << std::endl; 
    }
    */
    mVBO_Laser.bind();
    mVBO_Laser.write(0,bvhPathData.constData(), bvhPathData.size()*3*sizeof(float));
    mVBO_Laser.release();
}

void Viewer::drawBoundingVolumes(int nVols){
    int nPoints;
    if (nVols<0) {
        nPoints = bvhPathData.size();
    } else {
        nPoints = nVols*24; //12 edges * 2 points per
    }
    float val = 1.0;
    mProgram.setUniformValue("DRAW_LASER", val);

    setFaceColour(0.0, 0.0, 1.0);

    QMatrix4x4 mvMatrix = getViewMatrix();
    QMatrix3x3 normalMatrix = mvMatrix.normalMatrix();

    mProgram.setUniformValue("mvpMatrix", getCameraMatrix());
    mProgram.setUniformValue("mvMatrix", mvMatrix);
    mProgram.setUniformValue("normalMatrix", normalMatrix);
    mProgram.setUniformValue("lightPosition", getViewMatrix()*mLightPosition);

    glDrawArrays(GL_LINES, 0, bvhPathData.size());

    val = 0.0;
    mProgram.setUniformValue("DRAW_LASER", val);

}

void Viewer::updateLaserData(int player){
    currentLaserPath = m_game->getWorldCoordsLaserPath(player);
    /*
    std::cerr << "PATH WORLD\n";
    for (int i=0; i<currentLaserPath.size(); i++){
        std::cerr << currentLaserPath[i][0] << " " 
                    << currentLaserPath[i][1] << " " 
                    << currentLaserPath[i][2] << std::endl; 
    }
    */
    //currentLaserPath.clear();
    //currentLaserPath << QVector3D(0, 0.5, 0) << QVector3D(5,0.5,5);
    mVBO_Laser.bind();
    mVBO_Laser.write(0,currentLaserPath.constData(), currentLaserPath.size()*3*sizeof(float));
    mVBO_Laser.release();
}

void Viewer::drawLaser(int player){

    float val = 1.0;
    mProgram.setUniformValue("DRAW_LASER", val);

    if (player == 1){
        setFaceColour(1.0,0.0,0.0);
    }
    else if (player == 2){
        setFaceColour(0.0,1.0,0.0);
    }

    QMatrix4x4 mvMatrix = getViewMatrix();
    QMatrix3x3 normalMatrix = mvMatrix.normalMatrix();

    mProgram.setUniformValue("mvpMatrix", getCameraMatrix());
    mProgram.setUniformValue("mvMatrix", mvMatrix);
    mProgram.setUniformValue("normalMatrix", normalMatrix);
    mProgram.setUniformValue("lightPosition", getViewMatrix()*mLightPosition);

    glDrawArrays(GL_LINE_STRIP, 0, currentLaserPath.size());

    val = 0.0;
    mProgram.setUniformValue("DRAW_LASER", val);
}

void Viewer::reset(){
    mTransformMatrix.setToIdentity();
    lastMouseX = 0;
    secondLastMouseX = 0;
    mode = ViewerMode::REGULAR_MODE;
    showOtherTextures = false;
    showPerlinTextures = false;
    showBoundingVolumes = false;
}

unsigned int Viewer::loadTexture(std::string filename, std::string texname){
    QImage img;
    QString qfile = QString::fromStdString(filename);
    if(!img.load(qfile)){
        std::cerr << "ERROR couldn't load image " << filename << std::endl;
    }

    QImage image = QGLWidget::convertToGLFormat(img);
    //std::cerr << "Image format " << image.format() << std::endl;
    if(!image.isNull())
    {
        // Load up a single texture.
        //std::cerr << "Image loaded\n";
        GLuint newTex;
        glGenTextures(1, &newTex);
        glBindTexture(GL_TEXTURE_2D, newTex);
        textures[texname] = newTex;
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
        glBindTexture(GL_TEXTURE_2D, 0);
    } 

    return textures.size() - 1;
}

void Viewer::generateAndLoadTexture(std::string name){
    int width = 250;
    int height = 250;
    QImage img(width, height, QImage::Format_ARGB32);
    QRgb value;
    for (int i=0; i<width; i++){
        for (int j=0; j<height; j++){
            value = qRgb(0,0,(i+j)/2);
            img.setPixel(i,j,value);
        }
    }

    QImage image = QGLWidget::convertToGLFormat(img);
    std::cerr << "Image format " << image.format() << std::endl;
    if(!image.isNull())
    {
        // Load up a single texture.
        std::cerr << "CUSTOM image loaded\n";
        GLuint newTex;
        glGenTextures(1, &newTex);
        glBindTexture(GL_TEXTURE_2D, newTex);
        textures[name] = newTex;
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Viewer::resetPerlinGradients(){
    for (int i=0; i<PERLIN_GRID_DIM; i++){
        for (int j=0; j<PERLIN_GRID_DIM; j++){
            perlinGradients[i][j][0] = 0.0;
            perlinGradients[i][j][1] = 0.0;
        }
    }
}

void Viewer::setPerlinGradients(){
    for (int i=0; i<PERLIN_GRID_DIM; i++){
        for (int j=0; j<PERLIN_GRID_DIM; j++){
            perlinGradients[i][j][0] = getRandom();
            perlinGradients[i][j][1] = getRandom();
        }
    }
}

float Viewer::linearInterpolate(float a, float b, float w){
    return a*(1.0-w) + w*b;
}

float Viewer::getRandom(){
    float randn = -1.0 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/2.0) );
    //std::cerr << "rand is making " << randn << std::endl;
    return randn;
}

float Viewer::dotGridGradient(float x, float y, int gridx, int gridy, float gradx, float grady){
    float dx = x - (int)gridx;
    float dy = y - (int)gridy;
    return (dx*gradx) + (dy*grady);
}

void Viewer::generateAndLoadPerlinTexture(std::string name, int nOctaves, float persistence, QVector3D baseColour){
    int width = 250;
    int height = 250;
    QImage img(width, height, QImage::Format_ARGB32);
    QRgb colourValue;

    // Generate the Perlin coefs by looping for each octave
    float perlinCoefs[width][height] = {{0.0}};
    float maxPerlinCoef = std::numeric_limits<float>::min();
    float minPerlinCoef = std::numeric_limits<float>::max();

    for (int oct=0; oct<nOctaves; oct++){

        // Compute the gradient vectors for the corners of the grid
        setPerlinGradients();

        // Set up frequency and amplitude
        float freq = (float)pow(2.0, (double)oct);
        float amp = (float)pow((double)persistence, (double)oct); 

        // Calculate the perlin cooef for each pixel
        for (int i=0; i<width; i++){
            for (int j=0; j<height; j++){

                // Compute x & y as floats
                float x = (((float)i)/width + 1.0/(2*width))*freq;
                float y = (((float)j)/height + 1.0/(2*height))*freq;

                // Determine the corners of the grid surrounding the
                int x0 = (int)(floor((double)x));
                int y0 = (int)(floor((double)y));
                int x1 = (int)(ceil((double)x));
                int y1 = (int)(ceil((double)y));

                // Compute influence of each random gradient
                float s = dotGridGradient(x,y,x0,y0,perlinGradients[x0][y0][0],perlinGradients[x0][y0][1]);
                float t = dotGridGradient(x,y,x1,y0,perlinGradients[x1][y0][0],perlinGradients[x1][y0][1]);
                float u = dotGridGradient(x,y,x0,y1,perlinGradients[x0][y1][0],perlinGradients[x0][y1][1]);
                float v = dotGridGradient(x,y,x1,y1,perlinGradients[x1][y1][0],perlinGradients[x1][y1][1]);

                // Interpolate using weights
                int xint = (int)x;
                int yint = (int)y;
                float xfrac = x - (float)xint;
                float yfrac = y - (float)yint;

                float p1 = linearInterpolate(s,t,xfrac);
                float p2 = linearInterpolate(u,v,xfrac);
                float value = linearInterpolate(p1,p2,yfrac);

                perlinCoefs[i][j] += value*amp;
                //std::cerr << "perlin value " << value << std::endl;

                // Update the max so we can normalize later
                if (perlinCoefs[i][j] > maxPerlinCoef){
                    maxPerlinCoef = perlinCoefs[i][j];
                }
                // Update the max so we can normalize later
                if (perlinCoefs[i][j] < minPerlinCoef){
                    minPerlinCoef = perlinCoefs[i][j];
                }
            }
        }

    }

    //std::cerr << "max perlin value " << maxPerlinCoef << std::endl;
    //std::cerr << "min perlin value " << minPerlinCoef << std::endl;
    
    // Fill the image by applying the base colour * normalized Perlin coefficient
    float baseR = baseColour[0];
    float baseG = baseColour[1];
    float baseB = baseColour[2];

    for (int i=0; i<width; i++){
        for (int j=0; j<height; j++){

            float pCoef = (perlinCoefs[i][j] - minPerlinCoef)/(maxPerlinCoef - minPerlinCoef);
            //std::cerr << "pCoef " << pCoef << std::endl;
            int perlinR = (int)(baseR * pCoef);
            int perlinG = (int)(baseG * pCoef);
            int perlinB = (int)(baseB * pCoef);
            //std::cerr << "Perlin cols R " << perlinR << " G " << perlinG << " B " << perlinB << std::endl;
            colourValue = qRgb(perlinR, perlinG, perlinB);
            img.setPixel(i,j,colourValue);
        }
    }

    // Create texture from the image
    QImage image = QGLWidget::convertToGLFormat(img);
    if(!image.isNull())
    {
        // Load up a single texture.
        //std::cerr << "Perlin texture loaded\n";
        GLuint newTex;
        glGenTextures(1, &newTex);
        glBindTexture(GL_TEXTURE_2D, newTex);
        textures[name] = newTex;
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Viewer::togglePerlinDisplay(){
    showPerlinTextures = !showPerlinTextures;
    showOtherTextures = false;
    showBoundingVolumes = false;
}

void Viewer::toggleOtherTextureDisplay(){
    showOtherTextures = !showOtherTextures;
    showPerlinTextures = false;
    showBoundingVolumes = false;
}

void Viewer::toggleBVHDisplay(){
    showBoundingVolumes = !showBoundingVolumes;
    showPerlinTextures = false;
    showOtherTextures = false;
}

void Viewer::setTexture(std::string texname){
    //glActiveTexture(GL_TEXTURE0);
    float val = 0.0;
    mProgram.setUniformValue("USE_FACE_COLOUR", val);
    glBindTexture(GL_TEXTURE_2D, textures[texname]);
}

void Viewer::setFaceColour(float red, float green, float blue) {
    float val = 1.0;
    mProgram.setUniformValue("faceColour", red, green, blue);
    mProgram.setUniformValue("USE_FACE_COLOUR", val);
}

void Viewer::setRenderColour(int c) {
    const float* intensity = COLOUR_INTENSITIES[c];
    setFaceColour(intensity[0], intensity[1], intensity[2]);
}

void Viewer::setPhongParameters(QColor colours[], float coefs[], float shininess){
    mProgram.setUniformValue("ambientColour", colours[0]);
    mProgram.setUniformValue("diffuseColour", colours[1]);
    mProgram.setUniformValue("specularColour", colours[2]);
    mProgram.setUniformValue("ambientReflection", (GLfloat) coefs[0]);
    mProgram.setUniformValue("diffuseReflection", (GLfloat) coefs[1]);
    mProgram.setUniformValue("specularReflection", (GLfloat) coefs[2]);
    mProgram.setUniformValue("shininess", (GLfloat) shininess);
}

void Viewer::paintGL() {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawSkybox();

    // Change camera focus
    if (mode == Viewer::HIGHLIGHT_MODE){
        //focusCamera(m_game->highlightCentreWorld);
    }
    else if (mode == Viewer::SHOW_LASER_MODE){
        drawLaser((m_game->currentPlayer%2)+1);
    } 
    else {
        resetCameraFocus();
    }

    drawBoard();

    if (mode == Viewer::HIGHLIGHT_MODE
        && showBoundingVolumes 
        && m_game->highlightedPiece != NULL 
        && (m_game->highlightedPiece->type != "TILE")){

        if (m_game->highlightedPiece != lastHighlightedPiece){
            //std::cerr << (m_game->highlightedPiece)->type << std::endl;
            setBoundingVolumeData(m_game->highlightedPiece);
        }

        drawBoundingVolumes();
    }
    lastHighlightedPiece = m_game->highlightedPiece;
}

void Viewer::resizeGL(int width, int height) {
    if (height == 0) {
        height = 1;
    }

    mPerspMatrix.setToIdentity();
    mPerspMatrix.perspective(60.0, (float) width / (float) height, 0.001, 1000);

    glViewport(0, 0, width, height);
}

void Viewer::mousePressEvent ( QMouseEvent * event ) {
    //std::cerr << "Stub: button " << event->button() << " pressed\n";
    int button = event->button();
    lastMouseX = event->x();
    secondLastMouseX = lastMouseX;
    switch (button) {
        case Qt::LeftButton:
            mouseButtonDown[0] = true;
            break;
        case Qt::MidButton:
            mouseButtonDown[1] = true;
            break;
        case Qt::RightButton:
            mouseButtonDown[2] = true;
            break;
        default:
            QWidget::mousePressEvent(event);
            break;
    }
}

void Viewer::mouseReleaseEvent ( QMouseEvent * event ) {
    //std::cerr << "Stub: x " << event->x() << " released\n";
    int button = event->button();
    switch (button) {
        case Qt::LeftButton:
            mouseButtonDown[0] = false;
            break;
        case Qt::MidButton:
            mouseButtonDown[1] = false;
            break;
        case Qt::RightButton:
            mouseButtonDown[2] = false;
            break;
        default:
            QWidget::mousePressEvent(event);
            break;
    }
}

void Viewer::mouseMoveEvent ( QMouseEvent * event ) {
    //std::cerr << "Stub: Motion at " << event->button() << " , " << event->x() << ", " << event->y() << std::endl;
    int mouseX = event->x();
    float floatDiff = mouseX - lastMouseX;

    if (mouseButtonDown[0]) {
        moveCameraLatitude(floatDiff);

    } else if (mouseButtonDown[1]) {
        moveCameraZoom(floatDiff/10);

    } else if (mouseButtonDown[2]) {
        moveCameraLongitude(floatDiff);

    } else {
        QWidget::mouseMoveEvent(event);
    }

    secondLastMouseX = lastMouseX;
    lastMouseX = mouseX;
    update();
}

void Viewer::focusCamera(QVector3D p){
    cameraLookAt = p;
}

void Viewer::resetCameraFocus(){
    cameraLookAt = QVector3D(0,0,0);
}

void Viewer::updateCameraPosition(){
    float r = cameraRadVector.length();
    float z = r * cos(cameraTheta) * sin(cameraPhi);
    float x = r * sin(cameraTheta) * sin(cameraPhi);
    float y = r * cos(cameraPhi);
    cameraPosition = QVector3D(x,y,z);
    cameraRadVector = -1*cameraPosition;
    cameraLatVector[1] = 0.0;
}

void Viewer::moveCameraLatitude(float angle){
    if (angle == 0.0) return;
    float angleRad = (M_PI*angle/180);
    cameraTheta += (M_PI*angle/180);
    cameraTheta = fmod(cameraTheta, 2*M_PI);
    updateCameraPosition();

    // Update lateral vector via Rodrigues formula
    QVector3D v = cameraLatVector;
    QVector3D y = QVector3D(0,1,0);
    QVector3D crossterm = QVector3D::crossProduct(y,v);
    float dotterm = QVector3D::dotProduct(y,v);
    cameraLatVector = (cos(angleRad)*v) + (sin(angleRad)*crossterm)+ (dotterm*(1-cos(angleRad))*y);

    cameraUpVector = QVector3D::crossProduct(cameraLatVector, cameraRadVector);
    cameraLatVector[1] = 0.0;
}

void Viewer::moveCameraLongitude(float angle){
    if (angle == 0.0) return;
    float testAngle = cameraPhi + (M_PI*angle/180);
    if ((testAngle < 0.0) || (testAngle > M_PI)) {
        return;
    }
    cameraPhi += (M_PI*angle/180);
    cameraPhi = fmod(cameraPhi, M_PI);
    updateCameraPosition();
    // Affects up vector but not lat vector
    cameraUpVector = QVector3D::crossProduct(cameraLatVector, cameraRadVector);
}

void Viewer::moveCameraZoom(float dist){
    if (dist == 0.0) return;
    QVector3D normalizedRadVector = cameraRadVector;
    normalizedRadVector.normalize();
    cameraRadVector += -1*dist * normalizedRadVector;
    updateCameraPosition();
}

void Viewer::moveCameraRoll(float angle){
}

void Viewer::flipCameraPlayer(int player){
    cameraTheta = 0.0;
    cameraPhi = M_PI/2;
    cameraPosition = QVector3D(0, 0, 10.0);
    cameraUpVector = QVector3D(0,1,0);
    cameraRadVector = QVector3D(0,0,-10.0);
    cameraLatVector = QVector3D(1,0,0);
    cameraLookAt = QVector3D(0,0,0);
    moveCameraLongitude(-25.0);
    if (player == 1){
        moveCameraLatitude(180);
    }
}

QMatrix4x4 Viewer::getCameraMatrix() {
    QMatrix4x4 vMatrix;

    vMatrix.lookAt(cameraPosition, cameraLookAt, cameraUpVector);

    return mPerspMatrix * vMatrix * mTransformMatrix;
}

QMatrix4x4 Viewer::getCameraMatrixNoTranslation(){
    QMatrix4x4 vMatrix;
    QVector3D newCameraPos = 10*cameraPosition.normalized();

    vMatrix.lookAt(newCameraPos, cameraLookAt, cameraUpVector);

    return mPerspMatrix * vMatrix * mTransformMatrix;
}

QMatrix4x4 Viewer::getViewMatrix() {
    QMatrix4x4 vMatrix;
    vMatrix.lookAt(cameraPosition, cameraLookAt, cameraUpVector);
    return vMatrix;
}

void Viewer::translateWorld(float x, float y, float z) {
    mTransformMatrix.translate(x, y, z);
}

void Viewer::rotateWorld(float angle, float x, float y, float z) {
    mLastRotation[0] = angle;
    mLastRotation[1] = x;
    mLastRotation[2] = y;
    mLastRotation[3] = z;
    mTransformMatrix.rotate(angle, x, y, z);
}

void Viewer::scaleWorld(float x, float y, float z) {
    mTransformMatrix.scale(x, y, z);
}

void Viewer::advanceAnimatedPieces(){
    //std::cerr << "attempting animation\n";
    if (animatedPiece1 != NULL){
        if (animatedPiece1->isAnimated){
            //std::cerr << "Advancing piece1\n";
            animatedPiece1->advanceAnimationStep();
        } else {
            animatedPiece1 = NULL;
        }
    }
    if (animatedPiece2 != NULL){
        if (animatedPiece2->isAnimated){
            //std::cerr << "Advancing piece2\n";
            animatedPiece2->advanceAnimationStep();
        } else {
            animatedPiece2 = NULL;
        }
    }
    update();
}
