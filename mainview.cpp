#include "mainview.h"
#include "vertex.h"
#include <cstddef>
#include <iostream>

#include <QDateTime>

/**
 * @brief MainView::MainView Constructs a new main view.
 *
 * @param parent Parent widget.
 */
MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {
  qDebug() << "MainView constructor";

  connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
}

/**
 * @brief MainView::~MainView
 *
 * Destructor of MainView
 * This is the last function called, before exit of the program
 * Use this to clean up your variables, buffers etc.
 *
 */
MainView::~MainView() {
  qDebug() << "MainView destructor";
  glDeleteBuffers(1, &buffer);
  glDeleteVertexArrays(1, &array);
  glDeleteBuffers(1, &knotVBO);
  glDeleteVertexArrays(1, &knotVAO);
  makeCurrent();
}

// --- OpenGL initialization

/**
 * @brief MainView::initializeGL Called upon OpenGL initialization
 * Attaches a debugger and calls other init functions.
 */
void MainView::initializeGL() {
  qDebug() << ":: Initializing OpenGL";
  initializeOpenGLFunctions();

  connect(&debugLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this,
          SLOT(onMessageLogged(QOpenGLDebugMessage)), Qt::DirectConnection);

  if (debugLogger.initialize()) {
    qDebug() << ":: Logging initialized";
    debugLogger.startLogging(QOpenGLDebugLogger::SynchronousLogging);
  }

  QString glVersion{reinterpret_cast<const char *>(glGetString(GL_VERSION))};
  qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

  // Enable depth buffer
  glEnable(GL_DEPTH_TEST);

  // Enable backface culling
  glEnable(GL_CULL_FACE);

  // Default is GL_LESS
  glDepthFunc(GL_LEQUAL);

  // Set the color to be used by glClear. This is, effectively, the background
  // color.
  glClearColor(0.37f, 0.42f, 0.45f, 0.0f);

  // Loading knot model from the model directory
  Model knot(":/models/knot.obj");

  // Getting vertex coordinates of the knot model
  QVector<QVector3D> knotVertices = knot.getMeshCoords();
  Vertex knotArray[knotVertices.count()];
  for (int i = 0; i < knotVertices.count(); i++) {
      knotArray[i].x = knotVertices[i].x();
      knotArray[i].y = knotVertices[i].y();
      knotArray[i].z = knotVertices[i].z();
      knotArray[i].r = abs(knotVertices[i].x());
      knotArray[i].g = abs(knotVertices[i].y());
      knotArray[i].b = abs(knotVertices[i].z());
  }

  // Generating VBO and VAO for both pyramid and knot
  glGenBuffers(1, &buffer);
  glGenBuffers(1, &knotVBO);
  glGenVertexArrays(1, &array);
  glGenVertexArrays(1, &knotVAO);

  createShaderProgram();

  // Filling VBO with vertex array of the pyramid
  fillArrayAndBuffer(buffer, array, 18, pyramid);

  // Specifying how the data is laid out for the pyramid
  specifyDataLayout();

  // Filling VBO with vertex array of the knot
  fillArrayAndBuffer(knotVBO, knotVAO, 3840, knotArray);

  // Specifying how the data is laid out for the knot
  specifyDataLayout();

  // Setting Model transformation using the given translations
  model.translate(-2, 0, -6);
  knotModel.translate(2, 0, -6);

  // Setting Projection transformations using the given information
  projection.perspective(60.0, 4.0/3.0, 0.2, 20.0);
}

/**
 * @brief MainView::specifyDataLayout Specifying how the data is laid out for the different objects
 */
void MainView::specifyDataLayout(){
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, x));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, r));
}

/**
 * @brief MainView::fillArrayAndBuffer Fills VAO and VBO given the parameters
 * @param buf Initialized VBO
 * @param arr Initialized VAO
 * @param size Size of the Data
 * @param vertices Array of vertices
 */
