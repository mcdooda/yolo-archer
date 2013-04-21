#include "viewer.h"

#include <math.h>
#include <iostream>
#include <QTime>

using namespace std;

Viewer::Viewer(char *,const QGLFormat &format)
    : QGLWidget(format),
      _timer(new QTimer(this)),
      _currentshader(0),
      _light(glm::vec3(0,0,1)),
      _motion(glm::vec3(0,0,0)),
      _mode(false),
      _showShadowMap(false),
      _ndResol(512),
      _shadowMapResol(512) {

    setlocale(LC_ALL,"C");

    _grid = new Grid(_ndResol,-1.0f,1.0f);
    _cam  = new Camera(1.0f,glm::vec3(0.0f,0.0f,0.0f));

    _timer->setInterval(10);
    connect(_timer,SIGNAL(timeout()),this,SLOT(updateGL()));
}

Viewer::~Viewer() {
    delete _timer;
    delete _grid;
    delete _cam;

    for(unsigned int i=0;i<_shaders.size();++i) {
        delete _shaders[i];
    }

    cleanIds();
}

void Viewer::generateIds() {
    // init your FBOs here
    glGenFramebuffers(1,&_fbo);
    glGenFramebuffers(1,&_fbo2);
    glGenFramebuffers(1,&_fbo3);
    glGenTextures(1,&_texHeight);
    glGenTextures(1,&_texDepthLight);
    glGenTextures(1,&_texDepthCamera);
    glGenTextures(1,&_texPosition);
    glGenTextures(1,&_texPositionLight);
    glGenTextures(1,&_texSlant);
    glGenTextures(1,&_texVisibility);
    glGenTextures(1,&_texNormal);
    // VBOs
    glGenBuffers(1,&_quad);
    glGenBuffers(1,&_gridBufferFaces);
    glGenBuffers(1,&_gridBufferPosition);

    // generate your ids here

}

void Viewer::cleanIds() {
    // FBOs
    glDeleteFramebuffers(1,&_fbo);
    glDeleteFramebuffers(1,&_fbo2);
    glDeleteFramebuffers(1,&_fbo3);
    // textures
    glDeleteTextures(1,&_texHeight);
    glDeleteTextures(1,&_texDepthLight);
    glDeleteTextures(1,&_texDepthCamera);
    glDeleteTextures(1,&_texPosition);
    glDeleteTextures(1,&_texPositionLight);
    glDeleteTextures(1,&_texSlant);
    glDeleteTextures(1,&_texVisibility);
    glDeleteTextures(1,&_texNormal);
    // VBOs
    glDeleteBuffers(1,&_quad);
    glDeleteBuffers(1,&_gridBufferFaces);
    glDeleteBuffers(1,&_gridBufferPosition);
}

