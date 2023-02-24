#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QTimer>
#include <QVector3D>

#include "model.h"
#include "vertex.h"

/**
 * @brief The MainView class is resonsible for the actual content of the main
 * window.
 */
class MainView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
  Q_OBJECT

 public:
  MainView(QWidget *parent = nullptr);
  ~MainView() override;

  // Functions for widget input events
  void setRotation(int rotateX, int rotateY, int rotateZ);
  void setScale(float scale);
  void rotateAndScale();

 protected:
  void initializeGL() override;
  void resizeGL(int newWidth, int newHeight) override;
  void paintGL() override;

  // Functions for keyboard input events
  void keyPressEvent(QKeyEvent *ev) override;
  void keyReleaseEvent(QKeyEvent *ev) override;

  // Function for mouse input events
  void mouseDoubleClickEvent(QMouseEvent *ev) override;
  void mouseMoveEvent(QMouseEvent *ev) override;
  void mousePressEvent(QMouseEvent *ev) override;
  void mouseReleaseEvent(QMouseEvent *ev) override;
  void wheelEvent(QWheelEvent *ev) override;

 private slots:
  void onMessageLogged(QOpenGLDebugMessage Message);

 private:
  QOpenGLDebugLogger debugLogger;
  QTimer timer;  // timer used for animation

  QOpenGLShaderProgram shaderProgram;

  void fillArrayAndBuffer(GLuint buf, GLuint arr, int size, Vertex *vertices);
  void specifyDataLayout();
  void createShaderProgram();

  // Pyramid vertices
  Vertex a {-1,1,1,1,0,0};
  Vertex b {1,1,1,0,1,0};
  Vertex c {0,0,-1,1,0,1};
  Vertex d {1,-1,1,1,1,0};
  Vertex e {-1,-1,1,0,0,1};

  // Array of pyramid vertices arranged painstakingly :(
  Vertex pyramid[18] = {a,e,d,b,a,d,d,c,b,b,c,a,a,c,e,e,c,d};

  // For VBO and VAO of the pyramid and the knot
  GLuint buffer;
  GLuint array;

  GLuint knotVBO;
  GLuint knotVAO;

  // Creating QMatrix4x4 member representing Model transformation for the pyramid and for the knot
  QMatrix4x4 model;
  QMatrix4x4 knotModel;

  // Creating QMatrix4x4 member represeting Projection transformations for the pyramid
  QMatrix4x4 projection;

  // Model Location and Projection Location
  GLint modLoc;
  GLint projLoc;

  // Rotation and scaling variables
  int rotX = 0;
  int rotY = 0;
  int rotZ = 0;
  float scaling = 1;

};

#endif  // MAINVIEW_H
