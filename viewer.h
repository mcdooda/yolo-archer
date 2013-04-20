#ifndef VIEWER_H
#define VIEWER_H

// GLEW lib: needs to be included first!!
#include <GL/glew.h> 

// OpenGL library 
#include <GL/gl.h>

// OpenGL Utility library
#include <GL/glu.h>

// OpenGL Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <QGLFormat>
#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <stack>

#include "camera.h"
#include "shader.h"
#include "grid.h"

class Viewer : public QGLWidget {
 public:
  Viewer(char *filename,const QGLFormat &format=QGLFormat::defaultFormat());
  ~Viewer();
  
 protected :
  GLuint loadTexture2D(const char* filename);
  virtual void paintGL();
  virtual void drawSceneFromCamera();
  virtual void drawSceneFromLight();
  virtual void initializeGL();
  virtual void resizeGL(int width,int height);
  virtual void keyPressEvent(QKeyEvent *ke);
  virtual void mousePressEvent(QMouseEvent *me);
  virtual void mouseMoveEvent(QMouseEvent *me);

 private:
  // OpenGL objects creation
  void generateIds();
  void cleanIds();
  void initFBO();
  void initVBO();
  void createShaders();

  // init a shader (called to get all variable locations)
  void initShaders();

  QTimer        *_timer;    // timer that controls the animation
  unsigned int   _currentshader; // current shader index

  Grid   *_grid;   // the grid
  Camera *_cam;    // the camera

  glm::vec3 _light;  // light direction
  glm::vec3 _motion; // motion offset for the noise texture 
  bool      _mode;   // camera motion or light motion
  bool      _showShadowMap;

  std::vector<std::string> _vertexFilenames;   // all vertex filenames
  std::vector<std::string> _fragmentFilenames; // all fragment filenames
  std::vector<Shader *>    _shaders;           // all the shaders 

  // location ids for the noise texture pass 
  GLint _noiseVertexLoc;
  GLint _noiseMotionLoc;

  GLint _gridVertexLoc;

  // vbo ids 
  GLuint _quad;
  GLuint _gridBufferPosition;
  GLuint _gridBufferFaces;
  
  // noise resolution
  unsigned int _ndResol;
  unsigned int _shadowMapResol;

  //fbo
  GLuint _fbo;
  GLuint _fbo2;
  GLuint _fbo3;


  GLuint _texHeight;
  GLuint _texDepthLight;
  GLuint _texDepthCamera;
  GLuint _texSlant;
  GLuint _texPosition;
  GLuint _texPositionLight;
  GLuint _texNormal;

  GLuint _texColor;

  GLint _texHeightLoc;
  GLint _texHeightGBuffersLoc;
  GLint _texDepthGBuffersLoc;
  GLint _mvpMatGBuffersLoc;
  GLint _texHeightShadowLoc;
  GLint _rendMdvMatLoc;
  GLint _rendProjMatLoc;
  GLint _rendNormalMatLoc;
  GLint _rendLightLoc;
  GLint _shadMvpMatLoc;
  GLint _shadGridVertexLoc;

  GLint _texSlantLoc;
  GLint _texNormalLoc;
  GLint _texPositionLoc;
  GLint _texPositionLightLoc;
  GLint _texColorLoc;

  GLint _texSlantRenderLoc;
  GLint _texNormalRenderLoc;
  GLint _texPositionRenderLoc;
  GLint _texPositionLightRenderLoc;
  GLint _texDepthCameraRenderLoc;
  GLint _texDepthLightRenderLoc;
  GLint _lightLoc;

};

#endif // VIEWER_H