void Viewer::initFBO() {

    _texColor = loadTexture2D("textures/mountain2.png");

    // tex height
    glBindTexture(GL_TEXTURE_2D,_texHeight);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, _ndResol, _ndResol,0,GL_RGBA,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER,_fbo);
    glBindTexture(GL_TEXTURE_2D,_texHeight);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,_texHeight,0);

    // tex depth
    glBindTexture(GL_TEXTURE_2D,_texDepthLight);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24, _shadowMapResol, _shadowMapResol,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

    glBindFramebuffer(GL_FRAMEBUFFER,_fbo2);
    glBindTexture(GL_TEXTURE_2D,_texDepthLight);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,_texDepthLight,0);


    //FBO 2

    // tex normal
    glBindTexture(GL_TEXTURE_2D,_texNormal);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, width(), height(),0,GL_RGBA,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER,_fbo3);
    glBindTexture(GL_TEXTURE_2D,_texNormal);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,_texNormal,0);

    // tex position
    glBindTexture(GL_TEXTURE_2D,_texPosition);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, width(), height(),0,GL_RGBA,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER,_fbo3);
    glBindTexture(GL_TEXTURE_2D,_texPosition);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT2,GL_TEXTURE_2D,_texPosition,0);

    // tex position light
    glBindTexture(GL_TEXTURE_2D,_texPositionLight);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, width(), height(),0,GL_RGBA,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER,_fbo3);
    glBindTexture(GL_TEXTURE_2D,_texPositionLight);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT3,GL_TEXTURE_2D,_texPositionLight,0);

    // tex slant
    glBindTexture(GL_TEXTURE_2D,_texSlant);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED, width(), height(),0,GL_RED,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER,_fbo3);
    glBindTexture(GL_TEXTURE_2D,_texSlant);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT4,GL_TEXTURE_2D,_texSlant,0);

    // tex visibility
    glBindTexture(GL_TEXTURE_2D,_texVisibility);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED, width(), height(),0,GL_RED,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER,_fbo3);
    glBindTexture(GL_TEXTURE_2D,_texVisibility);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT5,GL_TEXTURE_2D,_texVisibility,0);


    //tex depth camera
    glBindTexture(GL_TEXTURE_2D,_texDepthCamera);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24, width(), height(),0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

    glBindFramebuffer(GL_FRAMEBUFFER,_fbo3);
    glBindTexture(GL_TEXTURE_2D,_texDepthCamera);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,_texDepthCamera,0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){
        cout << "probleme lors de la creation du fbo" << endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER,0);
}


void Viewer::initVBO() {
    // create your VBOs here

    glBindBuffer(GL_ARRAY_BUFFER,_gridBufferPosition);
    glBufferData(GL_ARRAY_BUFFER, _grid->nbVertices() * 3 * sizeof(float),_grid->vertices(),GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_gridBufferFaces); // indices
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,_grid->nbFaces()*3*sizeof(int),_grid->faces(),GL_STATIC_DRAW);

    // create the VBO associated with the screen quad
    const GLfloat quadData[] = {
        -1.0f,-1.0f,0.0f, 1.0f,-1.0f,0.0f, -1.0f,1.0f,0.0f, -1.0f,1.0f,0.0f, 1.0f,-1.0f,0.0f, 1.0f,1.0f,0.0f };
    glBindBuffer(GL_ARRAY_BUFFER,_quad); // vertices
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData),quadData,GL_STATIC_DRAW);
}

void Viewer::createShaders() {
    // *** height field ***
    _vertexFilenames.push_back("shaders/noise.vert");
    _fragmentFilenames.push_back("shaders/noise.frag");
    // ******************************

    // add your shaders here
    _vertexFilenames.push_back("shaders/show-noise-map.vert");
    _fragmentFilenames.push_back("shaders/show-noise-map.frag");

    _vertexFilenames.push_back("shaders/shadowmap.vert");
    _fragmentFilenames.push_back("shaders/shadowmap.frag");

    _vertexFilenames.push_back("shaders/gbuffers.vert");
    _fragmentFilenames.push_back("shaders/gbuffers.frag");

    _vertexFilenames.push_back("shaders/render.vert");
    _fragmentFilenames.push_back("shaders/render.frag");


}

