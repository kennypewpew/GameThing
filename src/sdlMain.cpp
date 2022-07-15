// Basic OpenGL ES 3 + SDL2 template code
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <GLES3/gl3.h>
#include <cstdio>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <iostream>
#include <fstream>


const unsigned int DISP_WIDTH = 640;
const unsigned int DISP_HEIGHT = 480;

const char* vertexSource = R"glsl(
    #version 150 core

    in vec3 position;
    uniform mat4 view;

    void main()
    {
        gl_Position = view * vec4(position, 1.0);
    }
)glsl";

const char* fragmentSource = R"glsl(
    #version 150 core
    
    void main()
    {
        gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
)glsl";


GLuint CompileShaderProgram() {
  // Compile shaders
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader,1,&vertexSource,NULL);
  glCompileShader(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader,1,&fragmentSource,NULL);
  glCompileShader(fragmentShader);

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);

  //glBindFragDataLocation(shaderProgram,0,"outColor");
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  GLint posAttrib = glGetAttribLocation(shaderProgram,"position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

  return shaderProgram;
}

struct MapTile {
  uint8_t height;
  char type;
};

class Map {
 public:
  std::vector<MapTile> _tiles;
  int _xdim;
  int _ydim;
  Map(const int &x, const int &y) {
    _tiles.resize(x*y);
  _xdim = x;
  _ydim = y;
  }
};

Map ReadMapFile( const std::string &mapFile ) {
  std::fstream mapFl;
  mapFl.open(mapFile,std::ios::in);
  // TODO: check
  std::string ln;
  getline(mapFl,ln);
  while ( ln[0] == '#' ) getline(mapFl,ln);
  int space = ln.find(' ');
  std::cout << ln.substr(0,space) << "\t" << ln.substr(space+1) << "\n";
  int xdim = std::stoi(ln.substr(0,space));
  int ydim = std::stoi(ln.substr(space));
  Map res(xdim,ydim);
  std::cout << xdim << "\t" << ydim << "\n";

  getline(mapFl,ln); // Separator
  getline(mapFl,ln); // Get first line
  while ( ln[0] != '-' ) {
    int dash = ln.find('-');
    std::string label = ln.substr(0,dash);
    std::string name = ln.substr(dash+1);
    std::cout << label << " --> " << name << "\n";
    getline(mapFl,ln);
  }

  //std::vector<MapTile> allTiles( xdim * ydim );
  for ( int i = 0 ; i < ydim ; ++i ) {
    getline(mapFl,ln);
    int prev = 0;
    for ( int j = 0 ; j < xdim ; ++j ) {
      int space = ln.find(' ',prev);
      std::string entry = ln.substr(prev == 0 ? prev : prev+1,space);
      //std::cout << entry << "\t" << entry.size() << "\n";
      res._tiles[i*xdim + j].height = stoi(entry.substr(0,entry.size()-1));
      res._tiles[i*xdim + j].type = entry.back();
      //std::cout << (int)res._tiles[i*xdim+j].height << " " << res._tiles[i*xdim+j].type << "\n";
      prev = space;
    }
    //std::cout << "\n";
  }
  return res;
}

void FillVertsAndInds( std::vector<glm::vec3> &vertices
                     , std::vector<GLuint> &indices
                     , const std::string &mapFile
                     ) {
  Map mp = ReadMapFile(mapFile);
  //while ( getline(mapFl,ln) ) {
  //  std::cout << ln << "\n";
  //}

  int xdim = mp._xdim;
  int ydim = mp._ydim;
  for ( int x = 0 ; x <= xdim ; ++x ) {
    for ( int y = 0 ; y <= ydim ; ++y ) {
      float xx = 1.5*float(x)/float(xdim)-.75;
      float yy = 1.5*float(y)/float(ydim)-.75;
      float zz = mp._tiles[y*xdim + x].height;
      //float zz = int(float(x)/5.);
      vertices.push_back(glm::vec3(xx,yy,zz));
    }
  }
  for ( int x = 0 ; x < xdim ; ++x ) {
    for ( int y = 0 ; y < ydim ; ++y ) {
      indices.push_back((x+0)*(ydim+1) + y+0);
      indices.push_back((x+0)*(ydim+1) + y+1);
      indices.push_back((x+0)*(ydim+1) + y+0);
      indices.push_back((x+1)*(ydim+1) + y+0);
    }
    indices.push_back((x+0)*(ydim+1) + ydim);
    indices.push_back((x+1)*(ydim+1) + ydim);
  }
  for ( int y = vertices.size()-(ydim+1) ; y < vertices.size()-1 ; ++y ) {
    indices.push_back(y);
    indices.push_back(y+1);
  }
}

void RotateView( const std::string &uniformName
               , const float &degrees
               , const glm::vec3 &rotationVector
               , const GLuint &program
               ) {
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::rotate(view,glm::radians(degrees), rotationVector);
    GLint uniTrans = glGetUniformLocation(program, uniformName.c_str());
    glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(view));
}

int SDL_main(int argc, char **argv) {
  // The window
  SDL_Window *window = NULL;
  // The OpenGL context
  SDL_GLContext context = NULL;
  // Init SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }
  // Setup the exit hook
  atexit(SDL_Quit);

  // Request OpenGL ES 3.0
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  // Want double-buffering
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // Create the window
  window = SDL_CreateWindow("GLES3+SDL2 Tutorial", SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, DISP_WIDTH, DISP_HEIGHT,
      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  if (!window) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
        "Couldn't create the main window.", NULL);
    return EXIT_FAILURE;
  }

  context = SDL_GL_CreateContext(window);
  if (!context) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
        "Couldn't create an OpenGL context.", NULL);
    return EXIT_FAILURE;
  }

  std::vector<glm::vec3> vertices;
  std::vector<GLuint> indices;
  FillVertsAndInds( vertices, indices , "session/default/maps/example.map" );

  // Vertex array needs to exist for things to appear
  GLuint vao;
  glGenVertexArrays(1,&vao);
  glBindVertexArray(vao);

  // Vertices
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);

  // Indices
  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*indices.size(), indices.data(),GL_STATIC_DRAW);

  GLuint shaderProgram = CompileShaderProgram();

  RotateView( "view" , 25.f , glm::vec3(0.0,0.2,0.8) , shaderProgram);
  glDrawElements(GL_LINES,indices.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

  // Update the window
  SDL_GL_SwapWindow(window);

  // Wait for the user to quit
  bool quit = false;
  while (!quit) {
    SDL_Event event;
    if (SDL_WaitEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
        // User wants to quit
        quit = true;
      }
    }
  }
  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  return SDL_main(argc,argv);
}