void MainView::fillArrayAndBuffer(GLuint buf, GLuint arr, int size, Vertex *vertices) {
  glBindVertexArray(arr);
  glBindBuffer(GL_ARRAY_BUFFER, buf);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*size, vertices, GL_STATIC_DRAW);
}

/**
 * @brief MainView::createShaderProgram Creates a new shader program with a
 * vertex and fragment shader.
 */
void MainView::createShaderProgram() {
  shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                        ":/shaders/vertshader.glsl");
  shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                        ":/shaders/fragshader.glsl");
  shaderProgram.link();

  // Extracting locations of the uniforms
  // check and see if the values returned here are correct -- they are correct
  modLoc = shaderProgram.uniformLocation("modelTransform");
  projLoc = shaderProgram.uniformLocation("projectionTransform");
}

/**
 * @brief MainView::paintGL Actual function used for drawing to the screen.
 *
 */
void MainView::paintGL() {
  // Clear the screen before rendering
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  shaderProgram.bind();

  // Setting value of each uniform for pyramid
  glUniformMatrix4fv(modLoc, 1, GL_FALSE, model.data());
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection.data());

  // Draw here
  glBindVertexArray(array);
  glDrawArrays(GL_TRIANGLES, 0, 18);

  // setting value of each uniform for knot
  glUniformMatrix4fv(modLoc, 1, GL_FALSE, knotModel.data());
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection.data());

  // Draw here
  glBindVertexArray(knotVAO);
  glDrawArrays(GL_TRIANGLES, 0, 3840);

  shaderProgram.release();
}

/**
 * @brief MainView::resizeGL Called upon resizing of the screen.
 *
 * @param newWidth The new width of the screen in pixels.
 * @param newHeight The new height of the screen in pixels.
 */
void MainView::resizeGL(int newWidth, int newHeight) {
  // updating projection to fit new aspect ratio
  projection.setToIdentity();
  projection.perspective(60.0, ((float)newWidth/(float)newHeight), 0.2, 20.0);
}

/**
 * @brief MainView::rotationAndScaling Combines scaling and rotation operations in one function
 */
void MainView::rotateAndScale() {
  // making sure the model is centered on the given coordinates
  model.setToIdentity();
  knotModel.setToIdentity();
  model.translate(-2, 0, -6);
  knotModel.translate(2, 0, -6);

  // applying rotations and scaling
  model.rotate(rotX, 1.0, 0.0, 0.0);
  model.rotate(rotY, 0.0, 1.0, 0.0);
  model.rotate(rotZ, 0.0, 0.0, 1.0);
  model.scale(scaling);
  knotModel.rotate(rotX, 1.0, 0.0, 0.0);
  knotModel.rotate(rotY, 0.0, 1.0, 0.0);
  knotModel.rotate(rotZ, 0.0, 0.0, 1.0);
  knotModel.scale(scaling);

  // updating the model
  update();
}

/**
 * @brief MainView::setRotation Changes the rotation of the displayed objects.
 * @param rotateX Number of degrees to rotate around the x axis.
 * @param rotateY Number of degrees to rotate around the y axis.
 * @param rotateZ Number of degrees to rotate around the z axis.
 */
void MainView::setRotation(int rotateX, int rotateY, int rotateZ) {
  rotX = rotateX;
  rotY = rotateY;
  rotZ = rotateZ;

  rotateAndScale();
}

/**
 * @brief MainView::setScale Changes the scale of the displayed objects.
 * @param scale The new scale factor. A scale factor of 1.0 should scale the
 * mesh to its original size.
 */
void MainView::setScale(float scale) {
  scaling = scale;

  rotateAndScale();
}

/**
 * @brief MainView::onMessageLogged OpenGL logging function, do not change.
 *
 * @param Message The message to be logged.
 */
void MainView::onMessageLogged(QOpenGLDebugMessage Message) {
  qDebug() << " → Log:" << Message;
}