void Viewer::initShaders() {
    // height field
    glUseProgram(_shaders[0]->id());
    _noiseVertexLoc = glGetAttribLocation (_shaders[0]->id() ,"position");
    _noiseMotionLoc = glGetUniformLocation(_shaders[0]->id() ,"motion");

    glUseProgram(_shaders[1]->id());
    _texHeightLoc = glGetUniformLocation(_shaders[1]->id(), "texHeight");

    glUseProgram(_shaders[2]->id());
    _shadMvpMatLoc = glGetUniformLocation(_shaders[2]->id(), "mvpMat");
    _shadGridVertexLoc = glGetAttribLocation(_shaders[2]->id(), "position");
    _texHeightShadowLoc   = glGetUniformLocation(_shaders[2]->id(), "texHeight");


    glUseProgram(_shaders[3]->id());
    _gridVertexLoc = glGetAttribLocation(_shaders[3]->id(), "position");

    // init your locations here
    _texHeightGBuffersLoc = glGetUniformLocation(_shaders[3]->id(), "texHeight");
    _rendMdvMatLoc        = glGetUniformLocation(_shaders[3]->id(),"mdvMat");
    _rendProjMatLoc       = glGetUniformLocation(_shaders[3]->id(),"projMat");
    _rendNormalMatLoc     = glGetUniformLocation(_shaders[3]->id(),"normalMat");
    _rendLightLoc         = glGetUniformLocation(_shaders[3]->id(),"light");
    _mvpMatGBuffersLoc    = glGetUniformLocation(_shaders[3]->id(),"mvpMat");
    _texDepthLightGBuffersLoc  = glGetUniformLocation(_shaders[3]->id(),"texDepthLight");

    //gbuffers
    _texSlantLoc         = glGetFragDataLocation(_shaders[3]->id(),"outSlant");
    _texVisibilityLoc    = glGetFragDataLocation(_shaders[3]->id(),"outVisibility");
    _texNormalLoc        = glGetFragDataLocation(_shaders[3]->id(),"outNormal");
    _texPositionLoc      = glGetFragDataLocation(_shaders[3]->id(),"outPosition");
    _texPositionLightLoc = glGetFragDataLocation(_shaders[3]->id(),"outPositionLight");

    //render
    _texSlantRenderLoc         = glGetUniformLocation(_shaders[4]->id(), "texSlant");
    _texVisibilityRenderLoc    = glGetUniformLocation(_shaders[4]->id(), "texVisibility");
    _texNormalRenderLoc        = glGetUniformLocation(_shaders[4]->id(), "texNormal");
    _texPositionRenderLoc      = glGetUniformLocation(_shaders[4]->id(), "texPosition");
    _texPositionLightRenderLoc = glGetUniformLocation(_shaders[4]->id(), "texPositionLight");
    _texDepthCameraRenderLoc   = glGetUniformLocation(_shaders[4]->id(), "texDepthCamera");
    _texDepthLightRenderLoc    = glGetUniformLocation(_shaders[4]->id(), "texDepthLight");
    _lightLoc                  = glGetUniformLocation(_shaders[4]->id(), "light");
    _texColorLoc               = glGetUniformLocation(_shaders[4]->id(), "texColor");



    // disable everything
    glUseProgram(0);
}

GLuint Viewer::loadTexture2D(const char* filename) {
    // load image
    QImage image = QGLWidget::convertToGLFormat(QImage(filename));

    // activate texture
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D,id);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_MIRRORED_REPEAT);

    // store texture in the GPU
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width(),image.height(),0,
                 GL_RGBA,GL_UNSIGNED_BYTE,(const GLvoid *)image.bits());

    return id;
}

void Viewer::paintGL() {

    // default : compute a 512*512 noise texture
    glBindFramebuffer(GL_FRAMEBUFFER,_fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // viewport at the size of the heightmap
    glViewport(0,0,_ndResol,_ndResol);

    // disable depth test
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // clear color buffer    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glClear(GL_COLOR_BUFFER_BIT);

    // activate the noise shader
    glUseProgram(_shaders[0]->id());

    // generate the noise texture
    glUniform3fv(_noiseMotionLoc,1,&(_motion[0]));

    // activate quad vertices
    glEnableVertexAttribArray(_noiseVertexLoc);
    glBindBuffer(GL_ARRAY_BUFFER,_quad);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);

    // draw the quad
    glDrawArrays(GL_TRIANGLES,0,6);

    // disable array
    glDisableVertexAttribArray(_noiseVertexLoc);
    glBindFramebuffer(GL_FRAMEBUFFER,0);


    // 2nd pass

    glBindFramebuffer(GL_FRAMEBUFFER,_fbo2);
    glViewport(0,0,_shadowMapResol, _shadowMapResol);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glDrawBuffer(GL_NONE);
    //glClear(GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(_shaders[2]->id());
    drawSceneFromLight();
    glBindFramebuffer(GL_FRAMEBUFFER,0);

/*
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);*/

    //3rd pass

    // *** TODO *** : activate FBO
    glBindFramebuffer(GL_FRAMEBUFFER,_fbo3);
    /*GLenum bufferlist [] = {
        GL_COLOR_ATTACHMENT0 + _textNormalLoc,
        GL_COLOR_ATTACHMENT0 + _textPositionLoc,
        GL_COLOR_ATTACHMENT0 + _textSlantLoc,
    };*/
    GLenum bufferlist [] = {
        GL_COLOR_ATTACHMENT1 + _texNormalLoc,
        GL_COLOR_ATTACHMENT1 + _texPositionLoc,
        GL_COLOR_ATTACHMENT1 + _texPositionLightLoc,
        GL_COLOR_ATTACHMENT1 + _texSlantLoc,
        GL_COLOR_ATTACHMENT1 + _texVisibilityLoc
    };
    glDrawBuffers(5, bufferlist);

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0,0,width(),height());

    glUseProgram(_shaders[3]->id());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,_texHeight);
    glUniform1i(_texHeightGBuffersLoc,0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,_texDepthLight);
    glUniform1i(_texDepthLightGBuffersLoc,1);

    drawSceneFromCamera();

    // render

    glBindFramebuffer(GL_FRAMEBUFFER,0);

    glViewport(0,0,width(),height());

    //glDisable(GL_DEPTH_TEST);
    //glDepthMask(GL_FALSE);
    glUseProgram(_shaders[4]->id());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,_texNormal);
    glUniform1i(_texNormalRenderLoc,0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,_texSlant);
    glUniform1i(_texSlantRenderLoc,1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D,_texPosition);
    glUniform1i(_texPositionRenderLoc,2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D,_texPositionLight);
    glUniform1i(_texPositionLightRenderLoc,3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D,_texDepthCamera);
    glUniform1i(_texDepthCameraRenderLoc,4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D,_texDepthLight);
    glUniform1i(_texDepthLightRenderLoc,5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D,_texColor);
    glUniform1i(_texColorLoc,6);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D,_texVisibility);
    glUniform1i(_texVisibilityRenderLoc,7);

    glUniform3fv(_lightLoc,1,&(_light[0]));


    // activate quad vertices
    glEnableVertexAttribArray(_noiseVertexLoc);
    glBindBuffer(GL_ARRAY_BUFFER,_quad);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);

    // draw the quad
    glDrawArrays(GL_TRIANGLES,0,6);

    // disable array
    glDisableVertexAttribArray(_noiseVertexLoc);

    // disable shader
    glUseProgram(0);

    if (_showShadowMap) {
        //DEBUGAGE

        //Affichage d'une texture
        glViewport(0,0,_shadowMapResol,_shadowMapResol);
        glUseProgram(_shaders[1]->id());
        glClear(GL_DEPTH_BUFFER_BIT);

        // dessinage de la texture
        glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D,_texDepthLight);
        //glBindTexture(GL_TEXTURE_2D,_texHeight);
        //_texDepthCamera;
        //_texSlant;
        //_texPosition;
        //_texNormal;
        glBindTexture(GL_TEXTURE_2D,_texDepthLight);
        //glBindTexture(GL_TEXTURE_2D,_texSlant);
        //glBindTexture(GL_TEXTURE_2D,_texNormal);


        //glBindTexture(GL_TEXTURE_2D,_texNormal);
        glUniform1i(_texHeightLoc,0);

        // activate quad vertices
        glEnableVertexAttribArray(_noiseVertexLoc);
        glBindBuffer(GL_ARRAY_BUFFER,_quad);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);

        // draw the quad
        glDrawArrays(GL_TRIANGLES,0,6);

        // disable array
        glDisableVertexAttribArray(_noiseVertexLoc);

        //FIN DEBUG
    }

    // restore previous state
    //glEnable(GL_DEPTH_TEST);
    //glDepthMask(GL_TRUE);

}

void Viewer::drawSceneFromCamera() {
    /////////TODO/////////

    /////////TODO/////////

    // mdv matrix from the light point of view
    const float size = 1.5;
    glm::vec3 l   = glm::transpose(_cam->normalMatrix())*_light;
    glm::mat4 p   = glm::ortho<float>(-size,size,-size,size,-size,2*size);
    glm::mat4 v   = glm::lookAt(l, glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 m   = glm::mat4(1.0);
    glm::mat4 mv  = v*m;

    // send uniform variables
    glUniformMatrix4fv(_rendProjMatLoc,1,GL_FALSE,&(_cam->projMatrix()[0][0]));
    glUniformMatrix4fv(_rendMdvMatLoc,1,GL_FALSE,&(_cam->mdvMatrix()[0][0]));
    glUniformMatrix3fv(_rendNormalMatLoc,1,GL_FALSE,&(_cam->normalMatrix()[0][0]));
    glUniform3fv(_rendLightLoc,1,&(_light[0]));

    const glm::mat4 mvpDepth = p*mv;
    glUniformMatrix4fv(_mvpMatGBuffersLoc,1,GL_FALSE,&mvpDepth[0][0]);

    // activate quad vertices
    glBindBuffer(GL_ARRAY_BUFFER,_gridBufferPosition);
    glEnableVertexAttribArray(_gridVertexLoc);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);

    // draw the quad
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _gridBufferFaces);
    glDrawElements(GL_TRIANGLES,3*_grid->nbFaces(),GL_UNSIGNED_INT,0);

    // disable array
    glDisableVertexAttribArray(_noiseVertexLoc);
}

void Viewer::drawSceneFromLight() {
    // *** TODO: draw the scene from the light point of view here ***

    // mdv matrix from the light point of view
    const float size = 1.5;
    glm::vec3 l   = glm::transpose(_cam->normalMatrix())*_light;
    glm::mat4 p   = glm::ortho<float>(-size,size,-size,size,-size,2*size);
    glm::mat4 v   = glm::lookAt(l, glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 m   = glm::mat4(1.0);
    glm::mat4 mv  = v*m;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,_texHeight);
    glUniform1i(_texHeightShadowLoc,0);

    // activate object vertices
    glBindBuffer(GL_ARRAY_BUFFER,_gridBufferPosition);
    glEnableVertexAttribArray(_shadGridVertexLoc);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);

    // draw one object
    {
        // send the modelview projection depth matrix
        const glm::mat4 mvpDepth = p*mv;
        glUniformMatrix4fv(_shadMvpMatLoc,1,GL_FALSE,&mvpDepth[0][0]);

        // draw faces
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _gridBufferFaces);
        glDrawElements(GL_TRIANGLES,3*_grid->nbFaces(),GL_UNSIGNED_INT,0);
    }

    glDisableVertexAttribArray(_shadGridVertexLoc);
}

void Viewer::resizeGL(int width,int height) {
    _cam->initialize(width,height,false);
    glViewport(0,0,width,height);
    initFBO();
    updateGL();
}

void Viewer::mousePressEvent(QMouseEvent *me) {
    const glm::vec2 p((float)me->x(),(float)(height()-me->y()));

    if(me->button()==Qt::LeftButton) {
        _cam->initRotation(p);
        _mode = false;
    } else if(me->button()==Qt::MidButton) {
        _cam->initMoveZ(p);
        _mode = false;
    } else if(me->button()==Qt::RightButton) {
        _light[0] = (p[0]-(float)(width()/2))/((float)(width()/2));
        _light[1] = (p[1]-(float)(height()/2))/((float)(height()/2));
        _light[2] = 1.0f-std::max(fabs(_light[0]),fabs(_light[1]));
        _light = glm::normalize(_light);
        _mode = true;
    }

    updateGL();
}

void Viewer::mouseMoveEvent(QMouseEvent *me) {
    const glm::vec2 p((float)me->x(),(float)(height()-me->y()));

    if(_mode) {
        // light mode
        _light[0] = (p[0]-(float)(width()/2))/((float)(width()/2));
        _light[1] = (p[1]-(float)(height()/2))/((float)(height()/2));
        _light[2] = 1.0f-std::max(fabs(_light[0]),fabs(_light[1]));
        _light = glm::normalize(_light);
    } else {
        // camera mode
        _cam->move(p);
    }

    updateGL();
}

void Viewer::keyPressEvent(QKeyEvent *ke) {
    const float step = 0.05;
    if(ke->key()==Qt::Key_Z) {
        glm::vec2 v = glm::normalize(glm::vec2(glm::transpose(_cam->normalMatrix())*glm::vec3(0,0,-1)))*step;
        _motion[0] += v[0];
        _motion[1] += v[1];
    }

    if(ke->key()==Qt::Key_S) {
        glm::vec2 v = glm::normalize(glm::vec2(glm::transpose(_cam->normalMatrix())*glm::vec3(0,0,-1)))*step;
        _motion[0] -= v[0];
        _motion[1] -= v[1];
    }

    if(ke->key()==Qt::Key_Q) {
        _motion[2] += step;
    }

    if(ke->key()==Qt::Key_D) {
        _motion[2] -= step;
    }





    // key a: play/stop animation
    if(ke->key()==Qt::Key_A) {
        if(_timer->isActive())
            _timer->stop();
        else
            _timer->start();
    }

    // key i: init camera
    if(ke->key()==Qt::Key_I) {
        _cam->initialize(width(),height(),true);
    }

    // key f: compute FPS
    if(ke->key()==Qt::Key_F) {
        int elapsed;
        QTime timer;
        timer.start();
        unsigned int nb = 500;
        for(unsigned int i=0;i<nb;++i) {
            paintGL();
        }
        elapsed = timer.elapsed();
        double t = (double)nb/((double)elapsed);
        cout << "FPS : " << t*1000.0 << endl;
    }

    // key r: reload shaders
    if(ke->key()==Qt::Key_R) {
        for(unsigned int i=0;i<_vertexFilenames.size();++i) {
            _shaders[i]->reload(_vertexFilenames[i].c_str(),_fragmentFilenames[i].c_str());
        }
        initShaders();
    }

    // key S: show the shadow map
    if(ke->key()==Qt::Key_X) {
        _showShadowMap = !_showShadowMap;
    }

    updateGL();
}

void Viewer::initializeGL() {
    // make this window the current one
    makeCurrent();

    // init and chack glew
    if(glewInit()!=GLEW_OK) {
        cerr << "Warning: glewInit failed!" << endl;
    }

    if(!GLEW_ARB_vertex_program   ||
            !GLEW_ARB_fragment_program ||
            !GLEW_ARB_texture_float    ||
            !GLEW_ARB_draw_buffers     ||
            !GLEW_ARB_framebuffer_object) {
        cerr << "Warning: Shaders not supported!" << endl;
    }

    // init OpenGL settings
    glClearColor(0.0,0.0,0.0,1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glViewport(0,0,width(),height());

    // initialize camera
    _cam->initialize(width(),height(),true);

    // load shader files
    createShaders();

    // init and load all shader files
    for(unsigned int i=0;i<_vertexFilenames.size();++i) {
        _shaders.push_back(new Shader());
        _shaders[i]->load(_vertexFilenames[i].c_str(),_fragmentFilenames[i].c_str());
    }

    // init shaders
    initShaders();

    // Ids for vbos, fbos and textures
    generateIds();

    // init VBO
    initVBO();
    
    // create/init FBO
    initFBO();

    // starts the timer
    _timer->start();
}

